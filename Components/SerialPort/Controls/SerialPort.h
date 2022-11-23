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
//macros:

#define SerialPortEventTxIRQ(port) xTxIRQListener(&port.Tx)
#define SerialPortEventRxIRQ(port) xTxIRQEventListener(&port.Rx)
//==============================================================================
//functions:

xResult SerialPortInit(SerialPortT* port, void* parent, SerialPortInterfaceT* interface);

void _SerialPortHandler(SerialPortT* port);
void _SerialPortTimeSynchronization(SerialPortT* port);
void _SerialPortIRQListener(SerialPortT* serial_port);
void _SerialPortEventListener(SerialPortT* port, SerialPortEventSelector selector, void* arg, ...);
xResult _SerialPortRequestListener(SerialPortT* port, SerialPortRequestSelector selector, void* arg, ...);
//==============================================================================
//import:


//==============================================================================
//override:

#define SerialPortHandler(port) _SerialPortHandler(port)
#define SerialPortTimeSynchronization(port) _SerialPortTimeSynchronization(port)

#define SerialPortIRQListener(port) _SerialPortIRQListener(port)

#define SerialPortEventListener(port, selector, arg, ...) ((SerialPortT*)port)->Interface->EventListener(port, selector, arg, ##__VA_ARGS__)
#define SerialPortRequestListener(port, selector, arg, ...) ((SerialPortT*)port)->Interface->RequestListener(port, selector, arg, ##__VA_ARGS__)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //SERIAL_PORT_H_
