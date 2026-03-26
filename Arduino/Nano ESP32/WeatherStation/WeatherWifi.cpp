#include "WeatherWifi.h"

#include <WiFi.h>
#include <time.h>

/*
===============================================================================
WeatherWifi.cpp

Reusable Wi-Fi and NTP helpers for ESP32-based sketches.
This module owns network connection flow, optional debug logging, and basic
system time synchronization with an NTP server.
===============================================================================
*/

namespace {
Stream* networkDebugStream = nullptr;

void networkDebugPrint(const char* message) {
  if (networkDebugStream != nullptr) {
    networkDebugStream->print(message);
  }
}

void networkDebugPrintln(const char* message) {
  if (networkDebugStream != nullptr) {
    networkDebugStream->println(message);
  }
}

void networkDebugPrintln(const String& message) {
  if (networkDebugStream != nullptr) {
    networkDebugStream->println(message);
  }
}

bool hasTimedOut(unsigned long startTime, unsigned long timeoutMs) {
  return timeoutMs > 0 && (millis() - startTime) >= timeoutMs;
}
}

/*
Set the stream used for optional network diagnostics.
Pass nullptr to disable all logging from this module.
*/
void setNetworkDebugStream(Stream* stream) {
  networkDebugStream = stream;
}

/*
Connect to a Wi-Fi network and optionally wait forever until available.
The function mirrors the sketch's existing blocking behavior by default.
*/
bool connectToWifi(const char* ssid,
                   const char* password,
                   unsigned long retryDelayMs,
                   unsigned long timeoutMs) {
  networkDebugPrintln("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (hasTimedOut(startTime, timeoutMs)) {
      networkDebugPrintln("Wi-Fi connection timed out");
      return false;
    }

    delay(retryDelayMs);
    networkDebugPrint(".");
  }

  networkDebugPrintln("");
  networkDebugPrintln("Wi-Fi connected");
  networkDebugPrint("IP address: ");
  networkDebugPrintln(WiFi.localIP().toString());
  return true;
}

/*
Configure NTP and wait until the ESP32 can provide valid local time.
The function mirrors the sketch's existing blocking behavior by default.
*/
bool syncTimeWithNtp(const char* ntpServer,
                     long gmtOffsetSec,
                     int daylightOffsetSec,
                     unsigned long retryDelayMs,
                     unsigned long timeoutMs) {
  configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
  networkDebugPrintln("Waiting for NTP time sync...");

  struct tm timeinfo;
  unsigned long startTime = millis();

  while (!getLocalTime(&timeinfo)) {
    if (hasTimedOut(startTime, timeoutMs)) {
      networkDebugPrintln("NTP time sync timed out");
      return false;
    }

    networkDebugPrint(".");
    delay(retryDelayMs);
  }

  networkDebugPrintln("");
  networkDebugPrintln("Time synchronized");
  return true;
}

/*
Return the current Wi-Fi local IP address as a human-readable string.
*/
String getWifiIpAddress() {
  return WiFi.localIP().toString();
}
