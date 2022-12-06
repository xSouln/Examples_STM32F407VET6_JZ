//==============================================================================
//includes:

#include <stdarg.h>
#include "Components.h"
//==============================================================================
//variables:

static uint8_t time1_ms;
static uint8_t time5_ms;
static uint16_t time1000_ms;
//==============================================================================
//functions:

static void PrivateTerminalComponentEventListener(TerminalT* terminal, TerminalEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		case TerminalEventTime_1000ms:
			xTxTransferSetTxLine(&Terminal.Transfer, &SerialPortUART.Tx);
			xTxTransferStart(&Terminal.Transfer, "qwerty", 6);
			break;
	}
}
//------------------------------------------------------------------------------

static void PrivateSerialPortComponentEventListener(SerialPortT* port, SerialPortEventSelector selector, void* arg, ...)
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
//==============================================================================
//default functions:

void ComponentsEventListener(ComponentObjectBaseT* object, int selector, void* arg, ...)
{
	if (object->Description->Key != OBJECT_DESCRIPTION_KEY)
	{
		return;
	}

	switch(object->Description->ObjectId)
	{
		case SERIAL_PORT_OBJECT_ID:
			PrivateSerialPortComponentEventListener((SerialPortT*)object, selector, arg);
			break;

		case TERMINAL_OBJECT_ID:
			PrivateTerminalComponentEventListener((TerminalT*)object, selector, arg);
			break;
	}
}
//------------------------------------------------------------------------------

void ComponentsRequestListener(ComponentObjectBaseT* port, int selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------
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

		#ifdef TCP_SERVER_COMPONENT_ENABLE
		TCPServerComponentHandler();
		#endif

		#ifdef ZIGBEE_COMPONENT_ENABLE
		ZigbeeComponentHandler();
		#endif
	}

	if (!time1000_ms)
	{
		time1000_ms = 1000;

		LED1_GPIO_Port->ODR ^= LED1_Pin;
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

	#ifdef TCP_SERVER_COMPONENT_ENABLE
	TCPServerComponentTimeSynchronization();
	#endif

	#ifdef ZIGBEE_COMPONENT_ENABLE
	ZigbeeComponentTimeSynchronization();
	#endif

	time1_ms = 0;

	if (time5_ms)
	{
		time5_ms--;
	}

	if (time1000_ms)
	{
		time1000_ms--;
	}
}
//------------------------------------------------------------------------------

void ComponentsSystemDelay(ComponentObjectBaseT* object, uint32_t time)
{
	HAL_Delay(time);
}
//------------------------------------------------------------------------------

void ComponentsTrace(char* text)
{

}
//------------------------------------------------------------------------------

void ComponentsSystemEnableIRQ(ComponentObjectBaseT* object)
{

}
//------------------------------------------------------------------------------

void ComponentsSystemDisableIRQ(ComponentObjectBaseT* object)
{

}
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
xResult ComponentsInit(void* parent)
{
	#if SERIAL_PORT_COMPONENT_ENABLE
	SerialPortComponentInit(parent);
	#endif

	#ifdef TCP_SERVER_COMPONENT_ENABLE
	TCPServerComponentInit(parent);
	#endif

	#ifdef ZIGBEE_COMPONENT_ENABLE
	ZigbeeComponentInit(parent);
	#endif

	#ifdef TERMINAL_COMPONENT_ENABLE
	TerminalComponentInit(parent);
	TerminalTxBind(&SerialPortUART.Tx);
	#endif

	return xResultAccept;
}
//==============================================================================
