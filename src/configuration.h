#ifndef ocs_configuration_h
#define ocs_configuration_h

// Used to invert some outputs, since estlcam has pullups on its inputs. 
#define ESTLCAM_CONTROLLER

// #define I2C_MASTER_ADDRESS 1
// #define I2C_OWN_ADDRESS 10

#define CONTROLLER_MAC_ADDRESS { 0x5E, 0x0, 0x0, 0x0, 0x0, 0x1 }

// Time in ms we wait between sending measured inputs
#define WIFI_DELAY 20

#define OCS_DEBUG

#endif