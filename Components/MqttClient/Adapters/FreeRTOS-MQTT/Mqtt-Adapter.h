//==============================================================================
//header:

#ifndef _MQTT_ADAPTER_H_
#define _MQTT_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xNet/xNet.h"
#include "Abstractions/xMQTT/xMQTT.h"

#include "MQTTClient.h"
//==============================================================================
//types:

typedef struct
{
	xMqttEventSubscriberT EventSubscriber;

} MqttAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
#ifdef INC_FREERTOS_H

#endif
	MqttAdapterInternalT Internal;

	xNetT* Net;

} MqttAdapterT;
//------------------------------------------------------------------------------

typedef struct
{
	xNetT* Net;

} MqttAdapterInitT;
//==============================================================================
//functions:

xResult MqttAdapterInit(xMqttT* mqtt, MqttAdapterT* adapter, MqttAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_ADAPTER_H_
