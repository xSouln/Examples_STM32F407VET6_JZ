//==============================================================================
#include "Common/xMemory.h"
#include "SerialPort.h"
//==============================================================================
void SerialPortHandler(SerialPortT* serial_port)
{
	xRxHandler(&serial_port->Rx);
	xTxHandler(&serial_port->Tx);
}
//------------------------------------------------------------------------------
void SerialPortEventTxIRQ(SerialPortT* serial_port)
{
	xTxEventListener(&serial_port->Tx, xTxEventIRQ, 0, 0);
}
//------------------------------------------------------------------------------
void SerialPortEventRxIRQ(SerialPortT* serial_port)
{
	xRxEventListener(&serial_port->Rx, xRxEventIRQ, 0, 0);
}
//------------------------------------------------------------------------------
xResult SerialPortInit(SerialPortT* serial_port, void* parent, SerialPortInterfaceT* interface)
{
	if (serial_port && interface)
	{
		serial_port->Description = "SerialPortT";
		serial_port->Parent = parent;
		
		serial_port->Interface = interface;
		
		serial_port->Status.InitResult = xResultAccept;
		
		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
