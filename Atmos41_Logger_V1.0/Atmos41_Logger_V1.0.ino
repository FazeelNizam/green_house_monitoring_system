// =========================================================================
// LIBRARIES
// =========================================================================
#include <SDI12.h>
#include <math.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

// =========================================================================
// PIN DEFINITIONS & CONFIGURATION
// =========================================================================
#define SDI12_DATA_PIN 3    // Data pin for the ATMOS 41 sensor
#define SDCARD_CS_PIN  5    // Chip Select pin for the SPI SD card module

const char* dataFileName = "/atmos_data.csv";

// =========================================================================
// OBJECT INSTANTIATIONS
// =========================================================================
SDI12 mySDI12(SDI12_DATA_PIN);
RTC_DS3231 rtc;

// =========================================================================
// DATA STRUCTURE TO HOLD SENSOR READINGS
// =========================================================================
// This struct holds all values from a single sensor reading
struct AtmosData {
  DateTime timestamp;
  float solar;
  float precip;
  float drops;
  float spoons;
  float lightning;
  float strikeDist;
  float windSpeed;
  float windDir;
  float gustSpeed;
  float airTemp;
  float pressure;
  float RH;
  float RHtemp;
  float xTilt;
  float yTilt;
  float EC;
  float tiltAngle;
};


// =========================================================================
// HELPER FUNCTIONS
// =========================================================================

/**
 * @brief Reads the full response from the SDI-12 buffer.
 * @return A String containing the sensor's response.
 */
String readSDI12Response() {
  String buffer = "";
  unsigned long startTime = millis();
  while (!mySDI12.available() && (millis() - startTime < 1500)) {
    delay(10);
  }

  while (mySDI12.available()) {
    char c = mySDI12.read();
    if (c != '\n' && c != '\r') {
      buffer += c;
    }
    delay(3);
  }
  return buffer;
}

/**
 * @brief Parses the complex string from the ATMOS 41 into an array of floats.
 * @return The number of values successfully parsed.
 */
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


// =========================================================================
// CORE MEASUREMENT FUNCTION
// =========================================================================
/**
 * @brief Takes a measurement, populates the data struct, and prints detailed output.
 * @param addr The SDI-12 address of the sensor.
 * @param data A reference to the AtmosData struct to be filled.
 * @return true on success, false if the reading failed.
 */
bool takeMeasurement(char addr, AtmosData &data) {
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
    return false;
  }

  // Capture the timestamp immediately after a successful read
  data.timestamp = rtc.now();

  // Map fields from the values array to the struct
  data.solar      = values[0];
  data.precip     = values[1];
  data.drops      = values[17];
  data.spoons     = values[18];
  data.lightning  = values[2];
  data.strikeDist = values[3];
  data.windSpeed  = values[4];
  data.windDir    = values[5];
  data.gustSpeed  = values[6];
  data.airTemp    = values[7];
  data.pressure   = values[9];
  data.RH         = values[10];
  data.RHtemp     = values[11];
  data.xTilt      = values[12];
  data.yTilt      = values[13];
  data.EC         = values[19] / 1000.0;
  data.tiltAngle  = sqrt(pow(data.xTilt, 2) + pow(data.yTilt, 2));

  // --- SERIAL PRINT ---
  Serial.println("---- ATMOS 41 Measurements ----");
  Serial.print("Solar Radiation: "); Serial.print(data.solar); Serial.println(" W/m²");
  Serial.print("Precipitation: "); Serial.print(data.precip); Serial.println(" mm");
  Serial.print("Drop Counts: "); Serial.println(data.drops);
  Serial.print("Spoon Tips: "); Serial.println(data.spoons);
  Serial.print("EC: "); Serial.print(data.EC, 3); Serial.println(" mS/cm");
  Serial.print("Lightning Activity: "); Serial.println(data.lightning);
  Serial.print("Lightning Distance: "); Serial.print(data.strikeDist); Serial.println(" km");
  Serial.print("Wind Direction: "); Serial.print(data.windDir); Serial.println(" °");
  Serial.print("Wind Speed: "); Serial.print(data.windSpeed); Serial.println(" m/s");
  Serial.print("Gust Speed: "); Serial.print(data.gustSpeed); Serial.println(" m/s");
  Serial.print("Air Temperature: "); Serial.print(data.airTemp); Serial.println(" °C");
  Serial.print("Relative Humidity: "); Serial.print(data.RH); Serial.println(" RH");
  Serial.print("Atmospheric Pressure: "); Serial.print(data.pressure); Serial.println(" kPa");
  Serial.print("Tilt Angle: "); Serial.print(data.tiltAngle); Serial.println(" °");
  Serial.print("RH Sensor Temp: "); Serial.print(data.RHtemp); Serial.println(" °C");
  Serial.println("--------------------------------\n");

  return true;
}


// =========================================================================
// SETUP
// =========================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nATMOS 41 Gen 2 ESP32 Logger Started");

  mySDI12.begin();
  delay(500);

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC! Check wiring. Halting.");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time to compile time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Serial.println("DS3231 RTC initialized.");

  Serial.print("Initializing SD card...");
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println(" initialization failed! Halting.");
    while (1);
  }
  Serial.println(" done.");

  File dataFile = SD.open(dataFileName, FILE_READ);
  if (!dataFile) {
    Serial.println("Data file not found, creating it with a header...");
    dataFile = SD.open(dataFileName, FILE_WRITE);
    if (dataFile) {
      String header = "Timestamp,Solar_Rad,Precip,Lightning_Count,Lightning_Dist,"
                      "Wind_Speed,Wind_Dir,Gust_Speed,Air_Temp,Pressure,Rel_Humid,"
                      "EC,Tilt_Angle,Drops,Spoons,RH_Temp,xTilt,yTilt";
      dataFile.println(header);
      dataFile.close();
      Serial.println("File created and header written.");
    } else {
      Serial.println("Error creating data file! Halting.");
      while(1);
    }
  } else {
    dataFile.close();
    Serial.println("Data file found.");
  }
}


// =========================================================================
// MAIN LOOP
// =========================================================================
void loop() {
  AtmosData currentData;

  if (takeMeasurement('0', currentData)) {
    // Format the timestamp from the data struct into a string buffer
    char timestamp_buf[20];
    sprintf(timestamp_buf, "%04d-%02d-%02d %02d:%02d:%02d",
            currentData.timestamp.year(), currentData.timestamp.month(), currentData.timestamp.day(),
            currentData.timestamp.hour(), currentData.timestamp.minute(), currentData.timestamp.second());
    
    // Construct the CSV data string
    String dataString = String(timestamp_buf);
    dataString += "," + String(currentData.solar, 2);
    dataString += "," + String(currentData.precip, 2);
    dataString += "," + String(currentData.lightning, 0);
    dataString += "," + String(currentData.strikeDist, 1);
    dataString += "," + String(currentData.windSpeed, 2);
    dataString += "," + String(currentData.windDir, 1);
    dataString += "," + String(currentData.gustSpeed, 2);
    dataString += "," + String(currentData.airTemp, 2);
    dataString += "," + String(currentData.pressure, 2);
    dataString += "," + String(currentData.RH, 2);
    dataString += "," + String(currentData.EC, 3);
    dataString += "," + String(currentData.tiltAngle, 2);
    dataString += "," + String(currentData.drops, 0);
    dataString += "," + String(currentData.spoons, 0);
    dataString += "," + String(currentData.RHtemp, 2);
    dataString += "," + String(currentData.xTilt, 2);
    dataString += "," + String(currentData.yTilt, 2);

    File dataFile = SD.open(dataFileName, FILE_APPEND);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println("-----> Data successfully logged to SD Card.");
    } else {
      Serial.println("Error opening data file for writing!");
    }
  } else {
    Serial.println("-----> Measurement failed. Nothing logged to SD Card.");
  }

  delay(10000); // Wait 10 seconds for the next cycle
}