//==============================================================================
//header:

#ifndef _TERMINAL_RX_TARNSFER_LAYER_ADAPTER_H_
#define _TERMINAL_RX_TARNSFER_LAYER_ADAPTER_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components.h"

#include "Abstractions/xTransferLayer/xTransferLayer.h"
#include "Abstractions/xPort/xPort.h"
#include "Common/xRxRequest.h"
//==============================================================================
//types:

typedef struct
{
	TerminalObjectT TerminalObject;

} TerminalRxTransferLayerAdapterT;
//------------------------------------------------------------------------------
typedef struct
{


} TerminalRxTransferLayerAdapterInitT;
//==============================================================================
//functions:

xResult TerminalRxTransferLayerAdapterInit(xTransferLayerT* layer, xTransferLayerAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TERMINAL_RX_TARNSFER_LAYER_ADAPTER_H_
