//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_LWIP_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPClient_LWIP_RxAdapter.h"
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
	TCPClientLWIPAdapterT* adapter = rx->Adapter;
	
	switch ((uint32_t)selector)
	{
		case xRxRequestClearBuffer:
			adapter->RxReceiver.BytesReceived = 0;
			break;

		case xRxRequestClearResponseBuffer:
			xDataBufferClear(adapter->ResponseBuffer);
			break;

		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateReceive(xRxT* rx, uint8_t* data, uint32_t size)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	xRxReceiverReceive(&adapter->RxReceiver, data, size);

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivatePutInResponseBuffer(xRxT* rx, uint8_t* data, uint32_t size)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	xDataBufferAdd(adapter->ResponseBuffer, data, size);

	return xResultAccept;
}
//------------------------------------------------------------------------------

static uint8_t* PrivateGetBuffer(xRxT* rx)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	return adapter->RxReceiver.Buffer;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBufferSize(xRxT* rx)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	return adapter->RxReceiver.BufferSize;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBytesCountInBuffer(xRxT* rx)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	return adapter->RxReceiver.BytesReceived;
}
//------------------------------------------------------------------------------

static uint8_t* PrivateGetResponseBuffer(xRxT* rx)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	return adapter->ResponseBuffer->Data;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetResponseBufferSize(xRxT* rx)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	return adapter->ResponseBuffer->Size;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBytesCountInResponseBuffer(xRxT* rx)
{
	TCPClientLWIPAdapterT* adapter = rx->Adapter;

	return adapter->ResponseBuffer->DataSize;
}
//------------------------------------------------------------------------------

static void PrivateRxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, void* arg, ...)
{
	TCPClientT* server = receiver->Parent->Object.Parent;

	TCPClientReceivedDataT event_arg =
	{
		.Data = arg,
		.Size = *(uint32_t*)(&arg + 1),
	};

	switch ((uint8_t)event)
	{
		case xRxReceiverEventEndLine:
			server->Interface->EventListener(server, TCPClientEventEndLine, &event_arg);
			break;

		case xRxReceiverEventBufferIsFull:
			server->Interface->EventListener(server, TCPClientEventBufferIsFull, &event_arg);
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

	.Receive = (xRxReceiveActionT)PrivateReceive,
	.PutInResponseBuffer = (xRxPutInResponseBufferActionT)PrivatePutInResponseBuffer,

	.GetBuffer = (xRxGetBufferActionT)PrivateGetBuffer,
	.GetBufferSize = (xRxGetBufferSizeActionT)PrivateGetBufferSize,
	.GetBytesCountInBuffer = (xRxGetBytesCountInBufferActionT)PrivateGetBytesCountInBuffer,

	.GetResponseBuffer = (xRxGetResponseBufferActionT)PrivateGetResponseBuffer,
	.GetResponseBufferSize = (xRxGetResponseBufferSizeActionT)PrivateGetResponseBufferSize,
	.GetBytesCountInResponseBuffer = (xRxGetBytesCountInResponseBufferActionT)PrivateGetBytesCountInResponseBuffer
};
//==============================================================================
//initialization:

xResult TCPClientLWIPRxAdapterInit(TCPClientT* server, TCPClientLWIPAdapterT* adapter)
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
#endif //TCP_CLIENT_LWIP_ADAPTER_ENABLE
