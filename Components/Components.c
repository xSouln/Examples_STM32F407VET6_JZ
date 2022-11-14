//==============================================================================
#include "Components.h"
//==============================================================================
static uint8_t time1_ms;
static uint8_t time5_ms;
//==============================================================================
/**
 * @brief main handler
 */
void ComponentsHandler()
{
	if (!time1_ms)
	{
		time1_ms = 1;
	}

	if (!time5_ms)
	{
		time5_ms = 5;

	#ifdef SERIAL_PORT_COMPONENT_ENABLE
	SerialPortComponentHandler();
	#endif

	#ifdef TERMINAL_COMPONENT_ENABLE
	TerminalComponentHandler();
	#endif
	}
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void ComponentsTimeSynchronization()
{
	#ifdef SERIAL_PORT_COMPONENT_ENABLE
	SerialPortComponentTimeSynchronization();
	#endif

	#ifdef TERMINAL_COMPONENT_ENABLE
	TerminalComponentTimeSynchronization();
	#endif

	time1_ms = 0;

	if (time5_ms)
	{
		time5_ms--;
	}
}
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
int ComponentsInit(void* parent)
{
	#ifdef TERMINAL_COMPONENT_ENABLE
	TerminalComponentInit(parent);
	#endif

	#if SERIAL_PORT_COMPONENT_ENABLE
	SerialPortComponentInit(parent);
	#endif

	return 0;
}
//==============================================================================
