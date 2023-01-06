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
* Filename: MQTT_internal.h   
* Author:   Tom Monkhouse 01/11/2019
* Purpose:   
*   
*            
**************************************************************************/

#ifndef __MQTT_INTERNAL_H_
#define __MQTT_INTERNAL_H_

#include "AWS.h"

char *mqtt_states[]={	"MQTT_STATE_INITIAL",
						"MQTT_STATE_GET_TIME",
						"MQTT_STATE_GET_CREDENTIALS",
						"MQTT_STATE_DELAY",
						"MQTT_STATE_CONNECT",
						"MQTT_STATE_SUBSCRIBE",
						"MQTT_STATE_CONNECTED",
						"MQTT_STATE_DISCONNECT",
						"MQTT_STATE_STOP",
						"MQTT_STATE_BACKSTOP"};


#define MQTT_CRED_RETRY_BASE		1000
#define MQTT_CRED_RETRY_MULT		2
#define MQTT_CRED_RETRY_JITTER		1000
#define MQTT_CRED_RETRY_MAX			5

#define MQTT_CONNECT_RETRY_BASE		1000	// Milliseconds
#define MQTT_CONNECT_RETRY_MULT		2
#define MQTT_CONNECT_RETRY_JITTER	100		// Milliseconds
#define MQTT_CONNECT_RETRY_MAX		5

#define MQTT_SUBS_RETRY_BASE		1000	// Milliseconds
#define MQTT_SUBS_RETRY_MULT		2
#define MQTT_SUBS_RETRY_JITTER		100		// Milliseconds
#define MQTT_SUBS_RETRY_MAX			5

#define HUB_API_SERVER			"hub.api.surehub.io"

static bool		MQTT_Init(AWS_IoT_Client* client);
static bool		MQTT_Get_Credentials(SUREFLAP_CREDENTIALS* credentials);
static bool		MQTT_Connect(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials, bool clean_connect);
static bool		MQTT_Subscribe(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials);
static bool		MQTT_Poll(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials);
static bool		MQTT_Disconnect(AWS_IoT_Client* client);
static void		AWS_Hash(uint8_t* data, uint32_t size, char* result);
static uint32_t	AWS_Unpack_Credentials(SUREFLAP_CREDENTIALS* credentials);
#endif
