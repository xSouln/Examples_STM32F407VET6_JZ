/*
 * File:   AWS.h
 * Author: tom monkhouse
 *
 * Created on 01 July 2019, 16:06
 */

#ifndef AWS_H
#define	AWS_H

#include <stdint.h>
#include <stdbool.h>

#include "aws_iot_mqtt_client.h"
#include "aws_iot_config.h"
#include "aws_iot_mqtt_client_interface.h"

#include "credentials.h"
#include "hermes-time.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define PRINT_AWS	false
#define PRINT_CREDS	false

#define AWS_KEEP_ALIVE_TIMEOUT  90 //60  // In seconds
#define AWS_YIELD_TIMEOUT       5 //100 // In milliseconds

#define AWS_COMMAND_TIMEOUT				1 //10 //5000 //40000
#define AWS_CONNECT_COMMAND_TIMEOUT		100
#define AWS_SUBSCRIBE_COMMAND_TIMEOUT	1000
#define AWS_PUBLISH_COMMAND_TIMEOUT		1000
#define AWS_READ_COMMAND_TIMEOUT		1
#define AWS_HANDSHAKE_TIMEOUT			5000

#define AWS_LIFE_TIMEOUT				(60 * usTICK_SECONDS) // Time to send "keep alive" cache messages.

#define AWS_MAX_RECEIVED_MESSAGES		8

#if PRINT_AWS
#define aws_printf(...)		zprintf(MEDIUM_IMPORTANCE, __VA_ARGS__)
#else
#define aws_printf(...)
#endif

#if PRINT_CREDS
#define creds_printf(...)		zprintf(MEDIUM_IMPORTANCE, __VA_ARGS__)
#else
#define creds_printf(...)
#endif

#define AWS_USE_MSG_LEN		-1

IoT_Error_t AWS_Init(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials);
IoT_Error_t AWS_Connect(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials, const char* will_message, bool clean_connect);
IoT_Error_t AWS_Subscribe(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials);
IoT_Error_t AWS_Resubscribe(AWS_IoT_Client* client);
IoT_Error_t AWS_Publish(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials, char* sub_topic, char* message, int32_t message_len, QoS qos);

void AWS_Message_Received(AWS_IoT_Client* pClient, char* pTopicName, uint16_t topicNameLen, IoT_Publish_Message_Params* pParams, void* pClientData);

#ifdef	__cplusplus
}
#endif

#endif	/* AWS_H */

