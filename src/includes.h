#ifndef ocs_includes_h
#define ocs_includes_h

// versioning
#define OCS_PANEL_VERSION "0.1"

#include <configuration.h>
#include <protocol.h>
#include <pinmap.h>
#include <iocontrol.h>

#include <Arduino.h>

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>

extern DATA_TO_CONTROL dataToControl;

// DEBUG MACRO
#ifdef OCS_DEBUG    //Macros are usually in all capital letters.
#define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)     //now defines a blank line
#define DPRINTLN(...)   //now defines a blank line
#endif

#endif