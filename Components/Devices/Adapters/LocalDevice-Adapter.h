//==============================================================================
//header:

#ifndef _LOCAL_DEVICE_ADAPTER_H_
#define _LOCAL_DEVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
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

} LocalDeviceAdapterContentT;
//------------------------------------------------------------------------------
typedef struct
{
	LocalDeviceAdapterContentT Content;

} LocalDeviceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{

} LocalDeviceAdapterInitT;
//==============================================================================
//functions:

xResult LocalDeviceAdapterInit(xDeviceT* device, LocalDeviceAdapterT* adapter, LocalDeviceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_LOCAL_DEVICE_ADAPTER_H_
