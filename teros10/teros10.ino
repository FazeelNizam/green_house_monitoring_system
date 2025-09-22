#include <Wire.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;

// Choose the correct equation here:
float calcVWC_mineral(float mV) {
  return 4.824e-10 * mV*mV*mV - 2.278e-6 * mV*mV + 3.898e-3 * mV - 2.154;
}

float calcVWC_soilless(float mV) {
  return 5.439e-10 * mV*mV*mV - 2.731e-6 * mV*mV + 4.868e-3 * mV - 2.683;
}

void setup() {
  Serial.begin(115200);
  ads.setGain(GAIN_ONE);
  ads.begin();
}

void loop() {
  delay(15);  // Sensor needs ~10 ms min to stabilize

  int16_t raw = ads.readADC_SingleEnded(0);
  float volts = raw * 0.000125;    
  float mV = volts * 1000.0;

  // Choose appropriate calculation:
  float vwc = calcVWC_mineral(mV) - 0.003;
  // float vwc = calcVWC_soilless(mV);

  Serial.print("ADC Raw: "); Serial.print(raw);
  Serial.print("  Voltage: "); Serial.print(mV, 2); Serial.print(" mV");
  Serial.print("  VWC: "); Serial.print(vwc, 3); Serial.println(" m3/m3");

  delay(1000);
}
