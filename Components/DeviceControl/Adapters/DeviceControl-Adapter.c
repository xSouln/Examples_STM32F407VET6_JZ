//==============================================================================
//includes:

#include "DeviceControl-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void PrivateHandler(xDeviceControlT* control)
{

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xDeviceControlT* control, xDeviceControlAdapterRequestSelector selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xDeviceControlT* control, xDeviceControlAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//==============================================================================
//initializations:

static xDeviceControlAdapterInterfaceT PrivatePortInterface =
{
	.Handler = (xDeviceControlAdapterHandlerT)PrivateHandler,

	.RequestListener = (xDeviceControlAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xDeviceControlAdapterEventListenerT)PrivateEventListener,
};
//------------------------------------------------------------------------------
xResult DeviceControlAdapterInit(xDeviceControlT* control, DeviceControlAdapterInitT* init)
{
	if (control && init)
	{
		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
