//==============================================================================
//module enable:

#include "TCPServer/TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_LWIP_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_SERVER_LWIP_COMPONENT_H
#define _TCP_SERVER_LWIP_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPServer_LWIP_ComponentConfig.h"
#include "TCPServer/Controls/TCPServer.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult _TCPServerLWIPComponentInit(void* parent);

void _TCPServerLWIPComponentHandler();
void _TCPServerLWIPComponentTimeSynchronization();

void _TCPServerLWIPComponentIRQListener();
//==============================================================================
//import:


//==============================================================================
//override:

#define TCPServerLWIPComponentInit(parent) _TCPServerLWIPComponentInit(parent)

#define TCPServerLWIPComponentHandler() TCPServerHandler(&TCPServerLWIP)
#define TCPServerLWIPComponentTimeSynchronization() TCPServerTimeSynchronization(&TCPServerLWIP)

#define TCPServerLWIPComponentIRQListener() TCPServerIRQListener(&TCPServerLWIP)
//------------------------------------------------------------------------------
//override main components functions:

#define TCPServerComponentInit(parent) TCPServerLWIPComponentInit(parent)

#define TCPServerComponentHandler() TCPServerLWIPComponentHandler()
#define TCPServerComponentTimeSynchronization() TCPServerLWIPComponentTimeSynchronization()

#define TCPServerComponentIRQListener() TCPServerLWIPComponentIRQListener()
//==============================================================================
//export:

extern TCPServerT TCPServerLWIP;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_LWIP_COMPONENT_H
#endif //TCP_SERVER_LWIP_COMPONENT_ENABLE
