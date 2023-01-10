/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright � 2013-2021 Sureflap Limited.
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
* Filename: SureNetDriver.h
* Author:   Chris Cowdery 
* 
* SureNet Driver top level file - effectively wraps Atmel Stack.
* It provides the following facilities:
* 1. Association with Devices
* 2. Sending and receiving messages to / from devices.
* We access the SPI via a FreeRTOS API, but the nRST, INT and SP signals are controlled directly
*           
**************************************************************************/

#ifndef __SURENETDRIVER_H__
#define __SURENETDRIVER_H__

#include "Components.h"
#include "FreeRTOS.h"   // has some standard definitions
#include "SureNet-Interface.h"

typedef struct  // used to pass a buffer to be transmitted
{
    uint8_t *pucTxBuffer;   // transmit buffer pointer
    uint64_t uiDestAddr;    // destination address
    uint8_t ucBufferLength;
    bool xRequestAck;   // set bit 5 of FCF to 1 if we want an ACK from the other end.
} TX_BUFFER;

// These #defines are used to create the Beacon Payload
#define SUREFLAP_HUB                    0x7e
#define HUB_SUPPORTS_THALAMUS           0x02
#define HUB_DOES_NOT_SUPPORT_THALAMUS   0x01
#define HUB_IS_SOLE_PAN_COORDINATOR     0x00

BaseType_t snd_init(uint64_t *mac_addr, uint16_t panid, uint8_t channel);
bool set_beacon_request_data(uint64_t mac_address, uint8_t src_address_mode, uint8_t data); // called by MAC with payload from Beacon Request message from device
uint16_t fcs_calculate(uint8_t *data, uint8_t len);
void snd_stack_init(void);
void snd_stack_task(void);
void snd_pairing_mode(PAIRING_REQUEST pairing);
void snd_set_channel(uint8_t ucChannel);
uint8_t snd_get_channel(void);
bool snd_transmit_packet(TX_BUFFER *pcTxBuffer);
bool snd_have_we_seen_beacon(uint64_t mac_address);
#endif
