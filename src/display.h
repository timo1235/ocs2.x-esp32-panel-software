#pragma once

enum DISPLAY_SCREENS{
    CALIBRATION_JOYSTICK_MIDDLE,
    CALIBRATION_JOYSTICK_X,
    CALIBRATION_JOYSTICK_Y,
    CALIBRATION_JOYSTICK_Z,
    CALIBRATION_FEEDRATE,
    CALIBRATION_ROTATION_SPEED,
    DEFAULT_SCREEN,
    WAIT_SCREEN,
    CALIBRATION_SCREEN,
    RESET_SCREEN, 
    SETTINGS1_SCREEN,
    SETTINGS2_SCREEN,
    SETTINGS3_SCREEN,
    SETTINGS4_SCREEN,
    SETTINGS5_SCREEN,
    AUTOSQUARE_SCREEN,
};

enum AS_STATES
{
    none,    // Motor has an unknown state
    squared, // Motor has reached his endstop and has stopped
    finish,  // Motor is driven down from the endstop and has stopped
    off      // Motor is off
};

class DISPLAYMANAGER
{
public:
    void setup();

    void setScreen(DISPLAY_SCREENS screen, uint32_t duration_MS = 0);
private:
    // Task handler
    static void displayTask(void *pvParameters);
    TaskHandle_t displayTaskHandle;

    char *getMacStrFromAddress(uint8_t *address);

    void showTemperature();
    void showDefaultScreen();
    void addJoystickStates();
    void addFeedrateState();
    void addRotationSpeedState();

    void addCalibrationHeadline();
    void showClibrationJoystickMiddleScreen();
    void showClibrationJoystickXScreen();
    void showClibrationJoystickYScreen();
    void showClibrationJoystickZScreen();
    void showCalibrationFeedrateScreen();
    void showCalibrationRotationSpeedScreen();
    void showWaitScreen();
    void showCalibrationScreen();
    void showResetScreen();
    void showSettings1Screen();
    void showSettings2Screen();
    void showSettings3Screen();
    void showSettings4Screen();
    void showSettings5Screen();
    void showAutoSquareScreen();

    DISPLAY_SCREENS currentScreen = DEFAULT_SCREEN; 

    bool splashFinished = false;
    uint32_t lastDisplayUpdate = 0;

    uint32_t screenStarted = 0;
    uint32_t screenDuration = 0;

    // Autosquare running circle animation
    void drawAutosquareMotorState(uint8_t posX, uint8_t posY, AS_STATES state);
    void drawCircleAnimation(uint8_t posX, uint8_t posY, uint8_t radius, uint8_t state);
    byte autosquareRunningAnimationState = 0;
    uint32_t lastAnimationStateChange = 0;
};
