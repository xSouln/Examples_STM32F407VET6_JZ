//==============================================================================
//module enable:

#include "SerialPort/Adapters/SerialPort_AdapterConfig.h"
#ifdef SERIAL_PORT_UART_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef SERIAL_PORT_UART_ADAPTER_H
#define SERIAL_PORT_UART_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SerialPort/Controls/SerialPort_Types.h"
#include "Common/xRxReceiver.h"
//==============================================================================
//types:

typedef struct
{
	REG_UART_T* Usart;

	DMA_HandleTypeDef* RxDMA;

	xDataBufferT* ResponseBuffer;

	xCircleBufferT RxCircleBuffer;
	xRxReceiverT RxReceiver;

	xCircleBufferT TxCircleBuffer;

} SerialPortUART_AdapterT;
//==============================================================================
//functions:

xResult SerialPortUART_AdapterInit(SerialPortT* serial_port, SerialPortUART_AdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //SERIAL_PORT_UART_ADAPTER_H
#endif //SERIAL_PORT_UART_ADAPTER_ENABLE
