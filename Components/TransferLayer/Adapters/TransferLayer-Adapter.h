//==============================================================================
//header:

#ifndef _TRANSFER_LAYER_ADAPTER_H_
#define _TRANSFER_LAYER_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xTransferLayer/xTransferLayer.h"
#include "Common/xCircleBuffer.h"
#include "Abstractions/xPort/xPort.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t CoreMutex;
#endif

	uint32_t RxPacketHandlerIndex;
	xCircleBufferT* PortRxCircleBuffer;

} TransferLayerAdapterContentT;
//------------------------------------------------------------------------------

typedef struct
{
	TransferLayerAdapterContentT Content;
	xPortT* Port;

} TransferLayerAdapterT;
//------------------------------------------------------------------------------

typedef struct
{
	xPortT* Port;

} TransferLayerAdapterInitT;
//==============================================================================
//functions:

xResult TransferLayerAdapterInit(xTransferLayerT* manager,
		TransferLayerAdapterT* adapter,
		TransferLayerAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TRANSFER_LAYER_ADAPTER_H_
