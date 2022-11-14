//==============================================================================
//module enable:

#include "SerialPort/SerialPort_ComponentConfig.h"
#ifdef SERIAL_PORT_UART_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SerialPort_UART_Component.h"
#include "SerialPort/Adapters/UART/SerialPort_UART_Adapter.h"
//==============================================================================
//defines:

#define RX_CIRCLE_BUF_SIZE_MASK (SERIAL_PORT_UART_RX_CIRCLE_BUF_SIZE_MASK)
#define RX_OBJECT_BUF_SIZE (SERIAL_PORT_UART_RX_OBJECT_BUF_SIZE)
#define TX_CIRCLE_BUF_SIZE_MASK (SERIAL_PORT_UART_TX_CIRCLE_BUF_SIZE_MASK)
//==============================================================================
//variables:

static uint8_t rx_circle_buf[RX_CIRCLE_BUF_SIZE_MASK + 1];
static uint8_t rx_object_buf[RX_OBJECT_BUF_SIZE];
static uint8_t tx_circle_buf[TX_CIRCLE_BUF_SIZE_MASK + 1];

SerialPortT SerialPortUART;
//==============================================================================
//import:

#ifdef TERMINAL_COMPONENT_ENABLE
extern void TerminalRequestsReceiver(xRxT* rx, uint8_t* data, uint32_t size);
#endif
//==============================================================================
//functions:

static void EventListener(SerialPortT* serial_port, SerialPortEventSelector selector, uint32_t args, uint32_t count)
{
	//SerialPortUART_AdapterT* adapter = serial_port->Adapter;

	switch ((uint8_t)selector)
	{
		case SerialPortEventEndLine:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalRequestsReceiver(&serial_port->Rx, (uint8_t*)args, count);
			#endif
			break;
		
		case SerialPortEventReceiverBufferIsFull:
			#ifdef TERMINAL_COMPONENT_ENABLE
			TerminalRequestsReceiver(&serial_port->Rx, (uint8_t*)args, count);
			#endif
			break;
	}
}
//==============================================================================
//component functions:
/**
 * @brief main handler
 */
inline void SerialPortUARTComponentHandler()
{
	SerialPortHandler(&SerialPortUART);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void SerialPortUARTComponentTimeSynchronization()
{

}
//------------------------------------------------------------------------------

inline void SerialPortUARTComponentTxIRQ()
{
	SerialPortEventTxIRQ(&SerialPortUART);
}
//------------------------------------------------------------------------------

inline void SerialPortUARTComponentRxIRQ()
{

}
//==============================================================================
//initialization:

static SerialPortInterfaceT Interface =
{
	.EventListener = (SerialPortEventListenerT)EventListener
};
//------------------------------------------------------------------------------

static SerialPortUART_AdapterT Adapter =
{
	.Usart =  (REG_UART_T*)SERIAL_PORT_UART_REG,

	#ifdef SERIAL_PORT_UART_RX_DMA
	.RxDMA = &SERIAL_PORT_UART_RX_DMA,
	#endif
};
//==============================================================================
//component initialization:

xResult SerialPortUARTComponentInit(void* parent)
{
	extern xDataBufferT ResponseBuffer;
	Adapter.ResponseBuffer = &ResponseBuffer;
	
	xCircleBufferInit(&Adapter.RxCircleBuffer,
										&SerialPortUART.Rx,
										rx_circle_buf,
										RX_CIRCLE_BUF_SIZE_MASK);
	
	xCircleBufferInit(&Adapter.TxCircleBuffer,
										&SerialPortUART.Tx,
										tx_circle_buf,
										TX_CIRCLE_BUF_SIZE_MASK);
	
	xRxReceiverInit(&Adapter.RxReceiver,
									&SerialPortUART.Rx,
									0,
									rx_object_buf,
									RX_OBJECT_BUF_SIZE);
	
	SerialPortInit(&SerialPortUART, parent, &Interface);
	SerialPortUART_AdapterInit(&SerialPortUART, &Adapter);
  
  return 0;
}
//==============================================================================
#endif //SERIAL_PORT_UART_COMPONENT_ENABLE
