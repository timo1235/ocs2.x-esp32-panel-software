#pragma once

// versioning
#define OCS_PANEL_SOFTWARE_VERSION 3

#include <Arduino.h>

#include <configuration.h>
#include <pinmap.h>

// Display libraries
#include <U8g2lib.h>
#include <Wire.h>

// ESPUI libraries
#include <DNSServer.h>
#include <ESPUI.h>

#include <LEDController.h>

// ESP32 Panel libraries
#include <configManager.h>
#include <display.h>
#include <display_logos.h>
#include <espui_handler.h>
#include <ioConfig.h>
#include <iocontrol.h>
#include <protocol.h>

// WiFi libraries
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
// Debouncing library
#include <Bounce2.h>
// For saving things in the flash memory
#include <Preferences.h>

// DEBUG MACRO
#ifdef OCS_DEBUG                                        // Macros are usually in all capital letters.
    #define DPRINT(...)   Serial.print(__VA_ARGS__)     // DPRINT is a macro, debug print
    #define DPRINTLN(...) Serial.println(__VA_ARGS__)   // DPRINTLN is a macro, debug print with new line
#else
    #define DPRINT(...)                                 // now defines a blank line
    #define DPRINTLN(...)                               // now defines a blank line
#endif
