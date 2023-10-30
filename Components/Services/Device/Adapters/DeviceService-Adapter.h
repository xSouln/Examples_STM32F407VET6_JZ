//==============================================================================
//header:

#ifndef _DEVICE_SERVICE_ADAPTER_H_
#define _DEVICE_SERVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
#include "Services/Device/DeviceService.h"
#include "Abstractions/xPort/xPort.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
#endif

	uint32_t TimeStamp;

	uint16_t RxPacketHandlerIndex;

} DeviceServiceAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
	DeviceServiceAdapterInternalT Internal;
	xPortT* Port;

} DeviceServiceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xServiceAdapterBaseInitT Base;

	xPortT* Port;

} DeviceServiceAdapterInitT;
//==============================================================================
//functions:

xResult DeviceServiceAdapterInit(DeviceServiceT* service,
		DeviceServiceAdapterT* adapter,
		DeviceServiceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_DEVICE_SERVICE_ADAPTER_H_
