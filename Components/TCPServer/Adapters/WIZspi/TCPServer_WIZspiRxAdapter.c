//==============================================================================
#include "TCPServer_WIZspiRxAdapter.h"
//==============================================================================
static void Handler(xRxT* rx)
{
	TCPServerWIZspiAdapterT* adapter = rx->Adapter;
	TCPServerT* server = adapter->Server;
	
	if (server->Sock.Status.Connected)
	{
		uint16_t len = getSn_RX_RSR(server->Sock.Number);		
    if (len)
    {
      if (len > adapter->RxBufferSize)
			{
				len = adapter->RxBufferSize;
			}
			
      len = recv(server->Sock.Number, adapter->RxBuffer, len);
			xRxReceiverReceive(&adapter->RxReceiver, adapter->RxBuffer, len);
    }
	}
}
//------------------------------------------------------------------------------
static void EventListener(xRxT* rx, xRxEventSelector event, uint32_t args, uint32_t count)
{
	switch ((uint32_t)event)
	{
		default : break;
	}
}
//------------------------------------------------------------------------------
static xResult RequestListener(xRxT* rx, xRxRequestSelector selector, uint32_t args, uint32_t count)
{
	TCPServerWIZspiAdapterT* adapter = rx->Adapter;
	
	switch ((uint32_t)selector)
	{
		case xRxRequestReceive :
			xRxReceiverReceive(&adapter->RxReceiver, (uint8_t*)args, count);
		
		case xRxRequestClearBuffer :
			adapter->RxReceiver.BytesReceived = 0;
			break;
		
		case xRxRequestPutInResponseBuffer :
			xDataBufferAdd(adapter->ResponseBuffer, (void*)args, count);
			break;
		
		case xRxRequestClearResponseBuffer :
			xDataBufferClear(adapter->ResponseBuffer);
			break;
		
		default : return xResultRequestIsNotFound;
	}
	
	return xResultRequestIsNotFound;
}
//------------------------------------------------------------------------------
static int GetValue(xRxT* rx, xRxValueSelector selector)
{
	TCPServerWIZspiAdapterT* adapter = rx->Adapter;
	
	switch ((uint32_t)selector)
	{
		case xRxValueResponseData :
			return (int)adapter->ResponseBuffer->Data;
		
		case xRxValueResponseDataSize :
			return (int)adapter->ResponseBuffer->DataSize;
		
		default : return -1;
	}
}
//------------------------------------------------------------------------------
static xResult SetValue(xRxT* rx, xRxValueSelector selector, uint32_t value)
{
	switch ((uint32_t)selector)
	{
		default : return xResultValueIsNotFound;
	}
	
	return xResultValueIsNotFound;
}
//------------------------------------------------------------------------------
static void RxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, uint32_t args, uint32_t count)
{
	TCPServerT* server = receiver->Parent->Parent;
	
	switch ((uint8_t)event)
	{
		case xRxReceiverEventEndLine:
			server->Interface->EventListener(server, TCPServerEventEndLine, args, count);
			break;
		
		case xRxReceiverEventBufferIsFull:
			server->Interface->EventListener(server, TCPServerEventReceiverBufferIsFull, args, count);
			break;
	}
}
//==============================================================================
static xRxReceiverInterfaceT rx_receiver_interface =
{
	.EventListener = (xRxReceiverEventListener)RxReceiverEventListener
};
//------------------------------------------------------------------------------
static xRxInterfaceT rx_interface =
{
	.Handler = (xRxHandlerT)Handler,
	.EventListener = (xRxEventListenerT)EventListener,
	.RequestListener = (xRxRequestListenerT)RequestListener,
	.GetValue = (xRxActionGetValueT)GetValue,
	.SetValue = (xRxActionSetValueT)SetValue,
};
//==============================================================================
xResult TCPServerWIZspiRxAdapterInit(TCPServerT* server, TCPServerWIZspiAdapterT* adapter)
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
