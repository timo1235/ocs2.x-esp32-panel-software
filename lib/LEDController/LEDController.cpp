#include <LEDController.h>

LEDCONTROLLER::LEDCONTROLLER(byte pin = 0)
{
    this->setup(pin);
}

void LEDCONTROLLER::setup(byte pin)
{
    this->pin = pin;
    ledcSetup(0, 13, 8);
    // attach the channel to the GPIO to be controlled
    ledcAttachPin(pin, 0);
    ledcWrite(0, 0);
}

void LEDCONTROLLER::startBlink()
{
    if (this->blinkState)
    {
        return;
    }
    this->blinkState = true;
    ledcWrite(0, 125);
}
void LEDCONTROLLER::stopBlink()
{
    if (!this->blinkState)
    {
        return;
    }
    this->blinkState = false;
    ledcWrite(0, 0);
}
void LEDCONTROLLER::loop()
{
}

void LEDCONTROLLER::setOn()
{
    ledcWrite(0, 255);
}
void LEDCONTROLLER::setOff()
{
    ledcWrite(0, 0);
}

void LEDCONTROLLER::blinkBlocking(uint16_t blinkTimeMS){
    this->setOn();
    delay(blinkTimeMS);
    this->setOff();
    delay(blinkTimeMS);
}