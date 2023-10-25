//==============================================================================
//header:

#ifndef _CLIENT_DEVICE_ADAPTER_H_
#define _CLIENT_DEVICE_ADAPTER_H_
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

} ClientDeviceAdapterContentT;
//------------------------------------------------------------------------------
typedef struct
{
	ClientDeviceAdapterContentT Content;

	xPortT* Port;

} ClientDeviceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xPortT* Port;

} ClientDeviceAdapterInitT;
//==============================================================================
//functions:

xResult ClientDeviceAdapterInit(xDeviceT* device, ClientDeviceAdapterT* adapter, ClientDeviceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CLIENT_DEVICE_ADAPTER_H_
