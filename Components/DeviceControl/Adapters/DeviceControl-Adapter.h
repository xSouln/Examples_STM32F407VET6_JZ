//==============================================================================
//header:

#ifndef _DEVICE_CONTROL_ADAPTER_H_
#define _DEVICE_CONTROL_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDeviceControl.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
#endif

} DeviceControlAdapterT;
//------------------------------------------------------------------------------
typedef struct
{

} DeviceControlAdapterInitT;
//==============================================================================
//functions:

xResult DeviceControlAdapterInit(xDeviceControlT* control, DeviceControlAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_DEVICE_CONTROL_ADAPTER_H_
