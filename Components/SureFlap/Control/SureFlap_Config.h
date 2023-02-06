//==============================================================================
#ifndef _SUREFLAP_CONFIG_H
#define _SUREFLAP_CONFIG_H
//------------------------------------------------------------------------------
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

//==============================================================================
//defines:

#define SUREFLAP_NUMBER_OF_DEVICES 					60

// default is every hour
#define SUREFLAP_HUB_SEND_TIME_UPDATES_FROM_DEVICES_EVERY_MINUTE		0x01
// default is every hour
#define SUREFLAP_HUB_SEND_HUB_STATUS_UPDATES_EVERY_MINUTE				0x02
#define SUREFLAP_HUB_SEND_TIME_UPDATES_FROM_DEVICES						0x04
//------------------------------------------------------------------------------
//SureFlap_Device block:
#define SUREFLAP_DEVICE_OFFLINE_TIMEOUT 			180000

// set to true to use a different truly random key when encrypting
// RF data packets between Hub and Devices.
#define	SUREFLAP_DEVICE_SECURITY_USE_RANDOM_KEY		true
#define SUREFLAP_DEVICE_SECURITY_KEY_SIZE			16
//------------------------------------------------------------------------------
//SureFlap_Zigbee block:

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
#define SUREFLAP_ZIGBEE_RX_SEQUENCE_BUFFER_SIZE		5
#define SUREFLAP_ZIGBEE_ACKNOWLEDGE_QUEUE_SIZE		16
#define SUREFLAP_ZIGBEE_DATA_ACKNOWLEDGE_QUEUE_SIZE	16
#define SUREFLAP_ZIGBEE_ACKNOWLEDGE_LIFETIME  		2000
#define SUREFLAP_ZIGBEE_DATA_ACKNOWLEDGE_LIFETIME  	2000
#define SUREFLAP_ZIGBEE_MAX_BAD_KEY					8
//#define SUREFLAP_ZIGBEE_ASSOCIATION_BUFFER_SIZE	16

//88+4  88 should match MAX_RF_PAYLOAD in global.h, 4 is header size (address and count)
#define SUREFLAP_ZIGBEE_MESSAGE_PAYLOAD_SIZE 92
#define SUREFLAP_ZIGBEE_MESSAGE_HEADER_SIZE	4
#define SUREFLAP_ZIGBEE_MESSAGE_PARITY_SIZE	1

//------------------------------------------------------------------------------
//SureFlap_Tcp block:

#define SUREFLAP_SERVER_BUFFER_ENTRIES 8
#define SUREFLAP_SERVER_MESSAGE_BUFFER_SIZE (30 + SUREFLAP_ZIGBEE_MESSAGE_PAYLOAD_SIZE * 3)

// Large enough for Thalamus messages + 64byte sig
#define SUREFLAP_MQTT_INCOMING_MESSAGE_SIZE_SMALL 	512

// Could be up to 82 chars.
#define SUREFLAP_MQTT_INCOMING_TOPIC_SIZE 			100
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_CONFIG_H
#endif //SUREFLAP_COMPONENT_ENABLE
