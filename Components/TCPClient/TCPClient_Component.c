//==============================================================================
//module enable:

#include "TCPClient_ComponentConfig.h"
#ifdef TCP_CLIENT_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPClient_Component.h"
//==============================================================================
//functions:

static void _TCPClientComponentEventListener(TCPClientT* port, TCPClientEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

static xResult _TCPClientComponentRequestListener(TCPClientT* port, TCPClientRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPClientComponentIRQListener(TCPClientT* server)
{
	TCPClientIRQListener(server);
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _TCPClientComponentHandler(TCPClientT* server)
{
	TCPClientHandler(server);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _TCPClientComponentTimeSynchronization(TCPClientT* server)
{
	TCPClientTimeSynchronization(server);
}
//==============================================================================
//initializations:
/*
static TCPClientInterfaceT TCPClientInterface =
{
	INITIALIZATION_EVENT_LISTENER(TCPClient, _TCPClientComponentEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPClient, _TCPClientComponentRequestListener)
};
*/
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
xResult _TCPClientComponentInit(TCPClientT* server, void* parent)
{
	return 0;
}
//==============================================================================
#endif //TCP_CLIENT_COMPONENT_ENABLE
