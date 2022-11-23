//==============================================================================
//module enable:

#include "SerialPort_ComponentConfig.h"
#ifdef SERIAL_PORT_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SerialPort_Component.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================

static void _SerialPortComponentEventListener(SerialPortT* port, SerialPortEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		case SerialPortEventEndLine:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalReceiveData(&port->Rx,
								((SerialPortReceivedDataT*)arg)->Data,
								((SerialPortReceivedDataT*)arg)->Size);
			#endif
			break;

		case SerialPortEventBufferIsFull:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalReceiveData(&port->Rx,
								((SerialPortReceivedDataT*)arg)->Data,
								((SerialPortReceivedDataT*)arg)->Size);
			#endif
			break;

		default: break;
	}
}
//------------------------------------------------------------------------------

static xResult _SerialPortComponentRequestListener(SerialPortT* port, SerialPortRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _SerialPortComponentIRQListener(SerialPortT* port)
{
	SerialPortIRQListener(port);
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void _SerialPortComponentHandler(SerialPortT* port)
{
	SerialPortHandler(port);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _SerialPortComponentTimeSynchronization(SerialPortT* port)
{
	SerialPortTimeSynchronization(port);
}
//==============================================================================

static SerialPortInterfaceT SerialPortInterface =
{
	INITIALIZATION_EVENT_LISTENER(SerialPort, _SerialPortComponentEventListener),
	INITIALIZATION_REQUEST_LISTENER(SerialPort, _SerialPortComponentRequestListener)
};
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
xResult _SerialPortComponentInit(SerialPortT* port, void* parent)
{
	SerialPortInit(port, parent, &SerialPortInterface);

	return 0;
}
//==============================================================================
#endif //SERIAL_PORT_COMPONENT_ENABLE
