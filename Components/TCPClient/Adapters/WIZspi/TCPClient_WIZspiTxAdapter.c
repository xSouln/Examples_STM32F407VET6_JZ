//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPClient_WIZspiTxAdapter.h"
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
	//TCPClientWIZspiAdapterT* adapter = tx->Adapter;
	
	switch ((int)selector)
	{
		default : break;
	}
}
//------------------------------------------------------------------------------
static void PrivateIRQListener(xTxT *tx)
{
	//TCPClientWIZspiAdapterT* adapter = tx->Adapter;

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xTxT* tx, xTxRequestSelector selector, void* arg, ...)
{
	//TCPClientWIZspiAdapterT* adapter = tx->Adapter;
	//TCPClientT* server = tx->Object.Parent;
	
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
	//TCPClientWIZspiAdapterT* adapter = tx->Adapter;
	TCPClientT* server = tx->Object.Parent;
	
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
	//TCPClientWIZspiAdapterT* adapter = tx->Adapter;
	
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

xResult TCPClientWIZspiTxAdapterInit(TCPClientT* server, TCPClientWIZspiAdapterT* adapter)
{
	if (server && adapter)
	{
		return xTxInit(&server->Tx, server, adapter, &interface);
	}
	
	return xResultError;
}
//==============================================================================
#endif //TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
