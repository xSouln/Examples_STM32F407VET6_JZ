//==============================================================================
//header:

#ifndef _MQTT_CLIENT_ADAPTER_H_
#define _MQTT_CLIENT_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xNet/xNet.h"
#include "Abstractions/xMQTT/xMQTT.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS-Plus-MQTT/include/core_mqtt.h"
//==============================================================================
//types:

//------------------------------------------------------------------------------

typedef struct
{
	TransportInterface_t TransportInterface;
	NetworkContextT NetworkContext;

	MQTTContext_t MQTTContext;
	xSocket_t Socket;

	uint32_t TxAttempts;
	uint32_t RxAttempts;

} MqttClientAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
#ifdef INC_FREERTOS_H

#endif
	MqttClientAdapterInternalT Internal;

	char* Id;
	uint32_t Address;
	uint16_t Port;

	MQTTFixedBuffer_t MQTTFixedBuffer;

} MqttClientAdapterT;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t* Buffer;
	uint16_t BufferSize;

} MqttClientAdapterInitT;
//==============================================================================
//functions:

xResult MqttClientAdapterInit(xMqttT* mqtt, MqttClientAdapterT* adapter, MqttClientAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_CLIENT_ADAPTER_H_
