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

#include "../TemperatureService.h"
#include "Abstractions/xDevice/xService.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
#endif

	uint32_t TimeStamp;

} TemperatureServiceAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
	TemperatureServiceAdapterInternalT Internal;

} TemperatureServiceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xServiceAdapterBaseInitT Base;

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
