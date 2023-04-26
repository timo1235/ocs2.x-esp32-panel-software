#pragma once

// including logos
#include "../include/logos/coldend_nok.xbm"
#include "../include/logos/coldend_ok.xbm"
#include "../include/logos/ocs2_connection_inverted.xbm"
#include "../include/logos/ocs2_connection_inverted_s.xbm"
#include "../include/logos/ocs2_nok.xbm"
#include "../include/logos/ocs2_ok.xbm"

enum DISPLAY_SCREENS {
    CALIBRATION_JOYSTICK_MIDDLE,
    CALIBRATION_JOYSTICK_X,
    CALIBRATION_JOYSTICK_Y,
    CALIBRATION_JOYSTICK_Z,
    CALIBRATION_FEEDRATE,
    CALIBRATION_ROTATION_SPEED,
    CALIBRATION_COLDEND_MIST,
    CALIBRATION_COLDEND_SPIT,
    DEFAULT_SCREEN,
    WAIT_SCREEN,
    AUTOSQUARE_SCREEN,
};

enum AS_STATES {
    none,      // Motor has an unknown state
    squared,   // Motor has reached his endstop and has stopped
    finish,    // Motor is driven down from the endstop and has stopped
    off        // Motor is off
};

enum DisplayMode {
    display_default,
    display_ocs2_only,
    display_coldend_only,
};

class DISPLAYMANAGER {
  public:
    void setup();

    void setScreen(DISPLAY_SCREENS screen, uint32_t duration_MS = 0);

  private:
    // Task handler
    static void  displayTask(void *pvParameters);
    TaskHandle_t displayTaskHandle;

    char *getMacStrFromAddress(uint8_t *address);

    void chooseDefaultScreen();
    void showDefaultDynamicScreen();
    void showDefaultOCS2OnlyScreen();
    void showDefaultColdendOnlyScreen();

    void showTemperature();
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
    void showCalibrationColdendMistScreen();
    void showCalibrationColdendSpitScreen();
    void showWaitScreen();
    void showAutoSquareScreen();

    DISPLAY_SCREENS currentScreen = DEFAULT_SCREEN;

    bool     splashFinished    = false;
    uint32_t lastDisplayUpdate = 0;

    uint32_t screenStarted  = 0;
    uint32_t screenDuration = 0;

    // circle animation
    void drawAutosquareMotorState(uint8_t posX, uint8_t posY, AS_STATES state);
    void drawCircleAnimation(uint8_t posX, uint8_t posY, uint8_t radius, uint8_t state);

    byte autosquareRunningAnimationState = 0;
    byte spindleRunningAnimationState    = 0;

    uint32_t lastAnimationStateChange = 0;
};

extern DISPLAYMANAGER display;