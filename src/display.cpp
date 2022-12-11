#include <includes.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);

extern uint8_t controllerMacAddress[];

#define FONT_10PX u8g2_font_crox2hb_tf
#define FONT_12PX u8g2_font_crox3hb_tf
#define FONT_17PX u8g2_font_utopia24_tr
#define FONT_24PX u8g2_font_helvB24_tf

void DISPLAYMANAGER::setup()
{
    u8g2.begin();

    // Create a task for the Display
    xTaskCreatePinnedToCore(
        DISPLAYMANAGER::displayTask, /* Task function. */
        "Display Task",              /* name of task. */
        10000,                       /* Stack size of task */
        this,                        /* parameter of the task */
        1,                           /* priority of the task */
        &displayTaskHandle,          /* Task handle to keep track of created task */
        0);
}

void DISPLAYMANAGER::displayTask(void *pvParameters)
{
    auto *display = (DISPLAYMANAGER *)pvParameters;
    for (;;)
    {
        if (!display->splashFinished)
        {
            u8g2.clearBuffer();
            u8g2.drawXBM(0, 0, OCS2_LOGO_width, OCS2_LOGO_height, (uint8_t *)OCS2_LOGO_bits);
            u8g2.sendBuffer();
            vTaskDelay(1000);
            display->splashFinished = true;
        }
        if (dataToClient.autosquareRunning)
        {
            display->showAutoSquareScreen();
        }
        else
        {
            switch (display->currentScreen)
            {
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
            case DEFAULT_SCREEN:
                display->showDefaultScreen();
                break;
            case WAIT_SCREEN:
                display->showWaitScreen();
                break;
            case CALIBRATION_SCREEN:
                display->showCalibrationScreen();
                break;
            case RESET_SCREEN:
                display->showResetScreen();
                break;
            case SETTINGS1_SCREEN:
                display->showSettings1Screen();
                break;
            case SETTINGS2_SCREEN:
                display->showSettings2Screen();
                break;
            case SETTINGS3_SCREEN:
                display->showSettings3Screen();
                break;
            case SETTINGS4_SCREEN:
                display->showSettings4Screen();
                break;
            case SETTINGS5_SCREEN:
                display->showSettings5Screen();
                break;
            }
        }

        vTaskDelay(20);
    }
}

void DISPLAYMANAGER::setScreen(DISPLAY_SCREENS screen, uint32_t duration_MS)
{
    this->currentScreen = screen;
    this->screenStarted = millis();
    this->screenDuration = duration_MS;
}

void DISPLAYMANAGER::showDefaultScreen()
{
    u8g2.clearBuffer();
    bool connected = PROTOCOL::isConnected();
    // Show banned status
    if (dataToClient.peerIgnored) {
        u8g2.setFont(FONT_10PX);
        u8g2.drawStr(0, 48, "Fehler: Peer ");
        u8g2.drawStr(0, 60, "gebannt - restart");
    }
    // Connection status
    else if (!connected)
    {
        u8g2.setFont(FONT_12PX);
        u8g2.drawStr(0, 60, "No Connection");
    }
    // Spindle and alarm status if connected
    if (connected)
    {
        u8g2.setFont(FONT_24PX);
        uint8_t spaceUsed;
        spaceUsed = u8g2.drawStr(0, 60, "S:");
        if (dataToClient.spindelState)
        {
            u8g2.drawXBM(spaceUsed + 3, 35, ON_LOGO_width, ON_LOGO_height, (uint8_t *)ON_LOGO_bits);
        }
        else
        {
            u8g2.drawXBM(spaceUsed + 3, 35, OFF_LOGO_width, OFF_LOGO_height, (uint8_t *)OFF_LOGO_bits);
        }
        spaceUsed = u8g2.drawStr(65, 60, "A:");
        if (dataToClient.alarmState)
        {
            u8g2.drawXBM(65 + spaceUsed + 3, 35, ALARM_LOGO_width, ALARM_LOGO_height, (uint8_t *)ALARM_LOGO_bits);
        }
        else
        {
            u8g2.drawXBM(65 + spaceUsed + 3, 35, ON_LOGO_width, ON_LOGO_height, (uint8_t *)ON_LOGO_bits);
        }
    }
    // Draw temperatures
    if (connected)
    {
        // First temperature
        u8g2.setFont(FONT_17PX);
        u8g2.setCursor(0, 20);
        u8g2.print(u8x8_u16toa(dataToClient.temperatures[0], 2));
        u8g2.drawCircle(30, 3, 3);
        // Second temperature if it is set
        if (dataToClient.temperatures[1] != 0)
        {
            u8g2.setCursor(94, 20);
            u8g2.print(u8x8_u16toa(dataToClient.temperatures[1], 2));
            u8g2.drawCircle(124, 3, 3);
        }
    }
// Show joystick state if it is connected
#if HAS_JOYSTICK
    this->addJoystickStates();
#endif
// Show feedrate state if it is connected
#if HAS_FEEDRATE_POTI
    this->addFeedrateState();
#endif
// Show rotation speed if it is connected
#if HAS_ROTATION_SPEED_POTI
    this->addRotationSpeedState();
#endif

    u8g2.sendBuffer();
}

void DISPLAYMANAGER::addRotationSpeedState()
{
    // DPRINTLN("Rotation Speed: " + String(dataToControl.rotationSpeed));
    byte startX = 64, startY = 26, width = 4, maxLength = 60;
    byte length = map(dataToControl.rotationSpeed, 0, 1023, 0, maxLength);
    u8g2.drawBox(startX, startY, length, width);
    u8g2.drawFrame(startX + length, startY, maxLength - length, width);
}

void DISPLAYMANAGER::addFeedrateState()
{
    // DPRINTLN("Feedrate: " + String(dataToControl.feedrate));
    byte startX = 0, startY = 26, width = 4, maxLength = 60;
    byte length = map(dataToControl.feedrate, 0, 1023, 0, maxLength);
    u8g2.drawBox(startX, startY, length, width);
    u8g2.drawFrame(startX + length, startY, maxLength - length, width);
}

void DISPLAYMANAGER::addJoystickStates()
{
    byte middleX = 60;
    byte length = 10;
    byte width = 6;
    byte padding = 2;
    byte threshold = 200;
    // DPRINTLN("Joystick X: " + String(dataToControl.joystickX) + " Y: " + String(dataToControl.joystickY) + " Z: " + String(dataToControl.joystickZ));
    // top
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

void DISPLAYMANAGER::addCalibrationHeadline()
{
    u8g2.setFont(FONT_10PX);
    u8g2.drawStr(0, 12, "Kalibrierung");
    if (this->screenDuration != 0)
    {
        u8g2.setCursor(96, 14);
        int timeLeft = (this->screenDuration - (millis() - this->screenStarted));
        timeLeft = (timeLeft < 0) ? 0 : timeLeft;
        u8g2.print(u8x8_u16toa(timeLeft / 1000, 2));
        u8g2.print("s");
    }
    u8g2.drawLine(0, 16, 128, 16);
}

void DISPLAYMANAGER::showClibrationJoystickMiddleScreen()
{
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_12PX);
    u8g2.drawStr(0, 34, "Joystick in");
    u8g2.drawStr(0, 52, "Mittelposition");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showClibrationJoystickXScreen()
{
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_12PX);
    u8g2.drawStr(0, 34, "Joystick");
    u8g2.drawStr(0, 52, "nach links");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showClibrationJoystickYScreen()
{
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_12PX);
    u8g2.drawStr(0, 34, "Joystick");
    u8g2.drawStr(0, 52, "nach unten");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showClibrationJoystickZScreen()
{
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_12PX);
    u8g2.drawStr(0, 34, "Joystick gegen");
    u8g2.drawStr(0, 52, "Uhrzeigersinn");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showCalibrationFeedrateScreen()
{
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_12PX);
    u8g2.drawStr(0, 34, "Feedrate in ");
    u8g2.drawStr(0, 52, "Aus-Position");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showCalibrationRotationSpeedScreen()
{
    u8g2.clearBuffer();
    this->addCalibrationHeadline();
    u8g2.setFont(FONT_12PX);
    u8g2.drawStr(0, 34, "Rotation Speed");
    u8g2.drawStr(0, 52, "in Aus-Position");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showWaitScreen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_24PX);
    u8g2.drawStr(0, 45, "Warten..");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showCalibrationScreen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_10PX);
    u8g2.drawStr(0, 20, "Kalibrierung");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showResetScreen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_12PX);
    u8g2.drawStr(0, 20, "Reset");
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showSettings1Screen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_10PX);
    u8g2.drawStr(0, 12, "SW-Version:");
    u8g2.setCursor(90, 12);
    u8g2.print(OCS_PANEL_SOFTWARE_VERSION);
    u8g2.drawStr(0, 26, "Wifi delay:");
    u8g2.setCursor(90, 26);
    u8g2.print(WIFI_DELAY);
    u8g2.drawStr(0, 40, "Controller Mac:");
    u8g2.drawStr(0, 54, getMacStrFromAddress(controllerMacAddress));
    u8g2.sendBuffer();
}
void DISPLAYMANAGER::showSettings2Screen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_10PX);
    u8g2.drawStr(0, 12, "Joystick:");
    u8g2.setCursor(90, 12);
    (HAS_JOYSTICK) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 26, "Feedrate:");
    u8g2.setCursor(90, 26);
    (HAS_FEEDRATE_POTI) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 40, "Rot. Speed:");
    u8g2.setCursor(90, 40);
    (HAS_ROTATION_SPEED_POTI) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 54, "OK Button:");
    u8g2.setCursor(90, 54);
    (HAS_OK_BUTTON) ? u8g2.print("on") : u8g2.print("off");
    u8g2.sendBuffer();
}
void DISPLAYMANAGER::showSettings3Screen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_10PX);
    u8g2.drawStr(0, 12, "Autosquare:");
    u8g2.setCursor(90, 12);
    (HAS_AUTOSQUARE_BUTTON) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 26, "Motor Start:");
    u8g2.setCursor(90, 26);
    (HAS_MOTOR_START_BUTTON) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 40, "Progr. Start:");
    u8g2.setCursor(90, 40);
    (HAS_PROGRAMM_START_BUTTON) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 54, "Axis Select:");
    u8g2.setCursor(90, 54);
    (HAS_AXIS_SELECT_BUTTONS) ? u8g2.print("on") : u8g2.print("off");
    u8g2.sendBuffer();
}
void DISPLAYMANAGER::showSettings4Screen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_10PX);
    u8g2.drawStr(0, 12, "ENA Button:");
    u8g2.setCursor(90, 12);
    (HAS_ENA_BUTTON) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 26, "Speed1:");
    u8g2.setCursor(90, 26);
    (HAS_SPEED1_BUTTON) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 40, "Speed2:");
    u8g2.setCursor(90, 40);
    (HAS_SPEED2_BUTTON) ? u8g2.print("on") : u8g2.print("off");
    u8g2.drawStr(0, 54, "Button1:");
    u8g2.setCursor(60, 54);
    u8g2.print(BUTTON_1);
    u8g2.sendBuffer();
}
void DISPLAYMANAGER::showSettings5Screen()
{
    u8g2.clearBuffer();
    u8g2.setFont(FONT_10PX);
    u8g2.drawStr(0, 12, "Button2:");
    u8g2.setCursor(60, 12);
    u8g2.print(BUTTON_2);
    u8g2.drawStr(0, 26, "Button3:");
    u8g2.setCursor(60, 26);
    u8g2.print(BUTTON_3);
    u8g2.drawStr(0, 40, "Button4:");
    u8g2.setCursor(60, 40);
    u8g2.print(BUTTON_4);
    u8g2.sendBuffer();
}

void DISPLAYMANAGER::showAutoSquareScreen()
{
    // Set correct animation state
    if (millis() - this->lastAnimationStateChange > 200)
    {
        this->lastAnimationStateChange = millis();
        this->autosquareRunningAnimationState++;
        if (this->autosquareRunningAnimationState > 3)
        {
            this->autosquareRunningAnimationState = 0;
        }
    }
    // Set display
    u8g2.clearBuffer();
    u8g2.setFont(FONT_17PX);
    byte fontSize = 17;

    if (dataToClient.autosquareState[0].axisActive)
    {
        u8g2.drawStr(0, 17, "Axis1:");
        this->drawAutosquareMotorState(80, 17 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[0].axisMotor1State);
        this->drawAutosquareMotorState(115, 17 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[0].axisMotor2State);
    }
    if (dataToClient.autosquareState[1].axisActive)
    {
        u8g2.drawStr(0, 40, "Axis2:");
        this->drawAutosquareMotorState(80, 38 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[1].axisMotor1State);
        this->drawAutosquareMotorState(115, 38 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[1].axisMotor2State);
    }

    if (dataToClient.autosquareState[2].axisActive)
    {
        u8g2.drawStr(0, 63, "Axis3:");
        this->drawAutosquareMotorState(80, 61 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[2].axisMotor1State);
        this->drawAutosquareMotorState(115, 61 - fontSize / 2, (AS_STATES) dataToClient.autosquareState[2].axisMotor2State);
    }

    u8g2.sendBuffer();
}

void DISPLAYMANAGER::drawAutosquareMotorState(uint8_t posX, uint8_t posY, AS_STATES state)
{
    switch (state)
    {
    case AS_STATES::none:
        this->drawCircleAnimation(posX, posY, 17 / 2, this->autosquareRunningAnimationState);
        break;
    case AS_STATES::squared:
        this->drawCircleAnimation(posX, posY, 17 / 2, 5); // Draw a complete circle
        break;
    case AS_STATES::finish:
        u8g2.drawXBM(posX-17/2, posY-17/2, ON17_LOGO_width, ON17_LOGO_height, (uint8_t *)ON17_LOGO_bits);
        break;
    }
}

void DISPLAYMANAGER::drawCircleAnimation(uint8_t posX, uint8_t posY, uint8_t radius, uint8_t state)
{
    switch (state)
    {
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

char *DISPLAYMANAGER::getMacStrFromAddress(uint8_t *address)
{
    static char macStr[18];
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);
    return macStr;
}

DISPLAYMANAGER display;