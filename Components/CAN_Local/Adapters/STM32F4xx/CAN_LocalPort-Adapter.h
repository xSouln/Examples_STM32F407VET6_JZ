//==============================================================================
//header:

#ifndef _CAN_LOCAL_PORT_ADAPTER_H_
#define _CAN_LOCAL_PORT_ADAPTER_H_
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
#include "CAN_Local/Control/CAN_Local-Types.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
	SemaphoreHandle_t TxSemaphore;
#endif

	CAN_HandleTypeDef* CAN;
	CAN_RegT* CAN_Register;

	xCircleBufferT TxCircleBuffer;
	xCircleBufferT RxCircleBuffer;

	CAN_TxHeaderTypeDef TxHeader;

	struct
	{
		uint8_t AwaitTxValidation : 1;
		uint8_t TxValidationComplite : 1;
		uint8_t TxError : 2;
	};

	uint8_t TxRequestsCount;
	uint8_t TxAcceptedRequestsCount;

} CAN_LocalPortAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	CAN_HandleTypeDef* CAN;
	xCAN_Numbers CAN_Number;

	uint16_t FilterBank : 5;
	uint16_t SlaveStartFilterBank : 5;
	uint16_t FilterMode : 1;
	uint16_t FilterScale : 1;
	uint16_t FilterFIFOAssignment : 4;

	uint32_t FilterIdHigh;
	uint32_t FilterIdLow;

	uint32_t FilterMaskIdHigh;
	uint32_t FilterMaskIdLow;

	CAN_LocalSegmentT* RxBuffer;
	uint16_t RxBufferSizeMask;

	CAN_LocalSegmentT* TxBuffer;
	uint16_t TxBufferSizeMask;

} CAN_LocalPortAdapterInitT;
//==============================================================================
//functions:

xResult CAN_LocalPortAdapterInit(xPortT* port, struct xPortAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_LOCAL_PORT_ADAPTER_H_
