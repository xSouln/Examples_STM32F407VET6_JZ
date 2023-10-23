//==============================================================================
//header:

#ifndef _RELAY_SERVICE_ADAPTER_H_
#define _RELAY_SERVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "../RelayService.h"
#include "Abstractions/xDevice/xService.h"
//==============================================================================
//types:

typedef struct
{
#ifdef INC_FREERTOS_H
	SemaphoreHandle_t TransactionMutex;
#endif

	uint32_t TimeStamp;

} RelayServiceAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
	RelayServiceAdapterInternalT Internal;

} RelayServiceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xServiceAdapterBaseInitT Base;

} RelayServiceAdapterInitT;
//==============================================================================
//functions:

xResult RelayServiceAdapterInit(RelayServiceT* service,
		RelayServiceAdapterT* adapter,
		RelayServiceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_RELAY_SERVICE_ADAPTER_H_
