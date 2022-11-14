//==============================================================================
#ifndef SERIAL_PORT_H_
#define SERIAL_PORT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SerialPort/Controls/SerialPort_Types.h"
//==============================================================================
//functions:

void SerialPortHandler(SerialPortT* serial_port);

xResult SerialPortInit(SerialPortT* serial_port, void* parent, SerialPortInterfaceT* interface);

void SerialPortEventTxIRQ(SerialPortT* serial_port);
void SerialPortEventRxIRQ(SerialPortT* serial_port);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* SERIAL_PORT_H_ */
