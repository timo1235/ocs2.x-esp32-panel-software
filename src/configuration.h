#pragma once

// This mac address has to match the mac address of the ESP32 on the OPEN-CNC-Shield 2
#define CONTROLLER_MAC_ADDRESS { 0x5E, 0x0, 0x0, 0x0, 0x0, 0x1 }

// Time in ms we wait between sending measured inputs
// Typical values for different purposes:
// - Control inputs like joystick or potentiometer: 25ms
// - Only use display for information like temperatur or Autosquare state: 500ms
#define WIFI_DELAY 50

// --- Inputs configuration ---
// This section defines, what types of inputs are connected to the panel
#define HAS_JOYSTICK                true // Includes joystick X, Y and Z
#define HAS_FEEDRATE_POTI           true
#define HAS_ROTATION_SPEED_POTI     true
#define HAS_OK_BUTTON               true
#define HAS_AUTOSQUARE_BUTTON       true
#define HAS_MOTOR_START_BUTTON      true
#define HAS_PROGRAMM_START_BUTTON   true
#define HAS_AXIS_SELECT_BUTTONS     false // Includes X, Y and Z axis select buttons

#define HAS_ENA_BUTTON              true // If true it needs to be mapped to a free button, see buttons 1-4 below
#define HAS_SPEED1_BUTTON           false // If true it needs to be mapped to a free button, see buttons 1-4 below
#define HAS_SPEED2_BUTTON           false // If true it needs to be mapped to a free button, see buttons 1-4 below
#define HAS_OUTPUT1_BUTTON          true // If true it needs to be mapped to a free button, see buttons 1-4 below
#define HAS_OUTPUT2_BUTTON          false // If true it needs to be mapped to a free button, see buttons 1-4 below
#define HAS_OUTPUT3_BUTTON          false // If true it needs to be mapped to a free button, see buttons 1-4 below
#define HAS_OUTPUT4_BUTTON          false // If true it needs to be mapped to a free button, see buttons 1-4 below

// Uncomment to enable debug messages over serial communication(usb)
#define OCS_DEBUG

// Button configuration - what is the button used for?
// Possible values: "ena", "speed1", "speed2", "output1", "output2", "output3", "output4"
#define BUTTON_1 "output1"
#define BUTTON_2 "speed1"
#define BUTTON_3 "speed2"
#define BUTTON_4 "free"
