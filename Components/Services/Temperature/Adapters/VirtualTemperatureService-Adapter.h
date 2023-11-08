//==============================================================================
//header:

#ifndef _VIRTUAL_TEMPERATURE_SERVICE_ADAPTER_H_
#define _VIRTUAL_TEMPERATURE_SERVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
#include "Services/Temperature/TemperatureService.h"
#include "Abstractions/xPort/xPort.h"
//==============================================================================
//types:

typedef struct
{
	uint32_t TimeStamp;

	uint16_t RxPacketHandlerIndex;

} VirtualTemperatureServiceAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
	VirtualTemperatureServiceAdapterInternalT Internal;

} VirtualTemperatureServiceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xServiceAdapterBaseInitT Base;

} VirtualTemperatureServiceAdapterInitT;
//==============================================================================
//functions:

xResult VirtualTemperatureServiceAdapterInit(TemperatureServiceT* service,
		VirtualTemperatureServiceAdapterT* adapter,
		VirtualTemperatureServiceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_VIRTUAL_TEMPERATURE_SERVICE_ADAPTER_H_
