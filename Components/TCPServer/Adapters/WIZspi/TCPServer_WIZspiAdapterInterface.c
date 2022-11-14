//==============================================================================
#include "TCPServer_WIZspiAdapter.h"
#include "TCPServer_WIZspiTxAdapter.h"
#include "TCPServer_WIZspiRxAdapter.h"
#include "Common/xMemory.h"
//==============================================================================
static void EventListener(TCPServerT* server, TCPServerAdapterEventSelector selector, uint32_t args, uint32_t count)
{
	switch ((uint32_t)selector)
	{
		default : return;
	}
}
//------------------------------------------------------------------------------
static xResult RequestListener(TCPServerT* server, TCPServerAdapterRequestSelector selector, uint32_t args, uint32_t count)
{
	switch ((uint32_t)selector)
	{
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
static int GetValue(TCPServerT* server, TCPServerAdapterValueSelector selector)
{
	switch ((uint32_t)selector)
	{
		default : return 0;
	}
}
//------------------------------------------------------------------------------
static xResult SetValue(TCPServerT* server, TCPServerAdapterValueSelector selector, uint32_t arg)
{
	switch ((uint32_t)selector)
	{
		default : return xResultValueIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
static void Handler(TCPServerT* server)
{
	//TCPServerWIZspiAdapterT* adapter = server->Adapter.Child;
	server->Sock.State = getSn_SR(server->Sock.Number);
  
  switch(server->Sock.State)
  {
    case SOCK_ESTABLISHED:
			if((server->Sock.State & Sn_IR_CON))
			{
				server->Sock.Status.Connected = true;
				setSn_IR(server->Sock.Number, Sn_IR_CON);
			}
			break;

    case SOCK_CLOSE_WAIT:      
			disconnect(server->Sock.Number);
			server->Sock.Status.Connected = false;
			break;

    case SOCK_CLOSED:
			close(server->Sock.Number);
			server->Sock.Status.Connected = false;
			socket(server->Sock.Number, Sn_MR_TCP, server->Sock.Port, 0);
			break;
    
    case SOCK_INIT:
			server->Sock.Status.Connected = false;
			listen(server->Sock.Number);
			break;
    
    default: break;    
  }
	
	xRxHandler(&server->Rx);
  xTxHandler(&server->Tx);
}
//==============================================================================
static TCPServerAdapterInterfaceT Interface =
{
	.Handler = (TCPServerAdapterHandlerT)Handler,
	.EventListener = (TCPServerAdapterEventListenerT)EventListener,
	.RequestListener = (TCPServerAdapterRequestListenerT)RequestListener,
	.GetValue = (TCPServerAdapterActionGetValueT)GetValue,
	.SetValue = (TCPServerAdapterActionSetValueT)SetValue,
};
//==============================================================================
xResult TCPServerWIZspiAdapterInterfaceInit(TCPServerAdapterBaseT* adapter)
{
	if (adapter)
	{
		adapter->Interface = &Interface;
		
		return xResultAccept;
	}
	
  return xResultError;
}
//==============================================================================
