//==============================================================================
//module enable:

#include "TCPClient/TCPClient_ComponentConfig.h"
#ifdef TCP_CLIENT_LWIP_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_LWIP_COMPONENT_H
#define _TCP_CLIENT_LWIP_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "TCPClient_LWIP_ComponentConfig.h"
#include "TCPClient/Controls/TCPClient.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult _TCPClientLWIPComponentInit(void* parent);

void _TCPClientLWIPComponentHandler();
void _TCPClientLWIPComponentTimeSynchronization();

void _TCPClientLWIPComponentIRQListener();
//==============================================================================
//import:


//==============================================================================
//override:

#define TCPClientLWIPComponentInit(parent) _TCPClientLWIPComponentInit(parent)

#define TCPClientLWIPComponentHandler() TCPClientHandler(&TCPClientLWIP)
#define TCPClientLWIPComponentTimeSynchronization() TCPClientTimeSynchronization(&TCPClientLWIP)

#define TCPClientLWIPComponentIRQListener() TCPClientIRQListener(&TCPClientLWIP)
//------------------------------------------------------------------------------
//override main components functions:

#define TCPClientComponentInit(parent) TCPClientLWIPComponentInit(parent)

#define TCPClientComponentHandler() TCPClientLWIPComponentHandler()
#define TCPClientComponentTimeSynchronization() TCPClientLWIPComponentTimeSynchronization()

#define TCPClientComponentIRQListener() TCPClientLWIPComponentIRQListener()
//==============================================================================
//export:

extern TCPClientT TCPClientLWIP;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_LWIP_COMPONENT_H
#endif //TCP_CLIENT_LWIP_COMPONENT_ENABLE
