/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright © 2013-2021 Sureflap Limited.
* All Rights Reserved.
*
* All information contained herein is, and remains the property of Sureflap
* Limited.
* The intellectual and technical concepts contained herein are proprietary to
* Sureflap Limited. and may be covered by U.S. / EU and other Patents, patents
* in process, and are protected by copyright law.
* Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Sureflap
* Limited.
*
* Filename: SureNet.h
* Author:   Chris Cowdery
* 31/07/2013 - cjc - first revision
* 02/07/2019 - cjc - Massive overhaul for Hub2 project.
*
*
**************************************************************************/

#ifndef SURENET_H
#define	SURENET_H

#define	USE_RANDOM_KEY	true	// set to true to use a different truly random key when encrypting
								// RF data packets between Hub and Devices.
#define RF_CHANNEL1 15
#define RF_CHANNEL2 20
#define RF_CHANNEL3 26
#define INITIAL_CHANNEL RF_CHANNEL1

#define PAIRING_SERVER_REQUEST_LOCKOUT_TIME	(10*usTICK_SECONDS)	// if the Server requests pairing mode
	 															// within this time window, it will be
	 															// denied. This is to stop a slow server
	 															// 'echo' of a change to the pairing mode
	 															// register putting the Hub back into
	 															// pairing mode immediately after a
	 															// successful pairing.

typedef struct  // used to transfer received message between SureNet and SureNetDriver
{
    uint64_t uiSrcAddr;     // Source address
    uint64_t uiDstAddr;
    uint8_t ucBufferLength;
    uint8_t ucRSSI;
    uint8_t ucRxBuffer[127];   // Receive buffer
} RX_BUFFER;

// Surenet->Surenet-Interface EventGroup flags
#define SURENET_GET_PAIRMODE        (1<<0)
#define SURENET_GET_NUM_PAIRS       (1<<1)
#define SURENET_CLEAR_PAIRING_TABLE (1<<2)
#define SURENET_TRIGGER_CHANNEL_HOP (1<<3)
#define SURENET_GET_CHANNEL			(1<<4)
#define SURENET_GET_DEVICE_TABLE	(1<<5)
#define SURENET_GET_LAST_HEARD_FROM (1<<6)

#define	SURENET_ACTIVITY_LOG	true	// records with timestamps some SureNet activity

#if SURENET_ACTIVITY_LOG
typedef enum
{
	SURENET_LOG_NOTHING,
	SURENET_LOG_DEVICE_AWAKE_RECIVED, // 1
	SURENET_LOG_STATE_MACHINE_RESPONDED, // 2
	SURENET_LOG_END_OF_CONVERSATION, // 3
	SURENET_LOG_TIMEOUT,       // 4
	SURENET_LOG_SECURITY_NACK, // 5
	SURENET_LOG_SECURITY_ACK_TIMEOUT, // 6
	SURENET_LOG_DATA_RETRY_FAIL, // 7
	SURENET_LOG_STATE_MACHINE_SWITCHING_TO_SEND, // 8
	SURENET_LOG_SENT_SECURITY_KEY, // 9
	SURENET_LOG_SECURITY_KEY_ACKNOWLEDGED, // 10
	SURENET_LOG_DATA_SEND,	 // 11
	SURENET_LOG_DATA_ACK,    // 12
	SURENET_LOG_DATA_NACK,   // 13
	SURENET_LOG_DATA_RETRY_ATTEMPT,  // 14
	SURENET_LOG_END_OF_SESSION,	 // 15

} SURENET_LOG_ACTIVITY;


typedef struct
{
	uint32_t 				timestamp;
	SURENET_LOG_ACTIVITY	activity;
	uint8_t					parameter;
	bool					used;
} SURENET_LOG_ENTRY;

#endif

BaseType_t sn_init(uint64_t *mac, uint16_t panid, uint8_t channel);
void sn_device_pairing_success(ASSOCIATION_SUCCESS_INFORMATION *assoc_info);    // call back to say association successful
void sn_association_changed(PAIRING_REQUEST);
uint8_t sn_how_many_pairs(void);
void sn_set_hub_pairing_mode(PAIRING_REQUEST state);
PAIRING_REQUEST sn_get_hub_pairing_mode(void);
void sn_unpair_device(uint64_t mac_addr);
bool sn_is_device_online(uint64_t mac_addr);
void sn_hub_clear_pairing_table(void);
void sn_mark_transmission_complete(void);
SN_DATA_RECEIVED_RESPONSE sn_data_received(RECEIVED_PACKET *rx_packet);
bool sn_process_received_packet(RX_BUFFER *rx_buffer);
void sn_GenerateSecurityKey(uint8_t device_index);
void sn_CalculateSecretKey(uint8_t device_index);

#endif	/* SURENET_H */

