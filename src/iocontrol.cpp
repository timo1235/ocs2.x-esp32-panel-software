#include <includes.h>

Preferences preferences;

void IOCONTROL::setup()
{
    preferences.begin("iocontrol", false);

    buttonAutosquare.attach(AUTOSQUARE_PIN, INPUT_PULLUP);
    buttonAutosquare.interval(5);
    buttonMotorStart.attach(MOTOR_START_PIN, INPUT_PULLUP);
    buttonMotorStart.interval(5);
    buttonProgrammStart.attach(PROGRAMM_START_PIN, INPUT_PULLUP);
    buttonProgrammStart.interval(5);
    buttonOk.attach(OK_PIN, INPUT_PULLUP);
    buttonOk.interval(5);
    buttonSelectAxisX.attach(SELECT_AXIS_X_PIN, INPUT_PULLUP);
    buttonSelectAxisX.interval(5);
    buttonSelectAxisY.attach(SELECT_AXIS_Y_PIN, INPUT_PULLUP);
    buttonSelectAxisY.interval(5);
    buttonSelectAxisZ.attach(SELECT_AXIS_Z_PIN, INPUT_PULLUP);
    buttonSelectAxisZ.interval(5);
    button1.attach(BUTTON_1_PIN, INPUT_PULLUP);
    button1.interval(5);
    button2.attach(BUTTON_2_PIN, INPUT_PULLUP);
    button2.interval(5);
    button3.attach(BUTTON_3_PIN, INPUT_PULLUP);
    button4.interval(5);
    button4.attach(BUTTON_4_PIN, INPUT_PULLUP);
    button4.interval(5);
    buttonMenu.attach(MENU_BUTTON_PIN, INPUT_PULLDOWN);
    buttonMenu.interval(5);

    // pinMode(RJ45_LED_L_PIN, OUTPUT);
    pinMode(RJ45_LED_R_PIN, OUTPUT);
    digitalWrite(RJ45_LED_R_PIN, HIGH);

    this->readCalibrationConfig();
}

void IOCONTROL::loop()
{
    this->RJ45LEDLeft.loop();

    buttonAutosquare.update();
    buttonMotorStart.update();
    buttonProgrammStart.update();
    buttonOk.update();
    buttonSelectAxisX.update();
    buttonSelectAxisY.update();
    buttonSelectAxisZ.update();
    button1.update();
    button2.update();
    button3.update();
    button4.update();
    buttonMenu.update();

    // Read inputs if time is due and we are not calibrating
    if (millis() - this->lastReadAll_MS > READ_INPUT_INTERVAL_MS && !this->calibrationInProgress)
    {
        this->readAll();
        this->lastReadAll_MS = millis();
    }

    this->handleMenuButton();

}

void IOCONTROL::handleMenuButton()
{
    Bounce *button;
    bool pressedState = HIGH;
    // Use button4 as menu button if configured
    if(BUTTON_4 == "menu") {
        button = &button4;
        pressedState = LOW;
    } else {
        button = &buttonMenu;
    }

    // Button is released and was pressed for some time
    if (button->read() == !pressedState && buttonMenuWasPressed)
    {
        this->buttonMenuWasPressed = false;
        // Handle longest press first
        // Reset calibration
        if (button->previousDuration() > TIME_TO_PRESS_BUTTON_BEFORE_RESET_CALIBRATION_MS)
        {
            DPRINTLN("Previous duration: " + String(button->previousDuration()) + " -- Resetting calibration");
            display.setScreen(WAIT_SCREEN);
            this->resetCalibrationConfig();
            delay(2000);
            display.setScreen(DEFAULT_SCREEN);
            return;
        }
        // Start Calibration
        if (button->previousDuration() > TIME_TO_PRESS_BUTTON_BEFORE_CALIBRATION_MS)
        {
            DPRINTLN("Previous duration: " + String(button->previousDuration()) + " -- Starting calibration");
            this->startCalibration();
            return;
        }
        // Handle short press
        this->showSettings();
        return;
    }
    // Button is pressed
    if (button->read() == pressedState)
    {
        this->stopBlinkRJ45LED();
        this->buttonMenuWasPressed = true;
        // Show reset screen
        if (button->currentDuration() > TIME_TO_PRESS_BUTTON_BEFORE_RESET_CALIBRATION_MS)
        {
            DPRINTLN("Longer pressed button 4");
            display.setScreen(RESET_SCREEN);
            this->RJ45LEDLeft.blinkBlocking(100);
            return;
        }
        // Show calibration screen
        if (button->currentDuration() > TIME_TO_PRESS_BUTTON_BEFORE_CALIBRATION_MS)
        {
            DPRINTLN("Long pressed button 4");
            display.setScreen(CALIBRATION_SCREEN);
            this->RJ45LEDLeft.blinkBlocking(100);
            this->RJ45LEDLeft.blinkBlocking(100);
            return;
        }
    }
}

void IOCONTROL::showSettings()
{
    Bounce *button = &buttonMenu;
    bool useButton4 = false;

    // Use button4 as menu button if configured
    if(BUTTON_4 == "menu") {
        button = &button4;
        useButton4 = true;
    }

    button->update();
    display.setScreen(SETTINGS1_SCREEN);
    while ((useButton4) ? !button->rose() : !button->fell())
    {
        button->update();
    }
    button->update();
    display.setScreen(SETTINGS2_SCREEN);
    while ((useButton4) ? !button->rose() : !button->fell())
    {
        button->update();
    }
    button->update();
    display.setScreen(SETTINGS3_SCREEN);
    while ((useButton4) ? !button->rose() : !button->fell())
    {
        button->update();
    }
    button->update();
    display.setScreen(SETTINGS4_SCREEN);
    while ((useButton4) ? !button->rose() : !button->fell())
    {
        button->update();
    }
    button->update();
    display.setScreen(SETTINGS5_SCREEN);
    while ((useButton4) ? !button->rose() : !button->fell())
    {
        button->update();
    }
    button->update();
    display.setScreen(DEFAULT_SCREEN);
}

void IOCONTROL::readAnalogPins()
{
}

void IOCONTROL::readCalibrationConfig()
{
    DPRINTLN("Read calibration config: ");
    size_t result = preferences.getBytes("config", &this->calibrationConfig, sizeof(this->calibrationConfig));
    if (result == 0)
    {
        DPRINTLN("No configuration saved in memory, assuming default values");
        this->calibrationConfig = {{0, 0, 0}, {0, 1, 2}, {false, false, false}, false, false};
    }
    DPRINT("Offsets: ");
    DPRINT(this->calibrationConfig.joystickOffsets[0]);
    DPRINT(", ");
    DPRINT(this->calibrationConfig.joystickOffsets[1]);
    DPRINT(", ");
    DPRINTLN(this->calibrationConfig.joystickOffsets[2]);
    DPRINT("Axes: ");
    DPRINT(this->calibrationConfig.joystickAxes[0]);
    DPRINT(", ");
    DPRINT(this->calibrationConfig.joystickAxes[1]);
    DPRINT(", ");
    DPRINTLN(this->calibrationConfig.joystickAxes[2]);
    DPRINT("Invert axis: ");
    DPRINT(this->calibrationConfig.invertAxis[0]);
    DPRINT(", ");
    DPRINT(this->calibrationConfig.invertAxis[1]);
    DPRINT(", ");
    DPRINTLN(this->calibrationConfig.invertAxis[2]);
    DPRINT("Invert feedrate: ");
    DPRINTLN(this->calibrationConfig.invertFeedrate);
    DPRINT("Invert rotation speed: ");
    DPRINTLN(this->calibrationConfig.invertRotationSpeed);
}

void IOCONTROL::saveCalibrationConfig()
{
    preferences.putBytes("config", &this->calibrationConfig, sizeof(this->calibrationConfig));
}

void IOCONTROL::resetCalibrationConfig()
{
    this->calibrationConfig = {{0, 0, 0}, {0, 1, 2}, {false, false, false}, false, false};
    preferences.clear();
}

void IOCONTROL::startCalibration()
{
    this->calibrationInProgress = true;
    this->stopBlinkRJ45LED();
    if (HAS_JOYSTICK)
    {
        display.setScreen(CALIBRATION_JOYSTICK_MIDDLE, 10000);
        DPRINTLN("Start calibration");
        DPRINTLN("Hold the Joystick in middle position");
        delay(9500);
        // Calibrate middle position and get offset
        uint32_t measurement[3] = {0, 0, 0};
        uint8_t readCount = 50;
        for (int i = 0; i < 50; i++)
        {
            measurement[0] += analogRead(JOYSTICK_X_PIN);
            measurement[1] += analogRead(JOYSTICK_Y_PIN);
            measurement[2] += analogRead(JOYSTICK_Z_PIN);
            delay(10);
        }
        for (int i = 0; i < 3; i++)
        {
            this->calibrationConfig.joystickOffsets[i] = 4095 / 2 - measurement[i] / readCount;
        }
        DPRINTLN("Joystick Axis 1(X) Offset: " + String(calibrationConfig.joystickOffsets[0]));
        DPRINTLN("Joystick Axis 2(Y) Offset: " + String(calibrationConfig.joystickOffsets[1]));
        DPRINTLN("Joystick Axis 3(Z) Offset: " + String(calibrationConfig.joystickOffsets[2]));
        // Calibrate Joystick Axes and directions
        // locate X axis and direction
        display.setScreen(WAIT_SCREEN);
        this->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_JOYSTICK_X);
        DPRINTLN("Move the Joystick to the left side");
        bool found = false;
        bool inverted = false;
        uint8_t axis = 0;
        while (!found)
        {
            uint32_t measurement[3] = {analogRead(JOYSTICK_X_PIN), analogRead(JOYSTICK_Y_PIN), analogRead(JOYSTICK_Z_PIN)};
            for (uint8_t i = 0; i < 3; i++)
            {
                if (measurement[i] < 100)
                {
                    found = true;
                    inverted = false;
                    axis = i;
                }
                if (measurement[i] > 4095 - 100)
                {
                    found = true;
                    inverted = true;
                    axis = i;
                }
                delay(10);
            }
        }
        DPRINTLN("Joystick X-Axis: " + String(axis));
        DPRINTLN("Joystick X-Axis inverted: " + String(inverted));
        this->calibrationConfig.joystickAxes[0] = axis;
        this->calibrationConfig.invertAxis[axis] = inverted;
        // locate Y axis and direction
        display.setScreen(WAIT_SCREEN);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_JOYSTICK_Y);
        DPRINTLN("Move the Joystick down");
        found = false;
        while (!found)
        {
            uint32_t measurement[3] = {analogRead(JOYSTICK_X_PIN), analogRead(JOYSTICK_Y_PIN), analogRead(JOYSTICK_Z_PIN)};
            for (uint8_t i = 0; i < 3; i++)
            {
                // Skip x axis
                if (this->calibrationConfig.joystickAxes[0] == i)
                {
                    continue;
                }
                if (measurement[i] < 100)
                {
                    found = true;
                    inverted = false;
                    axis = i;
                }
                if (measurement[i] > 4095 - 100)
                {
                    found = true;
                    inverted = true;
                    axis = i;
                }
                delay(10);
            }
        }
        DPRINTLN("Joystick Y-Axis: " + String(axis));
        DPRINTLN("Joystick Y-Axis inverted: " + String(inverted));
        this->calibrationConfig.joystickAxes[1] = axis;
        this->calibrationConfig.invertAxis[axis] = inverted;
        // locate Z axis and direction
        display.setScreen(WAIT_SCREEN);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_JOYSTICK_Z);
        DPRINTLN("Turn the Joystick counter-clockwise");
        found = false;
        while (!found)
        {
            uint32_t measurement[3] = {analogRead(JOYSTICK_X_PIN), analogRead(JOYSTICK_Y_PIN), analogRead(JOYSTICK_Z_PIN)};
            for (uint8_t i = 0; i < 3; i++)
            {
                // Skip x and y axis
                if (this->calibrationConfig.joystickAxes[0] == i || this->calibrationConfig.joystickAxes[1] == i)
                {
                    continue;
                }
                if (measurement[i] < 100)
                {
                    found = true;
                    inverted = false;
                    axis = i;
                }
                if (measurement[i] > 4095 - 100)
                {
                    found = true;
                    inverted = true;
                    axis = i;
                }
                delay(10);
            }
        }
        DPRINTLN("Joystick Z-Axis: " + String(axis));
        DPRINTLN("Joystick Z-Axis inverted: " + String(inverted));
        this->calibrationConfig.joystickAxes[2] = axis;
        this->calibrationConfig.invertAxis[axis] = inverted;
    }
    if (HAS_FEEDRATE_POTI)
    {
        display.setScreen(WAIT_SCREEN);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_FEEDRATE);
        DPRINTLN("Move the Feedrate Poti to the off position");
        bool inverted = false;
        bool found = false;
        while (!found)
        {
            uint16_t measurement = analogRead(FEEDRATE_PIN);
            if (measurement < 100)
            {
                found = true;
                inverted = false;
            }
            else if (measurement > 4095 - 100)
            {
                found = true;
                inverted = true;
            }
            delay(10);
        }
        this->calibrationConfig.invertFeedrate = inverted;
    }
    if (HAS_ROTATION_SPEED_POTI)
    {
        display.setScreen(WAIT_SCREEN);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        this->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_ROTATION_SPEED);
        DPRINTLN("Move the rotation speed Poti to the off position");
        bool inverted = false;
        bool found = false;
        while (!found)
        {
            uint16_t measurement = analogRead(ROTATION_SPEED_PIN);
            if (measurement < 100)
            {
                found = true;
                inverted = false;
            }
            else if (measurement > 4095 - 100)
            {
                found = true;
                inverted = true;
            }
            delay(10);
        }
        this->calibrationConfig.invertRotationSpeed = inverted;
    }

    this->saveCalibrationConfig();
    this->calibrationInProgress = false;
    display.setScreen(DEFAULT_SCREEN);

    // Needed so that calibration wont start directly again
    buttonMenu.update();
    button4.update();
}

void IOCONTROL::readDynamicButtons(const char *functionName, Bounce *button) {
    if(functionName == "ena") {
        dataToControl.ena = !button->read();
    } else if(functionName == "speed1") {
        dataToControl.speed1 = !button->read();
    } else if(functionName == "speed2") {
        dataToControl.speed2 = !button->read();
    } else if(functionName == "output1") {
        dataToControl.output1 = !button->read();
    } else if(functionName == "output2") {
        dataToControl.output2 = !button->read();
    } else if(functionName == "output3") {
        dataToControl.output3 = !button->read();
    } else if(functionName == "output4") {
        dataToControl.output4 = !button->read();
    }
}

void IOCONTROL::readAll()
{
    byte sampleCount = 10;
    uint16_t sampleDelay_Micros = 10;
    dataToControl.selectAxisX = !buttonSelectAxisX.read();
    dataToControl.selectAxisY = !buttonSelectAxisY.read();
    dataToControl.selectAxisZ = !buttonSelectAxisZ.read();
    dataToControl.ok = !buttonOk.read();
    dataToControl.motorStart = !buttonMotorStart.read();
    dataToControl.programmStart = !buttonProgrammStart.read();
    dataToControl.autosquare = !buttonAutosquare.read();
    this->readDynamicButtons(BUTTON_1, &this->button1);
    this->readDynamicButtons(BUTTON_2, &this->button2);
    this->readDynamicButtons(BUTTON_3, &this->button3);
    this->readDynamicButtons(BUTTON_4, &this->button4);

    int32_t measurement[3] = {0, 0, 0};
    // Read joystick with some samples to get a better value
    for (uint8_t i = 0; i < sampleCount; i++)
    {
        measurement[0] += analogRead(JOYSTICK_X_PIN);
        measurement[1] += analogRead(JOYSTICK_Y_PIN);
        measurement[2] += analogRead(JOYSTICK_Z_PIN);
        delayMicroseconds(sampleDelay_Micros);
    }
    measurement[0] /= sampleCount;
    measurement[1] /= sampleCount;
    measurement[2] /= sampleCount;
    
    for (uint8_t i = 0; i < 3; i++)
    {
        // Add offset
        measurement[i] += this->calibrationConfig.joystickOffsets[i];
        // Remap the values since we lost some way because of the offset
        if (measurement[i] > 4095 / 2)
        {
            int16_t maxForAxis = 4095 + this->calibrationConfig.joystickOffsets[i];
            measurement[i] = map(measurement[i], 4095 / 2, maxForAxis, 4095 / 2, 4095);
        }
        else
        {
            int16_t minForAxis = 0 + this->calibrationConfig.joystickOffsets[i];
            measurement[i] = map(measurement[i], minForAxis, 4095 / 2, 0, 4095 / 2);
        }
        // Invert if needed
        if (this->calibrationConfig.invertAxis[i])
        {
            measurement[i] = 4095 - measurement[i];
        }
    }
    dataToControl.joystickX = map(measurement[this->calibrationConfig.joystickAxes[0]], 0, 4095, 0, 1023);
    dataToControl.joystickY = map(measurement[this->calibrationConfig.joystickAxes[1]], 0, 4095, 0, 1023);
    dataToControl.joystickZ = map(measurement[this->calibrationConfig.joystickAxes[2]], 0, 4095, 0, 1023);

    // DPRINT("Joystick X(" + String(this->calibrationConfig.joystickAxes[0]) + "): \t" + String(dataToControl.joystickX));
    // DPRINT("\tJoystick Y(" + String(this->calibrationConfig.joystickAxes[1]) + "): \t" + String(dataToControl.joystickY));
    // DPRINTLN("\tJoystick Z(" + String(this->calibrationConfig.joystickAxes[2]) + "): \t" + String(dataToControl.joystickZ));

    uint32_t feedrate = 0;
    for (uint8_t i = 0; i < sampleCount; i++)
    {
        feedrate += analogRead(FEEDRATE_PIN);
        delayMicroseconds(sampleDelay_Micros);
    }
    feedrate /= sampleCount;
    
    if (this->calibrationConfig.invertFeedrate)
    {
        feedrate = 4095 - feedrate;
    }
    uint32_t rotationSpeed = 0;
    for (uint8_t i = 0; i < sampleCount; i++)
    {
        rotationSpeed += analogRead(ROTATION_SPEED_PIN);
        delayMicroseconds(sampleDelay_Micros);
    }
    rotationSpeed /= sampleCount;
    
    if (this->calibrationConfig.invertRotationSpeed)
    {
        rotationSpeed = 4095 - rotationSpeed;
    }

    dataToControl.feedrate = map(feedrate, 0, 4095, 0, 1023);
    dataToControl.rotationSpeed = map(rotationSpeed, 0, 4095, 0, 1023);
}

void IOCONTROL::startBlinkRJ45LED()
{
    this->RJ45LEDLeft.startBlink();
}

void IOCONTROL::stopBlinkRJ45LED()
{
    this->RJ45LEDLeft.stopBlink();
}

IOCONTROL ioControl;