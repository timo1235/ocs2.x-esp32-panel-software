#pragma once

#include <display.h>
#include <ioConfig.h>

typedef struct {
    int     joystickOffsets[3];
    uint8_t joystickAxes[3];   // 0 = X, 1 = Y, 2 = Z -> used to remap the axes, if they are connected to the wrong pin
    bool    invertAxis[3];
    bool    invertFeedrate;
    bool    invertRotationSpeed;
    bool    invertColdEndMistPoti;
    bool    invertColdEndSpitPoti;
} CALIBRATION_CONFIG;

enum CommunicationMode {
    automatic,
    onlySerial,
    onlyWifi,
};

// Configuration for the inputs
// There are 17 inputs which can be mapped to functions
// Example: inputs[0](Inputs::in_joystickX) mapped to InputFunctions::func_ok that means,
// the input joystickX is mapped to the function ok
typedef struct {
    InputFunctions    inputs[16];
    DisplayMode       displayMode;
    bool              wifiDefaultOn;
    CommunicationMode communicationMode;
    uint8_t           ocs2MacAddressCustomByte;   // Wireless Address of the OCS2
} CONFIGURATION;

extern CALIBRATION_CONFIG calibrationConfig;
extern CONFIGURATION      mainConfig;

class CONFIGMANAGER {
  public:
    // This is used to start the config hotspot, if there is no config is not saved in the memory
    static bool startConfigHotspot;

    void setup();

    static void loadCalibrationConfig();
    static void saveCalibrationConfig();
    static void resetCalibrationConfig();
    static void printCalibrationConfig();

    static void startHotspotAfterReset();
    static void loadHotspotConfig();

    static void loadMainConfig();
    static void saveMainConfig();
    static void resetMainConfig();
    static void printMainConfig();

  private:
};

extern CONFIGMANAGER configManager;
