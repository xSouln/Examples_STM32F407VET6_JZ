//==============================================================================
//module enable:

#include "Components_Config.h"
#ifdef TCP_SERVER_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef TCP_SERVER_COMPONENT_CONFIG_H
#define TCP_SERVER_COMPONENT_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

//==============================================================================
//defines:

#define TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE 1
//==============================================================================
//selector:

#include "SerialPort/Executions/SerialPort_UART_ComponentConfig.h"
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //TCP_SERVER_COMPONENT_CONFIG_H
#endif //TCP_SERVER_COMPONENT_ENABLE
