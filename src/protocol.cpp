#include <includes.h>

uint8_t controllerMacAddress[] = CONTROLLER_MAC_ADDRESS;
DATA_TO_CONTROL dataToControl = {OCS_PANEL_SOFTWARE_VERSION, 512, 512, 512, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
DATA_TO_CLIENT dataToClient = {};

esp_now_peer_info_t peerInfo;
uint16_t PROTOCOL::failedMessagesSuccessively = 0;
uint16_t PROTOCOL::successfulMessages = 0;
uint16_t PROTOCOL::failedMessages = 0;

void PROTOCOL::setup()
{
    // Initialize command
    this->initializeCommand();

    WiFi.enableLongRange(true);
    WiFi.mode(WIFI_STA);
    DPRINT("My Mac Address: ");
    DPRINTLN(WiFi.macAddress());

    // Controller MAC Address
    DPRINT("Controller Mac Address: ");
    DPRINTLN(PROTOCOL::getMacStrFromAddress(controllerMacAddress));
    // Init ESP-NOW
    if (esp_now_init() != 0)
    {
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

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        DPRINTLN("Failed to add peer. Things wont work");
    }
}

void PROTOCOL::loop()
{
    // Send data to controller
    if (millis() - this->lastMessageSent_MS > WIFI_DELAY && !ioControl.calibrationInProgress)
    {
        this->lastMessageSent_MS = millis();
        this->sendMessageToController();
    }

    // Send serial status message
    if (millis() - this->lastStatusMessage_MS > STATUS_MESSAGE_INTERVAL_MS && !ioControl.calibrationInProgress)
    {
        this->lastStatusMessage_MS = millis();
        if (dataToClient.peerIgnored)
        {
            DPRINTLN("Controllerconnection:\tNOK Stopped sending updates to the control since the peer is ignored.");
            return;
        }
        else if (PROTOCOL::failedMessagesSuccessively > 5)
        {
            DPRINT("Controllerconnection:\tNOK, failed messages: " + String(PROTOCOL::failedMessages) + ", successful messages: " + String(PROTOCOL::successfulMessages));
            DPRINTLN(", in " + String(STATUS_MESSAGE_INTERVAL_MS / 1000) + " seconds");
        }
        else
        {
            DPRINT("Controllerconnection:\tOK, failed messages: " + String(PROTOCOL::failedMessages) + ", successful messages: " + String(PROTOCOL::successfulMessages));
            DPRINTLN(", in " + String(STATUS_MESSAGE_INTERVAL_MS / 1000) + " seconds");
        }
        PROTOCOL::failedMessages = 0;
        PROTOCOL::successfulMessages = 0;

        DPRINTLN("Temperature 0:\t\t" + String(dataToClient.temperatures[0]));
        DPRINTLN("Temperature 1:\t\t" + String(dataToClient.temperatures[1]));

        DPRINTLN("Autosquare Running:\t" + String(dataToClient.autosquareRunning));
        DPRINTLN("Spindel State:\t\t" + String(dataToClient.spindelState));
        DPRINTLN("Alarm State:\t\t" + String(dataToClient.alarmState));
    }
}

void PROTOCOL::initializeCommand()
{
    dataToControl.command.setJoystick = HAS_JOYSTICK;
    dataToControl.command.setFeedrate = HAS_FEEDRATE_POTI;
    dataToControl.command.setRotationSpeed = HAS_ROTATION_SPEED_POTI;
    dataToControl.command.setAutosquare = HAS_AUTOSQUARE_BUTTON;
    dataToControl.command.setAxisSelect = HAS_AXIS_SELECT_BUTTONS;
    dataToControl.command.setOk = HAS_OK_BUTTON;
    dataToControl.command.setProgrammStart = HAS_PROGRAMM_START_BUTTON;
    dataToControl.command.setMotorStart = HAS_MOTOR_START_BUTTON;
    dataToControl.command.setEna = HAS_ENA_BUTTON;
    dataToControl.command.setSpeed1 = HAS_SPEED1_BUTTON;
    dataToControl.command.setSpeed2 = HAS_SPEED2_BUTTON;
    dataToControl.command.returnACK = 1;
    dataToControl.command.returnData = 1;
    dataToControl.command.updateInterval_MS = WIFI_DELAY;
}

void PROTOCOL::sendMessageToController()
{
    if (dataToClient.peerIgnored)
    {
        return;
    }
    esp_now_send(controllerMacAddress, (uint8_t *)&dataToControl, sizeof(dataToControl));
}

char *PROTOCOL::getMacStrFromAddress(uint8_t *address)
{
    static char macStr[18];
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);
    return macStr;
}

void PROTOCOL::onDataSent(const uint8_t *address, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        ioControl.startBlinkRJ45LED();
        PROTOCOL::failedMessagesSuccessively = 0;
        PROTOCOL::successfulMessages++;
    }
    else
    {
        ioControl.stopBlinkRJ45LED();
        PROTOCOL::failedMessagesSuccessively++;
        PROTOCOL::failedMessages++;
    }
}

void PROTOCOL::onDataRecv(const uint8_t *address, const uint8_t *incomingData, int len)
{
    memcpy(&dataToClient, incomingData, sizeof(DATA_TO_CLIENT));
    if (dataToClient.peerIgnored)
    {
        ioControl.stopBlinkRJ45LED();
        DPRINTLN("Stop sending updates to the control since the peer is ignored.");
        DPRINTLN("Check the output of the mainboard esp32 to see whats wrong.");
        DPRINTLN("Typically the reason is that this esp32 tries to update values, that are already updated by another esp32.");
    }
}

bool PROTOCOL::isConnected()
{
    if (dataToClient.peerIgnored || PROTOCOL::failedMessagesSuccessively >= 5)
    {
        return false;
    }
    if(PROTOCOL::successfulMessages == 0 && PROTOCOL::failedMessages > 0){
        return false;
    }
    return true;
}

PROTOCOL protocol;