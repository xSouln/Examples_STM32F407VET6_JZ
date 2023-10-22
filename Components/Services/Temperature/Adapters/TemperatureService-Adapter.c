//==============================================================================
//includes:

#include <stdlib.h>
#include "Abstractions/xSystem/xSystem.h"
#include "TemperatureService-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void privateHandler(TemperatureServiceT* service)
{
	TemperatureServiceAdapterT* adapter = service->Adapter.Content;

	uint32_t totalTime = xSystemGetTime(service);

	if (adapter->Internal.TimeStamp - totalTime > 500)
	{
		adapter->Internal.TimeStamp = totalTime;

		service->Temperature = 10.0f + (float)(rand() & 0x3fff) / 1000;
	}
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(TemperatureServiceT* service, int selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void privateEventListener(TemperatureServiceT* service, TemperatureServiceAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//==============================================================================
//initializations:

static TemperatureServiceAdapterInterfaceT privateInterface =
{
	.Handler = (TemperatureServiceAdapterHandlerT)privateHandler,

	.RequestListener = (TemperatureServiceAdapterRequestListenerT)privateRequestListener,
	.EventListener = (TemperatureServiceAdapterEventListenerT)privateEventListener,
};
//------------------------------------------------------------------------------
xResult TemperatureServiceAdapterInit(TemperatureServiceT* service,
		TemperatureServiceAdapterT* adapter,
		TemperatureServiceAdapterInitT* init)
{
	if (service && init)
	{
		service->Adapter.Content = adapter;
		service->Adapter.Interface = &privateInterface;
		service->Adapter.Description = nameof(TemperatureServiceAdapterT);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
