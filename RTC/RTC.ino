#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;
                    // ESP32        C3 Mini
// #define SDA_PIN 21  // GPIO21       GPIO4
// #define SCL_PIN 22  // GPIO22       GPIO5

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C with custom pins
  // Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  
  // Check if RTC lost power and then set the time
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    // Set to compile time
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Set a specific date/time:
    // rtc.adjust(DateTime(2024, 1, 15, 14, 30, 0)); // Year, Month, Day, Hour, Minute, Second
  }
  
  // Disable 32K output
  rtc.disable32K();
  
  
  Serial.println("DS3231 RTC initialized successfully!");
  Serial.println("Current time: " + getCurrentTimeString());
}

void loop() {
  // Read current time
  DateTime now = rtc.now();
  
  // Display time in various formats
  Serial.println("=== RTC Reading ===");
  Serial.println("DateTime: " + getCurrentTimeString());
  Serial.println("Unix timestamp: " + String(now.unixtime()));
  Serial.println("Temperature: " + String(rtc.getTemperature()) + "°C");
  
  // Display individual components
  Serial.println("Year: " + String(now.year()));
  Serial.println("Month: " + String(now.month()));
  Serial.println("Day: " + String(now.day()));
  Serial.println("Hour: " + String(now.hour()));
  Serial.println("Minute: " + String(now.minute()));
  Serial.println("Second: " + String(now.second()));
  Serial.println("Day of week: " + String(now.dayOfTheWeek()) + " (0=Sunday, 6=Saturday)");
  Serial.println();
  
  delay(5000); // Wait 5 seconds before next reading
}

// Helper function to get formatted time string
String getCurrentTimeString() {
  DateTime now = rtc.now();
  String timeStr = "";
  
  // Add leading zeros where needed
  if (now.day() < 10) timeStr += "0";
  timeStr += String(now.day()) + "/";
  
  if (now.month() < 10) timeStr += "0";
  timeStr += String(now.month()) + "/";
  
  timeStr += String(now.year()) + " ";
  
  if (now.hour() < 10) timeStr += "0";
  timeStr += String(now.hour()) + ":";
  
  if (now.minute() < 10) timeStr += "0";
  timeStr += String(now.minute()) + ":";
  
  if (now.second() < 10) timeStr += "0";
  timeStr += String(now.second());
  
  return timeStr;
}

// Additional utility functions

// Set a specific date and time
void setDateTime(int year, int month, int day, int hour, int minute, int second) {
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.println("Time set to: " + getCurrentTimeString());
}