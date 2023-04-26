#pragma once

#include <Arduino.h>
#include <esp_now.h>

#define STATUS_MESSAGE_INTERVAL_MS 10000

// This struct describes what data is send and what is expected
typedef struct {
    unsigned setJoystick : 1;
    unsigned setFeedrate : 1;
    unsigned setRotationSpeed : 1;
    unsigned setAutosquare : 1;
    unsigned setEna : 1;
    unsigned setAxisSelect : 1;
    unsigned setOk : 1;
    unsigned setProgramStart : 1;
    unsigned setMotorStart : 1;
    unsigned setSpeed1 : 1;
    unsigned setSpeed2 : 1;
    unsigned setOutput1 : 1;
    unsigned setOutput2 : 1;
    unsigned setOutput3 : 1;
    unsigned setOutput4 : 1;
    unsigned returnACK : 1;   // Return ACK to client
    unsigned
        returnData : 1;   // Return mainboard data to client, this can be the temperature, autosquaring state and so son and replaces ACK
    uint16_t updateInterval_MS;   // Interval in ms the client sends data to the mainboard
} DATA_COMMAND;

// This struct describes the auto square data
typedef struct {
    unsigned axisActive : 1;
    uint8_t  axisMotor1State;
    uint8_t  axisMotor2State;

} AUTOSQUARE_STATE;

// This struct represents the typical message, a client sends to the main ESP32
// Max size of this struct is 32 bytes - buffer limit
// uint16_t : 2Bytes
// unsigned : 1 Bit each
typedef struct {
    byte         softwareVersion;
    uint16_t     joystickX;
    uint16_t     joystickY;
    uint16_t     joystickZ;
    uint16_t     feedrate;
    uint16_t     rotationSpeed;
    unsigned     autosquare : 1;
    unsigned     ena : 1;
    unsigned     selectAxisX : 1;
    unsigned     selectAxisY : 1;
    unsigned     selectAxisZ : 1;
    unsigned     ok : 1;
    unsigned     programStart : 1;
    unsigned     motorStart : 1;
    unsigned     speed1 : 1;
    unsigned     speed2 : 1;
    unsigned     output1 : 1;
    unsigned     output2 : 1;
    unsigned     output3 : 1;
    unsigned     output4 : 1;
    DATA_COMMAND command;
} DATA_TO_CONTROL;

// This struct represents the typical message, the main ESP32 sends to a client
// Max size of this struct is 250
// uint16_t : 2Bytes
// unsigned : 1 Bit each
typedef struct {
    byte             softwareVersion;
    int              temperatures[5];
    unsigned         autosquareRunning : 1;
    unsigned         spindelState : 1;
    unsigned         alarmState : 1;
    unsigned         peerIgnored : 1;   // This peer is ignored by the mainboard and should stop sending data
    AUTOSQUARE_STATE autosquareState[3];
} DATA_TO_CLIENT;

// This struct describes what inputs are used for the coldend32
typedef struct {
    unsigned setPotMist : 1;
    unsigned setPotSpit : 1;
    unsigned setInMist : 1;
    unsigned setInFast : 1;
    unsigned setInAir : 1;
    unsigned returnACK : 1;       // Return ACK to client
    unsigned returnData : 1;      // Send some data back. This would be the current coldend state
    uint16_t updateInterval_MS;   // Interval in ms the client sends data to the coldend
} COLDEND_COMMAND;

typedef struct {
    unsigned coolantValve : 1;
    unsigned airValve : 1;
    unsigned spitMode : 1;
    float    mist_val;
    float    spit_val;
} DATA_COLDEND_TO_CLIENT;

typedef struct {
    byte            softwareVersion;
    uint16_t        pot_mist;
    uint16_t        pot_spit;
    unsigned        in_mist : 1;
    unsigned        in_fast : 1;
    unsigned        in_air : 1;
    COLDEND_COMMAND command;
} DATA_TO_COLDEND;

class PROTOCOL {
  public:
    void        setup();
    void        loop();
    static bool isOCS2Connected();
    static bool isColdEndConnected();
    void        initializeCommand();

    static char    *getMacStrFromAddress(uint8_t *address);
    static uint16_t getIntegerFromAddress(const uint8_t *address);

    static uint16_t failedOCS2MessagesSuccessively;
    static uint16_t successfulOCS2Messages;
    static uint16_t failedOCS2Messages;

    static uint16_t failedColdEndMessagesSuccessively;
    static uint16_t successfulColdEndMessages;
    static uint16_t failedColdEndMessages;

    static bool hasOCS2Functions, hasColdEndFunctions;

  private:
    void sendMessageToController();
    void sendMessageToColdEnd();

    static void onDataSent(const uint8_t *address, esp_now_send_status_t status);
    static void onDataRecv(const uint8_t *address, const uint8_t *incomingData, int len);

    uint32_t lastOCS2MessageSent_MS    = 0;
    uint32_t lastColdEndMessageSent_MS = 0;

    uint32_t lastStatusMessage_MS = 0;

    static void  loopTask(void *pvParameters);
    TaskHandle_t loopTaskHandle;
};

// Global variables
extern PROTOCOL               protocol;
extern DATA_TO_CONTROL        dataToControl;
extern DATA_TO_CLIENT         dataToClient;
extern DATA_TO_COLDEND        dataToColdEnd;
extern DATA_COLDEND_TO_CLIENT dataFromColdEnd;
