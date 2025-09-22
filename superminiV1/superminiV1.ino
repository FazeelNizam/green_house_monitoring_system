#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <esp32-sdi12.h>

// --- Configuration for TEROS 10 Sensors via ADS1115) ---
Adafruit_ADS1115 ads;

// Volumetric Water Content (VWC) for mineral soil.
float calcVWC_mineral(float mV) {
  return 4.824e-10 * mV*mV*mV - 2.278e-6 * mV*mV + 3.898e-3 * mV - 2.154;
}

// Volumetric Water Content (VWC) for soilless media
float calcVWC_soilless(float mV) {
  return 5.439e-10 * mV*mV*mV - 2.731e-6 * mV*mV + 4.868e-3 * mV - 2.683;
}


// --- Configuration for TEROS 22 Sensors SDI-12) ---
#define SDI12_PIN_20 10
#define SDI12_PIN_21 21

// separate SDI-12 objects for each sensor pin
ESP32_SDI12 sdi12_pin20(SDI12_PIN_20);
ESP32_SDI12 sdi12_pin21(SDI12_PIN_21);


void setup() {
  Serial.begin(115200);
  Serial.println("Initializing all sensors...");
  ads.setGain(GAIN_ONE); // Set gain to +/- 4.096V
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115. Check wiring.");
    while (1);
  }

  // --- Initialize SDI-12 for TEROS 22 ---
  sdi12_pin20.begin();
  sdi12_pin21.begin();

  Serial.println("Initialization complete. Starting measurements.");
  delay(1000);
}

void loop() {
  // --- Read from TEROS 10 on ADC Channel A0 ---
  Serial.println("--- Reading TEROS 10 #1 (A0) ---");
  delay(15); // Sensor needs ~10 ms min to stabilize
  int16_t raw_A0 = ads.readADC_SingleEnded(0);
  float volts_A0 = raw_A0 * 0.000125; // Voltage multiplier for GAIN_ONE
  float mV_A0 = volts_A0 * 1000.0;
  float vwc_A0 = calcVWC_mineral(mV_A0); // Choose the correct calculation

  Serial.print("ADC Raw: "); Serial.print(raw_A0);
  Serial.print("  |  Voltage: "); Serial.print(mV_A0, 2); Serial.print(" mV");
  Serial.print("  |  VWC: "); Serial.print(vwc_A0, 3); Serial.println(" m³/m³");
  Serial.println();
  delay(300);

  // --- Read from TEROS 10 on ADC Channel A1 ---
  Serial.println("--- Reading TEROS 10 #2 (A1) ---");
  delay(15);
  int16_t raw_A1 = ads.readADC_SingleEnded(1);
  float volts_A1 = raw_A1 * 0.000125;
  float mV_A1 = volts_A1 * 1000.0;
  float vwc_A1 = calcVWC_mineral(mV_A1); // Choose the correct calculation

  Serial.print("ADC Raw: "); Serial.print(raw_A1);
  Serial.print("  |  Voltage: "); Serial.print(mV_A1, 2); Serial.print(" mV");
  Serial.print("  |  VWC: "); Serial.print(vwc_A1, 3); Serial.println(" m³/m³");
  Serial.println();
  delay(300);

  // --- Read from TEROS 22 on Pin 20 ---
  Serial.println("--- Reading TEROS 22 #1 (Pin 20) ---");
  float values_20[3]; // Array to hold the 3 return values
  // Use SDI-12 address '0'. Change if your sensor has a different address.
  ESP32_SDI12::Status res_20 = sdi12_pin20.measure(0, values_20, 3);

  if (res_20 == ESP32_SDI12::SDI12_OK) {
    Serial.print("VWC: "); Serial.print(values_20[0], 3); Serial.println(" m³/m³");
    Serial.print("Temperature: "); Serial.print(values_20[1], 2); Serial.println(" °C");
    // Serial.print("Electrical Conductivity: "); Serial.print(values_20[2], 0); Serial.println(" µS/cm");
  } else {
    Serial.print("SDI-12 measurement failed with error: ");
    Serial.println(res_20);
  }
  Serial.println();
  delay(300);

  // --- Read from TEROS 22 on Pin 21 ---
  Serial.println("--- Reading TEROS 22 #2 (Pin 21) ---");
  float values_21[3]; // Array to hold the 3 return values
  ESP32_SDI12::Status res_21 = sdi12_pin21.measure(0, values_21, 3);

  if (res_21 == ESP32_SDI12::SDI12_OK) {
    Serial.print("VWC: "); Serial.print(values_21[0], 3); Serial.println(" m³/m³");
    Serial.print("Temperature: "); Serial.print(values_21[1], 2); Serial.println(" °C");
    // Serial.print("Electrical Conductivity: "); Serial.print(values_21[2], 0); Serial.println(" µS/cm");
  } else {
    Serial.print("SDI-12 measurement failed with error: ");
    Serial.println(res_21);
  }
  Serial.println();
  
  // --- Wait before next round of measurements ---
  Serial.println("------------------------------------");
  delay(5000); // Delay 5 seconds before the next reading cycle
}