//==============================================================================
//module enable:

#include "Components_Config.h"
#ifdef TCP_SERVER_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_SERVER_COMPONENT_CONFIG_H
#define _TCP_SERVER_COMPONENT_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:


//==============================================================================
//defines:

#define TCP_SERVER_DEFAULT_PORT 5000
//==============================================================================
//selector:

#include "TCPServer/Executions/LWIP/TCPServer_LWIP_ComponentDependencies.h"
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_COMPONENT_CONFIG_H
#endif //TCP_SERVER_COMPONENT_ENABLE
