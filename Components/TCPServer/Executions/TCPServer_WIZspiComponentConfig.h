//==============================================================================
//module enable:

#include "TCPServer/TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef TCP_SERVER_WIZ_SPI_COMPONENT_CONFIG_H
#define TCP_SERVER_WIZ_SPI_COMPONENT_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "Components_Config.h"
//==============================================================================
//import:

//==============================================================================
//defines:

#define TCP_SERVER_WIZ_SPI_ADAPTER_ENABLE 1

#define TCP_SERVER_WIZ_SPI_RX_BUF_SIZE 0x100
#define TCP_SERVER_WIZ_SPI_RX_RECEIVER_BUF_SIZE 0x200

#define TCP_SERVER_WIZ_SPI_REG SPI4
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //TCP_SERVER_WIZ_SPI_COMPONENT_CONFIG_H
#endif //TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
