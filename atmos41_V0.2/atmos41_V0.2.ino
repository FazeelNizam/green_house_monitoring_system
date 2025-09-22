#include <SDI12.h>
#include <math.h>

#define DATA_PIN 3
SDI12 mySDI12(DATA_PIN);

String readSDI12Response() {
  String buffer = "";
  while (mySDI12.available()) {
    char c = mySDI12.read();
    if (c != '\n' && c != '\r') buffer += c;
    delay(3);
  }
  return buffer;
}

int parseResponse(String response, float *values, int maxCount) {
  int count = 0;
  String token = "";

  if (response.length() > 1 && isDigit(response.charAt(0))) {
    response = response.substring(1);
  }

  for (int i = 0; i < response.length(); i++) {
    char c = response.charAt(i);

    if ((c == '+' || c == '-') && token.length() > 0) {
      if (count < maxCount) values[count++] = token.toFloat();
      token = "";
    }

    token += c;

    if (i == response.length() - 1) {
      if (count < maxCount) values[count++] = token.toFloat();
    }
  }
  return count;
}

void takeMeasurement(char addr) {
  String command = "";
  command += addr;
  command += "XR0!";
  mySDI12.sendCommand(command);

  delay(400);

  String response = readSDI12Response();
  mySDI12.clearBuffer();

  Serial.print("Raw: ");
  Serial.println(response);

  float values[24];
  int count = parseResponse(response, values, 24);

  Serial.print("Parsed "); 
  Serial.print(count); 
  Serial.println(" values from sensor.");

  if (count < 22) {
    Serial.println("Warning: Some fields missing");
  }

  // Map fields
  float solar      = values[0];
  float precip     = values[1];
  float drops      = values[17];
  float spoons     = values[18];
  float lightning  = values[2];
  float strikeDist = values[3];
  float windSpeed  = values[4];
  float windDir    = values[5];
  float gustSpeed  = values[6];
  float airTemp    = values[7];
  float pressure   = values[9];
  float RH         = values[10];
  float RHtemp     = values[11];
  float xTilt      = values[12];
  float yTilt      = values[13];
  float Tmin       = values[20];
  float Tmax       = values[21];
  float EC         = values[19] / 1000.0;

  // Derived
  float tiltAngle = sqrt(pow(xTilt, 2) + pow(yTilt, 2));

  // Print like Zentra
  Serial.println("---- ATMOS 41 Measurements ----");
  Serial.print("Solar Radiation: "); Serial.print(solar); Serial.println(" W/m²");
  Serial.print("Precipitation: "); Serial.print(precip); Serial.println(" mm");
  Serial.print("Drop Counts: "); Serial.println(drops);
  Serial.print("Spoon Tips: "); Serial.println(spoons);
  Serial.print("EC: "); Serial.print(EC); Serial.println(" mS/cm");
  Serial.print("Lightning Activity: "); Serial.println(lightning);
  Serial.print("Lightning Distance: "); Serial.print(strikeDist); Serial.println(" km");
  Serial.print("Wind Direction: "); Serial.print(windDir); Serial.println(" °");
  Serial.print("Wind Speed: "); Serial.print(windSpeed); Serial.println(" m/s");
  Serial.print("Gust Speed: "); Serial.print(gustSpeed); Serial.println(" m/s");
  Serial.print("Air Temperature: "); Serial.print(airTemp); Serial.println(" °C");
  Serial.print("Relative Humidity: "); Serial.print(RH); Serial.println(" RH");
  Serial.print("Atmospheric Pressure: "); Serial.print(pressure); Serial.println(" kPa");
  Serial.print("Tilt Angle: "); Serial.print(tiltAngle); Serial.println(" °");
  // Serial.print("Min Air Temp: "); Serial.print(Tmin); Serial.println(" °C");
  // Serial.print("Max Air Temp: "); Serial.print(Tmax); Serial.println(" °C");
  Serial.print("RH Sensor Temp: "); Serial.print(RHtemp); Serial.println(" °C");
  Serial.println("--------------------------------\n");
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  mySDI12.begin();
  delay(500);
  Serial.println("ATMOS 41 Gen 2 ESP32 Logger Started");
}

void loop() {
  takeMeasurement('0');
  delay(10000);
}
