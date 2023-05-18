#include <includes.h>

void setup() {
    Serial.begin(115200);

    // start config manager first - this loads all configuration
    configManager.setup();
    protocol.setup();
    ioControl.setup();
    display.setup();
#ifdef USE_WIFI_WEBINTERFACE
    #ifdef FORCE_WIFI_DEFAULT_ON
    uiHandler.setup();
    #else
    if (mainConfig.wifiDefaultOn) {
        uiHandler.setup();
    }
    #endif
#endif
}

void loop() {}