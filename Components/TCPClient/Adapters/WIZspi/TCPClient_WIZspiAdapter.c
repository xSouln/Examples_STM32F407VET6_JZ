//==============================================================================
//module enable:

#include "TCPClient/Adapters/TCPClient_AdapterConfig.h"
#ifdef TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "TCPClient_WIZspiAdapter.h"
#include "TCPClient_WIZspiTxAdapter.h"
#include "TCPClient_WIZspiRxAdapter.h"
#include "Common/xMemory.h"
#include "WIZ/W5500/w5500.h"
#include "WIZ/socket.h"
//==============================================================================
//functions:

static void PrivateHandler(TCPClientT* server)
{
	//TCPClientWIZspiAdapterT* adapter = server->Adapter.Child;
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
}
//------------------------------------------------------------------------------

static void PrivateEventListener(TCPClientT* server, TCPClientAdapterEventSelector selector, uint32_t args, ...)
{
	switch ((uint32_t)selector)
	{
		default : return;
	}
}
//------------------------------------------------------------------------------

static void PrivateIRQListener(TCPClientT* server)
{

}
//------------------------------------------------------------------------------

static xResult PrivateRequestListener(TCPClientT* server, TCPClientAdapterRequestSelector selector, uint32_t arg, ...)
{
	switch ((uint32_t)selector)
	{
		default : return xResultRequestIsNotFound;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateActionGetValue(TCPClientT* server, TCPClientAdapterValueSelector selector, uint32_t* value)
{
	switch ((uint32_t)selector)
	{
		default : return xResultNotSupported;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateActionSetValue(TCPClientT* server, TCPClientAdapterValueSelector selector, uint32_t* value)
{
	switch ((uint32_t)selector)
	{
		default : return xResultNotSupported;
	}
	
	return xResultAccept;
}
//==============================================================================
//interfaces:

static TCPClientAdapterInterfaceT Interface =
{
	INITIALIZATION_HANDLER(TCPClientAdapter, PrivateHandler),

	INITIALIZATION_IRQ_LISTENER(TCPClientAdapter, PrivateIRQListener),

	INITIALIZATION_EVENT_LISTENER(TCPClientAdapter, PrivateEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPClientAdapter, PrivateRequestListener),

	INITIALIZATION_GET_VALUE_ACTION(TCPClientAdapter, PrivateActionGetValue),
	INITIALIZATION_SET_VALUE_ACTION(TCPClientAdapter, PrivateActionSetValue)
};
//==============================================================================
//initialization:

xResult TCPClientWIZspiAdapterInit(TCPClientT* server, TCPClientWIZspiAdapterT* adapter)
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
	
	server->Adapter.Object.Description = "TCPClientWIZspiAdapterT";
	server->Adapter.Object.Parent = server;
	server->Adapter.Child = adapter;
	server->Adapter.Interface = &Interface;
	
	server->Status.TxInitResult = TCPClientWIZspiTxAdapterInit(server, adapter);
	server->Status.RxInitResult = TCPClientWIZspiRxAdapterInit(server, adapter);
	
	server->Rx.Tx = &server->Tx;
	
	uint8_t tx_mem_conf[8] = {16,0,0,0,0,0,0,0}; // for setting TMSR regsiter
	uint8_t rx_mem_conf[8] = {16,0,0,0,0,0,0,0}; // for setting RMSR regsiter
	
	adapter->BusInterface.HardwareResetOn();
	server->Interface->RequestListener(server, TCPClientRequestDelay, 100, 0);
	adapter->BusInterface.HardwareResetOff();
	server->Interface->RequestListener(server, TCPClientRequestDelay, 200, 0);

	reg_wizchip_spi_cbfunc(adapter->BusInterface.ReceiveByte, adapter->BusInterface.TransmiteByte);//TCP_driver_transmitter);
	reg_wizchip_cs_cbfunc(adapter->BusInterface.SelectChip, adapter->BusInterface.DeselectChip);
	
	wizchip_setinterruptmask(IK_SOCK_ALL);

	setMR(MR_RST);
	server->Interface->RequestListener(server, TCPClientRequestDelay, 300, 0);

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
#endif //TCP_CLIENT_WIZ_SPI_ADAPTER_ENABLE
