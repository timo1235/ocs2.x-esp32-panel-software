#include <includes.h>

void iocontrol_setup() {
    pinMode(AUTOSQUARE_PIN, INPUT_PULLUP);
    pinMode(MOTOR_START_PIN, INPUT_PULLUP);
    pinMode(PROGRAMM_START_PIN, INPUT_PULLUP);
    pinMode(OK_PIN, INPUT_PULLUP);
    pinMode(AUSWAHL_X_PIN, INPUT_PULLUP);
    pinMode(AUSWAHL_Y_PIN, INPUT_PULLUP);
    pinMode(AUSWAHL_Z_PIN, INPUT_PULLUP);

    pinMode(RJ45_LED_L_PIN, OUTPUT);
    pinMode(RJ45_LED_R_PIN, OUTPUT);
}

void iocontrol_readAnalogPins() {

}

void iocontrol_readAll() {
    dataToControl.auswahlX = !digitalRead(AUSWAHL_X_PIN);
    dataToControl.auswahlY = !digitalRead(AUSWAHL_Y_PIN);
    dataToControl.auswahlZ = !digitalRead(AUSWAHL_Z_PIN);

    dataToControl.ok = !digitalRead(OK_PIN);
    dataToControl.motorStart = !digitalRead(MOTOR_START_PIN);
    dataToControl.programmStart = !digitalRead(PROGRAMM_START_PIN);
    dataToControl.autosquare = !digitalRead(AUTOSQUARE_PIN);

    dataToControl.joystickX = map(analogRead(JOYSTICK_X_PIN), 0,4095,0,1023);
    dataToControl.joystickY = map(analogRead(JOYSTICK_Y_PIN), 0,4095,0,1023);
    dataToControl.joystickZ = map(analogRead(JOYSTICK_Z_PIN), 0,4095,0,1023);

    dataToControl.feedrate = map(analogRead(FEEDRATE_PIN), 0,4095,0,1023);
    dataToControl.rotationSpeed = map(analogRead(SPINDLE_SPEED_PIN), 0,4095,0,1023);
}