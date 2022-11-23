//==============================================================================
//module enable:

#include "TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_SERVER_COMPONENT_H
#define _TCP_SERVER_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Config.h"
#include "TCPServer/Controls/TCPServer.h"

#ifdef TCP_SERVER_LWIP_COMPONENT_ENABLE
#include "TCPServer/Executions/LWIP/TCPServer_LWIP_Component.h"
#endif
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult _TCPServerComponentInit(TCPServerT* server, void* parent);

void _TCPServerComponentHandler(TCPServerT* server);
void _TCPServerComponentTimeSynchronization(TCPServerT* server);

void _TCPServerComponentIRQListener(TCPServerT* server);
//==============================================================================
//export:


//==============================================================================
//override:

#ifndef TCPServerComponentInit
#define TCPServerComponentInit(server, parent)\
	_TCPServerComponentInit(server, parent)
#endif
//------------------------------------------------------------------------------
#ifndef TCPServerComponentHandler
#define TCPServerComponentHandler(server)\
	_TCPServerComponentHandler(server)
#endif
//------------------------------------------------------------------------------
#ifndef TCPServerComponentTimeSynchronization
#define TCPServerComponentTimeSynchronization(server)\
	_TCPServerComponentTimeSynchronization(server)
#endif
//------------------------------------------------------------------------------
#ifndef TCPServerComponentIRQListener
#define TCPServerComponentIRQListener(server)\
	_TCPServerComponentIRQListener(server)
#endif
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_COMPONENT_H
#endif //TCP_SERVER_COMPONENT_ENABLE
