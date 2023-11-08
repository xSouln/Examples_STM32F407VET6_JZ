//==============================================================================
//header:

#ifndef _CAN_LOCAL_REQUEST_CONTROL_ADAPTER_H_
#define _CAN_LOCAL_REQUEST_CONTROL_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xRequestControl/xRequestControl.h"
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

} CAN_LocalRequestControlAdapterContentT;
//------------------------------------------------------------------------------

typedef struct
{
	CAN_LocalRequestControlAdapterContentT Content;

	xRequestT* RequestBuffer;
	uint8_t RequestBufferSize;

	xPortT* Port;

} CAN_LocalRequestControlAdapterT;
//------------------------------------------------------------------------------

typedef struct
{
	xPortT* Port;

	xRequestT* RequestBuffer;
	uint8_t RequestBufferSize;

} CAN_LocalRequestControlAdapterInitT;
//==============================================================================
//functions:

xResult CAN_LocalRequestControlAdapterInit(xRequestControlT* control,
		CAN_LocalRequestControlAdapterT* adapter,
		CAN_LocalRequestControlAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_LOCAL_REQUEST_CONTROL_ADAPTER_H_
