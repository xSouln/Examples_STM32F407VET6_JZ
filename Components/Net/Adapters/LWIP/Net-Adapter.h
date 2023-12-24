//==============================================================================
//header:

#ifndef _NET_ADAPTER_H_
#define _NET_ADAPTER_H_
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

} NetAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	struct netif* gnetif;
	
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
