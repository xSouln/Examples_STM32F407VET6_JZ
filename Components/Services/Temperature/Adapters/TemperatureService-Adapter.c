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

static void PrivateHandler(xServiceT* service)
{

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xServiceT* service, xServiceAdapterRequestSelector selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xServiceT* service, xServiceAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//==============================================================================
//initializations:

static xServiceAdapterInterfaceT privateInterface =
{
	.Handler = (xServiceAdapterHandlerT)PrivateHandler,

	.RequestListener = (xServiceAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xServiceAdapterEventListenerT)PrivateEventListener,
};
//------------------------------------------------------------------------------
xResult TemperatureServiceAdapterInit(TemperatureServiceT* service,
		TemperatureServiceAdapterT* adapter,
		TemperatureServiceAdapterInitT* init)
{
	if (service && init)
	{
		service->Base.Adapter.Content = adapter;
		service->Base.Adapter.Interface = &privateInterface;
		service->Base.Adapter.Description = nameof(TemperatureServiceAdapterT);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
