#ifndef ocs_protocol_h
#define ocs_protocol_h

#include <Arduino.h>

// Max size of this struct is 32 bytes - buffer limit
// uint16_t : 2Bytes
// unsigned : 1 Bit each
typedef struct {
  uint16_t joystickX;
  uint16_t joystickY;
  uint16_t joystickZ;
  uint16_t feedrate;
  uint16_t rotationSpeed;
  unsigned autosquare:1;
  unsigned ena:1;
  unsigned auswahlX:1;
  unsigned auswahlY:1;
  unsigned auswahlZ:1;
  unsigned ok:1;
  unsigned programmStart:1;
  unsigned motorStart:1;
  unsigned speed1:1;
  unsigned speed2:1;
} DATA_TO_CONTROL;

void protocol_setup();
void protocol_sendMessageToController();


#endif