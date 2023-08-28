#include <includes.h>

Preferences preferences;

CALIBRATION_CONFIG calibrationConfig = {};
CONFIGURATION      mainConfig        = {};
CONFIGMANAGER      configManager;

bool CONFIGMANAGER::startConfigHotspot = false;

void CONFIGMANAGER::setup() {
    preferences.begin("config", false);

    loadCalibrationConfig();
    loadMainConfig();
    loadHotspotConfig();
    delay(100);
}

void CONFIGMANAGER::loadCalibrationConfig() {
    DPRINTLN("------------------------------------");
    DPRINTLN("Read stored calibration configuration.");
    if (!preferences.isKey("calibration")) {
        DPRINTLN("No calibration configuration saved in memory, assuming default values.");
        calibrationConfig = {
            {    0,     0,     0},
            {    0,     1,     2},
            {false, false, false},
            false, false, false, false
        };
    } else {
        size_t result = preferences.getBytes("calibration", &calibrationConfig, sizeof(calibrationConfig));
    }
    printCalibrationConfig();
    DPRINTLN("------------------------------------");
}

void CONFIGMANAGER::saveCalibrationConfig() {
    DPRINTLN("------------------------------------");
    DPRINTLN("Save calibration configuration.");
    preferences.putBytes("calibration", &calibrationConfig, sizeof(calibrationConfig));
    printCalibrationConfig();
    DPRINTLN("------------------------------------");
}

void CONFIGMANAGER::resetCalibrationConfig() {
    DPRINTLN("Reset calibration configuration.");
    calibrationConfig = {
        {    0,     0,     0},
        {    0,     1,     2},
        {false, false, false},
        false, false
    };
    preferences.remove("calibration");
}

void CONFIGMANAGER::printCalibrationConfig() {
    DPRINTLN("Printing calibration configuration: ");
    DPRINT("Offsets: ");
    DPRINT(calibrationConfig.joystickOffsets[0]);
    DPRINT(", ");
    DPRINT(calibrationConfig.joystickOffsets[1]);
    DPRINT(", ");
    DPRINTLN(calibrationConfig.joystickOffsets[2]);
    DPRINT("Axes: ");
    DPRINT(calibrationConfig.joystickAxes[0]);
    DPRINT(", ");
    DPRINT(calibrationConfig.joystickAxes[1]);
    DPRINT(", ");
    DPRINTLN(calibrationConfig.joystickAxes[2]);
    DPRINT("Invert axis: ");
    DPRINT(calibrationConfig.invertAxis[0]);
    DPRINT(", ");
    DPRINT(calibrationConfig.invertAxis[1]);
    DPRINT(", ");
    DPRINTLN(calibrationConfig.invertAxis[2]);
    DPRINT("Invert feedrate: ");
    DPRINTLN(calibrationConfig.invertFeedrate);
    DPRINT("Invert rotation speed: ");
    DPRINTLN(calibrationConfig.invertRotationSpeed);
    DPRINT("Invert cold end mist poti: ");
    DPRINTLN(calibrationConfig.invertColdEndMistPoti);
    DPRINT("Invert cold end spit poti: ");
    DPRINTLN(calibrationConfig.invertColdEndSpitPoti);
}

void CONFIGMANAGER::startHotspotAfterReset() {
    DPRINTLN("Starting config hotspot after next reset.");
    preferences.putBool("hotspot", true);
}

void CONFIGMANAGER::loadHotspotConfig() {
    if (!preferences.isKey("hotspot")) {
        return;
    }

    bool result = preferences.getBool("hotspot", false);
    if (result) {
        DPRINTLN("Start config hotspot.");
        preferences.remove("hotspot");
        CONFIGMANAGER::startConfigHotspot = true;
    }
}

void CONFIGMANAGER::loadMainConfig() {
    DPRINTLN("------------------------------------");
    DPRINTLN("Reading stored main configuration.");
    if (!preferences.isKey("main")) {
        CONFIGMANAGER::startConfigHotspot = true;
        DPRINTLN("No configuration saved in memory, assuming default values.");
        mainConfig.inputs[Inputs::in_joystickX]          = InputFunctions::func_joystickX;
        mainConfig.inputs[Inputs::in_joystickY]          = InputFunctions::func_joystickY;
        mainConfig.inputs[Inputs::in_joystickZ]          = InputFunctions::func_joystickZ;
        mainConfig.inputs[Inputs::in_feedrate]           = InputFunctions::func_feedrate;
        mainConfig.inputs[Inputs::in_rotationSpeed]      = InputFunctions::func_rotationSpeed;
        mainConfig.inputs[Inputs::in_okButton]           = InputFunctions::func_ok;
        mainConfig.inputs[Inputs::in_autosquareButton]   = InputFunctions::func_autosquare;
        mainConfig.inputs[Inputs::in_motorStartButton]   = InputFunctions::func_motorStart;
        mainConfig.inputs[Inputs::in_programStartButton] = InputFunctions::func_programStart;
        mainConfig.inputs[Inputs::in_axisXSelect]        = InputFunctions::func_axisXSelect;
        mainConfig.inputs[Inputs::in_axisYSelect]        = InputFunctions::func_axisYSelect;
        mainConfig.inputs[Inputs::in_axisZSelect]        = InputFunctions::func_axisZSelect;
        mainConfig.inputs[Inputs::in_button1]            = InputFunctions::func_ena;
        mainConfig.inputs[Inputs::in_button2]            = InputFunctions::func_empty;
        mainConfig.inputs[Inputs::in_button3]            = InputFunctions::func_empty;
        mainConfig.inputs[Inputs::in_button4]            = InputFunctions::func_menu;
        mainConfig.displayMode                           = DisplayMode::display_default;
        mainConfig.wifiDefaultOn                         = true;
        mainConfig.communicationMode                     = CommunicationMode::automatic;
    } else {
        size_t result                     = preferences.getBytes("main", &mainConfig, sizeof(mainConfig));
        CONFIGMANAGER::startConfigHotspot = false;
    }
    printMainConfig();
    DPRINTLN("------------------------------------");
}

void CONFIGMANAGER::saveMainConfig() {
    DPRINTLN("------------------------------------");
    DPRINTLN("Save main configuration.");
    preferences.putBytes("main", &mainConfig, sizeof(mainConfig));
    printMainConfig();
    DPRINTLN("------------------------------------");
}

void CONFIGMANAGER::resetMainConfig() {
    DPRINTLN("Reset main configuration.");
    preferences.remove("main");
}

void CONFIGMANAGER::printMainConfig() {
    String displayModeMapping[]       = {"Default", "OCS2 only", "ColdEnd only"};
    String communicationModeMapping[] = {"automatic(serial first)", "only serial", "only wifi"};
    DPRINTLN("Printing input to function mapping: ");
    DPRINTLN("Joystick X Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_joystickX]]);
    DPRINTLN("Joystick Y Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_joystickY]]);
    DPRINTLN("Joystick Z Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_joystickZ]]);
    DPRINTLN("Feedrate Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_feedrate]]);
    DPRINTLN("Rotation Speed Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_rotationSpeed]]);
    DPRINTLN("OK Button Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_okButton]]);
    DPRINTLN("Autosquare Button Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_autosquareButton]]);
    DPRINTLN("Motor Start Button Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_motorStartButton]]);
    DPRINTLN("Program Start Button Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_programStartButton]]);
    DPRINTLN("Axis X Select Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_axisXSelect]]);
    DPRINTLN("Axis Y Select Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_axisYSelect]]);
    DPRINTLN("Axis Z Select Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_axisZSelect]]);
    DPRINTLN("Button 1 Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_button1]]);
    DPRINTLN("Button 2 Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_button2]]);
    DPRINTLN("Button 3 Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_button3]]);
    DPRINTLN("Button 4 Input function: " + allInputFunctions[mainConfig.inputs[Inputs::in_button4]]);
    DPRINTLN("DisplayMode: " + displayModeMapping[mainConfig.displayMode]);
    DPRINTLN("Wifi default on: " + String(mainConfig.wifiDefaultOn));
    DPRINTLN("Communication mode: " + communicationModeMapping[mainConfig.communicationMode]);
}
