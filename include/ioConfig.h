#pragma once
// Import structs etc from espui_handler.h
// #include <espui_handler.h>
#include <Bounce2.h>

// These are the inputs of the pcb
enum Inputs {
    in_joystickX,
    in_joystickY,
    in_joystickZ,
    in_feedrate,
    in_rotationSpeed,
    in_okButton,
    in_autosquareButton,
    in_motorStartButton,
    in_programStartButton,
    in_axisXSelect,
    in_axisYSelect,
    in_axisZSelect,
    in_button1,
    in_button2,
    in_button3,
    in_button4,
    in_menu
};

// These are the functions which can be assigned to an input
// Order is critical and has to match the allInputFunctions array!!
enum InputFunctions {
    func_joystickX,
    func_joystickY,
    func_joystickZ,
    func_feedrate,
    func_rotationSpeed,
    func_ok,
    func_autosquare,
    func_motorStart,
    func_programStart,
    func_axisXSelect,
    func_axisYSelect,
    func_axisZSelect,
    func_ena,
    func_speed1,
    func_speed2,
    func_output1,
    func_output2,
    func_output3,
    func_output4,
    func_menu,
    func_coldend_in_mist,
    func_coldend_in_fast,
    func_coldend_in_air,
    func_coldend_pot_spit,
    func_coldend_pot_mist,
    func_empty
};

// clang-format off
static String allInputFunctions[]{
    "ocs2 joystick X",
    "osc2 joystick Y",
    "osc2 joystick Z",
    "ocs2 feedrate",
    "ocs2 rotation speed",
    "ocs2 ok",
    "ocs2 autosquare",
    "ocs2 motor start",
    "ocs2 program start",
    "ocs2 axis X select",
    "ocs2 axis Y select",
    "ocs2 axis Z select",
    "ocs2 ena - enable",
    "ocs2 speed 1",
    "ocs2 speed 2",
    "ocs2 output 1",
    "ocs2 output 2",
    "ocs2 output 3",
    "ocs2 output 4",
    "esp32 panel menu",
    "coldend button mist",
    "coldend button fast",
    "coldend button air",
    "coldend poti spit",
    "coldend poti mist",
    "empty"};

static String digitalInputFunctions[]{
    "ocs2 ok",
    "ocs2 autosquare",
    "ocs2 motor start",
    "ocs2 program start",
    "ocs2 axis X select",
    "ocs2 axis Y select",
    "ocs2 axis Z select",
    "ocs2 ena - enable",
    "ocs2 speed 1",
    "ocs2 speed 2",
    "ocs2 output 1",
    "ocs2 output 2",
    "ocs2 output 3",
    "ocs2 output 4",
    "esp32 panel menu",
    "coldend button mist",
    "coldend button fast",
    "coldend button air",
    "empty"};

static String analogInputFunctions[]{
    "ocs2 joystick X",
    "osc2 joystick Y",
    "osc2 joystick Z",
    "ocs2 feedrate",
    "ocs2 rotation speed",
    "coldend poti spit",
    "coldend poti mist",
    "empty"};
// clang-format on

enum GPIO_TYPE {
    GPIO_TYPE_INPUT,
    GPIO_TYPE_INPUT_PULLUP,
    GPIO_TYPE_INPUT_PULLUP_BOUNCE,
    GPIO_TYPE_OUTPUT,
    GPIO_TYPE_ANALOG,
};

class GPIO_CLASS {
  private:
    bool         active = false;
    static void  buttonUpdateTask(void *pvParameters);
    TaskHandle_t buttonTaskHandle;

  public:
    InputFunctions function;
    GPIO_TYPE      type;
    uint8_t        pin;
    Bounce         button = Bounce();

    void setPin(uint8_t pin);
    void setFunction(InputFunctions function);
    void setType(GPIO_TYPE type);
    void init();

    uint16_t read();
};

class IOCONFIG {
  private:
    // 16 GPIOs - see enum Inputs
    GPIO_CLASS gpio[16];

  public:
    IOCONFIG();
    void        setFunction(Inputs input, InputFunctions function);
    bool        hasFunction(InputFunctions function);
    GPIO_CLASS *getGPIO(Inputs input);
    GPIO_CLASS *getGPIOByFunction(InputFunctions function);
};

extern IOCONFIG ioConfig;
