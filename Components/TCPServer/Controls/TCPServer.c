//==============================================================================
//includes:

#include "Common/xMemory.h"
#include "TCPServer.h"
//==============================================================================
//functions:

void _TCPServerHandler(TCPServerT* server)
{
	xRxHandler(&server->Rx);
	xTxHandler(&server->Tx);

	server->Adapter.Interface->Handler(server);
}
//------------------------------------------------------------------------------

void _TCPServerTimeSynchronization(TCPServerT* server)
{
	if (server->Adapter.Interface)
	{
		server->Adapter.Interface->EventListener(server, TCPServerAdapterEventUpdateTime, 0);
	}
}
//------------------------------------------------------------------------------
void _TCPServerEventListener(TCPServerT* server, TCPServerEventSelector selector, void* arg, ...)
{
	//SerialPortComponentEventListener(port, selector, arg);

	switch((uint8_t)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

xResult _TCPServerRequestListener(TCPServerT* server, TCPServerRequestSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPServerIRQListener(TCPServerT* server)
{

}
//==============================================================================
//initialization:

static const ObjectDescriptionT TCPServerObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = TCP_SERVER_OBJECT_ID,
	.Type = nameof(TCPServerT)
};
//------------------------------------------------------------------------------
xResult TCPServerInit(TCPServerT* server, void* parent, TCPServerInterfaceT* interface)
{
	if (server && interface)
	{
		server->Object.Description = &TCPServerObjectDescription;
		server->Object.Parent = parent;

		server->Interface = interface;
		
		xMemoryCopy(&server->Options.Ip, (uint8_t*)TCP_SERVER_DEFAULT_IP, sizeof(server->Options.Ip));
		xMemoryCopy(&server->Options.Gateway, (uint8_t*)TCP_SERVER_DEFAULT_GETAWAY, sizeof(server->Options.Gateway));
		xMemoryCopy(&server->Options.NetMask, (uint8_t*)TCP_SERVER_DEFAULT_NETMASK, sizeof(server->Options.NetMask));
		xMemoryCopy(&server->Options.Mac, (uint8_t*)TCP_SERVER_DEFAULT_MAC_ADDRESS, sizeof(server->Options.Mac));
		
		server->Sock.Port = TCP_SERVER_DEFAULT_PORT;
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
