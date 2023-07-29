//==============================================================================
//header:

#ifndef _USART_PORT_ADAPTER_H_
#define _USART_PORT_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xPort/xPort.h"
#include "Common/xRxReceiver.h"
#include "Common/xDataBuffer.h"
#include "Registers/registers.h"
//==============================================================================
//types:

typedef struct
{
	SemaphoreHandle_t TransactionMutex;

	REG_UART_T* Usart;
	DMA_HandleTypeDef* RxDMA;

	//xDataBufferT* ResponseBuffer;

	xCircleBufferT RxCircleBuffer;
	xRxReceiverT RxReceiver;

	xCircleBufferT TxCircleBuffer;

} UsartPortAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	REG_UART_T* Usart;
	DMA_HandleTypeDef* RxDMA;

	uint8_t* RxBuffer;
	uint16_t RxBufferSizeMask;

	uint8_t* RxResponseBuffer;
	uint16_t RxResponseBufferSize;

	uint8_t* TxBuffer;
	uint16_t TxBufferSizeMask;

} UsartPortAdapterInitT;
//==============================================================================
//functions:

xResult UsartPortAdapterInit(xPortT* port, xPortAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_USART_PORT_ADAPTER_H_
