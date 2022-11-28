//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPClient_WIZspiRxAdapter.h"
#include "WIZ/W5500/w5500.h"
#include "WIZ/socket.h"
//==============================================================================
//functions:

static void PrivateHandler(xRxT* rx)
{
	TCPClientWIZspiAdapterT* adapter = rx->Adapter;
	TCPClientT* server = rx->Object.Parent;
	
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
static void PrivateEventListener(xRxT* rx, xRxEventSelector event, void* arg, ...)
{
	switch ((uint32_t)event)
	{
		default : break;
	}
}
//------------------------------------------------------------------------------
static void PrivateIRQListener(xRxT* rx)
{

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xRxT* rx, xRxRequestSelector selector, void* arg, ...)
{
	TCPClientWIZspiAdapterT* adapter = rx->Adapter;
	
	switch ((uint32_t)selector)
	{
	/*
		case xRxRequestReceive:
			xRxReceiverReceive(&adapter->RxReceiver, (uint8_t*)args, count);
		*/
		case xRxRequestClearBuffer :
			adapter->RxReceiver.BytesReceived = 0;
			break;
		/*
		case xRxRequestPutInResponseBuffer:
			xDataBufferAdd(adapter->ResponseBuffer, (void*)args, count);
			break;
		*/
		case xRxRequestClearResponseBuffer:
			xDataBufferClear(adapter->ResponseBuffer);
			break;
		
		default: return xResultRequestIsNotFound;
	}
	
	return xResultRequestIsNotFound;
}
//------------------------------------------------------------------------------
static xResult PrivateActionGetValue(xRxT* rx, xRxValueSelector selector, void* value)
{
	TCPClientWIZspiAdapterT* adapter = rx->Adapter;
	
	switch ((uint32_t)selector)
	{
		case xRxValueResponseData :
			return (int)adapter->ResponseBuffer->Data;
		
		case xRxValueResponseDataSize :
			return (int)adapter->ResponseBuffer->DataSize;
		
		default : return xResultNotSupported;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static xResult PrivateActionSetValue(xRxT* rx, xRxValueSelector selector, void* value)
{
	switch ((uint32_t)selector)
	{
		default : return xResultValueIsNotFound;
	}
	
	return xResultValueIsNotFound;
}
//------------------------------------------------------------------------------
static void PrivateRxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, void* arg, ...)
{
	TCPClientT* server = receiver->Parent->Object.Parent;

	TCPClientReceivedDataT received_data =
	{
		.Data = arg,
		.Size = *(uint32_t*)(&arg + 1),
	};

	switch ((uint8_t)event)
	{
		case xRxReceiverEventEndLine:
			TCPClientEventListener(server, TCPClientEventEndLine, &received_data);
			break;
		
		case xRxReceiverEventBufferIsFull:
			TCPClientEventListener(server, TCPClientEventBufferIsFull, &received_data);
			break;

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
	INITIALIZATION_GET_VALUE_ACTION(xRx, PrivateActionGetValue),
	INITIALIZATION_SET_VALUE_ACTION(xRx, PrivateActionSetValue)
};
//==============================================================================
//initialization:

xResult TCPClientWIZspiRxAdapterInit(TCPClientT* server, TCPClientWIZspiAdapterT* adapter)
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
#endif //TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
