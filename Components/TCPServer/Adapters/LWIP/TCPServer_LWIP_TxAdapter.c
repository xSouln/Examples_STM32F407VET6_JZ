//==============================================================================
//module enable:

#include "TCPServer/Adapters/TCPServer_AdapterConfig.h"
#ifdef TCP_SERVER_LWIP_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPServer_LWIP_TxAdapter.h"
//==============================================================================
//functions:

static void PrivateHandler(xTxT *tx)
{

}
//------------------------------------------------------------------------------
static void PrivateIRQListener(xTxT *tx)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;

}
//------------------------------------------------------------------------------
static void PrivateEventListener(xTxT *tx, xTxEventSelector selector, void* arg, ...)
{
	TCPServerLWIPAdapterT* adapter = tx->Adapter;
	TCPServerT* server = tx->Object.Parent;
	
	switch ((int)selector)
	{
		case xTxEventTransmissionComplete:
			if (server->Status.State == TCPServerIsOpen)
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
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	//TCPServerT* server = tx->Object.Parent;
	
	switch ((int)selector)
	{
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateTransmitData(xTxT* tx, void* data, uint32_t size)
{
	TCPServerLWIPAdapterT* adapter = tx->Adapter;

	return tcp_write(adapter->Data.SocketHandle, data, size, TCP_WRITE_FLAG_COPY) == ERR_OK ? xResultAccept : xResultError;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetBufferSize(xTxT* tx)
{
	TCPServerLWIPAdapterT* adapter = tx->Adapter;

	return 1000;
}
//------------------------------------------------------------------------------

static uint32_t PrivateGetFreeBufferSize(xTxT* tx)
{
	TCPServerLWIPAdapterT* adapter = tx->Adapter;

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

xResult TCPServerLWIPTxAdapterInit(TCPServerT* server, TCPServerLWIPAdapterT* adapter)
{
	if (server && adapter)
	{
		return xTxInit(&server->Tx, server, adapter, &interface);
	}
	
	return xResultError;
}
//==============================================================================
#endif //TCP_SERVER_LWIP_ADAPTER_ENABLE
