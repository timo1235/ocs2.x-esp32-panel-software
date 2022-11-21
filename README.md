# OPEN-CNC-Shield 2.x ESP32 Panel Software

This project contains the software for an ESP32 handwheel as an remote control for the OPEN-CNC-Shield 2.x and is still under development.
The corresponding ESP32 software of the OPEN-CNC-Shield ESP32 can be found here: [OPEN-CNC-Shield 2.x ESP32 Software](https://github.com/timo1235/ocs2.x-esp32-software).

## Howto install
Open this project folder in VS Code with PlatformIO IDE and upload it to the ESP32. A good documentation how to get the IDE up and running can be found here:
[Getting Started with VS Code and PlatformIO IDE](https://randomnerdtutorials.com/vs-code-platformio-ide-esp32-esp8266-arduino/)

### Configuration
Take a look at the `src/pinmap.h` for the pin mapping of functions and at `src/configuration` for the configuration.

## Features implemented so far
- Read inputs and send them via Wifi to the Shield ESP32
- Show the current status on a i2c display
- Calibrate all the joystick axes and feedrate and rotation speed poti
  - very useful if the axes are swaped or the middle position is not right

## To be done
- implement connection over RJ45 - Serial communication

# Changelog
## version 2
- Added possibility to control also output 1-4 of the open cnc shield with the buttons 1-4

## version 1
- initial version