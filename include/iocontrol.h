#pragma once

// Debouncing library
#include <Bounce2.h>
#include <configManager.h>
#include <ioConfig.h>

#define READ_INPUT_INTERVAL_MS                           100
#define TIME_TO_PRESS_BUTTON_BEFORE_CALIBRATION_MS       3000
#define TIME_TO_PRESS_BUTTON_BEFORE_RESET_CALIBRATION_MS 6000

class IOCONTROL {
  public:
    void setup();
    void startBlinkRJ45LED();
    void stopBlinkRJ45LED();
    void startCalibration();
    bool calibrationInProgress = false;

  private:
    void readAnalogPins();
    void readAll();

    static void  loopTask(void *pvParameters);
    TaskHandle_t loopTaskHandle;

    static void  calibrationTask(void *pvParameters);
    TaskHandle_t calibrationTaskHandle;

    // Input debouncing
    Bounce buttonMenu           = Bounce();
    void   handleMenuButton();

    uint32_t      lastReadAll_MS = 0;
    LEDCONTROLLER RJ45LEDLeft    = LEDCONTROLLER(RJ45_LED_L_PIN);
};

extern IOCONTROL ioControl;