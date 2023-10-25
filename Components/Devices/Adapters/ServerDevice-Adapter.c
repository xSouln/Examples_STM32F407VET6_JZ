//==============================================================================
//includes:

#include "ServerDevice-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void PrivateHandler(xDeviceT* device)
{
	ServerDeviceAdapterT* adapter = (ServerDeviceAdapterT*)device->Adapter.Content;

	if (adapter->Content.Command)
	{
		adapter->Content.CommandExecutionResult = adapter->Content.Command->Action(device, adapter->Content.Command->Arg);
		adapter->Content.Command = 0;
		xSemaphoreGive(adapter->Content.CommandAccomplishSemaphore);
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xDeviceT* device, xDeviceAdapterRequestSelector selector, void* arg)
{
	ServerDeviceAdapterT* adapter = (ServerDeviceAdapterT*)device->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xDeviceAdapterRequestExecuteCommand:
		{
			xSemaphoreTake(adapter->Content.CommandExecutionMutex, portMAX_DELAY);

			adapter->Content.Command = arg;

#ifdef INC_FREERTOS_H
			xSemaphoreTake(adapter->Content.CommandAccomplishSemaphore, portMAX_DELAY);
#endif
			xResult commandResult = adapter->Content.CommandExecutionResult;
			xSemaphoreGive(adapter->Content.CommandExecutionMutex);

			return commandResult;
		}

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xDeviceT* device, xDeviceAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//==============================================================================
//initializations:

static xDeviceAdapterInterfaceT privateAdapterInterface =
{
	.Handler = (xDeviceAdapterHandlerT)PrivateHandler,

	.RequestListener = (xDeviceAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xDeviceAdapterEventListenerT)PrivateEventListener,
};
//------------------------------------------------------------------------------
xResult ServerDeviceAdapterInit(xDeviceT* device, ServerDeviceAdapterT* adapter, ServerDeviceAdapterInitT* init)
{
	if (device && init)
	{
		device->Adapter.Content = adapter;
		device->Adapter.Interface = &privateAdapterInterface;
		device->Adapter.Description = nameof(ServerDeviceAdapterT);

#ifdef INC_FREERTOS_H
		adapter->Content.CommandExecutionMutex = xSemaphoreCreateMutex();
		adapter->Content.CommandAccomplishSemaphore = xSemaphoreCreateBinary();
#endif
		adapter->Port = init->Port;
		adapter->Content.PortRxCircleBuffer = xPortGetRxCircleBuffer(adapter->Port);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
