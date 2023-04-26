#include <includes.h>

IOCONFIG ioConfig;

void GPIO_CLASS::buttonUpdateTask(void *pvParameters) {
    auto *gpio = (GPIO_CLASS *) pvParameters;
    for (;;) {
        gpio->button.update();
        vTaskDelay(3);
    }
}

void GPIO_CLASS::setPin(uint8_t pin) { this->pin = pin; }

void GPIO_CLASS::init() {
    switch (this->type) {
    case GPIO_TYPE::GPIO_TYPE_INPUT:
        pinMode(this->pin, INPUT);
        break;
    case GPIO_TYPE::GPIO_TYPE_INPUT_PULLUP:
        pinMode(this->pin, INPUT_PULLUP);
        break;
    case GPIO_TYPE::GPIO_TYPE_OUTPUT:
        pinMode(this->pin, OUTPUT);
        break;
    case GPIO_TYPE::GPIO_TYPE_INPUT_PULLUP_BOUNCE:
        this->button.attach(this->pin, INPUT_PULLUP);
        this->button.interval(10);
        // Task updates the button state regulary
        xTaskCreatePinnedToCore(GPIO_CLASS::buttonUpdateTask, "Button Update", 1500, this, 1, &buttonTaskHandle, 0);
        break;
    default:
        break;
    }
}

void GPIO_CLASS::setFunction(InputFunctions function) { this->function = function; }

void GPIO_CLASS::setType(GPIO_TYPE type) { this->type = type; }

uint16_t GPIO_CLASS::read() {
    switch (this->type) {
    case GPIO_TYPE::GPIO_TYPE_INPUT:
        return digitalRead(this->pin);
        break;
    case GPIO_TYPE::GPIO_TYPE_INPUT_PULLUP:
        return !digitalRead(this->pin);
        break;
    case GPIO_TYPE::GPIO_TYPE_INPUT_PULLUP_BOUNCE:
        this->button.update();
        return !this->button.read();
        break;
    case GPIO_TYPE::GPIO_TYPE_ANALOG:
        return analogRead(this->pin);
        break;
    default:
        return 0;
        break;
    }
}

// IOCONFIG CLASS
IOCONFIG::IOCONFIG() {
    // Init all GPIO pins
    gpio[Inputs::in_joystickX].setPin(JOYSTICK_X_PIN);
    gpio[Inputs::in_joystickY].setPin(JOYSTICK_Y_PIN);
    gpio[Inputs::in_joystickZ].setPin(JOYSTICK_Z_PIN);
    gpio[Inputs::in_feedrate].setPin(FEEDRATE_PIN);
    gpio[Inputs::in_rotationSpeed].setPin(ROTATION_SPEED_PIN);
    gpio[Inputs::in_okButton].setPin(OK_PIN);
    gpio[Inputs::in_autosquareButton].setPin(AUTOSQUARE_PIN);
    gpio[Inputs::in_motorStartButton].setPin(MOTOR_START_PIN);
    gpio[Inputs::in_programStartButton].setPin(PROGRAMM_START_PIN);
    gpio[Inputs::in_axisXSelect].setPin(SELECT_AXIS_X_PIN);
    gpio[Inputs::in_axisYSelect].setPin(SELECT_AXIS_Y_PIN);
    gpio[Inputs::in_axisZSelect].setPin(SELECT_AXIS_Z_PIN);
    gpio[Inputs::in_button1].setPin(BUTTON_1_PIN);
    gpio[Inputs::in_button2].setPin(BUTTON_2_PIN);
    gpio[Inputs::in_button3].setPin(BUTTON_3_PIN);
    gpio[Inputs::in_button4].setPin(BUTTON_4_PIN);
    gpio[Inputs::in_menu].setPin(MENU_BUTTON_PIN);
}

void IOCONFIG::setFunction(Inputs input, InputFunctions function) {
    this->gpio[input].setFunction(function);

    switch (function) {
    case InputFunctions::func_joystickX:
    case InputFunctions::func_joystickY:
    case InputFunctions::func_joystickZ:
    case InputFunctions::func_feedrate:
    case InputFunctions::func_rotationSpeed:
    case InputFunctions::func_coldend_pot_mist:
    case InputFunctions::func_coldend_pot_spit:
        this->gpio[input].setType(GPIO_TYPE::GPIO_TYPE_ANALOG);
        break;
    default:
        this->gpio[input].setType(GPIO_TYPE::GPIO_TYPE_INPUT_PULLUP_BOUNCE);
        break;
    }

    this->gpio[input].init();
}

GPIO_CLASS *IOCONFIG::getGPIO(Inputs input) { return &this->gpio[input]; }

GPIO_CLASS *IOCONFIG::getGPIOByFunction(InputFunctions function) {
    for (int i = 0; i < sizeof(this->gpio) / sizeof(GPIO_CLASS); i++) {
        if (this->gpio[i].function == function) {
            return &this->gpio[i];
        }
    }
    DPRINTLN("ERROR: No GPIO found for function: '" + allInputFunctions[function] +
             "'. Use hasFunction() to check if function is available before!");
    return &this->gpio[0];
}

bool IOCONFIG::hasFunction(InputFunctions function) {
    for (int i = 0; i < sizeof(this->gpio) / sizeof(GPIO_CLASS); i++) {
        if (this->gpio[i].function == function) {
            return true;
        }
    }
    return false;
}
