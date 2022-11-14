//==============================================================================
//module enable:

#include "SerialPort_ComponentConfig.h"
#ifdef SERIAL_PORT_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef SERIAL_PORT_COMPONENT_H
#define SERIAL_PORT_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "SerialPort_ComponentTypes.h"
#include "SerialPort/SerialPort_ComponentConfig.h"
//==============================================================================
//configurations:

#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
#include "SerialPort/Executions/SerialPort_UART_Component.h"
#endif
//==============================================================================
//defines:


//==============================================================================
//functions:

int SerialPortComponentInit(void* parent);

void SerialPortComponentHandler();
void SerialPortComponentTimeSynchronization();
//==============================================================================
//export:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //SERIAL_PORT_COMPONENT_H
#endif //SERIAL_PORT_COMPONENT_ENABLE
