#pragma once
#include <Arduino.h>

class LEDCONTROLLER {
    public:
        LEDCONTROLLER(byte pin);
        void setup(byte pin);
        void loop();
        void startBlink();
        void stopBlink();
        bool getBusy();
        void setOn();
        void setOff();
        void blinkBlocking(uint16_t blinkTimeMS);
    private:
        byte blinkState; // 1 or 0
        byte pin;
        byte busy; // 1 when blink in progress or 0 when not
        uint32_t startTime;
        uint32_t blinkTime;
};