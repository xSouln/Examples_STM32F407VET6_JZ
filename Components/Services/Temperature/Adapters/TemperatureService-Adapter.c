//==============================================================================
//includes:

#include "TemperatureService-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void PrivateHandler(TemperatureServiceT* service)
{

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(TemperatureServiceT* service, TemperatureServiceAdapterRequestSelector selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(TemperatureServiceT* service, TemperatureServiceAdapterEventSelector selector, void* arg)
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
	.Handler = (TemperatureServiceAdapterHandlerT)PrivateHandler,

	.RequestListener = (TemperatureServiceAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (TemperatureServiceAdapterEventListenerT)PrivateEventListener,
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
