//==============================================================================
//includes:

#include "Common/xMemory.h"
#include "SerialPort.h"
#include "SerialPort/SerialPort_Component.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

void _SerialPortEventListener(SerialPortT* port, SerialPortEventSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------

xResult _SerialPortRequestListener(SerialPortT* port, SerialPortRequestSelector selector, void* arg, ...)
{
	switch((uint8_t)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _SerialPortIRQListener(SerialPortT* port)
{
	xRxIRQListener(&port->Rx);
	xTxIRQListener(&port->Tx);
}
//------------------------------------------------------------------------------

void _SerialPortHandler(SerialPortT* port)
{
	xRxHandler(&port->Rx);
	xTxHandler(&port->Tx);
}
//------------------------------------------------------------------------------

void _SerialPortTimeSynchronization(SerialPortT* port)
{

}
//==============================================================================
//initialization:

static const ObjectDescriptionT SerialPortObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = SERIAL_PORT_OBJECT_ID,
	.Type = nameof(SerialPortT)
};
//------------------------------------------------------------------------------

xResult SerialPortInit(SerialPortT* port, void* parent, SerialPortInterfaceT* interface)
{
	if (port)
	{
		port->Object.Description = &SerialPortObjectDescription;
		port->Object.Parent = parent;
		
		port->Interface = interface;
		
		port->Status.InitResult = xResultAccept;
		
		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
