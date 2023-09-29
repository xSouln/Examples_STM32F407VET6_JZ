//==============================================================================
//header:

#ifndef _CAN_PORT_ADAPTER_H_
#define _CAN_PORT_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Peripherals/CAN/xCAN.h"
#include "Abstractions/xPort/xPort.h"
#include "Common/xRxReceiver.h"
#include "Common/xDataBuffer.h"
#include "can.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
#endif

	CAN_HandleTypeDef* CAN;
	CAN_RegT* CAN_Register;

	xCircleBufferT RxCircleBuffer;
	xRxReceiverT RxReceiver;

	xCircleBufferT TxCircleBuffer;

	CAN_TxHeaderTypeDef TxHeader;

	struct
	{
		uint8_t AwaitTxValidation : 1;
		uint8_t TxValidationComplite : 1;
		uint8_t TxError : 2;
	};

	uint8_t TxRequestsCount;
	uint8_t TxAcceptedRequestsCount;

} CAN_PortAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	CAN_HandleTypeDef* CAN;
	xCAN_Numbers CAN_Number;

	CAN_FilterTypeDef* FilterConfig;

	uint8_t* RxBuffer;
	uint16_t RxBufferSizeMask;

	uint8_t* RxResponseBuffer;
	uint16_t RxResponseBufferSize;

	uint8_t* TxBuffer;
	uint16_t TxBufferSizeMask;

} CAN_PortAdapterInitT;
//==============================================================================
//functions:

xResult CAN_PortAdapterInit(xPortT* port, xPortAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_PORT_ADAPTER_H_
