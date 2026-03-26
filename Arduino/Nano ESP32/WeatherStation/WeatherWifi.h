#ifndef WEATHER_WIFI_H
#define WEATHER_WIFI_H

/*
===============================================================================
WeatherWifi.h

Public interface for reusable Wi-Fi and NTP helper functions.
This module centralizes network connection, time synchronization, and IP
address formatting so the same behavior can be shared across sketches.
===============================================================================
*/

#include <Arduino.h>

/*
Configure the optional debug output stream used by the network module.
Pass nullptr to disable diagnostic output.
*/
void setNetworkDebugStream(Stream* stream);

/*
Connect to a Wi-Fi network using the provided credentials.
When timeoutMs is 0, the function retries indefinitely until connected.
Returns true once the connection is established, false on timeout.
*/
bool connectToWifi(const char* ssid,
                   const char* password,
                   unsigned long retryDelayMs = 500,
                   unsigned long timeoutMs = 0);

/*
Synchronize the system clock with an NTP server.
When timeoutMs is 0, the function waits indefinitely for valid time data.
Returns true once time is synchronized, false on timeout.
*/
bool syncTimeWithNtp(const char* ntpServer,
                     long gmtOffsetSec,
                     int daylightOffsetSec,
                     unsigned long retryDelayMs = 500,
                     unsigned long timeoutMs = 0);

/*
Return the local IP address as a formatted string.
Returns an empty string if the network stack does not provide an address.
*/
String getWifiIpAddress();

#endif
