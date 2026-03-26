/*
===============================================================================
Project: Weather Station (Arduino Nano ESP32)

Description:
This project implements a connected weather station using an Arduino Nano ESP32.
It reads environmental data (temperature and humidity) from a DHT20 (AHT20)
sensor via the I2C bus, displays the information on an SH1106 OLED screen,
and synchronizes the system time using an NTP server over Wi-Fi.

Main Features:
- Temperature and humidity acquisition via DHT20 (I2C)
- Real-time display on SH1106 OLED (128x64)
- Wi-Fi connectivity for network access
- Automatic time synchronization using NTP (be.pool.ntp.org)
- Structured multi-line display with dynamic content
- Conditional debug logging (enabled only when USB is connected)

Hardware Components:
- Arduino Nano ESP32
- DHT20 (AHT20) temperature and humidity sensor (I2C address: 0x38)
- SH1106 OLED display 128x64 (I2C address: 0x3C)

Software Dependencies:
- esp32 (Espressif core)
- Adafruit AHTX0
- Adafruit GFX
- Adafruit SH110X

Architecture Overview:
- I2C bus used for sensor and display communication
- Wi-Fi used for time synchronization via NTP
- Modular functions for display rendering and time formatting
- Lightweight debug system with runtime enable/disable

Notes:
- The system is designed to run autonomously (e.g., battery-powered)
- Serial debug output is automatically disabled when no USB connection is detected
- Timezone and daylight saving offsets can be configured via constants

Author: Nicolas Debras <nicolas@debras.fr>
===============================================================================

*/

#include <Wire.h>                 // I2C communication library (used to communicate with sensors and display)
#include <Adafruit_AHTX0.h>       // Library for AHT20/DHT20 temperature and humidity sensor
#include <Adafruit_GFX.h>         // Core graphics library (text, shapes, etc.)
#include <Adafruit_SH110X.h>      // Driver for SH1106 OLED displays
#include <WiFi.h>                 // ESP32 Wi-Fi connectivity library
#include <time.h>                 // Time functions (NTP synchronization, struct tm, etc.)
//#include <FS.h>                   // File system base classes used by FFat
#include <FFat.h>                 // FAT file system library for ESP32 ffat partition
//#include <esp_partition.h>        // ESP32 partition table access
#include "FFatPreferences.h"      // Reusable FFat-backed key/value helpers
#include "WeatherDisplay.h"       // Reusable SH1106 display rendering helpers

Adafruit_AHTX0 aht;               // Create an instance of the AHT20/DHT20 sensor
bool debugEnabled = false;        // Flag used to enable/disable Serial debug output dynamically

/* OLED display configuration */
#define SCREEN_WIDTH 128          // Display width in pixels
#define SCREEN_HEIGHT 64          // Display height in pixels
#define OLED_ADDRESS 0x3C         // I2C address of the OLED display
#define I2C_SDA 21                // ESP32 SDA pin for I2C communication
#define I2C_SCL 22                // ESP32 SCL pin for I2C communication
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Create display object using I2C

// Wi-Fi network credentials (must match your local network)
const char* WIFI_SSID = "GooglePixel";
const char* WIFI_PASSWORD = "password";

// NTP server used for time synchronization
const char* NTP_SERVER = "be.pool.ntp.org";

// Time configuration parameters
// GMT_OFFSET_SEC: base timezone offset (in seconds)
// DAYLIGHT_OFFSET_SEC: additional offset for daylight saving time
const long GMT_OFFSET_SEC = 0;
const int DAYLIGHT_OFFSET_SEC = 3600;

//Preference file path (stored in FFat)
const char* PREFERENCE_FILE = "/preferences.txt";

// Debug macros
// These macros wrap Serial.print/println and only execute if debugEnabled is true
// This avoids unnecessary overhead when debugging is disabled
#define debugPrint(x) \
  if (debugEnabled) Serial.print(x)

#define debugPrintln(x) \
  if (debugEnabled) Serial.println(x)

/*
Print the ESP32 partition table to Serial for debugging.
Lists each partition's label, type, subtype, flash address and size.
Only outputs when debug is enabled.
*/
/*
void debugPartitions() {
  debugPrintln("=== Partition Table ===");

  esp_partition_iterator_t it =
    esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  while (it != NULL) {
    const esp_partition_t* p = esp_partition_get(it);

    char info[80];
    snprintf(info, sizeof(info),
             "%-16s | type %02X | sub %02X | addr 0x%06X | size %6u KB",
             p->label,
             (unsigned)p->type,
             (unsigned)p->subtype,
             (unsigned)p->address,
             (unsigned)(p->size / 1024));
    debugPrintln(info);

    it = esp_partition_next(it);
  }

  esp_partition_iterator_release(it);
  debugPrintln("======================");
}
*/

/*
Initialize the debug environment.
This function enables Serial output only if a USB connection is detected.
This avoids blocking or unnecessary logging when running on battery.
*/
void initDebug() {
  Serial.begin(115200);  // Initialize serial communication at 115200 baud

  // Wait for USB serial connection for up to 2 seconds
  unsigned long start = millis();
  while (!Serial && millis() - start < 2000) {
    delay(10);  // Small delay to avoid busy looping
  }

  // Check if Serial is available and writable
  if (Serial && Serial.availableForWrite()) {
    debugEnabled = true;   // Enable debug output
  } else {
    debugEnabled = false;  // Disable debug output
  }
}

/*
Retrieve current local time as a formatted string.
Parameters:
  - format: strftime format string (default: "%H:%M:%S - %d/%m/%Y")
Returns:
  - Formatted time string, or "Time not available" if time not synced
*/
String getTimeString(const char* format = "%H:%M:%S - %d/%m/%Y") {
  struct tm timeinfo;

  // If time is not available, return an error string
  if (!getLocalTime(&timeinfo)) {
    return "Time not available";
  }

  char buffer[50];  // Buffer to store formatted time string (increased size for flexibility)

  // Format time into buffer using provided format string
  strftime(buffer, sizeof(buffer), format, &timeinfo);

  return String(buffer);  // Convert C string to Arduino String
}

/*
Setup function (runs once at boot).
Initializes debug, I2C, sensor, display, Wi-Fi and time synchronization.
*/
void setup() {
  initDebug();                             // Initialize debug system
  setPreferenceDebugStream(debugEnabled ? &Serial : nullptr);
  
  if (!FFat.begin(true)) {                 // Mount and format FFat if needed
    debugPrintln("FFat initialization failed");
    while (true) {
      delay(100);
    }
  }

  Wire.begin(I2C_SDA, I2C_SCL);            // Initialize I2C bus with custom pins

  /* Initialize DHT20 (AHT20) sensor */
  debugPrintln("Initiating DHT20...");
  if (!aht.begin()) {                      // Try to initialize sensor
    debugPrintln("DHT20 not detected, check wiring.");
    showMessage(display, "DHT20 not detected", "Check wiring!"); // Display error message on screen
    while (1) {                            // Infinite loop if sensor not found
      delay(10);
    }
  }
  debugPrintln("DHT20 pret.");

  /* Initialize OLED display */
  if (!display.begin(OLED_ADDRESS, true)) { // Initialize display with I2C address
    debugPrintln("SH1106 not found");
    showMessage(display, "SH1106 not found", "Check wiring!"); // Display error message on screen
    while (true) {                         // Stop execution if display not found
      delay(100);
    }
  }

  display.clearDisplay();                  // Clear display memory
  display.setRotation(2);                  // Rotate display (180° orientation)
  display.display();                      // Apply changes

  showMessage(display, "System ready");    // Display startup message
  delay(1000);

  showMessage(display, "Connecting to Wi-Fi..."); // Inform user about Wi-Fi connection
  
  debugPrintln("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);    // Start Wi-Fi connection

  // Wait until connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debugPrint(".");                       // Print progress indicator
  }

  debugPrintln();
  debugPrintln("Wi-Fi connected");
  debugPrint("IP address: ");
  debugPrintln(WiFi.localIP());            // Display assigned IP address

  // Configure system time using NTP server
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  debugPrintln("Waiting for NTP time sync...");

  // Wait until time is successfully retrieved
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    debugPrint(".");
    delay(500);
  }

  debugPrintln();
  debugPrintln("Time synchronized");
}


/*
Main loop function (runs continuously).
Reads sensor data, updates display, and prints debug information.
*/
void loop() {
  sensors_event_t humidity, temp;          // Structures to store sensor data
  aht.getEvent(&humidity, &temp);          // Read temperature and humidity

  /* Prepare display content */
  char line1[20];
  char line2[20];
  char line3[20];
  char line4[20];

  // Format temperature and humidity values into strings
  snprintf(line1, sizeof(line1), "T : %.1f C", temp.temperature);
  snprintf(line2, sizeof(line2), "H : %.1f %%", humidity.relative_humidity);

  String currentTime = getTimeString("%H:%M:%S");    // Retrieve current time (uses default format)
                                                  // Or use custom format: getTimeString("%H:%M:%S")

  // Concatenate time with IP address
  String ipAddress = WiFi.localIP().toString();

  // Display all information on screen
  display4Lines(
    display,
    currentTime.c_str(),                     // Header
    line1,                                // Temperature
    line2,                                // Humidity
    ipAddress.c_str()                     // Time and IP address (converted to C string)
  );

  /* Debug output (only if enabled) */
  /*
  debugPrint("Temperature: ");
  debugPrint(temp.temperature);
  debugPrintln(" °C");

  debugPrint("Humidite: ");
  debugPrint(humidity.relative_humidity);
  debugPrintln(" %");

  debugPrintln("------------------------");
  */
 
  delay(1000);                            // Wait 1 second before next update
}