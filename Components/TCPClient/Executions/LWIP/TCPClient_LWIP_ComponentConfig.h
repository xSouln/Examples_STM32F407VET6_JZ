//==============================================================================
//module enable:

#include "TCPClient/TCPClient_ComponentConfig.h"
#ifdef TCP_CLIENT_LWIP_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _TCP_CLIENT_LWIP_COMPONENT_CONFIG_H
#define _TCP_CLIENT_LWIP_COMPONENT_CONFIG_H
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

#define TCP_CLIENT_LWIP_RX_OBJECT_BUF_SIZE (0x3ff + 1)
#define TCP_CLIENT_LWIP_RX_CIRCLE_BUF_SIZE_MASK 0xfff
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TCP_CLIENT_LWIP_COMPONENT_CONFIG_H
#endif //TCP_CLIENT_LWIP_COMPONENT_ENABLE
