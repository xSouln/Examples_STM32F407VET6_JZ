//==============================================================================
//header:

#include "Components-Config.h"

#if !defined(_NET_ADAPTER_H_) && NET_COMPONENT_ENABLE == 1 && NET_TARGET_LAYOUT == NET_FREERTOS_LAYOUT
#define _NET_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xNet/xNet.h"

#include "FreeRTOS_IP.h"
//==============================================================================
//types:

typedef struct
{


} NetAdapterT;
//------------------------------------------------------------------------------
typedef struct
{

	
} NetAdapterInitT;
//==============================================================================
//functions:

xResult NetAdapterInit(xNetT* net, NetAdapterT* adapter, NetAdapterInitT* adapterInit);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_NET_ADAPTER_H_
