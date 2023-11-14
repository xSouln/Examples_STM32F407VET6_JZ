//==============================================================================
//header:

#ifndef _TEMPERATURE_SERVICE_ADAPTER_H_
#define _TEMPERATURE_SERVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
#include "Services/Temperature/TemperatureService.h"
#include "Abstractions/xPort/xPort.h"
#include "Common/xCircleBuffer.h"
//==============================================================================
//types:

typedef struct
{
	uint32_t TimeStamp;

	uint16_t RxPacketHandlerIndex;

} TemperatureServiceAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
	TemperatureServiceAdapterInternalT Internal;
	xPortT* Port;

} TemperatureServiceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xServiceAdapterBaseInitT Base;
	xPortT* Port;

} TemperatureServiceAdapterInitT;
//==============================================================================
//functions:

xResult TemperatureServiceAdapterInit(TemperatureServiceT* service,
		TemperatureServiceAdapterT* adapter,
		TemperatureServiceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TEMPERATURE_SERVICE_ADAPTER_H_
