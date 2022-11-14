//==============================================================================
//module enable:

#include "TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef TCP_SERVER_COMPONENT_H
#define TCP_SERVER_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Config.h"
#include "TCPServer/Controls/TCPServer.h"
//==============================================================================
//functions:

xResult TCPServerComponentInit(void* parent);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* TCP_SERVER_COMPONENT_ENABLE */
