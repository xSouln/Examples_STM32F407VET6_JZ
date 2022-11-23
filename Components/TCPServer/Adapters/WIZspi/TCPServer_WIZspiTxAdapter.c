//==============================================================================
//module enable:

#include "TCPServer/Adapters/TCPServer_AdapterConfig.h"
#ifdef TCP_SERVER_WIZ_SPI_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPServer_WIZspiTxAdapter.h"
#include "WIZ/W5500/w5500.h"
#include "socket.h"
//==============================================================================
//functions:

static void PrivateHandler(xTxT *tx)
{

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
static void PrivateIRQListener(xTxT *tx)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xTxT* tx, xTxRequestSelector selector, void* arg, ...)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	//TCPServerT* server = tx->Object.Parent;
	
	switch ((int)selector)
	{
		/*
		case xTxRequestTransmitData:
			if (server->Sock.Status.Connected && args && count)
			{
				while (send(server->Sock.Number, (uint8_t*)args, count) != count) { }
				return xResultAccept;
			}
			return xResultError;
		*/
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
static xResult PrivateActionGetValue(xTxT* tx, xTxValueSelector selector, uint32_t* value)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	TCPServerT* server = tx->Object.Parent;
	
	switch ((int)selector)
	{
		case xTxValueFreeBufferSize:
			*value = server->Sock.Status.Connected ? getSn_TX_FSR(server->Sock.Number) : 0;
			break;
		
		case xTxValueBufferSize:
			*value = server->Sock.Status.Connected ? getSn_TxMAX(server->Sock.Number) : 0;
			break;
		
		case xTxValueTransmitterStatus:
			return xTxStatusIdle;
		
		default : return xResultNotSupported;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static xResult PrivateActionSetValue(xTxT* tx, xTxValueSelector selector, void* value)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	
	switch ((uint32_t)selector)
	{
		default : return xResultValueIsNotFound;
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
	INITIALIZATION_GET_VALUE_ACTION(xTx, PrivateActionGetValue),
	INITIALIZATION_SET_VALUE_ACTION(xTx, PrivateActionSetValue)
};
//==============================================================================
//initialization:

xResult TCPServerWIZspiTxAdapterInit(TCPServerT* server, TCPServerWIZspiAdapterT* adapter)
{
	if (server && adapter)
	{
		return xTxInit(&server->Tx, server, adapter, &interface);
	}
	
	return xResultError;
}
//==============================================================================
#endif //TCP_SERVER_WIZ_SPI_ADAPTER_ENABLE
