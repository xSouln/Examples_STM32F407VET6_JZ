//==============================================================================
//module enable:

#include "TCPClient_ComponentConfig.h"
#ifdef TCP_CLIENT_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_COMPONENT_H
#define _TCP_CLIENT_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Config.h"
#include "TCPClient/Controls/TCPClient.h"

#ifdef TCP_CLIENT_LWIP_COMPONENT_ENABLE
#include "TCPClient/Executions/LWIP/TCPClient_LWIP_Component.h"
#endif
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult _TCPClientComponentInit(TCPClientT* server, void* parent);

void _TCPClientComponentHandler(TCPClientT* server);
void _TCPClientComponentTimeSynchronization(TCPClientT* server);

void _TCPClientComponentIRQListener(TCPClientT* server);
//==============================================================================
//export:


//==============================================================================
//override:

#ifndef TCPClientComponentInit
#define TCPClientComponentInit(server, parent)\
	_TCPClientComponentInit(server, parent)
#endif
//------------------------------------------------------------------------------
#ifndef TCPClientComponentHandler
#define TCPClientComponentHandler(server)\
	_TCPClientComponentHandler(server)
#endif
//------------------------------------------------------------------------------
#ifndef TCPClientComponentTimeSynchronization
#define TCPClientComponentTimeSynchronization(server)\
	_TCPClientComponentTimeSynchronization(server)
#endif
//------------------------------------------------------------------------------
#ifndef TCPClientComponentIRQListener
#define TCPClientComponentIRQListener(server)\
	_TCPClientComponentIRQListener(server)
#endif
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_COMPONENT_H
#endif //TCP_CLIENT_COMPONENT_ENABLE
