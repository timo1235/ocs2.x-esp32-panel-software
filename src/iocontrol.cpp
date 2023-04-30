#include <includes.h>

IOCONTROL ioControl;

char ptrTaskList[250];

void IOCONTROL::setup() {
    // Set the gpio function mapping according to the config
    for (uint8_t i = 0; i < 16; i++) {
        ioConfig.setFunction((Inputs) i, mainConfig.inputs[i]);
    }

    // Set the command according to the functions that are used
    protocol.initializeCommand();

    // Onboard menu button
    buttonMenu.attach(MENU_BUTTON_PIN, INPUT_PULLDOWN);
    buttonMenu.interval(5);

    // pinMode(RJ45_LED_L_PIN, OUTPUT);
    pinMode(RJ45_LED_R_PIN, OUTPUT);
    digitalWrite(RJ45_LED_R_PIN, HIGH);

    delay(100);
    xTaskCreatePinnedToCore(IOCONTROL::loopTask, "ioControl loop", 3000, this, 1, &loopTaskHandle, 1);
}

void IOCONTROL::loopTask(void *pvParameters) {
    auto *ioControl = (IOCONTROL *) pvParameters;
    for (;;) {
        ioControl->buttonMenu.update();

        // Read inputs if time is due and we are not calibrating
        if (millis() - ioControl->lastReadAll_MS > READ_INPUT_INTERVAL_MS && !ioControl->calibrationInProgress) {
            ioControl->readAll();
            ioControl->lastReadAll_MS = millis();
        }

        ioControl->handleMenuButton();
        vTaskDelay(2);
    }
}

void IOCONTROL::handleMenuButton() {
    // Button is released and was pressed for some time
    if (buttonMenu.rose()) {
        // Handle short press
        DPRINTLN("Restarting ESP");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        ESP.restart();
    }
}

void IOCONTROL::readAnalogPins() {}

void IOCONTROL::startCalibration() {
    if (this->calibrationInProgress) return;

    xTaskCreatePinnedToCore(IOCONTROL::calibrationTask, "calibration task", 5000, this, 1, &calibrationTaskHandle, 1);
}

void IOCONTROL::calibrationTask(void *pvParameters) {
    auto *ioControl = (IOCONTROL *) pvParameters;

    vTaskSuspend(ioControl->loopTaskHandle);

    ioControl->calibrationInProgress = true;
    ioControl->stopBlinkRJ45LED();
    DPRINTLN("Start calibration");
    if (ioConfig.hasFunction(InputFunctions::func_joystickX) && ioConfig.hasFunction(InputFunctions::func_joystickY) &&
        ioConfig.hasFunction(InputFunctions::func_joystickZ)) {
        display.setScreen(CALIBRATION_JOYSTICK_MIDDLE, 10000);
        DPRINTLN("Hold the Joystick in middle position");
        vTaskDelay(9500);
        // Calibrate middle position and get offset
        uint32_t measurement[3] = {0, 0, 0};
        uint8_t  readCount      = 50;
        for (int i = 0; i < 50; i++) {
            measurement[0] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickX)->read();
            measurement[1] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickY)->read();
            measurement[2] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickZ)->read();
            vTaskDelay(10);
        }
        for (int i = 0; i < 3; i++) {
            calibrationConfig.joystickOffsets[i] = 4095 / 2 - measurement[i] / readCount;
        }
        DPRINTLN("Joystick Axis 1(X) Offset: " + String(calibrationConfig.joystickOffsets[0]));
        DPRINTLN("Joystick Axis 2(Y) Offset: " + String(calibrationConfig.joystickOffsets[1]));
        DPRINTLN("Joystick Axis 3(Z) Offset: " + String(calibrationConfig.joystickOffsets[2]));
        // Calibrate Joystick Axes and directions
        // locate X axis and direction
        display.setScreen(WAIT_SCREEN);
        ioControl->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_JOYSTICK_X);
        DPRINTLN("Move the Joystick to the left side");
        bool    found    = false;
        bool    inverted = false;
        uint8_t axis     = 0;
        while (!found) {
            uint32_t measurement[3] = {0, 0, 0};
            measurement[0] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickX)->read();
            measurement[1] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickY)->read();
            measurement[2] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickZ)->read();
            for (uint8_t i = 0; i < 3; i++) {
                if (measurement[i] < 100) {
                    found    = true;
                    inverted = false;
                    axis     = i;
                }
                if (measurement[i] > 4095 - 100) {
                    found    = true;
                    inverted = true;
                    axis     = i;
                }
                vTaskDelay(10);
            }
        }
        DPRINTLN("Joystick X-Axis: " + String(axis));
        DPRINTLN("Joystick X-Axis inverted: " + String(inverted));
        calibrationConfig.joystickAxes[0]  = axis;
        calibrationConfig.invertAxis[axis] = inverted;
        // locate Y axis and direction
        display.setScreen(WAIT_SCREEN);
        ioControl->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_JOYSTICK_Y);
        DPRINTLN("Move the Joystick down");
        found = false;
        while (!found) {
            uint32_t measurement[3] = {0, 0, 0};
            measurement[0] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickX)->read();
            measurement[1] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickY)->read();
            measurement[2] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickZ)->read();
            for (uint8_t i = 0; i < 3; i++) {
                // Skip x axis
                if (calibrationConfig.joystickAxes[0] == i) {
                    continue;
                }
                if (measurement[i] < 100) {
                    found    = true;
                    inverted = false;
                    axis     = i;
                }
                if (measurement[i] > 4095 - 100) {
                    found    = true;
                    inverted = true;
                    axis     = i;
                }
                vTaskDelay(10);
            }
        }
        DPRINTLN("Joystick Y-Axis: " + String(axis));
        DPRINTLN("Joystick Y-Axis inverted: " + String(inverted));
        calibrationConfig.joystickAxes[1]  = axis;
        calibrationConfig.invertAxis[axis] = inverted;
        // locate Z axis and direction
        display.setScreen(WAIT_SCREEN);
        ioControl->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_JOYSTICK_Z);
        DPRINTLN("Turn the Joystick counter-clockwise");
        found = false;
        while (!found) {
            uint32_t measurement[3] = {0, 0, 0};
            measurement[0] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickX)->read();
            measurement[1] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickY)->read();
            measurement[2] += ioConfig.getGPIOByFunction(InputFunctions::func_joystickZ)->read();
            for (uint8_t i = 0; i < 3; i++) {
                // Skip x and y axis
                if (calibrationConfig.joystickAxes[0] == i || calibrationConfig.joystickAxes[1] == i) {
                    continue;
                }
                if (measurement[i] < 100) {
                    found    = true;
                    inverted = false;
                    axis     = i;
                }
                if (measurement[i] > 4095 - 100) {
                    found    = true;
                    inverted = true;
                    axis     = i;
                }
                vTaskDelay(10);
            }
        }
        DPRINTLN("Joystick Z-Axis: " + String(axis));
        DPRINTLN("Joystick Z-Axis inverted: " + String(inverted));
        calibrationConfig.joystickAxes[2]  = axis;
        calibrationConfig.invertAxis[axis] = inverted;
    }
    if (ioConfig.hasFunction(InputFunctions::func_feedrate)) {
        display.setScreen(WAIT_SCREEN);
        ioControl->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_FEEDRATE);
        DPRINTLN("Move the Feedrate Poti to the off position");
        bool inverted = false;
        bool found    = false;
        while (!found) {
            uint16_t measurement = ioConfig.getGPIOByFunction(InputFunctions::func_feedrate)->read();
            if (measurement < 100) {
                found    = true;
                inverted = false;
            } else if (measurement > 4095 - 100) {
                found    = true;
                inverted = true;
            }
            vTaskDelay(10);
        }
        calibrationConfig.invertFeedrate = inverted;
    }
    if (ioConfig.hasFunction(InputFunctions::func_rotationSpeed)) {
        display.setScreen(WAIT_SCREEN);
        ioControl->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_ROTATION_SPEED);
        DPRINTLN("Move the rotation speed Poti to the off position");
        bool inverted = false;
        bool found    = false;
        while (!found) {
            uint16_t measurement = ioConfig.getGPIOByFunction(InputFunctions::func_rotationSpeed)->read();
            if (measurement < 100) {
                found    = true;
                inverted = false;
            } else if (measurement > 4095 - 100) {
                found    = true;
                inverted = true;
            }
            vTaskDelay(10);
        }
        calibrationConfig.invertRotationSpeed = inverted;
    }

    if (ioConfig.hasFunction(InputFunctions::func_coldend_pot_mist)) {
        display.setScreen(WAIT_SCREEN);
        ioControl->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_COLDEND_MIST);
        DPRINTLN("Move the coldend mist Poti to the off position");
        bool inverted = false;
        bool found    = false;
        while (!found) {
            uint16_t measurement = ioConfig.getGPIOByFunction(InputFunctions::func_coldend_pot_mist)->read();
            if (measurement < 100) {
                found    = true;
                inverted = false;
            } else if (measurement > 4095 - 100) {
                found    = true;
                inverted = true;
            }
            vTaskDelay(10);
        }
        calibrationConfig.invertColdEndMistPoti = inverted;
    }

    if (ioConfig.hasFunction(InputFunctions::func_coldend_pot_spit)) {
        display.setScreen(WAIT_SCREEN);
        ioControl->RJ45LEDLeft.blinkBlocking(500);
        display.setScreen(CALIBRATION_COLDEND_SPIT);
        DPRINTLN("Move the coldend spit Poti to the off position");
        bool inverted = false;
        bool found    = false;
        while (!found) {
            uint16_t measurement = ioConfig.getGPIOByFunction(InputFunctions::func_coldend_pot_spit)->read();
            if (measurement < 100) {
                found    = true;
                inverted = false;
            } else if (measurement > 4095 - 100) {
                found    = true;
                inverted = true;
            }
            vTaskDelay(10);
        }
        calibrationConfig.invertColdEndSpitPoti = inverted;
    }

    configManager.saveCalibrationConfig();
    ioControl->calibrationInProgress = false;
    display.setScreen(DEFAULT_SCREEN);
    vTaskResume(ioControl->loopTaskHandle);
    vTaskDelay(10);
    vTaskDelete(NULL);
}

void IOCONTROL::readAll() {
    byte     sampleCount        = 10;
    uint16_t sampleDelay_Micros = 10;
    if (dataToControl.command.setAxisSelect) {
        dataToControl.selectAxisX = ioConfig.getGPIOByFunction(InputFunctions::func_axisXSelect)->read();
        dataToControl.selectAxisY = ioConfig.getGPIOByFunction(InputFunctions::func_axisYSelect)->read();
        dataToControl.selectAxisZ = ioConfig.getGPIOByFunction(InputFunctions::func_axisZSelect)->read();
    }
    if (dataToControl.command.setOk) {
        dataToControl.ok = ioConfig.getGPIOByFunction(InputFunctions::func_ok)->read();
    }
    if (dataToControl.command.setMotorStart) {
        dataToControl.motorStart = ioConfig.getGPIOByFunction(InputFunctions::func_motorStart)->read();
    }
    if (dataToControl.command.setProgramStart) {
        dataToControl.programStart = ioConfig.getGPIOByFunction(InputFunctions::func_programStart)->read();
    }
    if (dataToControl.command.setAutosquare) {
        dataToControl.autosquare = ioConfig.getGPIOByFunction(InputFunctions::func_autosquare)->read();
    }
    if (dataToControl.command.setEna) {
        dataToControl.ena = ioConfig.getGPIOByFunction(InputFunctions::func_ena)->read();
    }
    if (dataToControl.command.setSpeed1) {
        dataToControl.speed1 = ioConfig.getGPIOByFunction(InputFunctions::func_speed1)->read();
    }
    if (dataToControl.command.setSpeed2) {
        dataToControl.speed2 = ioConfig.getGPIOByFunction(InputFunctions::func_speed2)->read();
    }
    if (dataToControl.command.setOutput1) {
        dataToControl.output1 = ioConfig.getGPIOByFunction(InputFunctions::func_output1)->read();
    }
    if (dataToControl.command.setOutput2) {
        dataToControl.output2 = ioConfig.getGPIOByFunction(InputFunctions::func_output2)->read();
    }
    if (dataToControl.command.setOutput3) {
        dataToControl.output3 = ioConfig.getGPIOByFunction(InputFunctions::func_output3)->read();
    }
    if (dataToControl.command.setOutput4) {
        dataToControl.output4 = ioConfig.getGPIOByFunction(InputFunctions::func_output4)->read();
    }
    if (dataToControl.command.setJoystick) {
        int32_t measurement[3] = {
            ioConfig.getGPIOByFunction(InputFunctions::func_joystickX)->read(),
            ioConfig.getGPIOByFunction(InputFunctions::func_joystickY)->read(),
            ioConfig.getGPIOByFunction(InputFunctions::func_joystickZ)->read()
            };

        for (uint8_t i = 0; i < 3; i++) {
            // Add offset
            measurement[i] += calibrationConfig.joystickOffsets[i];
            // Remap the values since we lost some way because of the offset
            if (measurement[i] > 4095 / 2) {
                int16_t maxForAxis = 4095 + calibrationConfig.joystickOffsets[i];
                measurement[i]     = map(measurement[i], 4095 / 2, maxForAxis, 4095 / 2, 4095);
            } else {
                int16_t minForAxis = 0 + calibrationConfig.joystickOffsets[i];
                measurement[i]     = map(measurement[i], minForAxis, 4095 / 2, 0, 4095 / 2);
            }
            // Invert if needed
            if (calibrationConfig.invertAxis[i]) {
                measurement[i] = 4095 - measurement[i];
            }
        }
        dataToControl.joystickX = map(measurement[calibrationConfig.joystickAxes[0]], 0, 4095, 0, 1023);
        dataToControl.joystickY = map(measurement[calibrationConfig.joystickAxes[1]], 0, 4095, 0, 1023);
        dataToControl.joystickZ = map(measurement[calibrationConfig.joystickAxes[2]], 0, 4095, 0, 1023);
        // DPRINT("Joystick X(" + String(calibrationConfig.joystickAxes[0]) + "): \t" + String(dataToControl.joystickX));
        // DPRINT("\tJoystick Y(" + String(calibrationConfig.joystickAxes[1]) + "): \t" + String(dataToControl.joystickY));
        // DPRINTLN("\tJoystick Z(" + String(calibrationConfig.joystickAxes[2]) + "): \t" + String(dataToControl.joystickZ));
    }
    if (dataToControl.command.setFeedrate) {
        uint32_t feedrate =  ioConfig.getGPIOByFunction(InputFunctions::func_feedrate)->read();
        if (calibrationConfig.invertFeedrate) {
            feedrate = 4095 - feedrate;
        }
        dataToControl.feedrate = map(feedrate, 0, 4095, 0, 1023);
    }
    if (dataToControl.command.setRotationSpeed) {
        uint32_t rotationSpeed =  ioConfig.getGPIOByFunction(InputFunctions::func_rotationSpeed)->read();
        if (calibrationConfig.invertRotationSpeed) {
            rotationSpeed = 4095 - rotationSpeed;
        }
        dataToControl.rotationSpeed = map(rotationSpeed, 0, 4095, 0, 1023);
    }

    // Read ColdEnd inputs
    if (dataToColdEnd.command.setInAir) {
        dataToColdEnd.in_air = ioConfig.getGPIOByFunction(InputFunctions::func_coldend_in_air)->read();
    }
    if (dataToColdEnd.command.setInMist) {
        dataToColdEnd.in_mist = ioConfig.getGPIOByFunction(InputFunctions::func_coldend_in_mist)->read();
    }
    if (dataToColdEnd.command.setInFast) {
        dataToColdEnd.in_fast = ioConfig.getGPIOByFunction(InputFunctions::func_coldend_in_fast)->read();
    }
    if (dataToColdEnd.command.setPotMist) {
        uint16_t value = ioConfig.getGPIOByFunction(InputFunctions::func_coldend_pot_mist)->read();
        if (calibrationConfig.invertColdEndMistPoti) {
            value = 4095 - value;
        }
        dataToColdEnd.pot_mist = value;
    }
    if (dataToColdEnd.command.setPotSpit) {
        uint16_t value = ioConfig.getGPIOByFunction(InputFunctions::func_coldend_pot_spit)->read();
        if (calibrationConfig.invertColdEndSpitPoti) {
            value = 4095 - value;
        }
        dataToColdEnd.pot_spit = value;
    }
}

void IOCONTROL::startBlinkRJ45LED() { this->RJ45LEDLeft.startBlink(); }

void IOCONTROL::stopBlinkRJ45LED() { this->RJ45LEDLeft.stopBlink(); }