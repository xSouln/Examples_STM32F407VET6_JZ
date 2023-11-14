//==============================================================================
//header:

#ifndef _VIRTUAL_GAP_SERVICE_ADAPTER_H_
#define _VIRTUAL_GAP_SERVICE_ADAPTER_H_
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

typedef struct PACKED_PREFIX
{
	uint32_t TimeStamp;

	uint16_t RxPacketHandlerIndex;

} VirtualGAPServiceAdapterInternalT;
//------------------------------------------------------------------------------

typedef struct
{
	VirtualGAPServiceAdapterInternalT Internal;

} VirtualGAPServiceAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	xServiceAdapterBaseInitT Base;

} VirtualGAPServiceAdapterInitT;
//==============================================================================
//functions:

xResult VirtualGAPServiceAdapterInit(GAPServiceT* service,
		VirtualGAPServiceAdapterT* adapter,
		VirtualGAPServiceAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_VIRTUAL_GAP_SERVICE_ADAPTER_H_
