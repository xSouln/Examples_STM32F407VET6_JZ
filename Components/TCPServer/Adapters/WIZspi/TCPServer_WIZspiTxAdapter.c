//==============================================================================
#include "TCPServer_WIZspiTxAdapter.h"
#include "socket.h"
//==============================================================================
static void Handler(xTxT *tx)
{

}
//------------------------------------------------------------------------------
static void EventListener(xTxT *tx, xTxEventSelector event, uint32_t args, uint32_t count)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	
	switch ((uint32_t)event)
	{
		default : break;
	}
}
//------------------------------------------------------------------------------
static xResult RequestListener(xTxT* tx, xTxRequestSelector selector, uint32_t args, uint32_t count)
{
	TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	TCPServerT* server = adapter->Server;
	
	switch ((uint32_t)selector)
	{
		case xTxRequestTransmitData:
			if (server->Sock.Status.Connected && args && count)
			{
				while (send(server->Sock.Number, (uint8_t*)args, count) != count) { }
				return xResultAccept;
			}
			return xResultError;
			
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
static int GetValue(xTxT* tx, xTxValueSelector selector)
{
	TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	TCPServerT* server = adapter->Server;
	
	switch ((uint32_t)selector)
	{
		case xTxValueFreeBufferSize:
			return server->Sock.Status.Connected ? getSn_TX_FSR(server->Sock.Number) : 0;
		
		case xTxValueBufferSize:
			return server->Sock.Status.Connected ? getSn_TxMAX(server->Sock.Number) : 0;
		
		case xTxValueTransmitterStatus :
			return xTxStatusIdle;
		
		default : return 0;
	}
}
//------------------------------------------------------------------------------
static xResult SetValue(xTxT* tx, xTxValueSelector selector, uint32_t value)
{
	//TCPServerWIZspiAdapterT* adapter = tx->Adapter;
	
	switch ((uint32_t)selector)
	{
		default : return xResultValueIsNotFound;
	}
	
	return xResultValueIsNotFound;
}
//------------------------------------------------------------------------------
static xTxInterfaceT interface =
{
	.Handler = (xTxHandlerT)Handler,
	.EventListener = (xTxEventListenerT)EventListener,
	.RequestListener = (xTxRequestListenerT)RequestListener,
	.GetValue = (xTxActionGetValueT)GetValue,
	.SetValue = (xTxActionSetValueT)SetValue
};
//==============================================================================
xResult TCPServerWIZspiTxAdapterInit(TCPServerT* server, TCPServerWIZspiAdapterT* adapter)
{
	if (server && adapter)
	{
		return xTxInit(&server->Tx, server, adapter, &interface);
	}
	
	return xResultError;
}
//==============================================================================
