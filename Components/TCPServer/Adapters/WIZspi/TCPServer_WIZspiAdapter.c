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
xResult TCPServerWIZspiAdapterInit(TCPServerT* server, TCPServerWIZspiAdapterT* adapter)
{
	if (!server || !adapter)
	{
		server->Status.AdapterInitResult = xResultLinkError;
		goto end;
	}
	
	if (xMemoryCheckLincs(&adapter->BusInterface, sizeof(adapter->BusInterface)) != 0)
	{
		server->Status.AdapterInitResult = xResultLinkError;
		goto end;
	}
	
	server->Adapter.Description = "TCPServerWIZspiAdapterT";
	server->Adapter.Parent = server;
	server->Adapter.Child = adapter;
	server->Adapter.Interface = &Interface;
	adapter->Server = server;
	
	server->Status.TxInitResult = TCPServerWIZspiTxAdapterInit(server, adapter);
	server->Status.RxInitResult = TCPServerWIZspiRxAdapterInit(server, adapter);
	
	server->Rx.Tx = &server->Tx;
	
	uint8_t tx_mem_conf[8] = {16,0,0,0,0,0,0,0}; // for setting TMSR regsiter
  uint8_t rx_mem_conf[8] = {16,0,0,0,0,0,0,0}; // for setting RMSR regsiter
	
	adapter->BusInterface.HardwareResetOn();
  server->Interface->RequestListener(server, TCPServerRequestDelay, 100, 0);
  adapter->BusInterface.HardwareResetOff();
  server->Interface->RequestListener(server, TCPServerRequestDelay, 200, 0);
  
  reg_wizchip_spi_cbfunc(adapter->BusInterface.ReceiveByte, adapter->BusInterface.TransmiteByte);//TCP_driver_transmitter);
  reg_wizchip_cs_cbfunc(adapter->BusInterface.SelectChip, adapter->BusInterface.DeselectChip);
	
	wizchip_setinterruptmask(IK_SOCK_ALL);
  
  setMR(MR_RST);  
  server->Interface->RequestListener(server, TCPServerRequestDelay, 300, 0);
  
  wizchip_init(tx_mem_conf, rx_mem_conf);
  
  setSHAR(server->Options.Mac); // set source hardware address
  setGAR(server->Options.Gateway); // set gateway IP address
  setSUBR(server->Options.NetMask); // set netmask
  setSIPR(server->Options.Ip); // set source IP address
  
  getSHAR(server->Info.Mac); // get source hardware address	
  getGAR(server->Info.Gateway); // get gateway IP address
  getSIPR(server->Info.Ip); // get source IP address	
	getSUBR(server->Info.NetMask); // set netmask
	
	if (xMemoryCompare(&server->Options, &server->Info, sizeof(server->Info)) != 0)
	{
		server->Status.AdapterInitResult = xResultError;
		goto end;
	}
  
  close(server->Sock.Number);
  disconnect(server->Sock.Number);
	
	server->Status.AdapterInitResult = xResultAccept;
  
	end:;
  return server->Status.AdapterInitResult;
}
//==============================================================================
