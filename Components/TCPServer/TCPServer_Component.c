//==============================================================================
//module enable:

#include "TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPServer_Component.h"
//==============================================================================
//functions:

static void _TCPServerComponentEventListener(TCPServerT* port, TCPServerEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

static xResult _TCPServerComponentRequestListener(TCPServerT* port, TCPServerRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _TCPServerComponentIRQListener(TCPServerT* server)
{
	TCPServerIRQListener(server);
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _TCPServerComponentHandler(TCPServerT* server)
{
	TCPServerHandler(server);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _TCPServerComponentTimeSynchronization(TCPServerT* server)
{
	TCPServerTimeSynchronization(server);
}
//==============================================================================
//initializations:
/*
static TCPServerInterfaceT TCPServerInterface =
{
	INITIALIZATION_EVENT_LISTENER(TCPServer, _TCPServerComponentEventListener),
	INITIALIZATION_REQUEST_LISTENER(TCPServer, _TCPServerComponentRequestListener)
};
*/
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
xResult _TCPServerComponentInit(TCPServerT* server, void* parent)
{
	return 0;
}
//==============================================================================
#endif //TCP_SERVER_COMPONENT_ENABLE
