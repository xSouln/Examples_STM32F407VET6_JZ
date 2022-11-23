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
static void PrivateIRQListener(xTxT *tx, ...)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;

}
//------------------------------------------------------------------------------
static void PrivateEventListener(xTxT *tx, xTxEventSelector selector, void* arg, ...)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	
	switch ((int)selector)
	{
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
//==============================================================================
//interfaces:

static xTxInterfaceT interface =
{
	INITIALIZATION_HANDLER(xTx, PrivateHandler),
	INITIALIZATION_EVENT_LISTENER(xTx, PrivateEventListener),
	INITIALIZATION_IRQ_LISTENER(xTx, PrivateIRQListener),
	INITIALIZATION_REQUEST_LISTENER(xTx, PrivateRequestListener),
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
