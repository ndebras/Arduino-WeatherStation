#ifndef FFAT_PREFERENCES_H
#define FFAT_PREFERENCES_H


/*
===============================================================================
FFatPreferences.h

Public interface for reusable FFat-backed preference helpers.
This module provides a lightweight key/value storage API for Arduino sketches
that persist settings in a plain text file on the ESP32 FFat file system.
===============================================================================
*/

#include <Arduino.h>
#include <FFat.h>

/*
Configure the optional debug output stream used by the implementation.
Pass nullptr to disable diagnostic output.
*/
void setPreferenceDebugStream(Stream* stream);

/*
Read a single value identified by key from the specified preference file.
Returns defaultValue if the file cannot be opened or the key is not found.
*/
String readPreference(const char* filePath,
                      const char* key,
                      const char* defaultValue = "");

/*
Read the complete preference file content as a single string.
Returns defaultValue if the file cannot be opened.
*/
String readAllPreferences(const char* filePath,
                          const char* defaultValue = "");

/*
Create or update a key/value pair in the specified preference file.
Returns true when the file is successfully written.
*/
bool writePreference(const char* filePath,
                     const char* key,
                     const char* value);

/*
Delete a key/value pair from the specified preference file.
Returns true only when the key exists and the updated file is written.
*/
bool deletePreference(const char* filePath, const char* key);

/*
Print the raw content of the preference file to the configured debug stream.
Returns true when the file is successfully opened and dumped.
*/
bool debugPreference(const char* filePath);

#endif
