#include <includes.h>

// Preferences espui_preferences;

const char *hostname = "ESP32-Panel";

// Ui handles
uint16_t uiHandles[16], displayMode, communicationMode;
// Analog inputs
uint16_t firstElement, sliderXAxis, sliderYAxis, sliderZAxis, sliderFeedrate, sliderRotationSpeed, sliderColdEndMist, sliderColdEndSpit;
// Tabs
uint16_t editConfigurationTab, showConfigurationTab, statusTab, calibrationTab;
// status elements
uint16_t statusOCS2, statusColdEnd, statusInputMapping, statusCalibrationConfig, statusOCS2Data, statusColdEndData;
// Buttons
uint16_t firstButton, okButton, autosquareButton, motorStartButton, programmStartButton, axisXSelectButton, axisYSelectButton,
    axisZSelectButton, enaButton, speed1Button, speed2Button, output1Button, output2Button, output3Button, output4Button, coldEndMistButton,
    coldEndAirButton, coldEndFastButton;

String emeraldColor  = "background-color: #2FCC71;";
String alizarinColor = "background-color: #E74C3C;";

static String axeMapping[]               = {"X", "Y", "Z"};
static String invertAxisMapping[]        = {"no", "yes"};
static String autosquareStateMapping[]   = {"unknown", "squared", "finished", "not active"};
static String displayModeMapping[]       = {"Default", "OCS2 only", "ColdEnd only"};
static String communicationModeMapping[] = {"automatic(serial first)", "only serial", "only wifi"};

void emptyCallback(Control *sender, int type) {
    Serial.print("CB: id(");
    Serial.print(sender->id);
    Serial.print(") Type(");
    Serial.print(type);
    Serial.print(") '");
    Serial.print(sender->label);
    Serial.print("' = ");
    Serial.println(sender->value);
}

void displayModeCallback(Control *sender, int type) {
    // map value to a function
    byte displayModeIndex = 99;
    for (byte i = 0; i < 3; i++) {
        if (sender->value == displayModeMapping[i]) {
            displayModeIndex = i;
            break;
        }
    }
    mainConfig.displayMode = (DisplayMode) displayModeIndex;
}

void communicationModeCallback(Control *sender, int type) {
    // map value to a function
    byte index = 99;
    for (byte i = 0; i < 3; i++) {
        if (sender->value == communicationModeMapping[i]) {
            index = i;
            break;
        }
    }
    mainConfig.communicationMode = (CommunicationMode) index;
}

void wifiModeCallback(Control *sender, int type) { mainConfig.wifiDefaultOn = sender->value == "1" ? true : false; }

void saveAndRestartCallback(Control *sender, int type) {
    configManager.saveMainConfig();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    ESP.restart();
}

void resetAndRestartCallback(Control *sender, int type) {
    configManager.resetMainConfig();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    ESP.restart();
}

void startCalibrationCallback(Control *sender, int type) { ioControl.startCalibration(); }

void extendedCallback(Control *sender, int type, void *param) {
    // map value to a function
    byte functionIndex = 99;
    for (byte i = 0; i < 26; i++) {
        if (sender->value == allInputFunctions[i]) {
            functionIndex = i;
            break;
        }
    }

    mainConfig.inputs[(int) param] = (InputFunctions) functionIndex;
}

const byte DNS_PORT = 53;
IPAddress  apIP(192, 168, 4, 1);
DNSServer  dnsServer;

void UIHANDLER::setup() {
    WiFi.mode(WIFI_AP_STA);
    // ESPUI.setVerbosity(Verbosity::VerboseJSON);
    WiFi.setHostname(hostname);
    Serial.print("Creating hotspot");
    delay(100);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8) {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    char ssid[30];
    snprintf(ssid, 31, "ESP32-Panel-%06X", chipId);
    WiFi.softAP(ssid, NULL, 1);

    dnsServer.start(DNS_PORT, "*", apIP);

    DPRINTLN("Use your phone or computer to connect to the Wi-Fi network " + String(ssid));
    DPRINTLN("Then open http://" + WiFi.softAPIP().toString() + " in your browser");
    DPRINTLN("Go to the configuration tab, adjust to your needs and click on save&restart");
    DPRINTLN("----------------------------------------");

    statusTab = ESPUI.addControl(Tab, "", "Status");

    // OCS2 connection status and information if connected
    statusOCS2     = ESPUI.addControl(Label, "OCS2 connection", "unconnected", Dark, statusTab);
    statusOCS2Data = ESPUI.addControl(Label, "OCS2 Data", "empty", Dark, statusTab);
    ESPUI.updateVisibility(statusOCS2Data, false);

    // ColdEnd connection status and information if connected
    statusColdEnd     = ESPUI.addControl(Label, "ColdEnd32 connection", "unconnected", Dark, statusTab);
    statusColdEndData = ESPUI.addControl(Label, "ColdEnd Data", "empty", Dark, statusTab);
    ESPUI.updateVisibility(statusColdEndData, false);

    // Analog inputs
    String clearLabelStyle = "background-color: unset; width: 100%;";
    firstElement           = ESPUI.addControl(Slider, "Analog Inputs", "-", Turquoise, statusTab, &emptyCallback);
    if (dataToControl.command.setJoystick) {
        sliderXAxis = ESPUI.addControl(Slider, "", "0", None, firstElement, &emptyCallback);
        ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Joystick X-Axis", None, firstElement), clearLabelStyle);
        sliderYAxis = ESPUI.addControl(Slider, "", "0", None, firstElement, &emptyCallback);
        ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Joystick Y-Axis", None, firstElement), clearLabelStyle);
        sliderZAxis = ESPUI.addControl(Slider, "", "0", None, firstElement, &emptyCallback);
        ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Joystick Z-Axis", None, firstElement), clearLabelStyle);
        ESPUI.setEnabled(sliderXAxis, false);
        ESPUI.setEnabled(sliderYAxis, false);
        ESPUI.setEnabled(sliderZAxis, false);
    }
    if (dataToControl.command.setFeedrate) {
        sliderFeedrate = ESPUI.addControl(Slider, "", "0", None, firstElement, &emptyCallback);
        ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Feedrate Poti", None, firstElement), clearLabelStyle);
        ESPUI.setEnabled(sliderFeedrate, false);
    }
    if (dataToControl.command.setRotationSpeed) {
        sliderRotationSpeed = ESPUI.addControl(Slider, "", "0", None, firstElement, &emptyCallback);
        ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Rotation Speed Poti", None, firstElement), clearLabelStyle);
        ESPUI.setEnabled(sliderRotationSpeed, false);
    }
    if (dataToColdEnd.command.setPotMist) {
        sliderColdEndMist = ESPUI.addControl(Slider, "", "0", None, firstElement, &emptyCallback);
        ESPUI.setElementStyle(ESPUI.addControl(Label, "", "ColdEnd Mist", None, firstElement), clearLabelStyle);
        ESPUI.setEnabled(sliderColdEndMist, false);
    }
    if (dataToColdEnd.command.setPotSpit) {
        sliderColdEndSpit = ESPUI.addControl(Slider, "", "0", None, firstElement, &emptyCallback);
        ESPUI.setElementStyle(ESPUI.addControl(Label, "", "ColdEnd Spit", None, firstElement), clearLabelStyle);
        ESPUI.setEnabled(sliderColdEndSpit, false);
    }

    ESPUI.setElementStyle(firstElement, "display:none;");

    // Digital inputs
    firstButton = ESPUI.addControl(ControlType::Button, "Digital Inputs", "Button A", ControlColor::Turquoise, statusTab, &emptyCallback);
    if (dataToControl.command.setOk) {
        okButton = ESPUI.addControl(ControlType::Button, "", "OK", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(okButton, false);
    }
    if (dataToControl.command.setAutosquare) {
        autosquareButton = ESPUI.addControl(ControlType::Button, "", "Autosquare", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(autosquareButton, false);
    }
    if (dataToControl.command.setMotorStart) {
        motorStartButton = ESPUI.addControl(ControlType::Button, "", "Motor Start", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(motorStartButton, false);
    }
    if (dataToControl.command.setProgramStart) {
        programmStartButton = ESPUI.addControl(ControlType::Button, "", "Program Start", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(programmStartButton, false);
    }
    if (dataToControl.command.setAxisSelect) {
        axisXSelectButton = ESPUI.addControl(ControlType::Button, "", "Axis X Select", ControlColor::None, firstButton, &emptyCallback);
        axisYSelectButton = ESPUI.addControl(ControlType::Button, "", "Axis Y Select", ControlColor::None, firstButton, &emptyCallback);
        axisZSelectButton = ESPUI.addControl(ControlType::Button, "", "Axis Z Select", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(axisXSelectButton, false);
        ESPUI.setEnabled(axisYSelectButton, false);
        ESPUI.setEnabled(axisZSelectButton, false);
    }
    if (dataToControl.command.setEna) {
        enaButton = ESPUI.addControl(ControlType::Button, "", "Ena", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(enaButton, false);
    }
    if (dataToControl.command.setSpeed1) {
        speed1Button = ESPUI.addControl(ControlType::Button, "", "Speed 1", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(speed1Button, false);
    }
    if (dataToControl.command.setSpeed2) {
        speed2Button = ESPUI.addControl(ControlType::Button, "", "Speed 2", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(speed2Button, false);
    }
    if (dataToControl.command.setOutput1) {
        output1Button = ESPUI.addControl(ControlType::Button, "", "Output 1", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(output1Button, false);
    }
    if (dataToControl.command.setOutput2) {
        output2Button = ESPUI.addControl(ControlType::Button, "", "Output 2", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(output2Button, false);
    }
    if (dataToControl.command.setOutput3) {
        output3Button = ESPUI.addControl(ControlType::Button, "", "Output 3", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(output3Button, false);
    }
    if (dataToControl.command.setOutput4) {
        output4Button = ESPUI.addControl(ControlType::Button, "", "Output 4", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(output4Button, false);
    }
    if (dataToColdEnd.command.setInMist) {
        coldEndMistButton = ESPUI.addControl(ControlType::Button, "", "ColdEnd Mist", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(coldEndMistButton, false);
    }
    if (dataToColdEnd.command.setInAir) {
        coldEndAirButton = ESPUI.addControl(ControlType::Button, "", "ColdEnd Air", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(coldEndAirButton, false);
    }
    if (dataToColdEnd.command.setInFast) {
        coldEndFastButton = ESPUI.addControl(ControlType::Button, "", "ColdEnd Fast", ControlColor::None, firstButton, &emptyCallback);
        ESPUI.setEnabled(coldEndFastButton, false);
    }
    ESPUI.setElementStyle(firstButton, "display:none;");

    // Show configuration tab
    showConfigurationTab = ESPUI.addControl(Tab, "", "Show Configuration");
    // clang-format off
    String controlElements = "<table>"
            "<tr style='border:1px solid black'><th>Input</th><th>Function</th></tr>"
            "<tr style='border:1px solid black'><td>Joystick X:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_joystickX]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Joystick Y:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_joystickY]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Joystick Z:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_joystickZ]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Feedrate:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_feedrate]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Rotation Speed:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_rotationSpeed]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>OK:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_okButton]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Autosquare:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_autosquareButton]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Motor Start:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_motorStartButton]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Program Start:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_programStartButton]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Axis X Select:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_axisXSelect]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Axis Y Select:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_axisYSelect]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Axis Z Select:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_axisZSelect]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Button 1:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_button1]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Button 2:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_button2]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Button 3:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_button3]]+"</td></tr>"
            "<tr style='border:1px solid black'><td>Button 4:</td><td>"+ allInputFunctions[mainConfig.inputs[Inputs::in_button4]]+"</td></tr>"
            "</table>";
    // clang-format on
    statusInputMapping = ESPUI.addControl(Label, "Input to function mapping", controlElements, Carrot, showConfigurationTab);

    // Edit configuration tab
    editConfigurationTab = ESPUI.addControl(Tab, "", "Edit Configuration");
    ESPUI.addControl(Separator, "Mapping Inputs to Functions", "", None, editConfigurationTab);
    // Analog Inputs
    ESPUI.addControl(Separator, "Analog Inputs - these can be used for analog functions", "", None, editConfigurationTab);
    uiHandles[Inputs::in_joystickX] =
        ESPUI.addControl(Select, "Map Joystick X Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_joystickX]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_joystickX);
    for (auto const &v : analogInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_joystickX]);
    }
    uiHandles[Inputs::in_joystickY] =
        ESPUI.addControl(Select, "Map Joystick Y Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_joystickY]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_joystickY);
    for (auto const &v : analogInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_joystickY]);
    }
    uiHandles[Inputs::in_joystickZ] =
        ESPUI.addControl(Select, "Map Joystick Z Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_joystickZ]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_joystickZ);
    for (auto const &v : analogInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_joystickZ]);
    }
    uiHandles[Inputs::in_feedrate] =
        ESPUI.addControl(Select, "Map Feedrate Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_feedrate]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_feedrate);
    for (auto const &v : analogInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_feedrate]);
    }
    uiHandles[Inputs::in_rotationSpeed] =
        ESPUI.addControl(Select, "Map Rotation Speed Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_rotationSpeed]],
                         Wetasphalt, editConfigurationTab, extendedCallback, (void *) Inputs::in_rotationSpeed);
    for (auto const &v : allInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_rotationSpeed]);
    }
    uiHandles[Inputs::in_programStartButton] =
        ESPUI.addControl(Select, "Map Program Start Button Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_programStartButton]],
                         Wetasphalt, editConfigurationTab, extendedCallback, (void *) Inputs::in_programStartButton);
    for (auto const &v : allInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_programStartButton]);
    }

    // Digital Inputs
    ESPUI.addControl(Separator, "Digital Inputs - these can be used for digital functions ", "", None, editConfigurationTab);

    uiHandles[Inputs::in_okButton] =
        ESPUI.addControl(Select, "Map OK Button Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_okButton]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_okButton);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_okButton]);
    }
    uiHandles[Inputs::in_autosquareButton] =
        ESPUI.addControl(Select, "Map Auto Square Button Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_autosquareButton]],
                         Wetasphalt, editConfigurationTab, extendedCallback, (void *) Inputs::in_autosquareButton);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_autosquareButton]);
    }
    uiHandles[Inputs::in_motorStartButton] =
        ESPUI.addControl(Select, "Map Motor Start Button Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_motorStartButton]],
                         Wetasphalt, editConfigurationTab, extendedCallback, (void *) Inputs::in_motorStartButton);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_motorStartButton]);
    }
    uiHandles[Inputs::in_axisXSelect] =
        ESPUI.addControl(Select, "Map Axis X Select Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_axisXSelect]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_axisXSelect);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_axisXSelect]);
    }
    uiHandles[Inputs::in_axisYSelect] =
        ESPUI.addControl(Select, "Map Axis Y Select Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_axisYSelect]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_axisYSelect);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_axisYSelect]);
    }
    uiHandles[Inputs::in_axisZSelect] =
        ESPUI.addControl(Select, "Map Axis Z Select Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_axisZSelect]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_axisZSelect);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_axisZSelect]);
    }
    uiHandles[Inputs::in_button1] =
        ESPUI.addControl(Select, "Map Button 1 Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_button1]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_button1);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_button1]);
    }
    uiHandles[Inputs::in_button2] =
        ESPUI.addControl(Select, "Map Button 2 Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_button2]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_button2);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_button2]);
    }
    uiHandles[Inputs::in_button3] =
        ESPUI.addControl(Select, "Map Button 3 Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_button3]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_button3);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_button3]);
    }

    uiHandles[Inputs::in_button4] =
        ESPUI.addControl(Select, "Map Button 4 Input to..", allInputFunctions[mainConfig.inputs[Inputs::in_button4]], Wetasphalt,
                         editConfigurationTab, extendedCallback, (void *) Inputs::in_button4);
    for (auto const &v : digitalInputFunctions) {
        ESPUI.addControl(Option, v.c_str(), v, None, uiHandles[Inputs::in_button4]);
    }
    // Display
    ESPUI.addControl(Separator, "Display Settings ", "", None, editConfigurationTab);
    displayMode = ESPUI.addControl(Select, "Display Mode - Can be checked directly on the screen. Resets after restart if not saved.",
                                   displayModeMapping[mainConfig.displayMode], Wetasphalt, editConfigurationTab, &displayModeCallback);
    for (auto const &v : displayModeMapping) {
        ESPUI.addControl(Option, v.c_str(), v, None, displayMode);
    }
    // Communication
    ESPUI.addControl(Separator, "Communication Settings ", "", None, editConfigurationTab);
    communicationMode = ESPUI.addControl(Select, "Communication Mode to OCS2.", communicationModeMapping[mainConfig.communicationMode],
                                         Wetasphalt, editConfigurationTab, &communicationModeCallback);
    for (auto const &v : communicationModeMapping) {
        ESPUI.addControl(Option, v.c_str(), v, None, communicationMode);
    }
    // Wifi default on
    String labelText =
        "Wifi default on"
        "<br/><br/><small>When enabled, the Wifi Hotspot will be started automatically on boot.</small>"
        "<br/><br/><small>When disabled, the Wifi Hotspot will be started only when the menu button on the PCB is pressed</small>";
    uint16_t label = ESPUI.addControl(Label, "Wifi Hotspot", labelText, Carrot, editConfigurationTab);
    ESPUI.setElementStyle(label, clearLabelStyle);
    String state = mainConfig.wifiDefaultOn ? "1" : "0";
    ESPUI.addControl(Switcher, "", state, Dark, label, &wifiModeCallback);
    // OCS2 Mac Address
    ESPUI.addControl(Number, "OCS2 Wireless ID(0-255) - Has to match the setting in the OCS2 ESP32 - default 0",
                     String(mainConfig.ocs2MacAddressCustomByte), Wetasphalt, editConfigurationTab,
                     [](Control *sender, int type) { mainConfig.ocs2MacAddressCustomByte = sender->value.toInt(); });
    // Actions
    ESPUI.addControl(Separator, "Actions ", "", None, editConfigurationTab);

    ESPUI.addControl(Button, "Save Configuration and Restart ESP32", "Save & Restart", Peterriver, editConfigurationTab,
                     &saveAndRestartCallback);
    ESPUI.addControl(Button, "Reset Configuration to Defaults and Restart ESP32", "Reset & Restart", Alizarin, editConfigurationTab,
                     &resetAndRestartCallback);

    // Calibration Tab
    calibrationTab = ESPUI.addControl(Tab, "", "Calibration");
    // clang-format off
    String curCalibrationConfig = "<table>"
        "<tr style='border:1px solid black'><td>Joystick X Offset:</td><td>" + String(calibrationConfig.joystickOffsets[0]) + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick Y Offset:</td><td>" + String(calibrationConfig.joystickOffsets[1]) + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick Z Offset:</td><td>" + String(calibrationConfig.joystickOffsets[2]) + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick X Axe:</td><td>" + axeMapping[calibrationConfig.joystickAxes[0]] + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick Y Axe:</td><td>" + axeMapping[calibrationConfig.joystickAxes[1]] + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick Z Axe:</td><td>" + axeMapping[calibrationConfig.joystickAxes[2]] + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick X Inverted:</td><td>" + invertAxisMapping[calibrationConfig.invertAxis[0]] + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick Z Inverted:</td><td>" + invertAxisMapping[calibrationConfig.invertAxis[1]] + "</td></tr>"
        "<tr style='border:1px solid black'><td>Joystick Z Inverted:</td><td>" + invertAxisMapping[calibrationConfig.invertAxis[2]] + "</td></tr>"
        "<tr style='border:1px solid black'><td>Feedrate Inverted:</td><td>" + invertAxisMapping[calibrationConfig.invertFeedrate] + "</td></tr>"
        "<tr style='border:1px solid black'><td>Rotation Speed Inverted:</td><td>" + invertAxisMapping[calibrationConfig.invertRotationSpeed] + "</td></tr>"
        "<tr style='border:1px solid black'><td>ColdEnd Mist Inverted:</td><td>" + invertAxisMapping[calibrationConfig.invertColdEndMistPoti] + "</td></tr>"
        "<tr style='border:1px solid black'><td>ColdEnd Spit Inverted:</td><td>" + invertAxisMapping[calibrationConfig.invertColdEndSpitPoti] + "</td></tr>"
        "</table>";
    // clang-format on
    statusCalibrationConfig = ESPUI.addControl(Label, "calibration configuration", curCalibrationConfig, Carrot, calibrationTab);
    ESPUI.addControl(Separator, "Actions ", "", None, calibrationTab);
    ESPUI.addControl(Button, "Start Calibration", "Start", Peterriver, calibrationTab, &startCalibrationCallback);

    ESPUI.begin("ESP32 Panel");

    // Create loop task
    xTaskCreatePinnedToCore(UIHANDLER::loopTask, "uiHandler loop", 5000, this, 1, &loopTaskHandle, 1);
}

void UIHANDLER::loopTask(void *pvParameters) {
    auto *uiHandler = (UIHANDLER *) pvParameters;
    for (;;) {
        dnsServer.processNextRequest();

        // Update the UI every 250ms
        if (millis() - uiHandler->lastUiUpdate_MS > 250) {
            uiHandler->updateStatusUI();
            uiHandler->lastUiUpdate_MS = millis();
        }
        vTaskDelay(10);
    }
}

void UIHANDLER::updateStatusUI() {
    if (PROTOCOL::hasOCS2Functions) {
        if (PROTOCOL::isOCS2Connected()) {
            ESPUI.updateLabel(statusOCS2, "successful");
            ESPUI.setElementStyle(statusOCS2, emeraldColor);
            // clang-format off
            String ocs2Info = "<table>"
                "<tr style='border:1px solid black'><td>OCS2 Software Version</td><td>"+ String(dataToClient.softwareVersion) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Temperature 1</td><td>"+ String(dataToClient.temperatures[0]) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Temperature 2</td><td>"+ String(dataToClient.temperatures[1]) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Spindle running</td><td>"+ String(dataToClient.spindelState) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Alarm triggered</td><td>"+ String(dataToClient.alarmState) +"</td></tr>"
                "<tr style='border:1px solid black'><td>This panel is ignored?</td><td>"+ String(dataToClient.peerIgnored) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare X Active?</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[0].axisActive] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare Y Active?</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[1].axisActive] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare Z Active?</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[2].axisActive] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare X1 State</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[0].axisMotor1State] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare X2 State</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[0].axisMotor2State] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare Y1 State</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[1].axisMotor1State] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare Y2 State</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[1].axisMotor2State] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare Z1 State</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[2].axisMotor1State] +"</td></tr>"
                "<tr style='border:1px solid black'><td>Autosquare Z2 State</td><td>"+ autosquareStateMapping[dataToClient.autosquareState[2].axisMotor2State] +"</td></tr>"
                "</table>";
            // clang-format on
            ESPUI.updateLabel(statusOCS2Data, ocs2Info);
            ESPUI.updateVisibility(statusOCS2Data, true);
        } else {
            ESPUI.updateLabel(statusOCS2, "failed");
            ESPUI.setElementStyle(statusOCS2, alizarinColor);
            ESPUI.updateVisibility(statusOCS2Data, false);
        }
    } else {
        ESPUI.updateLabel(statusOCS2, "off -> no ocs2 function mapped");
    }

    if (PROTOCOL::hasColdEndFunctions) {
        if (PROTOCOL::isColdEndConnected()) {
            ESPUI.updateLabel(statusColdEnd, "successful");
            ESPUI.setElementStyle(statusColdEnd, emeraldColor);
            // clang-format off
            String coldEndInfo = "<table>"
                "<tr style='border:1px solid black'><td>Cooland Valve</td><td>"+ String(dataFromColdEnd.coolantValve) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Air Valve</td><td>"+ String(dataFromColdEnd.airValve) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Spit Mode running?</td><td>"+ String(dataFromColdEnd.spitMode) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Mist value(rpm)</td><td>"+ String(dataFromColdEnd.mist_val) +"</td></tr>"
                "<tr style='border:1px solid black'><td>Spit value(s)</td><td>"+ String(dataFromColdEnd.spit_val) +"</td></tr>"
                "</table>";
            // clang-format on
            ESPUI.updateLabel(statusColdEndData, coldEndInfo);
            ESPUI.updateVisibility(statusColdEndData, true);
        } else {
            ESPUI.updateLabel(statusColdEnd, "failed");
            ESPUI.setElementStyle(statusColdEnd, alizarinColor);
            ESPUI.updateVisibility(statusColdEndData, false);
        }
    } else {
        ESPUI.updateLabel(statusColdEnd, "off -> no ColdEnd function mapped");
    }
    // Update Analog Inputs
    this->updateSliderHelper(dataToControl.command.setJoystick, map(dataToControl.joystickX, 0, 1023, 0, 100), &sliderXAxis);
    this->updateSliderHelper(dataToControl.command.setJoystick, map(dataToControl.joystickY, 0, 1023, 0, 100), &sliderYAxis);
    this->updateSliderHelper(dataToControl.command.setJoystick, map(dataToControl.joystickZ, 0, 1023, 0, 100), &sliderZAxis);
    this->updateSliderHelper(dataToControl.command.setFeedrate, map(dataToControl.feedrate, 0, 1023, 0, 100), &sliderFeedrate);
    this->updateSliderHelper(dataToControl.command.setRotationSpeed, map(dataToControl.rotationSpeed, 0, 1023, 0, 100),
                             &sliderRotationSpeed);
    this->updateSliderHelper(dataToColdEnd.command.setPotMist, map(dataToColdEnd.pot_mist, 0, 4095, 0, 100), &sliderColdEndMist);
    this->updateSliderHelper(dataToColdEnd.command.setPotSpit, map(dataToColdEnd.pot_spit, 0, 4095, 0, 100), &sliderColdEndSpit);

    // Update Digital Inputs
    this->updateButtonStyleHelper(dataToControl.command.setOk, dataToControl.ok, &okButton);
    this->updateButtonStyleHelper(dataToControl.command.setAutosquare, dataToControl.autosquare, &autosquareButton);
    this->updateButtonStyleHelper(dataToControl.command.setMotorStart, dataToControl.motorStart, &motorStartButton);
    this->updateButtonStyleHelper(dataToControl.command.setProgramStart, dataToControl.programStart, &programmStartButton);
    this->updateButtonStyleHelper(dataToControl.command.setAxisSelect, dataToControl.selectAxisX, &axisXSelectButton);
    this->updateButtonStyleHelper(dataToControl.command.setAxisSelect, dataToControl.selectAxisY, &axisYSelectButton);
    this->updateButtonStyleHelper(dataToControl.command.setAxisSelect, dataToControl.selectAxisZ, &axisZSelectButton);
    this->updateButtonStyleHelper(dataToControl.command.setEna, dataToControl.ena, &enaButton);
    this->updateButtonStyleHelper(dataToControl.command.setSpeed1, dataToControl.speed1, &speed1Button);
    this->updateButtonStyleHelper(dataToControl.command.setSpeed2, dataToControl.speed2, &speed2Button);
    this->updateButtonStyleHelper(dataToControl.command.setOutput1, dataToControl.output1, &output1Button);
    this->updateButtonStyleHelper(dataToControl.command.setOutput2, dataToControl.output2, &output2Button);
    this->updateButtonStyleHelper(dataToControl.command.setOutput3, dataToControl.output3, &output3Button);
    this->updateButtonStyleHelper(dataToControl.command.setOutput4, dataToControl.output4, &output4Button);
    this->updateButtonStyleHelper(dataToColdEnd.command.setInMist, dataToColdEnd.in_mist, &coldEndMistButton);
    this->updateButtonStyleHelper(dataToColdEnd.command.setInAir, dataToColdEnd.in_air, &coldEndAirButton);
    this->updateButtonStyleHelper(dataToColdEnd.command.setInFast, dataToColdEnd.in_fast, &coldEndFastButton);
}

void UIHANDLER::updateSliderHelper(bool enabled, uint16_t value, uint16_t *uiHandle) {
    if (enabled) {
        ESPUI.updateSlider(*uiHandle, value);
    }
}

void UIHANDLER::updateButtonStyleHelper(bool enabled, bool active, uint16_t *uiHandle) {
    if (enabled) {
        if (active) {
            ESPUI.setElementStyle(*uiHandle, emeraldColor);
        } else {
            ESPUI.setElementStyle(*uiHandle, alizarinColor);
        }
    }
}

UIHANDLER uiHandler;