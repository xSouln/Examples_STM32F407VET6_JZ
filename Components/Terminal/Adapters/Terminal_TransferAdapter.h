/*
 *
 */
#ifndef _TERMINAL_TRANSFER_ADAPTER_H
#define _TERMINAL_TRANSFER_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
#include "Common/xTypes.h"
#include "Common/xTxTransfer.h"
#include "Common/xTx.h"
#include "Common/xRxTransaction.h"
//==============================================================================
typedef struct
{
	char* HeaderTransferStart;
	char* HeaderTransfer;
	char* HeaderTransferEnd;
	
} TerminalTransferAdapterT;
//==============================================================================
xResult TerminalTransferAdapterInit(xTxTransferT* layer, TerminalTransferAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
