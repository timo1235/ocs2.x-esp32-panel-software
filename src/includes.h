#pragma once

// versioning
#define OCS_PANEL_SOFTWARE_VERSION 2

#include <U8g2lib.h>
#include <Wire.h>

#include <LEDController.h>

#include <configuration.h>
#include <protocol.h>
#include <pinmap.h>
#include <iocontrol.h>
#include <display.h>
#include <display_logos.h>

#include <Arduino.h>

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
// Debouncing library
#include <Bounce2.h>
// For saving things in the flash memory
#include <Preferences.h>

// This makes it usable in all files
extern IOCONTROL ioControl;
extern DATA_TO_CONTROL dataToControl;
extern DATA_TO_CLIENT dataToClient;
extern PROTOCOL protocol;
extern DISPLAYMANAGER display;

// DEBUG MACRO
#ifdef OCS_DEBUG    //Macros are usually in all capital letters.
#define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)     //now defines a blank line
#define DPRINTLN(...)   //now defines a blank line
#endif
