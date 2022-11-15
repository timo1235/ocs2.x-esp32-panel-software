#include <includes.h>

void setup() {
  Serial.begin(115200);

  protocol.setup();
  ioControl.setup();
  display.setup();
}

void loop() {
  ioControl.loop();
  protocol.loop();
}