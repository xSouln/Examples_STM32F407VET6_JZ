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
xResult SerialPortUART_AdapterInit(SerialPortT* port, SerialPortUART_AdapterT* adapter)
{
	if (port && adapter)
	{
		port->Adapter.Object.Description = nameof(SerialPortUART_AdapterT);
		port->Adapter.Object.Parent = port;

		port->Adapter.Child = adapter;
		
		port->Status.AdapterInitResult = xResultAccept;
	
		port->Status.TxInitResult = SerialPortUART_TxAdapterInit(port, adapter);
		port->Status.TxInitResult = SerialPortUART_RxAdapterInit(port, adapter);
		
		xRxTxBind(&port->Rx, &port->Tx);
		
		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //SERIAL_PORT_UART_ADAPTER_ENABLE
