#include <includes.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);

extern uint8_t controllerMacAddress[];

#define FONT_B8PX  u8g2_font_helvB08_tr
#define FONT_B10PX u8g2_font_helvB10_tr
#define FONT_B12PX u8g2_font_helvB12_tr
#define FONT_B14PX u8g2_font_helvB14_tr
#define FONT_B18PX u8g2_font_helvB18_tr
#define FONT_B24PX u8g2_font_helvB24_tr

void DISPLAYMANAGER::setup() {
    u8g2.begin();

    // Create a task for the Display
    xTaskCreatePinnedToCore(DISPLAYMANAGER::displayTask, /* Task function. */
                            "Display Task",              /* name of task. */
                            3000,                        /* Stack size of task */
                            this,                        /* parameter of the task */
                            1,                           /* priority of the task */
                            &displayTaskHandle,          /* Task handle to keep track of created task */
                            0);
}

void DISPLAYMANAGER::displayTask(void *pvParameters) {
    auto *display = (DISPLAYMANAGER *) pvParameters;
    for (;;) {
        if (!display->splashFinished) {
            u8g2.clearBuffer();
            u8g2.drawXBM(0, 0, OCS2_LOGO_width, OCS2_LOGO_height, (uint8_t *) OCS2_LOGO_bits);
            u8g2.sendBuffer();
            vTaskDelay(1000);
            display->splashFinished = true;
        }
        if (dataToClient.autosquareRunning) {
            display->showAutoSquareScreen();
        } else {
            switch (display->currentScreen) {
            case DEFAULT_SCREEN:
                display->chooseDefaultScreen();
                break;
            case CALIBRATION_JOYSTICK_MIDDLE:
                display->showClibrationJoystickMiddleScreen();
                break;
            case CALIBRATION_JOYSTICK_X:
                display->showClibrationJoystickXScreen();
                break;
            case CALIBRATION_JOYSTICK_Y:
                display->showClibrationJoystickYScreen();
                break;
            case CALIBRATION_JOYSTICK_Z:
                display->showClibrationJoystickZScreen();
                break;
            case CALIBRATION_FEEDRATE:
                display->showCalibrationFeedrateScreen();
                break;
            case CALIBRATION_ROTATION_SPEED:
                display->showCalibrationRotationSpeedScreen();
                break;
            case CALIBRATION_COLDEND_MIST:
                display->showCalibrationColdendMistScreen();
                break;
            case CALIBRATION_COLDEND_SPIT:
                display->showCalibrationColdendSpitScreen();
                break;
            case WAIT_SCREEN:
                display->showWaitScreen();
                break;
            }
        }
        vTaskDelay(20);
    }
}

void DISPLAYMANAGER::setScreen(DISPLAY_SCREENS screen, uint32_t duration_MS) {
    this->currentScreen  = screen;
    this->screenStarted  = millis();
    this->screenDuration = duration_MS;
}

void DISPLAYMANAGER::chooseDefaultScreen() {
    // If default screen is set, may choose ocs2 or coldend only if there are no functions for both
    if (mainConfig.displayMode == DisplayMode::display_default) {
        if (PROTOCOL::hasOCS2Functions && !PROTOCOL::hasColdEndFunctions) mainConfig.displayMode = DisplayMode::display_ocs2_only;
        if (!PROTOCOL::hasOCS2Functions && PROTOCOL::hasColdEndFunctions) mainConfig.displayMode = DisplayMode::display_coldend_only;
    }
    switch ((DisplayMode) mainConfig.displayMode) {
    case DisplayMode::display_ocs2_only:
        showDefaultOCS2OnlyScreen();
        break;
    case DisplayMode::display_coldend_only:
        showDefaultColdendOnlyScreen();
        break;
    default:
        showDefaultDynamicScreen();
        break;
    }
}

void DISPLAYMANAGER::showDefaultDynamicScreen() {
    // Showing the automatically adjusted screen which can combine coldend and ocs2 data
    u8g2.clearBuffer();
    // OCS2 connection status logo
    if (PROTOCOL::hasOCS2Functions) {
        if (PROTOCOL::isOCS2Connected()) {
            u8g2.drawXBM(0, 0, ocs2_ok_width, ocs2_ok_height, (uint8_t *) ocs2_ok_bits);
        } else {
            u8g2.drawXBM(0, 0, ocs2_nok_width, ocs2_nok_height, (uint8_t *) ocs2_nok_bits);
        }
    }
    // ColdEnd connection status logo
    if (PROTOCOL::hasColdEndFunctions) {
        if (PROTOCOL::isColdEndConnected()) {
            u8g2.drawXBM(128 - coldend_ok_width, 0, coldend_ok_width, coldend_ok_height, (uint8_t *) coldend_ok_bits);
        } else {
            u8g2.drawXBM(128 - coldend_nok_width, 0, coldend_nok_width, coldend_nok_height, (uint8_t *) coldend_nok_bits);
        }
    }
    // Draw temperatures
    if (PROTOCOL::isOCS2Connected()) {
        // Only one temperature
        if (dataToClient.temperatures[1] == 0) {
            // First temperature
            u8g2.setFont(FONT_B18PX);
            u8g2.setCursor(50, 18);
            u8g2.print(u8x8_u16toa(dataToClient.temperatures[0], 2));
            u8g2.drawCircle(81, 3, 3);
        } else {
            // First temperature
            u8g2.setFont(FONT_B12PX);
            u8g2.setCursor(35, 14);
            u8g2.print(u8x8_u16toa(dataToClient.temperatures[0], 2));
            u8g2.drawCircle(57, 3, 3);
            // Second temperature
            u8g2.setCursor(68, 14);
            u8g2.print(u8x8_u16toa(dataToClient.temperatures[1], 2));
            u8g2.drawCircle(90, 3, 3);
        }
    }

    // Draw alarm state if it is triggered
    if (PROTOCOL::isOCS2Connected() && dataToClient.alarmState) {
        u8g2.setFont(FONT_B24PX);
        u8g2.drawStr(5, 45, "ALARM");
        u8g2.setFont(FONT_B12PX);
        u8g2.drawStr(30, 60, "triggered");
        u8g2.sendBuffer();
        return;
    }

    // Spindle status if connected
    if (PROTOCOL::isOCS2Connected()) {
        u8g2.setFont(FONT_B24PX);
        uint8_t spaceUsed = u8g2.drawStr(5, 53, "S");
        // Spindle is running
        if (dataToClient.spindelState) {
            // Set correct animation state
            if (millis() - this->lastAnimationStateChange > 200) {
                this->lastAnimationStateChange = millis();
                this->spindleRunningAnimationState++;
            }
            this->drawCircleAnimation(spaceUsed / 2 + 5, 53 - 12, 17, this->spindleRunningAnimationState % 4);
        } else {
            u8g2.drawFrame(2, 24, spaceUsed + 5, 34);
        }
    }

    // ColdEnd status if connected
    if (PROTOCOL::isColdEndConnected()) {
        // Adjust size of mist value to the screen
        if (dataFromColdEnd.mist_val > 99) {
            u8g2.setFont(FONT_B18PX);
            u8g2.setCursor(35, 45);
            u8g2.print(dataFromColdEnd.mist_val, 0);
        } else if (dataFromColdEnd.mist_val > 9.5) {
            u8g2.setFont(FONT_B24PX);
            u8g2.setCursor(35, 45);
            u8g2.print(dataFromColdEnd.mist_val, 0);
        } else {
            u8g2.setFont(FONT_B24PX);
            u8g2.setCursor(35, 45);
            u8g2.print(dataFromColdEnd.mist_val, 1);
        }
        u8g2.setFont(FONT_B8PX);
        u8g2.print(" rpm");
        u8g2.setFont(FONT_B24PX);
        u8g2.setCursor(105, 45);
        u8g2.print(dataFromColdEnd.spit_val, 0);
        u8g2.setFont(FONT_B8PX);
        u8g2.print("s");
        u8g2.setFont(FONT_B12PX);
        if (dataFromColdEnd.spitMode) {
            u8g2.setDrawColor(1);
            u8g2.drawBox(40, 48, 40, 16);
            u8g2.setDrawColor(0);
            u8g2.drawStr(45, 62, "Spit");
        } else if (dataFromColdEnd.coolantValve) {
            u8g2.setDrawColor(1);
            u8g2.drawBox(32, 48, 65, 16);
            u8g2.setDrawColor(0);
            u8g2.drawStr(34, 62, "Coolant");
        }
        if (dataFromColdEnd.airValve) {
            u8g2.setDrawColor(1);
            u8g2.drawBox(100, 48, 38, 16);
            u8g2.setDrawColor(0);
            u8g2.drawStr(104, 62, "Air");
        }
        u8g2.setDrawColor(1);
    }

    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showDefaultOCS2OnlyScreen() {
    u8g2.clearBuffer();
    bool connected = PROTOCOL::isOCS2Connected();
    // Show banned status
    if (dataToClient.peerIgnored) {
        u8g2.setFont(FONT_B10PX);
        u8g2.drawStr(0, 48, "Fehler: Peer ");
        u8g2.drawStr(0, 60, "gebannt - restart");
    }
    // Connection status
    else if (!connected) {
        u8g2.setFont(FONT_B12PX);
        u8g2.drawStr(0, 60, "No Connection");
    }
    // Spindle and alarm status if connected
    if (connected) {
        u8g2.setFont(FONT_B24PX);
        uint8_t spaceUsed;
        spaceUsed = u8g2.drawStr(0, 60, "S:");
        if (dataToClient.spindelState) {
            u8g2.drawXBM(spaceUsed + 3, 35, ON_LOGO_width, ON_LOGO_height, (uint8_t *) ON_LOGO_bits);
        } else {
            u8g2.drawXBM(spaceUsed + 3, 35, OFF_LOGO_width, OFF_LOGO_height, (uint8_t *) OFF_LOGO_bits);
        }
        spaceUsed = u8g2.drawStr(65, 60, "A:");
        if (dataToClient.alarmState) {
            u8g2.drawXBM(65 + spaceUsed + 3, 35, ALARM_LOGO_width, ALARM_LOGO_height, (uint8_t *) ALARM_LOGO_bits);
        } else {
            u8g2.drawXBM(65 + spaceUsed + 3, 35, ON_LOGO_width, ON_LOGO_height, (uint8_t *) ON_LOGO_bits);
        }
    }
    // Draw temperatures
    if (connected) {
        // First temperature
        u8g2.setFont(FONT_B18PX);
        u8g2.setCursor(0, 20);
        u8g2.print(u8x8_u16toa(dataToClient.temperatures[0], 2));
        u8g2.drawCircle(30, 3, 3);
        // Second temperature if it is set
        if (dataToClient.temperatures[1] != 0) {
            u8g2.setCursor(94, 20);
            u8g2.print(u8x8_u16toa(dataToClient.temperatures[1], 2));
            u8g2.drawCircle(124, 3, 3);
        }
    }
    // Show joystick state if it is connected
    if (ioConfig.hasFunction(InputFunctions::func_joystickX) && ioConfig.hasFunction(InputFunctions::func_joystickY) &&
        ioConfig.hasFunction(InputFunctions::func_joystickZ)) {
        this->addJoystickStates();
    }

    // Show feedrate if it is connected
    if (ioConfig.hasFunction(InputFunctions::func_feedrate)) {
        this->addFeedrateState();
    }
    // Show rotation speed if it is connected
    if (ioConfig.hasFunction(InputFunctions::func_rotationSpeed)) {
        this->addRotationSpeedState();
    }

    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showDefaultColdendOnlyScreen() {
    float mist_val      = dataFromColdEnd.mist_val;
    float spit_val      = dataFromColdEnd.spit_val;
    bool  coolant_valve = dataFromColdEnd.coolantValve;
    bool  air_valve     = dataFromColdEnd.airValve;
    bool  spit_mode     = dataFromColdEnd.spitMode;

    uint8_t cursor_pos = 0;
    uint8_t digits     = 0;

    u8g2.clearBuffer();

    if (mist_val < 10) {
        digits     = 1;    // If value < 10, display as float with one decimal place
        cursor_pos = 50;   // Set cursor depending on string length
    } else if (mist_val < 100) {
        digits     = 0;    // Display rounded to int
        cursor_pos = 41;
    } else {
        digits     = 0;
        cursor_pos = 60;
    }
    u8g2.clearBuffer();
    u8g2.setFontMode(0);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_lucasfont_alternate_tr);
    u8g2.drawStr(0, 7, "Coolant");
    u8g2.drawStr(101, 7, "Spit");
    u8g2.drawLine(0, 15, 128, 15);
    u8g2.setFont(u8g2_font_fur25_tf);
    u8g2.setCursor(0, 43);
    u8g2.print(mist_val, digits);
    u8g2.setFont(u8g2_font_lucasfont_alternate_tr);
    u8g2.drawStr(cursor_pos, 43, "rpm");
    u8g2.setFont(u8g2_font_fur25_tf);
    u8g2.setCursor(100, 43);
    u8g2.print(spit_val, 0);
    u8g2.setFont(u8g2_font_lucasfont_alternate_tr);
    u8g2.drawStr(122, 43, "s");
    u8g2.drawLine(0, 48, 128, 48);
    if (spit_mode == true) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, 53, 55, 11);
        u8g2.setDrawColor(0);
        u8g2.drawStr(3, 62, "Spit Mode");
    } else if (coolant_valve == true) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, 53, 60, 11);
        u8g2.setDrawColor(0);
        u8g2.drawStr(3, 62, "Coolant On");
    }
    if (air_valve == true) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(93, 53, 39, 11);
        u8g2.setDrawColor(0);
        u8g2.drawStr(96, 62, "Air On");
    }

    // Connection status
    u8g2.setDrawColor(1);
    if (PROTOCOL::isColdEndConnected()) {
        u8g2.drawXBM(55, -1, coldend_ok_width, coldend_ok_height, (uint8_t *) coldend_ok_bits);
    } else {
        u8g2.drawXBM(55, -1, coldend_nok_width, coldend_nok_height, (uint8_t *) coldend_nok_bits);
    }

    u8g2.sendBuffer();
}

void DISPLAYMANAGER::addRotationSpeedState() {
    // DPRINTLN("Rotation Speed: " + String(dataToControl.rotationSpeed));
    byte startX = 64, startY = 26, width = 4, maxLength = 60;
    byte length = map(dataToControl.rotationSpeed, 0, 1023, 0, maxLength);
    u8g2.drawBox(startX, startY, length, width);
    u8g2.drawFrame(startX + length, startY, maxLength - length, width);
}

void DISPLAYMANAGER::addFeedrateState() {
    // DPRINTLN("Feedrate: " + String(dataToControl.feedrate));
    byte startX = 0, startY = 26, width = 4, maxLength = 60;
    byte length = map(dataToControl.feedrate, 0, 1023, 0, maxLength);
    u8g2.drawBox(startX, startY, length, width);
    u8g2.drawFrame(startX + length, startY, maxLength - length, width);
}

void DISPLAYMANAGER::addJoystickStates() {
    byte middleX   = 60;
    byte length    = 10;
    byte width     = 6;
    byte padding   = 2;
    byte threshold = 200;
    // DPRINTLN("Joystick X: " + String(dataToControl.joystickX) + " Y: " + String(dataToControl.joystickY) + " Z: " +
    // String(dataToControl.joystickZ)); top
    byte lengthYTop = (dataToControl.joystickY > 512) ? map(dataToControl.joystickY, 512, 1023, 0, length) : 0;
    u8g2.drawFrame(middleX - width / 2, 0, width, length - lengthYTop);
    u8g2.drawBox(middleX - width / 2, length - lengthYTop, width, lengthYTop);
    // bottom
    byte lengthYBottom = (dataToControl.joystickY <= 512) ? map(1023 - dataToControl.joystickY, 512, 1023, 0, length) : 0;
    u8g2.drawBox(middleX - width / 2, length + padding, width, lengthYBottom);
    u8g2.drawFrame(middleX - width / 2, length + padding + lengthYBottom, width, length - lengthYBottom);
    // left
    byte lengthXLeft = (dataToControl.joystickX <= 512) ? map(1023 - dataToControl.joystickX, 512, 1023, 0, length) : 0;
    u8g2.drawFrame(middleX - width / 2 - length - padding, length - width / 2, length - lengthXLeft, width);
    u8g2.drawBox(middleX - width / 2 - length - padding + length - lengthXLeft, length - width / 2, lengthXLeft, width);
    // right
    byte lengthXRight = (dataToControl.joystickX > 512) ? map(dataToControl.joystickX, 512, 1023, 0, length) : 0;
    u8g2.drawBox(middleX + width / 2 + padding, length - width / 2, lengthXRight, width);
    u8g2.drawFrame(middleX + width / 2 + padding + lengthXRight, length - width / 2, length - lengthXRight, width);
    // Axis z top
    byte lengthZTop = (dataToControl.joystickZ > 512) ? map(dataToControl.joystickZ, 512, 1023, 0, length) : 0;
    u8g2.drawFrame(middleX + width / 2 + padding + length + padding, 0, width, length - lengthZTop);
    u8g2.drawBox(middleX + width / 2 + padding + length + padding, length - lengthZTop, width, lengthZTop);
    // Axis z bottom
    byte lengthZBottom = (dataToControl.joystickZ <= 512) ? map(1023 - dataToControl.joystickZ, 512, 1023, 0, length) : 0;
    u8g2.drawBox(middleX + width / 2 + padding + length + padding, length + padding, width, lengthZBottom);
    u8g2.drawFrame(middleX + width / 2 + padding + length + padding, length + padding + lengthZBottom, width, length - lengthZBottom);
}

void DISPLAYMANAGER::addCalibrationHeadline() {
    u8g2.setFont(FONT_B10PX);
    u8g2.drawStr(0, 12, "Calibration");
    if (this->screenDuration != 0) {
        u8g2.setCursor(96, 14);
        int timeLeft = (this->screenDuration - (millis() - this->screenStarted));
        timeLeft     = (timeLeft < 0) ? 0 : timeLeft;
        u8g2.print(u8x8_u16toa(timeLeft / 1000, 2));
        u8g2.print("s");
    }
    u8g2.drawLine(0, 16, 128, 16);
}

void DISPLAYMANAGER::showClibrationJoystickMiddleScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "joystick in");
    u8g2.drawStr(0, 52, "middle position");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showClibrationJoystickXScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "joystick to");
    u8g2.drawStr(0, 52, "the left");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showClibrationJoystickYScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "joystick to");
    u8g2.drawStr(0, 52, "the bottom");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showClibrationJoystickZScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "joystick turn");
    u8g2.drawStr(0, 52, "counter clock");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showCalibrationFeedrateScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "feedrate to ");
    u8g2.drawStr(0, 52, "off position");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showCalibrationRotationSpeedScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "rotation speed");
    u8g2.drawStr(0, 52, "to off position");
    u8g2.sendBuffer();
}
void DISPLAYMANAGER::showCalibrationColdendMistScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "ColdEnd Mist");
    u8g2.drawStr(0, 52, "to off position");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showCalibrationColdendSpitScreen() {
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_B12PX);
    u8g2.drawStr(0, 34, "ColdEnd Spit");
    u8g2.drawStr(0, 52, "to off position");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showWaitScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(FONT_B24PX);
    u8g2.drawStr(0, 45, "Wait..");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showAutoSquareScreen() {
    // Set correct animation state
    if (millis() - this->lastAnimationStateChange > 200) {
        this->lastAnimationStateChange = millis();
        this->autosquareRunningAnimationState++;
    }
    // Set display
    u8g2.clearBuffer();
    u8g2.setFont(FONT_B18PX);
    byte fontSize = 18;

    if (dataToClient.autosquareState[0].axisActive) {
        u8g2.drawStr(0, 17, "Axis1:");
        this->drawAutosquareMotorState(80, 17 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[0].axisMotor1State);
        this->drawAutosquareMotorState(115, 17 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[0].axisMotor2State);
    }
    if (dataToClient.autosquareState[1].axisActive) {
        u8g2.drawStr(0, 40, "Axis2:");
        this->drawAutosquareMotorState(80, 38 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[1].axisMotor1State);
        this->drawAutosquareMotorState(115, 38 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[1].axisMotor2State);
    }

    if (dataToClient.autosquareState[2].axisActive) {
        u8g2.drawStr(0, 63, "Axis3:");
        this->drawAutosquareMotorState(80, 61 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[2].axisMotor1State);
        this->drawAutosquareMotorState(115, 61 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[2].axisMotor2State);
    }

    u8g2.sendBuffer();
}

void DISPLAYMANAGER::drawAutosquareMotorState(uint8_t posX, uint8_t posY, AS_STATES state) {
    switch (state) {
    case AS_STATES::none:
        this->drawCircleAnimation(posX, posY, 17 / 2, this->autosquareRunningAnimationState % 4);
        break;
    case AS_STATES::squared:
        this->drawCircleAnimation(posX, posY, 17 / 2, 5);   // Draw a complete circle
        break;
    case AS_STATES::finish:
        u8g2.drawXBM(posX - 17 / 2, posY - 17 / 2, ON17_LOGO_width, ON17_LOGO_height, (uint8_t *) ON17_LOGO_bits);
        break;
    }
}

void DISPLAYMANAGER::drawCircleAnimation(uint8_t posX, uint8_t posY, uint8_t radius, uint8_t state) {
    switch (state) {
    case 0:
        u8g2.drawCircle(posX, posY, radius, U8G2_DRAW_UPPER_RIGHT);
        break;
    case 1:
        u8g2.drawCircle(posX, posY, radius, U8G2_DRAW_LOWER_RIGHT);
        break;
    case 2:
        u8g2.drawCircle(posX, posY, radius, U8G2_DRAW_LOWER_LEFT);
        break;
    case 3:
        u8g2.drawCircle(posX, posY, radius, U8G2_DRAW_UPPER_LEFT);
        break;
    default:
        u8g2.drawCircle(posX, posY, radius, U8G2_DRAW_ALL);
    }
}

char *DISPLAYMANAGER::getMacStrFromAddress(uint8_t *address) {
    static char macStr[18];
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4],
             address[5]);
    return macStr;
}

DISPLAYMANAGER display;