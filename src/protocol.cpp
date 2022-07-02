#include <includes.h>

uint8_t controllerMacAddress[] = CONTROLLER_MAC_ADDRESS;
DATA_TO_CONTROL dataToControl = {512,512,512,0,0,0,0,0,0,0,0,0,0,0,0};

esp_now_peer_info_t peerInfo;
RTC_DATA_ATTR uint16_t bootCount = 0;

void protocol_setup(){

    WiFi.enableLongRange(true);
    WiFi.mode(WIFI_STA);
    DPRINT("My Mac Address: ");
    DPRINTLN(WiFi.macAddress());
    //Init ESP-NOW
    if (esp_now_init() != 0)
    {
        DPRINTLN("Error initializing ESP-NOW. Things wont work");
        return;
    }
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

void protocol_sendMessageToController() {
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(controllerMacAddress, (uint8_t *)&dataToControl, sizeof(dataToControl));
    // while(esp_now_send(controllerMacAddress, (uint8_t *)&dataToControl, sizeof(dataToControl)) != ESP_OK) {
    //     delay(1);
    // }
    if (result != ESP_OK)
    {
        DPRINTLN("Error sending the data - Things wont work");
    }
}

char *getMacStrFromAddress(uint8_t *address)
{
    static char macStr[18];
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);
    return macStr;
}