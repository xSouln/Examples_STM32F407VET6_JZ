#include <stdio.h>

#include "AWS.h"
#include "MQTT.h"

#include "aws_iot_error.h"
#include "aws_iot_log.h"

#include "wolfssl/wolfcrypt/error-crypt.h"
#include "wolfssl/wolfcrypt/pkcs12.h"
#include "wolfssl/wolfcrypt/coding.h"

// Starfield Class 2 Root:
const char* starfield_fixed_ca_cert = "-----BEGIN CERTIFICATE-----\nMIIEDzCCAvegAwIBAgIBADANBgkqhkiG9w0BAQUFADBoMQswCQYDVQQGEwJVUzEl\nMCMGA1UEChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMp\nU3RhcmZpZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDQw\nNjI5MTczOTE2WhcNMzQwNjI5MTczOTE2WjBoMQswCQYDVQQGEwJVUzElMCMGA1UE\nChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMpU3RhcmZp\nZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwggEgMA0GCSqGSIb3\nDQEBAQUAA4IBDQAwggEIAoIBAQC3Msj+6XGmBIWtDBFk385N78gDGIc/oav7PKaf\n8MOh2tTYbitTkPskpD6E8J7oX+zlJ0T1KKY/e97gKvDIr1MvnsoFAZMej2YcOadN\n+lq2cwQlZut3f+dZxkqZJRRU6ybH838Z1TBwj6+wRir/resp7defqgSHo9T5iaU0\nX9tDkYI22WY8sbi5gv2cOj4QyDvvBmVmepsZGD3/cVE8MC5fvj13c7JdBmzDI1aa\nK4UmkhynArPkPw2vCHmCuDY96pzTNbO8acr1zJ3o/WSNF4Azbl5KXZnJHoe0nRrA\n1W4TNSNe35tfPe/W93bC6j67eA0cQmdrBNj41tpvi/JEoAGrAgEDo4HFMIHCMB0G\nA1UdDgQWBBS/X7fRzt0fhvRbVazc1xDCDqmI5zCBkgYDVR0jBIGKMIGHgBS/X7fR\nzt0fhvRbVazc1xDCDqmI56FspGowaDELMAkGA1UEBhMCVVMxJTAjBgNVBAoTHFN0\nYXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xMjAwBgNVBAsTKVN0YXJmaWVsZCBD\nbGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8w\nDQYJKoZIhvcNAQEFBQADggEBAAWdP4id0ckaVaGsafPzWdqbAYcaT1epoXkJKtv3\nL7IezMdeatiDh6GX70k1PncGQVhiv45YuApnP+yz3SFmH8lU+nLMPUxA2IGvd56D\neruix/U0F47ZEUD0/CwqTRV/p2JdLiXTAAsgGh1o+Re49L2L7ShZ3U0WixeDyLJl\nxy16paq8U4Zt3VekyvggQQto8PT7dL5WXXp59fkdheMtlb71cZBDzI0fmgAKhynp\nVSJYACPq4xJDKVtHCN2MQWplBqjlIapBtJUhlbl90TSrE9atvNziPTnNvT51cKEY\nWQPJIrSPnNVeKtelttQKbfi3QBFGmh95DmK/D5fs4C8fF5Q=\n-----END CERTIFICATE-----\n";

static char	aws_wildcard_topic[]	= "v2/production/00000000-0000-0000-0000-000000000000/#\0\0\0\0\0\0\0\0\0";
static char	aws_messages_topic[]	= "v2/production/00000000-0000-0000-0000-000000000000/messages\0\0\0\0\0\0\0\0\0";
static char	aws_child_topic[]		= "v2/production/00000000-0000-0000-0000-000000000000/messages/XXXXXXXXXXXXXXXX\0\0\0\0\0\0\0\0\0";

#if AWS_INTERFACE_DEBUG
static const char* mqtt_message_actions[] =
{
	"No Action",
	"Reflected Message Dropped",
	"Version Info Sent",
	"Repogram Countdown Begun",
	"Flash Set",
	"Rebooting",
	"Hub Set Reg",
	"Hub Send Reg",
	"Hub Debug Mode",
	"Hub Message Ignored",
	"Thalamus Message Forwarded",
	"Thalamus Error: Wrong Size",
	"Thalamus Error: Wrong Device",
	"Device Error: Wrong Size",
	"Device Set Reg",
	"Device Send Reg",
	"Device Message Ignored"
};
#endif

IoT_Error_t AWS_Init(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials)
{
	IoT_Error_t ret = SUCCESS;
	IoT_Client_Init_Params client_init_params = iotClientInitParamsDefault;

	aws_printf("\tAWS Initialising.\r\n");

	client_init_params.enableAutoReconnect = false;
	client_init_params.pHostURL = credentials->host;
	client_init_params.port = AWS_IOT_MQTT_PORT;
	client_init_params.pRootCALocation = (char*)starfield_fixed_ca_cert; // Auto-filled in network_tls_wrapper.
	client_init_params.pDeviceCertLocation = (char*)credentials;
	client_init_params.pDevicePrivateKeyLocation = (char*)credentials;
	client_init_params.mqttCommandTimeout_ms = AWS_COMMAND_TIMEOUT; // TAM Default was 20,000, think Xively was 1500.
	client_init_params.tlsHandshakeTimeout_ms = AWS_HANDSHAKE_TIMEOUT;
	client_init_params.isSSLHostnameVerify = true; 
	client_init_params.disconnectHandler = NULL; // Disconnects fully handled by the yield return.
	client_init_params.disconnectHandlerData = NULL;

	ret = aws_iot_mqtt_init(client, &client_init_params);

	sprintf(aws_messages_topic, "%s/messages", credentials->base_topic);
	sprintf(aws_wildcard_topic, "%s/#", credentials->base_topic);
	sprintf(aws_child_topic, "%s/messages/XXXXXXXXXXXXXXXX", credentials->base_topic);

	client->clientData.messageHandlers[0].topicName = aws_wildcard_topic;
	client->clientData.messageHandlers[0].topicNameLen = strlen(aws_wildcard_topic);
	client->clientData.messageHandlers[0].pApplicationHandler = AWS_Message_Received;
	client->clientData.messageHandlers[0].pApplicationHandlerData = NULL;
	client->clientData.messageHandlers[0].qos = QOS1;

	if( SUCCESS != ret )
	{
		aws_printf("\tFailed to initialise AWS.\r\n");
	}

	return ret;
}

IoT_Error_t AWS_Connect(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials,
						const char* will_message, bool clean_connect)
{
	IoT_Client_Connect_Params connect_params = iotClientConnectParamsDefault;

	IoT_Error_t ret = AWS_Init(client, credentials);
	if( SUCCESS != ret )
	{
		aws_printf("\tError (%d) initialising.\r\n", ret);
		return ret;
	}

	connect_params.MQTTVersion = MQTT_3_1_1;
	connect_params.pClientID = credentials->client_id;
	connect_params.clientIDLen = strlen(credentials->client_id);
	connect_params.keepAliveIntervalInSec = AWS_KEEP_ALIVE_TIMEOUT;
	connect_params.isCleanSession = clean_connect;
	if( NULL != will_message )
	{
		connect_params.isWillMsgPresent = true;
		connect_params.will.pTopicName = aws_messages_topic;
		connect_params.will.topicNameLen = strlen(aws_messages_topic);
		connect_params.will.pMessage = (char*)will_message;
		connect_params.will.msgLen = strlen(will_message);
		connect_params.will.isRetained = false;
		connect_params.will.qos = QOS1;
	} else
	{
		connect_params.isWillMsgPresent = false;
	}
	// Username and Password not used in AWS IoT, but still included in Connection Params.
	connect_params.pUsername = credentials->username;
	connect_params.usernameLen = strlen(credentials->username);
	connect_params.pPassword = credentials->password;
	connect_params.passwordLen = strlen(credentials->password);

	client->clientData.commandTimeoutMs = AWS_CONNECT_COMMAND_TIMEOUT;
	ret = aws_iot_mqtt_connect(client, &connect_params);
	client->clientData.commandTimeoutMs = AWS_COMMAND_TIMEOUT;

	aws_printf("\r\n\t@@@ AWS Connect @@@\r\n\t@\tClient ID:\t\t%s\r\n\t@\tLast Will:\tTopic:\t\t%s\r\n\t@\t\t\tMessage:\t%s\r\n\t@\tResult:\t\t%d\r\n\t@@@@@@@@@@@@@@@@@@@\r\n", credentials->client_id, connect_params.will.pTopicName, connect_params.will.pMessage, (int)ret);

	if( SUCCESS != ret )
	{
		IOT_ERROR("Error (%d) connecting %s.\n\r", ret, client_id);
	} else
	{
		client->networkStack.tlsDataParams.connected = true;
	}

	return ret;
}

IoT_Error_t AWS_Subscribe(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials)
{
	client->clientData.commandTimeoutMs = AWS_SUBSCRIBE_COMMAND_TIMEOUT;
	IoT_Error_t result = aws_iot_mqtt_subscribe(client, aws_wildcard_topic, strlen(aws_wildcard_topic), QOS1, &AWS_Message_Received, NULL);
	client->clientData.commandTimeoutMs = AWS_COMMAND_TIMEOUT;
	aws_printf("\r\n\t+++ AWS Subscription +++\r\n\t+\tTopic:\t%s\r\n\t+\tResult:\t%d\r\n\t++++++++++++++++++++++++\r\n", aws_wildcard_topic, (int)result);

	return result;
}

IoT_Error_t AWS_Resubscribe(AWS_IoT_Client* client)
{
	client->clientData.commandTimeoutMs	= AWS_SUBSCRIBE_COMMAND_TIMEOUT;
	IoT_Error_t result = aws_iot_mqtt_resubscribe(client);
	client->clientData.commandTimeoutMs = AWS_COMMAND_TIMEOUT;
	aws_printf("\r\n\t??? AWS Resubscription ???\r\n\t?\tResult:\t%d\r\n\t????????????????????????\r\n", (uint32_t)result);
	return result;
}

IoT_Error_t AWS_Publish(AWS_IoT_Client* client, SUREFLAP_CREDENTIALS* credentials, char* sub_topic, char* message, int32_t message_len, QoS qos)
{
	IoT_Publish_Message_Params publish_params;
	publish_params.qos = qos;
	publish_params.isRetained = false; // Always false with AWS.
	publish_params.payload = message;

	if(	message_len < 0 )
	{
		message_len = strlen(message);
	}
	publish_params.payloadLen = message_len;

	sprintf(aws_child_topic, "%s/messages%s\0", credentials->base_topic, sub_topic); // Sub-topic must start with '/' or '\0'
	client->clientData.commandTimeoutMs = AWS_PUBLISH_COMMAND_TIMEOUT;
	IoT_Error_t result = aws_iot_mqtt_publish(client, aws_child_topic, strlen(aws_child_topic), &publish_params);
	client->clientData.commandTimeoutMs = AWS_COMMAND_TIMEOUT;

	aws_printf("\r\n\t>>> AWS Publish >>>\r\n\t>\tTopic:\t\t%s\r\n\t>\tMessage:\t%.*s\r\n\t>\tResult:\t\t%d\r\n\t>>>>>>>>>>>>>>>>>>>\r\n", aws_child_topic, message_len, message, (int)result);

	return result;
}

void AWS_Message_Received(AWS_IoT_Client* pClient, char* pTopicName, uint16_t topicNameLen, IoT_Publish_Message_Params* pParams, void* pClientData)
{
	extern QueueHandle_t xIncomingMQTTMessageMailbox;
	MQTT_MESSAGE	msg;

	aws_printf("\r\n\t\t<<< AWS Message Received <<<\r\n\t\t<\tTopic:\t\t%.*s\r\n\t\t<\tMessage:\t%.*s\r\n\t\t<\tAction:\t\t?\r\n\t\t<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n",
			   topicNameLen, pTopicName,
			   pParams->payloadLen, pParams->payload);

	if( pParams->payloadLen >MAX_INCOMING_MQTT_MESSAGE_SIZE_SMALL)
	{
		zprintf(CRITICAL_IMPORTANCE,"*** Incoming MQTT message too long. Dropping it.\r\n");
		return;	// no point going further as it will fail the signature check
	}

	sprintf(msg.message, "%.*s\0", pParams->payloadLen, (char*)pParams->payload); // Fixed length string insertion with added null terminator.

	uint32_t	i;
	for( i = topicNameLen; i > 0; i-- )
	{	// Find the last sub-topic by searching backwards.
		if( pTopicName[i] == '/' )
		{	// We don't want the slash.
			i++;
			break;
		}
	}
	sprintf(msg.subtopic, "%.*s\0", topicNameLen-i, &pTopicName[i]); // Fixed length string insertion with added null terminator.

	xQueueSend(xIncomingMQTTMessageMailbox, &msg, 0);
	return;
}
