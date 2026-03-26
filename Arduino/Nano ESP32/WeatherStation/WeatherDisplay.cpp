#include "WeatherDisplay.h"
#include <Arduino.h>


/*
===============================================================================
WeatherDisplay.cpp

Reusable SH1106 OLED rendering helpers for common screen layouts used by the
weather station project and other compatible sketches.
===============================================================================
*/

/*
Display a simple message on the OLED screen.
Supports one mandatory line and one optional line.
*/
void showMessage(Adafruit_SH1106G& display,
                 const char* line1,
                 const char* line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(0, 10);
  display.println(line1);

  if (line2[0] != '\0') {
    display.setCursor(0, 30);
    display.println(line2);
  }

  display.display();
}

/*
Display four lines of text with structured layout:
- Header line (small text)
- Two main data lines (large text)
- Footer line (small text)
*/
void display4Lines(Adafruit_SH1106G& display,
                   const char* line1,
                   const char* line2,
                   const char* line3,
                   const char* line4) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println(line1);
  display.drawLine(0, 10, 127, 10, SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println(line2);

  display.setTextSize(2);
  display.setCursor(0, 32);
  display.println(line3);

  display.drawLine(0, 52, 127, 52, SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.println(line4);

  display.display();
}
