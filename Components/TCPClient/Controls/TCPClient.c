//==============================================================================
//includes:

#include "Common/xMemory.h"
#include "TCPClient.h"
//==============================================================================
//functions:

void _TCPClientHandler(TCPClientT* server)
{
	xRxHandler(&server->Rx);
	xTxHandler(&server->Tx);

	server->Adapter.Interface->Handler(server);
}
//------------------------------------------------------------------------------

void _TCPClientTimeSynchronization(TCPClientT* server)
{
	if (server->Adapter.Interface)
	{
		server->Adapter.Interface->EventListener(server, TCPClientAdapterEventUpdateTime, 0);
	}
}
//------------------------------------------------------------------------------
void _TCPClientEventListener(TCPClientT* server, TCPClientEventSelector selector, void* arg, ...)
{
	//SerialPortComponentEventListener(port, selector, arg);

	switch((uint8_t)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

xResult _TCPClientRequestListener(TCPClientT* server, TCPClientRequestSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPClientIRQListener(TCPClientT* server)
{

}
//==============================================================================
//initialization:

static const ObjectDescriptionT TCPClientObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = TCP_CLIENT_OBJECT_ID,
	.Type = nameof(TCPClientT)
};
//------------------------------------------------------------------------------
xResult TCPClientInit(TCPClientT* server, void* parent, TCPClientInterfaceT* interface)
{
	if (server && interface)
	{
		server->Object.Description = &TCPClientObjectDescription;
		server->Object.Parent = parent;

		server->Interface = interface;
		/*
		xMemoryCopy(&server->Options.Ip, (uint8_t*)TCP_SERVER_DEFAULT_IP, sizeof(server->Options.Ip));
		xMemoryCopy(&server->Options.Gateway, (uint8_t*)TCP_SERVER_DEFAULT_GETAWAY, sizeof(server->Options.Gateway));
		xMemoryCopy(&server->Options.NetMask, (uint8_t*)TCP_SERVER_DEFAULT_NETMASK, sizeof(server->Options.NetMask));
		xMemoryCopy(&server->Options.Mac, (uint8_t*)TCP_SERVER_DEFAULT_MAC_ADDRESS, sizeof(server->Options.Mac));
		*/
		server->Sock.Port = TCP_CLIENT_DEFAULT_PORT;
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
