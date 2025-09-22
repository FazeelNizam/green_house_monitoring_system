// =================================================================================
// LIBRARIES
// =================================================================================
#include <Wire.h>
#include <HardwareSerial.h>
#include "Adafruit_SHT31.h"
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>

// =================================================================================
// SENSOR CONFIGURATION AND PINS
// =================================================================================

// MH-Z19C CO2 Sensor -------------------------------------------------------------
#define RXD2 16  // MH-Z19C TX -> ESP32 RX
#define TXD2 17  // MH-Z19C RX <- ESP32 TX
HardwareSerial mhz19(2); // Use UART2
byte readCO2[] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
byte autoCalibOff[] = { 0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86 };

// SHT3x Temperature & Humidity Sensor -------------------------------------------
Adafruit_SHT31 sht3x = Adafruit_SHT31();

// Pyranometer (ADS1115) -----------------------------------------------------------
Adafruit_ADS1115 ads;
const float PYRANOMETER_CALIBRATION_FACTOR = 75.7; // µV per W/m²

// MQ-4 Methane (CH4) Sensor ------------------------------------------------------
#define MQ_PIN 34 // Analog pin for MQ sensor
#define RL_VALUE 5.0  // Load resistor value in kOhms for your MQ sensor module

// IMPORTANT: YOU MUST CALIBRATE THIS VALUE FOR YOUR SENSOR IN CLEAN AIR
#define RO_IN_CLEAN_AIR 26.0 // Base resistance of the sensor in clean air (kOhms).

// These values represent points on the CH4 sensitivity curve from the MQ-4 datasheet
// The curve is on a log-log scale. We use two points to calculate the line equation.
// Point 1: (200 ppm, Rs/Ro = 4.5)
const float CH4_LOG_POINT1_X = log10(200);
const float CH4_LOG_POINT1_Y = log10(4.5);
// Point 2: (10000 ppm, Rs/Ro = 0.7)
const float CH4_LOG_POINT2_X = log10(10000);
const float CH4_LOG_POINT2_Y = log10(0.7);

// Calculate the slope (m) and y-intercept (c) for the log-log line equation: y = mx + c
const float LOG_SLOPE = (CH4_LOG_POINT2_Y - CH4_LOG_POINT1_Y) / (CH4_LOG_POINT2_X - CH4_LOG_POINT1_X);
const float LOG_INTERCEPT = CH4_LOG_POINT1_Y - LOG_SLOPE * CH4_LOG_POINT1_X;


// =================================================================================
// WIFI AND WEB SERVER CONFIGURATION
// =================================================================================
const char* ssid = "Environment_Sensors";
const char* password = "password123";
WiFiServer server(80); // Web server will be on port 80

// Global structure to hold the latest sensor data
struct SensorData {
  float temperature;
  float humidity;
  float co2;
  float solar_radiation;
  float ch4_ppm;
} latestData;

// =================================================================================
// SENSOR READING FUNCTIONS
// =================================================================================

// --- Read CO2 from MH-Z19C ---
float readMHZ19C() {
  // Flush any old bytes from the UART buffer
  while (mhz19.available()) {
    mhz19.read();
  }
  
  int co2 = 0;
  // Send the command to read CO2 value
  mhz19.write(readCO2, 9);
  delay(100);

  if (mhz19.available()) {
    byte response[9];
    mhz19.readBytes(response, 9);
    // Check for valid response header
    if (response[0] == 0xFF && response[1] == 0x86) {
      co2 = response[2] * 256 + response[3];
    }
  }
  return (float)co2;
}

// --- Read Solar Radiation from Pyranometer ---
float readPyranometer() {
  int16_t adc0 = ads.readADC_Differential_0_1();
  // Convert ADC counts to voltage (7.8125 µV per bit for ±0.256V gain)
  float volts = adc0 * 0.0000078125;
  // Convert voltage (in Volts) to microvolts (µV) and then to W/m²
  float radiation = (volts * 1e6) / PYRANOMETER_CALIBRATION_FACTOR;
  return (radiation < 0) ? 0 : radiation;
}

// --- Calculate MQ-4 Sensor Resistance (Rs) ---
float getMQSensorResistance() {
  int adc_value = 0;
  // Take an average of 10 readings for stability
  for (int i = 0; i < 10; i++) {
      adc_value += analogRead(MQ_PIN);
      delay(5);
  }
  adc_value = adc_value / 10;

  // Convert the analog value to voltage
  float voltage = adc_value * (3.3 / 4095.0);
  
  // Calculate sensor resistance (Rs) using the voltage divider formula
  // Handle division by zero edge case
  if (voltage == 0) return 0;
  float Rs = (3.3 * RL_VALUE / voltage) - RL_VALUE;
  
  Serial.print("Sensor Resistance (Rs): ");
  Serial.println(Rs);
  
  return Rs;
}

// --- Calculate Methane (CH4) Concentration in PPM ---
float readCH4_ppm() {
  float Rs = getMQSensorResistance();
  if (Rs == 0) return 0;
  
  float Rs_Ro_ratio = Rs / RO_IN_CLEAN_AIR;
  
  // Calculate ppm using the log-log line equation derived from the datasheet
  // log10(ppm) = (log10(Rs/Ro) - c) / m
  float log_ppm = (log10(Rs_Ro_ratio) - LOG_INTERCEPT) / LOG_SLOPE;
  
  // Return the anti-log to get the ppm value
  return pow(10, log_ppm);
}

// --- Main function to update all sensor readings ---
void updateAllSensors() {
  latestData.temperature = sht3x.readTemperature();
  latestData.humidity = sht3x.readHumidity();
  latestData.co2 = readMHZ19C();
  latestData.solar_radiation = readPyranometer();
  latestData.ch4_ppm = readCH4_ppm();

  // Handle potential NaN (Not a Number) readings from the SHT sensor
  latestData.temperature = isnan(latestData.temperature) ? -99.9 : latestData.temperature;
  latestData.humidity = isnan(latestData.humidity) ? -99.9 : latestData.humidity;

  // Print to serial monitor for debugging
  Serial.println("--- Updated Sensor Readings ---");
  Serial.printf("Temperature: %.2f *C\n", latestData.temperature);
  Serial.printf("Humidity: %.2f %%\n", latestData.humidity);
  Serial.printf("CO2: %.0f ppm\n", latestData.co2);
  Serial.printf("Solar Radiation: %.2f W/m^2\n", latestData.solar_radiation);
  Serial.printf("Methane (CH4): %.2f ppm\n", latestData.ch4_ppm);
  Serial.println("--------------------------------\n");
}


// =================================================================================
// SETUP FUNCTION
// =================================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n--- Sensor Hub Sender Initializing ---");

  // --- Initialize Sensors ---
  mhz19.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(100);
  mhz19.write(autoCalibOff, 9); // Disable auto-calibration for MH-Z19C

  if (!sht3x.begin(0x44)) {
    Serial.println("Couldn't find SHT3x sensor");
    while (1) delay(1);
  }

  if (!ads.begin()) {
    Serial.println("Couldn't find ADS1115");
    while (1) delay(1);
  }
  ads.setGain(GAIN_SIXTEEN); // Set gain for pyranometer's small voltage output

  pinMode(MQ_PIN, INPUT);
  
  Serial.println("Sensors initialized.");
  Serial.println("Warming up sensors... (allow 5-10 mins for MQ sensor to stabilize)");

  // --- Initialize WiFi Access Point ---
  Serial.print("Setting up Access Point...");
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print(" AP Ready. IP address: ");
  Serial.println(IP);

  server.begin();
}


// =================================================================================
// MAIN LOOP
// =================================================================================
void loop() {
  // Update all sensor readings
  updateAllSensors();

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected!");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            
            // Create JSON response with the latest data
            String json = "{";
            json += "\"temperature\":" + String(latestData.temperature, 2) + ",";
            json += "\"humidity\":" + String(latestData.humidity, 2) + ",";
            json += "\"co2\":" + String(latestData.co2, 0) + ",";
            json += "\"solar_radiation\":" + String(latestData.solar_radiation, 2) + ",";
            json += "\"ch4_ppm\":" + String(latestData.ch4_ppm, 2);
            json += "}";

            // Send standard HTTP headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();
            
            // Send the JSON payload
            client.println(json);
            break; // Break out of the while loop
          } else {
            // Clear the line for the next one
            currentLine = "";
          }
        } else if (c != '\r') {
          // Add character to the current line
          currentLine += c;
        }
      }
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
  }

  // Wait before taking the next set of readings
  delay(5000);
}