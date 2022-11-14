//==============================================================================
//module enable:

#include "SerialPort/Adapters/SerialPort_AdapterConfig.h"
#ifdef SERIAL_PORT_UART_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "SerialPort_UART_Adapter.h"
#include "SerialPort_UART_TxAdapter.h"
#include "SerialPort_UART_RxAdapter.h"
//==============================================================================
xResult SerialPortUART_AdapterInit(SerialPortT* serial_port, SerialPortUART_AdapterT* adapter)
{
	if (serial_port && adapter)
	{
		serial_port->Adapter = adapter;
		
		serial_port->Status.AdapterInitResult = xResultAccept;
	
		serial_port->Status.TxInitResult = SerialPortUART_TxAdapterInit(serial_port, adapter);
		serial_port->Status.TxInitResult = SerialPortUART_RxAdapterInit(serial_port, adapter);
		
		serial_port->Rx.Tx = &serial_port->Tx;
		
		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //SERIAL_PORT_UART_ADAPTER_ENABLE
