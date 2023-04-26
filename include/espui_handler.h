#pragma once

#include <WiFi.h>

class UIHANDLER {
  public:
    void setup();
    void updateStatusUI();

  private:
    void         updateButtonStyleHelper(bool enabled, bool active, uint16_t *uiHandle);
    void         updateSliderHelper(bool enabled, uint16_t value, uint16_t *uiHandle);
    uint32_t     lastUiUpdate_MS = 0;
    static void  loopTask(void *pvParameters);
    TaskHandle_t loopTaskHandle;
};

extern UIHANDLER uiHandler;
