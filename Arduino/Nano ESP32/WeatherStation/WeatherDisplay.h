#ifndef WEATHER_DISPLAY_H
#define WEATHER_DISPLAY_H


/*
===============================================================================
WeatherDisplay.h

Public interface for reusable OLED display rendering helpers.
This module centralizes common SH1106 display layouts so the same rendering
logic can be shared across multiple Arduino sketches.
===============================================================================
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

/*
Display a simple message using one mandatory line and one optional line.
The function clears the display, renders the content, and pushes the buffer.
*/
void showMessage(Adafruit_SH1106G& display,
                 const char* line1,
                 const char* line2 = "");

/*
Display four lines of text using the project dashboard layout.
The function renders a small header, two large body lines, and a footer.
*/
void display4Lines(Adafruit_SH1106G& display,
                   const char* line1,
                   const char* line2,
                   const char* line3,
                   const char* line4);

#endif
