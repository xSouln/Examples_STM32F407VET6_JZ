//==============================================================================
//header:

#ifndef _GAP_SERVICE_ADAPTER_H_
#define _GAP_SERVICE_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xDevice.h"
#include "Services/GAP/GAPService.h"
#include "Abstractions/xPort/xPort.h"
//==============================================================================
//types:

typedef struct
{
	uint32_t TimeStamp;

	uint16_t RxPacketHandlerIndex;

} GAPServiceAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
	GAPServiceAdapterInternalT Internal;
	xPortT* Port;

} GAPServiceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xServiceAdapterBaseInitT Base;

	xPortT* Port;

} GAPServiceAdapterInitT;
//==============================================================================
//functions:

xResult GAPServiceAdapterInit(GAPServiceT* service,
		GAPServiceAdapterT* adapter,
		GAPServiceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_GAP_SERVICE_ADAPTER_H_
