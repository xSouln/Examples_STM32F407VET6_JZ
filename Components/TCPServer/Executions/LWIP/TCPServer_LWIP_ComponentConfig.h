//==============================================================================
//module enable:

#include "TCPServer/TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_LWIP_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_SERVER_LWIP_COMPONENT_CONFIG_H
#define _TCP_SERVER_LWIP_COMPONENT_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Types.h"
//==============================================================================
//macros:


//==============================================================================
//import:


//==============================================================================
//defines:

#define TCP_SERVER_LWIP_RX_BUF_SIZE 0x100
#define TCP_SERVER_LWIP_RX_RECEIVER_BUF_SIZE 0x200
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_SERVER_LWIP_COMPONENT_CONFIG_H
#endif //TCP_SERVER_LWIP_COMPONENT_ENABLE
