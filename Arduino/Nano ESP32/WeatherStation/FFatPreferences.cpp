#include "FFatPreferences.h"
#include <Arduino.h>


/*
===============================================================================
FFatPreferences.cpp

Reusable FFat-backed helpers for storing simple key/value preferences.
This module centralizes preference file parsing, update logic, and optional
debug output so the same behavior can be shared across multiple sketches.
===============================================================================
*/

namespace {
// Optional debug output stream configured by the host sketch.
Stream* preferenceDebugStream = nullptr;

// Write a message without a trailing newline when debug output is enabled.
void preferenceDebugPrint(const char* message) {
  if (preferenceDebugStream != nullptr) {
    preferenceDebugStream->print(message);
  }
}

// Write a C-string message followed by a newline when debug output is enabled.
void preferenceDebugPrintln(const char* message) {
  if (preferenceDebugStream != nullptr) {
    preferenceDebugStream->println(message);
  }
}

// Write an Arduino String followed by a newline when debug output is enabled.
void preferenceDebugPrintln(const String& message) {
  if (preferenceDebugStream != nullptr) {
    preferenceDebugStream->println(message);
  }
}
}

/*
Set the stream used for optional debug output.
Pass nullptr to disable all logging from this module.
*/
void setPreferenceDebugStream(Stream* stream) {
  preferenceDebugStream = stream;
}

/*
Read a single preference value from a text file.
The file format is one "key=value" entry per line, with support for comments
starting with '#'. Returns defaultValue if the file or key does not exist.
*/
String readPreference(const char* filePath,
                      const char* key,
                      const char* defaultValue) {
  File file = FFat.open(filePath, "r");
  if (!file) {
    preferenceDebugPrint("Unable to open preference file for reading: ");
    preferenceDebugPrintln(filePath);
    return String(defaultValue);
  }

  String targetKey = String(key);

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() == 0 || line.startsWith("#")) {
      continue;
    }

    int separatorIndex = line.indexOf('=');
    if (separatorIndex < 0) {
      continue;
    }

    String currentKey = line.substring(0, separatorIndex);
    String currentValue = line.substring(separatorIndex + 1);
    currentKey.trim();
    currentValue.trim();

    if (currentKey == targetKey) {
      file.close();
      return currentValue;
    }
  }

  file.close();
  return String(defaultValue);
}

/*
Read the full preference file content as a single string.
Lines are trimmed before being appended to the returned content.
Returns defaultValue if the file cannot be opened.
*/
String readAllPreferences(const char* filePath,
                          const char* defaultValue) {
  File file = FFat.open(filePath, "r");
  if (!file) {
    preferenceDebugPrint("Unable to open preference file for full read: ");
    preferenceDebugPrintln(filePath);
    return String(defaultValue);
  }

  String content;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    content += line;

    if (file.available()) {
      content += "\n";
    }
  }

  file.close();
  return content;
}

/*
Create or update a key/value pair in the preference file.
Existing entries are preserved, comment lines remain untouched, and the target
key is either replaced in place or appended if it does not exist yet.
*/
bool writePreference(const char* filePath,
                     const char* key,
                     const char* value) {
  String targetKey = String(key);
  String updatedContent;
  bool keyUpdated = false;

  File inputFile = FFat.open(filePath, "r");
  if (inputFile) {
    while (inputFile.available()) {
      String line = inputFile.readStringUntil('\n');
      line.trim();

      if (line.length() == 0) {
        continue;
      }

      if (line.startsWith("#")) {
        updatedContent += line + "\n";
        continue;
      }

      int separatorIndex = line.indexOf('=');
      if (separatorIndex < 0) {
        updatedContent += line + "\n";
        continue;
      }

      String currentKey = line.substring(0, separatorIndex);
      currentKey.trim();

      if (currentKey == targetKey) {
        updatedContent += targetKey + "=" + String(value) + "\n";
        keyUpdated = true;
      } else {
        updatedContent += line + "\n";
      }
    }

    inputFile.close();
  }

  if (!keyUpdated) {
    updatedContent += targetKey + "=" + String(value) + "\n";
  }

  File outputFile = FFat.open(filePath, "w");
  if (!outputFile) {
    preferenceDebugPrint("Unable to open preference file for writing: ");
    preferenceDebugPrintln(filePath);
    return false;
  }

  size_t written = outputFile.print(updatedContent);
  outputFile.close();

  return written == updatedContent.length();
}

/*
Remove a key/value pair from the preference file.
Returns false if the file cannot be read, the key is not present, or the file
cannot be rewritten after filtering the target entry out.
*/
bool deletePreference(const char* filePath, const char* key) {
  File inputFile = FFat.open(filePath, "r");
  if (!inputFile) {
    preferenceDebugPrint("Unable to open preference file for deletion: ");
    preferenceDebugPrintln(filePath);
    return false;
  }

  String targetKey = String(key);
  String updatedContent;
  bool keyDeleted = false;

  while (inputFile.available()) {
    String line = inputFile.readStringUntil('\n');
    line.trim();

    if (line.length() == 0) {
      continue;
    }

    if (line.startsWith("#")) {
      updatedContent += line + "\n";
      continue;
    }

    int separatorIndex = line.indexOf('=');
    if (separatorIndex < 0) {
      updatedContent += line + "\n";
      continue;
    }

    String currentKey = line.substring(0, separatorIndex);
    currentKey.trim();

    if (currentKey == targetKey) {
      keyDeleted = true;
      continue;
    }

    updatedContent += line + "\n";
  }

  inputFile.close();

  if (!keyDeleted) {
    return false;
  }

  File outputFile = FFat.open(filePath, "w");
  if (!outputFile) {
    preferenceDebugPrint("Unable to rewrite preference file after deletion: ");
    preferenceDebugPrintln(filePath);
    return false;
  }

  size_t written = outputFile.print(updatedContent);
  outputFile.close();

  return written == updatedContent.length();
}

/*
Print the raw preference file content to the configured debug stream.
This is intended for troubleshooting preference storage and parsing behavior.
*/
bool debugPreference(const char* filePath) {
  File file = FFat.open(filePath, "r");
  if (!file) {
    preferenceDebugPrint("Unable to open preference file for debug: ");
    preferenceDebugPrintln(filePath);
    return false;
  }

  preferenceDebugPrint("Preference file dump: ");
  preferenceDebugPrintln(filePath);
  preferenceDebugPrintln("------------------------");

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    preferenceDebugPrintln(line);
  }

  preferenceDebugPrintln("------------------------");
  file.close();
  return true;
}
