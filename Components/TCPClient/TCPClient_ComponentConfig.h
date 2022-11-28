//==============================================================================
//module enable:

#include "Components_Config.h"
#ifdef TCP_CLIENT_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_COMPONENT_CONFIG_H
#define _TCP_CLIENT_COMPONENT_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:


//==============================================================================
//defines:

#define TCP_CLIENT_DEFAULT_PORT 5000
//==============================================================================
//selector:

#include "TCPClient/Executions/LWIP/TCPClient_LWIP_ComponentDependencies.h"
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_COMPONENT_CONFIG_H
#endif //TCP_CLIENT_COMPONENT_ENABLE
