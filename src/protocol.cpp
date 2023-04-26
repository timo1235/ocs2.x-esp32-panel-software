#include <includes.h>

uint8_t         controllerMacAddress[] = CONTROLLER_MAC_ADDRESS;
DATA_TO_CONTROL dataToControl          = {OCS_PANEL_SOFTWARE_VERSION, 512, 512, 512, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
DATA_TO_CLIENT  dataToClient           = {};

uint8_t                coldEndMacAddress[] = COLDEND_MAC_ADDRESS;
DATA_TO_COLDEND        dataToColdEnd       = {};
DATA_COLDEND_TO_CLIENT dataFromColdEnd     = {};
esp_now_peer_info_t    peerInfoColdEnd;

esp_now_peer_info_t peerInfo;

uint16_t PROTOCOL::failedOCS2MessagesSuccessively = 0;
uint16_t PROTOCOL::successfulOCS2Messages         = 0;
uint16_t PROTOCOL::failedOCS2Messages             = 0;

uint16_t PROTOCOL::failedColdEndMessagesSuccessively = 0;
uint16_t PROTOCOL::successfulColdEndMessages         = 0;
uint16_t PROTOCOL::failedColdEndMessages             = 0;

bool PROTOCOL::hasOCS2Functions    = false;
bool PROTOCOL::hasColdEndFunctions = false;

void PROTOCOL::setup() {
    // WiFi.mode(WIFI_AP_STA);
    DPRINT("My Mac Address: ");
    DPRINTLN(WiFi.macAddress());

    // Controller MAC Address
    DPRINT("Controller Mac Address: ");
    DPRINTLN(PROTOCOL::getMacStrFromAddress(controllerMacAddress));
    DPRINT("ColdEnd Mac Address: ");
    DPRINTLN(PROTOCOL::getMacStrFromAddress(coldEndMacAddress));
    // Init ESP-NOW
    if (esp_now_init() != 0) {
        DPRINTLN("Error initializing ESP-NOW. Things wont work");
        return;
    }

    // Register callbacks
    esp_now_register_recv_cb(PROTOCOL::onDataRecv);
    esp_now_register_send_cb(PROTOCOL::onDataSent);

    // Register controller esp32 as peer
    memcpy(peerInfo.peer_addr, controllerMacAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Register coldend esp32 as peer
    memcpy(peerInfoColdEnd.peer_addr, coldEndMacAddress, 6);
    peerInfoColdEnd.channel = 0;
    peerInfoColdEnd.encrypt = false;

    // Add OCS2 peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        DPRINTLN("Failed to add OCS2 controller peer. Things wont work");
    }
    // Add ColdEnd32 peer
    if (esp_now_add_peer(&peerInfoColdEnd) != ESP_OK) {
        DPRINTLN("Failed to add ColdEnd32 peer. Things wont work");
    }

    xTaskCreatePinnedToCore(PROTOCOL::loopTask, "protocol loop", 3000, this, 1, &loopTaskHandle, 1);
}

void PROTOCOL::loopTask(void *pvParameters) {
    auto *protocol = (PROTOCOL *) pvParameters;
    for (;;) {
        // Send data to controller
        if (millis() - protocol->lastOCS2MessageSent_MS > OCS2_WIFI_DELAY && !ioControl.calibrationInProgress &&
            PROTOCOL::hasOCS2Functions) {
            protocol->lastOCS2MessageSent_MS = millis();
            protocol->sendMessageToController();
        }
        if (millis() - protocol->lastColdEndMessageSent_MS > COLDEND_WIFI_DELAY && !ioControl.calibrationInProgress &&
            PROTOCOL::hasColdEndFunctions) {
            protocol->lastColdEndMessageSent_MS = millis();
            protocol->sendMessageToColdEnd();
        }

        // Send serial status message
        if (millis() - protocol->lastStatusMessage_MS > STATUS_MESSAGE_INTERVAL_MS && !ioControl.calibrationInProgress) {
            protocol->lastStatusMessage_MS = millis();
            DPRINTLN("----------------------------------------");
            DPRINTLN("Status report:");
            if (dataToClient.peerIgnored) {
                DPRINTLN("Controllerconnection:\tNOK Stopped sending updates to the control since the peer is ignored.");
                return;
            } else if (PROTOCOL::failedOCS2MessagesSuccessively > 5) {
                DPRINT("Controller connection:\tNOK, failed messages: " + String(PROTOCOL::failedOCS2Messages) +
                       ", successful messages: " + String(PROTOCOL::successfulOCS2Messages));
                DPRINTLN(", in " + String(STATUS_MESSAGE_INTERVAL_MS / 1000) + " seconds");
            } else if (PROTOCOL::hasOCS2Functions) {
                DPRINT("Controller connection:\tOK, failed messages: " + String(PROTOCOL::failedOCS2Messages) +
                       ", successful messages: " + String(PROTOCOL::successfulOCS2Messages));
                DPRINTLN(", in " + String(STATUS_MESSAGE_INTERVAL_MS / 1000) + " seconds");
            }
            PROTOCOL::failedOCS2Messages     = 0;
            PROTOCOL::successfulOCS2Messages = 0;

            if (PROTOCOL::failedColdEndMessagesSuccessively > 5) {
                DPRINT("ColdEnd32 connection:\tNOK, failed messages: " + String(PROTOCOL::failedColdEndMessages) +
                       ", successful messages: " + String(PROTOCOL::successfulColdEndMessages));
                DPRINTLN(", in " + String(STATUS_MESSAGE_INTERVAL_MS / 1000) + " seconds");
            } else if (PROTOCOL::hasColdEndFunctions) {
                DPRINT("ColdEnd32 connection:\tOK, failed messages: " + String(PROTOCOL::failedColdEndMessages) +
                       ", successful messages: " + String(PROTOCOL::successfulColdEndMessages));
                DPRINTLN(", in " + String(STATUS_MESSAGE_INTERVAL_MS / 1000) + " seconds");
            }
            PROTOCOL::failedColdEndMessages     = 0;
            PROTOCOL::successfulColdEndMessages = 0;
            DPRINTLN("----------------------------------------");
        }
        vTaskDelay(5);
    }
}

void PROTOCOL::initializeCommand() {
    bool hasJoystick = ioConfig.hasFunction(InputFunctions::func_joystickX) && ioConfig.hasFunction(InputFunctions::func_joystickY) &&
                       ioConfig.hasFunction(InputFunctions::func_joystickZ);
    dataToControl.command.setJoystick      = hasJoystick;
    dataToControl.command.setFeedrate      = ioConfig.hasFunction(InputFunctions::func_feedrate);
    dataToControl.command.setRotationSpeed = ioConfig.hasFunction(InputFunctions::func_rotationSpeed);
    dataToControl.command.setAutosquare    = ioConfig.hasFunction(InputFunctions::func_autosquare);
    bool hasAxisSelect = ioConfig.hasFunction(InputFunctions::func_axisXSelect) && ioConfig.hasFunction(InputFunctions::func_axisYSelect) &&
                         ioConfig.hasFunction(InputFunctions::func_axisZSelect);
    dataToControl.command.setAxisSelect     = hasAxisSelect;
    dataToControl.command.setOk             = ioConfig.hasFunction(InputFunctions::func_ok);
    dataToControl.command.setProgramStart   = ioConfig.hasFunction(InputFunctions::func_programStart);
    dataToControl.command.setMotorStart     = ioConfig.hasFunction(InputFunctions::func_motorStart);
    dataToControl.command.setEna            = ioConfig.hasFunction(InputFunctions::func_ena);
    dataToControl.command.setSpeed1         = ioConfig.hasFunction(InputFunctions::func_speed1);
    dataToControl.command.setSpeed2         = ioConfig.hasFunction(InputFunctions::func_speed2);
    dataToControl.command.setOutput1        = ioConfig.hasFunction(InputFunctions::func_output1);
    dataToControl.command.setOutput2        = ioConfig.hasFunction(InputFunctions::func_output2);
    dataToControl.command.setOutput3        = ioConfig.hasFunction(InputFunctions::func_output3);
    dataToControl.command.setOutput4        = ioConfig.hasFunction(InputFunctions::func_output4);
    dataToControl.command.returnACK         = 1;
    dataToControl.command.returnData        = 1;
    dataToControl.command.updateInterval_MS = OCS2_WIFI_DELAY;

    dataToColdEnd.command.setPotMist        = ioConfig.hasFunction(InputFunctions::func_coldend_pot_mist);
    dataToColdEnd.command.setPotSpit        = ioConfig.hasFunction(InputFunctions::func_coldend_pot_spit);
    dataToColdEnd.command.setInAir          = ioConfig.hasFunction(InputFunctions::func_coldend_in_air);
    dataToColdEnd.command.setInMist         = ioConfig.hasFunction(InputFunctions::func_coldend_in_mist);
    dataToColdEnd.command.setInFast         = ioConfig.hasFunction(InputFunctions::func_coldend_in_fast);
    dataToColdEnd.command.returnACK         = 1;
    dataToColdEnd.command.returnData        = 1;
    dataToColdEnd.command.updateInterval_MS = COLDEND_WIFI_DELAY;

    // See if we have any OCS2 or ColdEnd functions for the 16 GPIOs
    // look at the InputFunctions enum to see what the numbers mean
    for (int i = 0; i < 16; i++) {
        byte function = (int) ioConfig.getGPIO((Inputs) i)->function;
        if (function > 0 && function <= 18) {
            PROTOCOL::hasOCS2Functions = true;
        }
        if (function > 19 && function <= 24) {
            PROTOCOL::hasColdEndFunctions = true;
        }
    }
}

void PROTOCOL::sendMessageToController() {
    if (dataToClient.peerIgnored) {
        return;
    }
    esp_now_send(controllerMacAddress, (uint8_t *) &dataToControl, sizeof(dataToControl));
}

void PROTOCOL::sendMessageToColdEnd() {
    // only send message if we have something to say
    if (dataToColdEnd.command.setPotMist == 0 && dataToColdEnd.command.setPotSpit == 0 && dataToColdEnd.command.setInAir == 0 &&
        dataToColdEnd.command.setInMist == 0 && dataToColdEnd.command.setInFast == 0) {
        return;
    }
    esp_now_send(coldEndMacAddress, (uint8_t *) &dataToColdEnd, sizeof(dataToColdEnd));
}

char *PROTOCOL::getMacStrFromAddress(uint8_t *address) {
    static char macStr[18];
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4],
             address[5]);
    return macStr;
}

uint16_t PROTOCOL::getIntegerFromAddress(const uint8_t *address) {
    uint16_t integer = 0;
    integer += address[0];
    integer += address[1];
    integer += address[2];
    integer += address[3];
    integer += address[4];
    integer += address[5];
    return integer;
}

void PROTOCOL::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    uint16_t receiverAddress = PROTOCOL::getIntegerFromAddress((uint8_t *) mac_addr);

    if (receiverAddress == PROTOCOL::getIntegerFromAddress(coldEndMacAddress)) {
        if (status == ESP_NOW_SEND_SUCCESS) {
            PROTOCOL::failedColdEndMessagesSuccessively = 0;
            PROTOCOL::successfulColdEndMessages++;
        } else {
            PROTOCOL::failedColdEndMessagesSuccessively++;
            PROTOCOL::failedColdEndMessages++;
        }
    } else if (receiverAddress == PROTOCOL::getIntegerFromAddress(controllerMacAddress)) {
        if (status == ESP_NOW_SEND_SUCCESS) {
            ioControl.startBlinkRJ45LED();
            PROTOCOL::failedOCS2MessagesSuccessively = 0;
            PROTOCOL::successfulOCS2Messages++;
        } else {
            ioControl.stopBlinkRJ45LED();
            PROTOCOL::failedOCS2MessagesSuccessively++;
            PROTOCOL::failedOCS2Messages++;
        }
    }
}

void PROTOCOL::onDataRecv(const uint8_t *address, const uint8_t *incomingData, int len) {
    uint16_t receiverAddress = PROTOCOL::getIntegerFromAddress((uint8_t *) address);

    if (receiverAddress == PROTOCOL::getIntegerFromAddress(coldEndMacAddress)) {
        memcpy(&dataFromColdEnd, incomingData, sizeof(DATA_COLDEND_TO_CLIENT));
    } else if (receiverAddress == PROTOCOL::getIntegerFromAddress(controllerMacAddress)) {
        memcpy(&dataToClient, incomingData, sizeof(DATA_TO_CLIENT));
        if (dataToClient.peerIgnored) {
            ioControl.stopBlinkRJ45LED();
            DPRINTLN("Stop sending updates to the ocs2 control since this esp32 is ignored.");
            DPRINTLN("Check the output of the ocs2 mainboard esp32 to see whats wrong.");
            DPRINTLN("Typically the reason is that this esp32 tries to update values, that are already updated by another esp32.");
        }
    }
}

bool PROTOCOL::isOCS2Connected() {
    if (dataToClient.peerIgnored || PROTOCOL::failedOCS2MessagesSuccessively >= 5) {
        return false;
    }
    if (PROTOCOL::successfulOCS2Messages == 0 && PROTOCOL::failedOCS2Messages > 0) {
        return false;
    }
    return true;
}

bool PROTOCOL::isColdEndConnected() {
    if (PROTOCOL::failedColdEndMessagesSuccessively >= 5) {
        return false;
    }
    if (PROTOCOL::successfulColdEndMessages == 0 && PROTOCOL::failedColdEndMessages > 0) {
        return false;
    }
    return true;
}

PROTOCOL protocol;