//==============================================================================
//header:

#ifndef _LWIP_TCP_SERVER_COMPONENT_H_
#define _LWIP_TCP_SERVER_COMPONENT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "LWIP-NetTcpServer-ComponentConfig.h"
#include "Abstractions/xNet/xNet.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult LWIP_NetTcpServerComponentInit(void* parent);
void LWIP_NetTcpServerComponentHandler();
//==============================================================================
//import:


//==============================================================================
//override:

//#define LWIP_NetTcpServerComponentHandler() //TCPServerHandler(&TCPServerWIZspi)

#define LWIP_NetTcpServerComponentTimeSynchronization() //TCPServerTimeSynchronization(&TCPServerWIZspi)
//==============================================================================
//export:

extern xNetT LWIP_Net;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_LWIP_TCP_SERVER_COMPONENT_H_
