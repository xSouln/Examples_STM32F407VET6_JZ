//==============================================================================
//includes:

#include "Mqtt-Adapter.h"
#include "Components.h"
#include "Common/xCircleBuffer.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void privateHandler(xMqttT* mqtt)
{

}
//------------------------------------------------------------------------------
static xResult privateRequestListener(xMqttT* mqtt, xMqttRequestSelector selector, uint32_t mode, void* in, void* out)
{
	MqttAdapterT* adapter = (MqttAdapterT*)mqtt->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xMqttRequestCreate:
		{
			if (adapter->Internal.Client)
			{

			}
			FreeRTOS_mqtt_Connect(&mqttClient, &mqttConnectInfo, 1000);


			break;
		}

		default : return xResultNotSupported;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static xResult privateEventListener(xMqttT* mqtt, xMqttEventSelector selector, uint32_t mode, void* in, void* out)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return xResultAccept;
	}
}
//------------------------------------------------------------------------------
static void privateNetEventListener(xNetT* net, int eventSelector, void* subscriber, void* arg)
{
	xMqttT* mqtt = subscriber;


}
//==============================================================================
//initializations:

static xMqttInterfaceT privateInterface =
{
	.Handler = (xMqttHandlerT)privateHandler,

	.RequestListener = (xMqttRequestListenerT)privateRequestListener,
	.EventListener = (xMqttEventListenerT)privateEventListener,
};
//------------------------------------------------------------------------------
xResult MqttAdapterInit(xMqttT* mqtt, MqttAdapterT* adapter, MqttAdapterInitT* init)
{
	if (mqtt && init)
	{
		mqtt->Adapter.Description = nameof(MqttPortAdapterT);
		mqtt->Adapter.Content = adapter;
		mqtt->Adapter.Interface = &privateInterface;

#ifdef INC_FREERTOS_H

#endif

		adapter->Net = init->Net;

		adapter->Internal.EventSubscriber.Context = mqtt;
		adapter->Internal.EventSubscriber.EventListener = privateNetEventListener;

		xNetAddEventListener(&adapter->Net, &adapter->Internal.EventSubscriber);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
