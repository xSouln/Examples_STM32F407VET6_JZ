//==============================================================================
//header:

#ifndef _LWIP_NET_ADAPTER_H_
#define _LWIP_NET_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xNet/xNet.h"
#include "lwip.h"
//==============================================================================
//types:

typedef struct
{
	struct netif* netif;

} LWIP_NetAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	struct netif* gnetif;
	
} LWIP_NetAdapterInitT;
//==============================================================================
//functions:

xResult LWIP_NetAdapterInit(xNetT* net, xNetAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_LWIP_NET_ADAPTER_H_
