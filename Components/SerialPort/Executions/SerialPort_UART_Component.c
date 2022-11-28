//==============================================================================
//module enable:

#include "SerialPort_UART_ComponentConfig.h"
#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SerialPort_UART_Component.h"
#include "SerialPort/Adapters/UART/SerialPort_UART_Adapter.h"

#include "Components.h"

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Controls/Terminal.h"
#endif
//==============================================================================
//defines:

#define RX_CIRCLE_BUF_SIZE_MASK (SERIAL_PORT_UART_RX_CIRCLE_BUF_SIZE_MASK)
#define RX_OBJECT_BUF_SIZE (SERIAL_PORT_UART_RX_OBJECT_BUF_SIZE)
#define TX_CIRCLE_BUF_SIZE_MASK (SERIAL_PORT_UART_TX_CIRCLE_BUF_SIZE_MASK)
//==============================================================================
//variables:

static uint8_t uart_rx_circle_buf[RX_CIRCLE_BUF_SIZE_MASK + 1];
static uint8_t rx_object_buf[RX_OBJECT_BUF_SIZE];
static uint8_t tx_circle_buf[TX_CIRCLE_BUF_SIZE_MASK + 1];

SerialPortT SerialPortUART;
//==============================================================================
//import:

//==============================================================================
//functions:

static void _SerialPortUARTComponentEventListener(SerialPortT* port, SerialPortEventSelector selector, void* arg, ...)
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

static xResult _SerialPortUARTComponentRequestListener(SerialPortT* port, SerialPortRequestSelector selector, void* arg, ...)
{
	switch ((uint8_t)selector)
	{
		default: return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

void _SerialPortUARTComponentIRQListener()
{
	SerialPortIRQListener(&SerialPortUART);
}
//------------------------------------------------------------------------------
//component functions:
/**
 * @brief main handler
 */
void _SerialPortUARTComponentHandler()
{
	SerialPortHandler(&SerialPortUART);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void _SerialPortUARTComponentTimeSynchronization()
{
	SerialPortTimeSynchronization(&SerialPortUART);
}

//==============================================================================
//initialization:

static SerialPortUART_AdapterT SerialPortUART_Adapter =
{
	.Usart =  (REG_UART_T*)SERIAL_PORT_UART_REG,

	#ifdef SERIAL_PORT_UART_RX_DMA
	.RxDMA = &SERIAL_PORT_UART_RX_DMA,
	#endif
};
//------------------------------------------------------------------------------

static SerialPortInterfaceT SerialPortInterface =
{
	INITIALIZATION_EVENT_LISTENER(SerialPort, ComponentsEventListener),
	INITIALIZATION_REQUEST_LISTENER(SerialPort, ComponentsRequestListener)
};
//==============================================================================
//component initialization:

xResult _SerialPortUARTComponentInit(void* parent)
{
	#ifdef TERMINAL_COMPONENT_ENABLE
	SerialPortUART_Adapter.ResponseBuffer = &Terminal.ResponseBuffer;
	#endif

	xCircleBufferInit(&SerialPortUART_Adapter.RxCircleBuffer,
										&SerialPortUART.Rx,
										uart_rx_circle_buf,
										RX_CIRCLE_BUF_SIZE_MASK);
	
	xCircleBufferInit(&SerialPortUART_Adapter.TxCircleBuffer,
										&SerialPortUART.Tx,
										tx_circle_buf,
										TX_CIRCLE_BUF_SIZE_MASK);
	
	xRxReceiverInit(&SerialPortUART_Adapter.RxReceiver,
									&SerialPortUART.Rx,
									0,
									rx_object_buf,
									RX_OBJECT_BUF_SIZE);
	
	SerialPortUART_AdapterInit(&SerialPortUART, &SerialPortUART_Adapter);
	SerialPortInit(&SerialPortUART, parent, &SerialPortInterface);
  
	return 0;
}
//==============================================================================
#endif //SERIAL_PORT_UART_COMPONENT_ENABLE
