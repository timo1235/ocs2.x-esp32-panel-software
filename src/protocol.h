#pragma once

#include <Arduino.h>
#include <esp_now.h>

#define STATUS_MESSAGE_INTERVAL_MS 10000


// This struct describes what data is send and what is expected
typedef struct
{
    unsigned setJoystick : 1;
    unsigned setFeedrate : 1;
    unsigned setRotationSpeed : 1;
    unsigned setAutosquare : 1;
    unsigned setEna : 1;
    unsigned setAxisSelect : 1;
    unsigned setOk : 1;
    unsigned setProgrammStart : 1;
    unsigned setMotorStart : 1;
    unsigned setSpeed1 : 1;
    unsigned setSpeed2 : 1;
    unsigned setOutput1 : 1;
    unsigned setOutput2 : 1;
    unsigned setOutput3 : 1;
    unsigned setOutput4 : 1;
    unsigned returnACK : 1;  // Return ACK to client
    unsigned returnData : 1; // Return mainboard data to client, this can be the temperature, autosquaring state and so son and replaces ACK
    uint16_t updateInterval_MS; // Interval in ms the client sends data to the mainboard
} DATA_COMMAND;

// This struct describes the auto square data
typedef struct
{
    unsigned axisActive : 1;
    uint8_t axisMotor1State;
    uint8_t axisMotor2State;

} AUTOSQUARE_STATE;

// This struct represents the typical message, a client sends to the main ESP32
// Max size of this struct is 32 bytes - buffer limit
// uint16_t : 2Bytes
// unsigned : 1 Bit each
typedef struct
{
    byte softwareVersion;
    uint16_t joystickX;
    uint16_t joystickY;
    uint16_t joystickZ;
    uint16_t feedrate;
    uint16_t rotationSpeed;
    unsigned autosquare : 1;
    unsigned ena : 1;
    unsigned selectAxisX : 1;
    unsigned selectAxisY : 1;
    unsigned selectAxisZ : 1;
    unsigned ok : 1;
    unsigned programmStart : 1;
    unsigned motorStart : 1;
    unsigned speed1 : 1;
    unsigned speed2 : 1;
    unsigned output1 : 1;
    unsigned output2 : 1;
    unsigned output3 : 1;
    unsigned output4 : 1;
    DATA_COMMAND command;
} DATA_TO_CONTROL;

// This struct represents the typical message, the main ESP32 sends to a client
// Max size of this struct is 250
// uint16_t : 2Bytes
// unsigned : 1 Bit each
typedef struct
{
    byte softwareVersion;
    int temperatures[5];
    unsigned autosquareRunning : 1;
    unsigned spindelState : 1;
    unsigned alarmState : 1;
    unsigned peerIgnored : 1; // This peer is ignored by the mainboard and should stop sending data
    AUTOSQUARE_STATE autosquareState[3];
} DATA_TO_CLIENT;

class PROTOCOL
{
public:
    void setup();
    void loop();
    static bool isConnected();

    static uint16_t failedMessagesSuccessively;
    static uint16_t successfulMessages;
    static uint16_t failedMessages;

private:
    void sendMessageToController();
    void initializeCommand();
    static void onDataSent(const uint8_t *address, esp_now_send_status_t status);
    static void onDataRecv(const uint8_t *address, const uint8_t *incomingData, int len);
    char *getMacStrFromAddress(uint8_t *address);

    uint32_t lastMessageSent_MS = 0;
    uint32_t lastStatusMessage_MS = 0;
};
