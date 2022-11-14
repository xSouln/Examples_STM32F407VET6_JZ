//==============================================================================
//module enable:

#include "SerialPort_ComponentConfig.h"
#ifdef SERIAL_PORT_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SerialPort_Component.h"
//==============================================================================
/**
 * @brief main handler
 */
inline void SerialPortComponentHandler()
{
	#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
	SerialPortUARTComponentHandler();
	#endif
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void SerialPortComponentTimeSynchronization()
{
	#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
	SerialPortUARTComponentTimeSynchronization();
	#endif
}
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
int SerialPortComponentInit(void* parent)
{
	#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
	SerialPortUARTComponentInit(parent);
	#endif

	return 0;
}
//==============================================================================
#endif //SERIAL_PORT_COMPONENT_ENABLE
