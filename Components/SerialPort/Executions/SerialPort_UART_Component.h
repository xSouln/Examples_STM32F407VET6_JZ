//==============================================================================
//module enable:

#include "SerialPort/SerialPort_ComponentConfig.h"
#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef SERIAL_PORT_UART_COMPONENT_H
#define SERIAL_PORT_UART_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SerialPort_UART_ComponentConfig.h"
#include "SerialPort/Controls/SerialPort.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult SerialPortUARTComponentInit(void* parent);
void SerialPortUARTComponentHandler();
void SerialPortUARTComponentTimeSynchronization();

void SerialPortUARTComponentTxIRQ();
void SerialPortUARTComponentRxIRQ();
//==============================================================================
//export:

extern SerialPortT SerialPortUART;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //SERIAL_PORT_UART_COMPONENT_H
#endif //SERIAL_PORT_UART_COMPONENT_ENABLE
