#include <includes.h>

void setup() {
  Serial.begin(115200);

  protocol_setup();
  iocontrol_setup();
}

void loop() {
  iocontrol_readAll();
  protocol_sendMessageToController();
  delay(WIFI_DELAY);
}