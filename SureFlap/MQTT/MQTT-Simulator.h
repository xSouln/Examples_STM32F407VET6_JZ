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
* Filename: MQTT-Simulator.c   
* Author:   Nailed Barnacle
* Purpose:   
*   
*            
**************************************************************************/

#ifndef __MQTT_SIMULATOR_H__
#define __MQTT_SIMULATOR_H__

#include "message_parser.h"
#include "RegisterMap.h"

typedef struct
{
	char utc[10];			// time - 8 characters and space
	char hdr[6];			// header - 4 characters and space (always '1000 ' from server)
	char msg[3];			// message - single character and space
	char addval[256];		// address and value - up to three chars each, space between, variable
} S_MSG_SET_ONE_REG;

typedef struct
{
	char server_time[10];	// the server's utc as eight hex chars
	char hdr[6];			// four chars and space
	char tmsg[5];			// always '127 ' 
	char type[7];			// message type; '00 00 ' for an ack
	char corr[7];			// two hex chars, with spaces
	char utc[13];			// four hex chars, with space
	char msg[7];			// two hex chars, with spaces
	char err[3];			// hex pair error code
} S_MSG_THALAMUS_ACK;
	
typedef struct
{
	char server_time[10];	// the server's utc as eight hex chars
	char hdr[6];			// four chars and space
	char tmsg[5];			// always '127 ' 
	char type[7];			// message type; '00 00 ' for an ack
	char corr[7];			// two hex chars, with spaces
	char utc[13];			// four hex chars, with space
	char msg[255];			// the message - there is no length code but \0 terminated
} S_MSG_THALAMUS_MSG;
	
	

typedef union
{
	S_MSG_SET_ONE_REG		set_one_reg;
	S_MSG_THALAMUS_ACK		thal_ack;
	S_MSG_THALAMUS_MSG		thal_msg;
} U_MESSAGES;

void MQTT_ping_device_message (uint64_t mac, int8_t type);
void MQTT_set_register_message (MSG_TYPE msg_type, HUB_REGISTER_TYPE reg, uint8_t val);
void MQTT_set_training_mode_3 (uint64_t mac);
void MQTT_request_profiles (uint64_t mac);
void MQTT_request_settings (uint64_t mac);
void MQTT_request_curfews (uint64_t mac);
void MQTT_set_fastrf (uint64_t mac);
void MQTT_get_info (uint64_t mac);
void MQTT_set_setting (uint64_t mac, uint16_t setting, int32_t value);
void MQTT_generic_device_msg (uint64_t mac, uint16_t type, char * pcParameter);
void MQTT_petdoor_device_msg (uint64_t mac, uint16_t type, char * pcParameter);
void MQTT_generic_hub_msg (char * pcParameter);

void MQTT_Simulator_task(void *pvParameters);

#endif
