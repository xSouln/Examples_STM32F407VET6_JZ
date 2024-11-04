//==============================================================================
//header:

#include "Components-Config.h"

#if !defined(_NET_PORT_ADAPTER_H_) && NET_COMPONENT_ENABLE == 1 && NET_TARGET_LAYOUT == NET_FREERTOS_LAYOUT
#define _NET_PORT_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components-Types.h"
#include "Common/xRxReceiver.h"
#include "Common/xDataBuffer.h"
#include "Common/xCircleBuffer.h"
#include "Abstractions/xNet/xNet.h"
#include "Abstractions/xPort/xPort.h"
//==============================================================================
//types:

typedef struct
{
	xCircleBufferT RxCircleBuffer;
	uint8_t RxCircleBufferMemory[512];

	TaskHandle_t RxTaskHandle;

} NetPortAdapterInternalT;
//------------------------------------------------------------------------------
typedef struct
{
	//xPortAdapterBaseT Base;

	xNetT* Net;

	xRxReceiverT RxReceiver;
	xDataBufferT TxBuffer;

	NetPortAdapterInternalT Internal;

	uint8_t* RxOperationBuffer;
	int RxOperationBufferSize;

	SemaphoreHandle_t TransactionMutex;

} NetPortAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xNetT* Net;

	uint8_t* RxOperationBuffer;
	int RxOperationBufferSize;

	uint8_t* TxBuffer;
	int TxBufferSize;

	uint8_t* RxBuffer;
	int RxBufferSize;

} NetPortAdapterInitT;
//==============================================================================
//functions:

xResult NetPortAdapterInit(xPortT* port, NetPortAdapterT* adapter, NetPortAdapterInitT* adapterInit);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_NET_PORT_ADAPTER_H_
