#pragma once

// Debouncing library
#include <Bounce2.h>

#define READ_INPUT_INTERVAL_MS 5
#define TIME_TO_PRESS_BUTTON_BEFORE_CALIBRATION_MS 3000
#define TIME_TO_PRESS_BUTTON_BEFORE_RESET_CALIBRATION_MS 6000

typedef struct
{
    int joystickOffsets[3];
    uint8_t joystickAxes[3]; // 0 = X, 1 = Y, 2 = Z -> used to remap the axes, if they are connected to the wrong pin
    bool invertAxis[3];
    bool invertFeedrate;
    bool invertRotationSpeed;
} CALIBRATION_CONFIG;

class IOCONTROL
{
public:
    void setup();
    void loop();
    void startBlinkRJ45LED();
    void stopBlinkRJ45LED();

    bool calibrationInProgress = false;

private:
    void readAnalogPins();
    void readAll();
    void readDynamicButtons(const char *functionName, Bounce *button);
    void readCalibrationConfig();
    void saveCalibrationConfig();
    void resetCalibrationConfig();
    void startCalibration();
    void showSettings();
    
    // Input debouncing
    Bounce buttonAutosquare = Bounce();
    Bounce buttonMotorStart = Bounce();
    Bounce buttonProgrammStart = Bounce();
    Bounce buttonOk = Bounce();
    Bounce buttonSelectAxisX = Bounce();
    Bounce buttonSelectAxisY = Bounce();
    Bounce buttonSelectAxisZ = Bounce();
    Bounce button1 = Bounce();
    Bounce button2 = Bounce();
    Bounce button3 = Bounce();
    Bounce button4 = Bounce();
    Bounce buttonMenu = Bounce();
    bool buttonMenuWasPressed = false;
    void handleMenuButton();

    uint32_t lastReadAll_MS = 0;
    LEDCONTROLLER RJ45LEDLeft = LEDCONTROLLER(RJ45_LED_L_PIN);

    CALIBRATION_CONFIG calibrationConfig;
};
