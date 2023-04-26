#include <includes.h>

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_AP_STA);
    // start config manager first - this loads all configuration
    configManager.setup();
    protocol.setup();
    ioControl.setup();
    display.setup();
#ifdef USE_WIFI_WEBINTERFACE
    uiHandler.setup();
#endif
}

void loop() {}