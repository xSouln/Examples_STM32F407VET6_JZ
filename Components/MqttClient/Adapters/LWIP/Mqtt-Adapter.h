//==============================================================================
//header:

#ifndef _MQTT_PORT_ADAPTER_H_
#define _MQTT_PORT_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Common/xRxReceiver.h"
#include "Common/xDataBuffer.h"
#include "MQTTClient.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
	SemaphoreHandle_t TxSemaphore;
#endif

	char* RxTopic;
	char* TxTopic;

	xDataBufferT TxDataBuffer;

	mqtt_client_t* Client;

	struct
	{
		uint8_t RxTopicConfirmed : 1;
		uint8_t TxResult : 3;
	};

} MqttPortAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	char* RxTopic;
	char* TxTopic;

	uint8_t* TxBuffer;
	uint16_t TxBufferSize;

} MqttPortAdapterInitT;
//==============================================================================
//functions:

xResult MqttPortAdapterInit(xPortT* port, MqttPortAdapterT* adapter, MqttPortAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_PORT_ADAPTER_H_
