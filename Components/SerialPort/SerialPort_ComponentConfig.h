//==============================================================================
//module enable:

#include "Components_Config.h"
#ifdef SERIAL_PORT_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef SERIAL_PORT_COMPONENT_CONFIG_H
#define SERIAL_PORT_COMPONENT_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

//==============================================================================
//defines:

#define SERIAL_PORT_UART_COMPONENT_ENABLE 1
//==============================================================================
//selector:

#include "SerialPort/Executions/SerialPort_UART_ComponentConfig.h"
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //SERIAL_PORT_COMPONENT_CONFIG_H
#endif //SERIAL_PORT_COMPONENT_ENABLE
