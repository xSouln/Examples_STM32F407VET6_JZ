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
* Filename: Server_Buffer.h
* Author:   Chris Cowdery 09/10/2019
* Purpose:  Manages a buffer for server-bound messages.
*
**************************************************************************/
#ifndef __SERVER_BUFFER_H__
#define __SERVER_BUFFER_H__

#include "../MQTT/MQTT.h"
#include "hermes.h"

typedef struct
{
    uint8_t*	message_ptr;   // pointer to message as text string.
    uint64_t	source_mac;    // source MAC of Device from which this message originated. We'll derive the sub-topic from this.
} SERVER_MESSAGE;

#define MAX_MESSAGE_SIZE_SERVER_BUFFER 512

#define PRINT_SERVER_BUFFER	false

#if PRINT_SERVER_BUFFER
#define sbuffer_printf(...)	zprintf(TERMINAL_IMPORTANCE, __VA_ARGS__)
#else
#define sbuffer_printf(...)
#endif

void			server_buffer_init(void);  // initialise our buffer
bool			server_buffer_add(SERVER_MESSAGE *message);   // Add this message and topic
bool			server_buffer_process_reflected_message(char *message);
MQTT_MESSAGE*	server_buffer_get_next_message(void);
void			server_buffer_dump();  // dump valid messages out of the UART
#endif
