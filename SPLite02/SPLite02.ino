#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

float calibration_factor = 75.7;  // µV per W/m²

void setup(void) {
  Serial.begin(115200);

  if (!ads.begin()) {
    Serial.println("Couldn't find ADS");
    Serial.flush();
    while (1) delay(10);
  }

  ads.setGain(GAIN_SIXTEEN);  // ±0.256V range
  ads.begin();
}

void loop(void) {
  // Differential read between A0 and A1
  int16_t adc0 = ads.readADC_Differential_0_1();

  // Convert ADC counts to voltage
  float volts = adc0 * 0.0000078125;  // (7.8125 µV per bit for ±0.256V)

  // Convert voltage to W/m² using calibration factor
  float radiation = (volts * 1e6) / calibration_factor;

  Serial.print("Voltage: ");
  Serial.print(volts * 1000, 3);
  Serial.print(" mV  |  Solar Radiation: ");
  Serial.print(radiation, 2);
  Serial.println(" W/m²");

  delay(1000);
}
