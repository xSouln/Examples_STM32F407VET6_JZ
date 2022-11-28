//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_LWIP_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPClient_LWIP_TxAdapter.h"
//==============================================================================
//functions:

static void PrivateHandler(xTxT *tx)
{

}
//------------------------------------------------------------------------------
static void PrivateIRQListener(xTxT *tx)
{
	//TCPClientWIZspiAdapterT* adapter = tx->Adapter;

}
//------------------------------------------------------------------------------
static void PrivateEventListener(xTxT *tx, xTxEventSelector selector, void* arg, ...)
{
	TCPClientLWIPAdapterT* adapter = tx->Adapter;
	TCPClientT* server = tx->Object.Parent;
	
	switch ((int)selector)
	{
		case xTxEventTransmissionComplete:
			if (server->Status.State == TCPClientIsOpen)
			{
				tcp_output(adapter->Data.SocketHandle);
			}
			break;

		default : break;
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xTxT* tx, xTxRequestSelector selector, void* arg, ...)
{
	//TCPClientWIZspiAdapterT* adapter = tx->Adapter;
	//TCPClientT* server = tx->Object.Parent;
	
	switch ((int)selector)
	{
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateTransmitData(xTxT* tx, void* data, uint32_t size)
{
	TCPClientLWIPAdapterT* adapter = tx->Adapter;

	return tcp_write(adapter->Data.SocketHandle, data, size, TCP_WRITE_FLAG_COPY) == ERR_OK ? xResultAccept : xResultError;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBufferSize(xTxT* tx)
{
	TCPClientLWIPAdapterT* adapter = tx->Adapter;

	return 1000;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetFreeBufferSize(xTxT* tx)
{
	TCPClientLWIPAdapterT* adapter = tx->Adapter;

	return 1000;
}
//==============================================================================
//interfaces:

static xTxInterfaceT interface =
{
	INITIALIZATION_HANDLER(xTx, PrivateHandler),
	INITIALIZATION_EVENT_LISTENER(xTx, PrivateEventListener),
	INITIALIZATION_IRQ_LISTENER(xTx, PrivateIRQListener),
	INITIALIZATION_REQUEST_LISTENER(xTx, PrivateRequestListener),

	.TransmitData = (xTxTransmitDataT)PrivateTransmitData,

	.GetBufferSize = (xTxGetBufferSizeActionT)PrivateGetBufferSize,
	.GetFreeBufferSize = (xTxGetFreeBufferSizeActionT)PrivateGetFreeBufferSize
};
//==============================================================================
//initialization:

xResult TCPClientLWIPTxAdapterInit(TCPClientT* server, TCPClientLWIPAdapterT* adapter)
{
	if (server && adapter)
	{
		return xTxInit(&server->Tx, server, adapter, &interface);
	}
	
	return xResultError;
}
//==============================================================================
#endif //TCP_CLIENT_LWIP_ADAPTER_ENABLE
