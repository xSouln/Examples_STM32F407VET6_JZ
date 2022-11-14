//==============================================================================
//module enable:

#include "SerialPort/Adapters/SerialPort_AdapterConfig.h"
#ifdef SERIAL_PORT_UART_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef SERIAL_PORT_UART_RX_ADAPTER_H
#define SERIAL_PORT_UART_RX_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SerialPort_UART_Adapter.h"
//==============================================================================
//functions:

xResult SerialPortUART_RxAdapterInit(SerialPortT* serial_port, SerialPortUART_AdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //SERIAL_PORT_UART_RX_ADAPTER_H
#endif //SERIAL_PORT_UART_ADAPTER_ENABLE
