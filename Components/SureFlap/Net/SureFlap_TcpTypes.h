//==============================================================================
#ifndef _SUREFLAP_TCP_TYPES_H
#define _SUREFLAP_TCP_TYPES_H
//------------------------------------------------------------------------------
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap/SureFlap_ComponentTypes.h"
#include "SureFlap_Config.h"
//==============================================================================
//defines:


//==============================================================================
//types:

typedef enum
{
	SUREFLAP_TCP_MSG_NONE = 0,
	SUREFLAP_TCP_MSG_SET_ONE_REG = 1, //WEB sends a single register value
	SUREFLAP_TCP_MSG_SET_REG_RANGE = 2, //WEB sends a range of register values
	SUREFLAP_TCP_MSG_SEND_REG_RANGE = 3, //WEB asks the HUB/DEVICE to send a range of register values
	SUREFLAP_TCP_MSG_REG_VALUES = 4,  //HUB/DEVICE reports a range of register values to WEB
	SUREFLAP_TCP_MSG_SEND_REG_DUMP = 5,  //WEB tels HUB/DEVICE to send a full register dump (to register queue)
	SUREFLAP_TCP_MSG_REPROGRAM_HUB = 6,  //WEB tels HUB to reprogram itself
	SUREFLAP_TCP_MSG_FULL_REG_SET = 7,  //WEB sends full set of register values for HUB/DEVICE to use
	SUREFLAP_TCP_MSG_MOVEMENT_EVENT = 8,
	SUREFLAP_TCP_MSG_GET_REG_RANGE = 9,  //WEB sends a range of register values
	SUREFLAP_TCP_MSG_HUB_UPTIME = 10,
	SUREFLAP_TCP_MSG_FLASH_SET = 11,
	SUREFLAP_TCP_MSG_HUB_REBOOT = 12,
	SUREFLAP_TCP_MSG_HUB_VERSION_INFO = 13,
	SUREFLAP_TCP_MSG_WEB_PING = 14,  //ping from web
	SUREFLAP_TCP_MSG_HUB_PING = 15,  //also sent on channel change
	SUREFLAP_TCP_MSG_LAST_WILL = 16,
	SUREFLAP_TCP_MSG_SET_DEBUG_MODE = 17,
	SUREFLAP_TCP_MSG_DEBUG_DATA = 18,
	SUREFLAP_TCP_MSG_CHANGE_SIGNING_KEY = 20,
	SUREFLAP_TCP_MSG_HUB_THALAMUS = 127,  //NB used in both directions
	SUREFLAP_TCP_MSG_HUB_THALAMUS_MULTIPLE = 126,  //NB currently only used from hiub to Xively
	SUREFLAP_TCP_MSG_REG_VALUES_INDEX_ADDED = 0x84     // This is magic - it is 0x80 | MSG_REG_VALUES

} SureFlapTCP_MessageTypes;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_MQTT_STATE_INITIAL,
	SUREFLAP_MQTT_STATE_GET_TIME,
	SUREFLAP_MQTT_STATE_GET_CREDENTIALS,
	SUREFLAP_MQTT_STATE_DELAY,
	SUREFLAP_MQTT_STATE_CONNECT,
	SUREFLAP_MQTT_STATE_SUBSCRIBE,
	SUREFLAP_MQTT_STATE_CONNECTED,
	SUREFLAP_MQTT_STATE_DISCONNECT,
	SUREFLAP_MQTT_STATE_STOP,
	SUREFLAP_MQTT_STATE_BACKSTOP

} SureFlapMQTT_States;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_MQTT_MESSAGE_FAILED_A_FEW_TIMES,
	SUREFLAP_MQTT_MESSAGE_FAILED_TOO_MANY_TIMES,

} SureFlapMQTT_Alarms;
//------------------------------------------------------------------------------

typedef struct
{
    char Data[SUREFLAP_MQTT_INCOMING_MESSAGE_SIZE_SMALL];
    char Subtopic[SUREFLAP_MQTT_INCOMING_TOPIC_SIZE];

} SureFlapMQTT_MassageT;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_TCP_BACKOFF_READY,
	SUREFLAP_TCP_BACKOFF_WAITING,
	SUREFLAP_TCP_BACKOFF_FAILED,
	SUREFLAP_TCP_BACKOFF_FINAL_ATTEMPT

} SureFlapTcpBackoffResults;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t	BaseRetryIntervalMs;
	uint32_t	RetryMultiBase;
	uint32_t	RetryJitterMaxMs;
	uint8_t		MaxRetries;

} SureFlapTcpBackoffSpecsT;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t	LastRetryTime;
	uint32_t	RetryDelayMs;
	uint8_t		Retryes;

	const SureFlapTcpBackoffSpecsT*	Specs;

} SureFlapTcpBackoffT;
//------------------------------------------------------------------------------

typedef struct
{
	// most economical way of storing the topic. Will be NULL for Hub sourced messages
	uint64_t MAC;

	// UTC when this message entered the buffer
	uint32_t EntryTimestamp;

	// Details required for retries and backing off publish attempts.
	SureFlapTcpBackoffT Backoff;

	// note index is a magic number - setting it to 0 marks this entry as free
	// ID of this message, given when it entered the buffer. Used for removing messages when reflection is received from server
	uint8_t Id;

	uint8_t Data[SUREFLAP_SERVER_MESSAGE_BUFFER_SIZE];

	struct
	{
		uint8_t InProcessing : 1;
	};

} SureFlapTcpMessageT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	SureFlapTcpMessageT MessagesFromDevices[SUREFLAP_SERVER_BUFFER_ENTRIES];

} SureFlapTcpT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_TCP_TYPES_H
#endif //SUREFLAP_COMPONENT_ENABLE
