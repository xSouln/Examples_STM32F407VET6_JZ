//==============================================================================
//header:

#ifndef _TERMINAL_TX_TARNSFER_LAYER_ADAPTER_H_
#define _TERMINAL_TX_TARNSFER_LAYER_ADAPTER_H_
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

	const char* HeaderTransferStart;
	const char* HeaderTransfer;
	const char* HeaderTransferEnd;

} TerminalTxTransferLayerAdapterT;
//------------------------------------------------------------------------------
typedef struct
{
	const char* HeaderTransferStart;
	const char* HeaderTransfer;
	const char* HeaderTransferEnd;

} TerminalTxTransferLayerAdapterInitT;
//==============================================================================
//functions:

xResult TerminalTxTransferLayerAdapterInit(xTransferLayerT* layer, xTransferLayerAdapterInitT* init);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TERMINAL_TX_TARNSFER_LAYER_ADAPTER_H_
