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
#include "Abstractions/xPort/xPort.h"
#include "Abstractions/xNet/xNet.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS-Plus-MQTT/include/core_mqtt.h"
#include "Abstractions/xMQTT/xMQTT.h"
//==============================================================================
//defines:


//==============================================================================
//types:

typedef enum
{
	MqttPortProviderGetOptions,
	MqttPortProviderSetOptions,

	MqttPortProviderSaveOptions

} MqttPortProviderFunctions;
//------------------------------------------------------------------------------

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
	SemaphoreHandle_t TxSemaphore;
#endif

	TransportInterface_t TransportInterface;
	NetworkContextT NetworkContext;

	MQTTContext_t MQTTContext;
	xSocket_t Socket;

	uint32_t TxAttempts;
	uint32_t RxAttempts;

	MQTTFixedBuffer_t MQTTFixedBuffer;

	xDataBufferT TxDataBuffer;
	uint8_t ConnectState;

} MqttPortAdapterInternalT;
//------------------------------------------------------------------------------
typedef struct
{
	MqttPortAdapterInternalT Internal;

	xMqttPortOptionsT Options;

} MqttPortAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	char* RxTopic;
	char* TxTopic;
	char* ClientId;

	uint8_t* MqttBuffer;
	uint16_t MqttBufferSize;

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
