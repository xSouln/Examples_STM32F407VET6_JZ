//==============================================================================
//includes:

#include <stdlib.h>
#include "Abstractions/xSystem/xSystem.h"
#include "RelayService-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void privateHandler(RelayServiceT* service)
{
	RelayServiceAdapterT* adapter = service->Adapter.Content;

	uint32_t totalTime = xSystemGetTime(service);

	if (adapter->Internal.TimeStamp - totalTime > 500)
	{
		adapter->Internal.TimeStamp = totalTime;

		//service->Relay = 10.0f + (float)(rand() & 0x3fff) / 1000;
	}
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(RelayServiceT* service, int selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//==============================================================================
//initializations:

static RelayServiceAdapterInterfaceT privateInterface =
{
	.Handler = (RelayServiceAdapterHandlerT)privateHandler,
	.RequestListener = (RelayServiceAdapterRequestListenerT)privateRequestListener
};
//------------------------------------------------------------------------------
xResult RelayServiceAdapterInit(RelayServiceT* service,
		RelayServiceAdapterT* adapter,
		RelayServiceAdapterInitT* init)
{
	if (service && init)
	{
		service->Adapter.Content = adapter;
		service->Adapter.Interface = &privateInterface;
		service->Adapter.Description = nameof(RelayServiceAdapterT);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
