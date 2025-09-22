// =========================================================================
// LIBRARIES
// =========================================================================
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <RTClib.h>
#include <esp32-sdi12.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// =========================================================================
// PIN ASSIGNMENTS & SENSOR CONFIG
// =========================================================================
// I2C for RTC & ADS1115 modules:
// SDA: GPIO21
// SCL: GPIO22

// SDI-12 for TEROS 22 sensors:
// TEROS 22 #1: GPIO13, #2: GPIO12, #3: GPIO14, #4: GPIO27, #5: GPIO26, #6: GPIO25

// --- System Configuration ---
const int TEROS10_COUNT = 6;
const int TEROS22_COUNT = 6;
const float TEROS10_MIN_MV = 100.0;
const float TEROS10_MAX_MV = 2000.0;

// --- RTC Configuration ---
RTC_DS3231 rtc;

// --- Configuration for TEROS 10 Sensors ---
Adafruit_ADS1115 ads1; // ADC #1 (I2C address 0x48)
Adafruit_ADS1115 ads2; // ADC #2 (I2C address 0x49)

// --- Configuration for TEROS 22 Sensors ---
const int sdi12_pins[TEROS22_COUNT] = {13, 12, 14, 27, 26, 25};
ESP32_SDI12 sdi_buses[TEROS22_COUNT] = {
  ESP32_SDI12(sdi12_pins[0]), ESP32_SDI12(sdi12_pins[1]),
  ESP32_SDI12(sdi12_pins[2]), ESP32_SDI12(sdi12_pins[3]),
  ESP32_SDI12(sdi12_pins[4]), ESP32_SDI12(sdi12_pins[5])
};

// =========================================================================
// NEW: WI-FI CLIENT CONFIGURATION
// =========================================================================
const char* ssid = "Environment_Sensors";
const char* password = "password123";
const char* server_url = "http://192.168.4.1/data";

// Structure to hold data received over Wi-Fi.
// Initialized to NAN (Not-a-Number) to handle connection failures gracefully.
struct ExternalSensorData {
  float temperature = NAN;
  float humidity = NAN;
  float co2 = NAN;
  float solar_radiation = NAN;
  float ch4_ppm = NAN;
} externalData;


// =========================================================================
// SENSOR READING & HELPER FUNCTIONS
// =========================================================================
float calcVWC_mineral(float mV) {
  return 4.824e-10 * pow(mV, 3) - 2.278e-6 * pow(mV, 2) + 3.898e-3 * mV - 2.154;
}

String getTimestampForCSV() {
  DateTime now = rtc.now();
  char timestamp[20];
  sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());
  return String(timestamp);
}

String takeTEROS22Measurement(int sensorIndex){
  ESP32_SDI12 &sdi_bus = sdi_buses[sensorIndex];
  float values[3]; // VWC, Temp, and EC
  String buffer = "";
  ESP32_SDI12::Status res = sdi_bus.measure(0, values, 3);
  if (res == ESP32_SDI12::SDI12_OK) {
    buffer += String(values[0], 3); // Volumetric Water Content
    buffer += ",";
    buffer += String(values[1], 2); // Temperature
    buffer += ",";
    buffer += String(values[2], 2); // EC
  } else {
    buffer = "NaN,NaN,NaN";
  }
  return buffer;
}

// =========================================================================
// FUNCTION TO FETCH DATA OVER WI-FI
// =========================================================================
void fetchExternalSensorData() {
  // Check if we are connected to Wi-Fi before making a request
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Skipping data fetch.");
    // Set all external data to NaN if connection is lost
    externalData.temperature = NAN;
    externalData.humidity = NAN;
    externalData.co2 = NAN;
    externalData.solar_radiation = NAN;
    externalData.ch4_ppm = NAN;
    return;
  }

  HTTPClient http;
  http.begin(server_url);
  http.setConnectTimeout(2000);
  int httpResponseCode = http.GET();

  if (httpResponseCode == HTTP_CODE_OK) {
    String payload = http.getString();
    JSONVar sensorData = JSON.parse(payload);

    if (JSON.typeof(sensorData) != "undefined") {
      externalData.temperature = (double)sensorData["temperature"];
      externalData.humidity = (double)sensorData["humidity"];
      externalData.co2 = (double)sensorData["co2"];
      externalData.solar_radiation = (double)sensorData["solar_radiation"];
      externalData.ch4_ppm = (double)sensorData["ch4_ppm"];
    } else {
      Serial.println("JSON parsing failed!");
    }
  } else {
    Serial.printf("HTTP request failed, error code: %d\n", httpResponseCode);
    // set to NaN on HTTP failure
    externalData.temperature = NAN;
    externalData.humidity = NAN;
    externalData.co2 = NAN;
    externalData.solar_radiation = NAN;
    externalData.ch4_ppm = NAN;
  }
  
  http.end();
}

// =========================================================================
// SETUP FUNCTION
// =========================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nInitializing system...");

  Wire.begin();

  // --- Initialize WiFi ---
  Serial.print("Connecting to AP: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // --- Initialize RTC ---
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC! Halting.");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // --- Initialize ADS1115 ADCs ---
  ads1.setGain(GAIN_ONE);
  ads2.setGain(GAIN_ONE);
  if (!ads1.begin(0x48) || !ads2.begin(0x49)) {
    Serial.println("Failed to initialize one or more ADS1115 modules. Halting.");
    while (1);
  }

  // --- Initialize all SDI-12 buses ---
  for(int i = 0; i < TEROS22_COUNT; i++) {
    sdi_buses[i].begin();
  }
  delay(500);

  // --- Print Header with new columns ---
  String header = "DateTime,T10_1_VWC,T10_2_VWC,T10_3_VWC,T10_4_VWC,T10_5_VWC,T10_6_VWC,"
                  "T22_1_VWC,T22_1_Temp,T22_1_EC,T22_2_VWC,T22_2_Temp,T22_2_EC,"
                  "T22_3_VWC,T22_3_Temp,T22_3_EC,T22_4_VWC,T22_4_Temp,T22_4_EC,"
                  "T22_5_VWC,T22_5_Temp,T22_5_EC,T22_6_VWC,T22_6_Temp,T22_6_EC,"
                  "Ext_Temp,Ext_Humid,Ext_CO2,Ext_SolarRad,Ext_CH4";
  Serial.println("\nCSV Header:");
  Serial.println(header);

  Serial.println("\nInitialization complete. Starting data logging.");
}

// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
  // --- Fetch external sensor data over Wi-Fi at the start of the loop ---
  fetchExternalSensorData();

  // --- Get Timestamp ---
  String timestamp = getTimestampForCSV();
  String dataString = timestamp;

  // --- Read from the 6 TEROS 10 sensors ---
  Adafruit_ADS1115* adcs[] = {&ads1, &ads2};
  for (int i = 0; i < TEROS10_COUNT; i++) {
    Adafruit_ADS1115& current_adc = (i < 4) ? ads1 : ads2;
    int channel = (i < 4) ? i : i - 4;
    int16_t raw = current_adc.readADC_SingleEnded(channel);
    float mv = (raw * 0.125);

    if (mv < TEROS10_MIN_MV || mv > TEROS10_MAX_MV) {
      dataString += ",NaN";
    } else {
      float vwc = calcVWC_mineral(mv);
      dataString += "," + String(vwc, 3);
    }
    delay(20);
  }

  // --- Read from the 6 TEROS 22 sensors ---
  for (int i = 0; i < TEROS22_COUNT; i++) {
    String t22_data = takeTEROS22Measurement(i);
    dataString += "," + t22_data;
    delay(20);
  }

  // The String() constructor handles NAN values automatically, printing "nan"
  dataString += "," + String(externalData.temperature, 2);
  dataString += "," + String(externalData.humidity, 2);
  dataString += "," + String(externalData.co2, 0);
  dataString += "," + String(externalData.solar_radiation, 2);
  dataString += "," + String(externalData.ch4_ppm, 2);

  // --- Print final combined CSV String ---
  Serial.println(dataString);

  delay(15000); // Wait 15 seconds for the next logging cycle
}