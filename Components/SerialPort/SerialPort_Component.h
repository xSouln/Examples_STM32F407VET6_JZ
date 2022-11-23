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
#include "SerialPort/Controls/SerialPort.h"
//==============================================================================
//configurations:

#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
#include "SerialPort/Executions/SerialPort_UART_Component.h"
#endif
//==============================================================================
//defines:


//==============================================================================
//macros:


//==============================================================================
//functions:

xResult _SerialPortComponentInit(SerialPortT* port, void* parent);

void _SerialPortComponentHandler(SerialPortT* port);
void _SerialPortComponentTimeSynchronization(SerialPortT* port);

void _SerialPortComponentIRQListener(SerialPortT* port);
//==============================================================================
//export:


//==============================================================================
//override:

#ifndef SerialPortComponentInit
#define SerialPortComponentInit(port, parent)\
	_SerialPortComponentInit(port, parent)
#endif
//------------------------------------------------------------------------------
#ifndef SerialPortComponentHandler
#define SerialPortComponentHandler(port)\
	_SerialPortComponentHandler(port)
#endif
//------------------------------------------------------------------------------
#ifndef SerialPortComponentTimeSynchronization
#define SerialPortComponentTimeSynchronization(port)\
	_SerialPortComponentTimeSynchronization(port)
#endif
//------------------------------------------------------------------------------
#ifndef SerialPortComponentIRQListener
#define SerialPortComponentIRQListener(port)\
	_SerialPortComponentIRQListener(port)
#endif
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //SERIAL_PORT_COMPONENT_H
#endif //SERIAL_PORT_COMPONENT_ENABLE
