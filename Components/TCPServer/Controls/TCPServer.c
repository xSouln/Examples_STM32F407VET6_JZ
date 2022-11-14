//==============================================================================
#include "Common/xMemory.h"
#include "TCPServer.h"
//==============================================================================
void TCPServerHandler(TCPServerT* server)
{
	server->Adapter.Interface->Handler(server);
}
//------------------------------------------------------------------------------
xResult TCPServerInit(TCPServerT* server, void* parent, TCPServerInterfaceT* interface)
{
	if (server && interface)
	{
		server->Description = "TCPServerT";
		server->Parent = parent;
		server->Interface = interface;
		
		xMemoryCopy(&server->Options.Ip, (uint8_t*)TCP_SERVER_DEFAULT_IP, sizeof(server->Options.Ip));
		xMemoryCopy(&server->Options.Gateway, (uint8_t*)TCP_SERVER_DEFAULT_GETAWAY, sizeof(server->Options.Gateway));
		xMemoryCopy(&server->Options.NetMask, (uint8_t*)TCP_SERVER_DEFAULT_NETMASK, sizeof(server->Options.NetMask));
		xMemoryCopy(&server->Options.Mac, (uint8_t*)TCP_SERVER_DEFAULT_MAC_ADDRESS, sizeof(server->Options.Mac));
		
		server->Sock.Port = TCP_SERVER_DEFAULT_PORT;
		
		server->Status.InitResult = xResultAccept;
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
