#pragma once

// This mac address has to match the mac address of the ESP32 on the ColdEnd32
#define COLDEND_MAC_ADDRESS                                                                                                                \
    { 0x5E, 0x0, 0x0, 0x0, 0x0, 0x5 }

#define USE_WIFI_WEBINTERFACE   // Comment this line to disable the webinterface
// #define FORCE_WIFI_DEFAULT_ON   // Uncomment this line to force the webinterface to be always on, useful if it has been deactivated in
// the config and the menu button on the pcb does not work

// Time in ms we wait between sending measured inputs over WiFi
// Typical values for different purposes:
// - Control inputs like joystick or potentiometer: 50ms
// - Only use display for information like temperatur or Autosquare state: 500ms
// - Typical value for the coldend delay: 100ms
#define OCS2_WIFI_DELAY    50
#define COLDEND_WIFI_DELAY 100

// Time in ms we wait between sending measured inputs over Serial
// Typical values for different purposes:
// - Control inputs like joystick or potentiometer: 50ms
// - Only use display for information like temperatur or Autosquare state: 500ms
#define OCS2_SERIAL_DELAY 50

// Uncomment to enable debug messages over serial communication(usb)
#define OCS_DEBUG