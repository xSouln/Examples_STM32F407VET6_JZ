//==============================================================================
//module enable:

#include "TCPServer/Adapters/TCPServer_AdapterConfig.h"
#ifdef TCP_SERVER_LWIP_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPServer_LWIP_RxAdapter.h"
//==============================================================================
//functions:

static void PrivateHandler(xRxT* rx)
{
	
}
//------------------------------------------------------------------------------
static void PrivateIRQListener(xRxT* rx)
{

}
//------------------------------------------------------------------------------
static void PrivateEventListener(xRxT* rx, xRxEventSelector event, void* arg, ...)
{

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xRxT* rx, xRxRequestSelector selector, void* arg, ...)
{
	//TCPServerWIZspiAdapterT* adapter = rx->Adapter;
	
	switch ((uint32_t)selector)
	{
		default: return xResultRequestIsNotFound;
	}
	
	return xResultRequestIsNotFound;
}
//------------------------------------------------------------------------------
static void PrivateRxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, void* arg, ...)
{
	//TCPServerT* server = receiver->Parent->Object.Parent;

	switch ((uint8_t)event)
	{
		default: return;
	}
}
//==============================================================================
//interfaces:

static xRxReceiverInterfaceT rx_receiver_interface =
{
	INITIALIZATION_EVENT_LISTENER(xRxReceiver, PrivateRxReceiverEventListener),
};
//------------------------------------------------------------------------------
static xRxInterfaceT rx_interface =
{
	INITIALIZATION_HANDLER(xRx, PrivateHandler),
	INITIALIZATION_EVENT_LISTENER(xRx, PrivateEventListener),
	INITIALIZATION_IRQ_LISTENER(xRx, PrivateIRQListener),
	INITIALIZATION_REQUEST_LISTENER(xRx, PrivateRequestListener),
};
//==============================================================================
//initialization:

xResult TCPServerLWIPRxAdapterInit(TCPServerT* server, TCPServerLWIPAdapterT* adapter)
{
	if (server && adapter)
	{
		if (!adapter->RxReceiver.Interface)
		{
			adapter->RxReceiver.Interface = &rx_receiver_interface;
		}
		
		return xRxInit(&server->Rx, server, adapter, &rx_interface);
	}
	
	return xResultError;
}
//==============================================================================
#endif //TCP_SERVER_LWIP_ADAPTER_ENABLE
