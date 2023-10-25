//==============================================================================
//header:

#ifndef _SERVER_DEVICE_ADAPTER_H_
#define _SERVER_DEVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
#include "Abstractions/xPort/xPort.h"
#include "Common/xCircleBuffer.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t CommandExecutionMutex;
	SemaphoreHandle_t CommandAccomplishSemaphore;
#endif

	xDeviceCommandT* Command;
	xResult CommandExecutionResult;

	uint32_t OperationTimeStamp;
	uint32_t OperationTimeOut;
	uint16_t Operation;

	uint16_t RxPacketHandlerIndex;
	xCircleBufferT* PortRxCircleBuffer;

} ServerDeviceAdapterContentT;
//------------------------------------------------------------------------------
typedef struct
{
	ServerDeviceAdapterContentT Content;

	xPortT* Port;

} ServerDeviceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xPortT* Port;

} ServerDeviceAdapterInitT;
//==============================================================================
//functions:

xResult ServerDeviceAdapterInit(xDeviceT* device, ServerDeviceAdapterT* adapter, ServerDeviceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SERVER_DEVICE_ADAPTER_H_
