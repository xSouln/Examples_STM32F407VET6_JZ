//==============================================================================
#ifndef _SUREFLAP_CONFIG_H
#define _SUREFLAP_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Config.h"
//==============================================================================
//defines:

#define SUREFLAP_NUMBER_OF_DEVICES 					60

#define SUREFLAP_DEVICE_OFFLINE_TIMEOUT 			180000

// set to true to use a different truly random key when encrypting
// RF data packets between Hub and Devices.
#define	SUREFLAP_DEVICE_SECURITY_USE_RANDOM_KEY		true
#define SUREFLAP_DEVICE_SECURITY_KEY_SIZE			16

#define SUREFLAP_ZIGBEE_MAX_PAYLOAD_SIZE  			256
#define SUREFLAP_ZIGBEE_PAN_ID						0x3421
#define SUREFLAP_ZIGBEE_RF_CHANNEL1 				15
#define SUREFLAP_ZIGBEE_RF_CHANNEL2 				20
#define SUREFLAP_ZIGBEE_RF_CHANNEL3 				26
#define SUREFLAP_ZIGBEE_INITIAL_CHANNEL 			SUREFLAP_ZIGBEE_RF_CHANNEL1
#define SUREFLAP_ZIGBEE_CHANNELS_COUNT 				27
#define SUREFLAP_ZIGBEE_BEACON_PAILOAD_SIZE			3
#define SUREFLAP_ZIGBEE_DEFAULT_TRANSMITTER_POWER	4
#define SUREFLAP_ZIGBEE_MAX_PACKET_SIZE				127
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_CONFIG_H
