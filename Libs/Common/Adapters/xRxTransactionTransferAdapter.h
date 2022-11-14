/*
 *
 */
#ifndef _X_RX_TRANSACTION_TRANSFER_ADAPTER_H
#define _X_RX_TRANSACTION_TRANSFER_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
#include "Common/xTypes.h"
#include "Common/xTxTransfer.h"
//==============================================================================
typedef struct
{
	uint32_t DeviceId;
	
	uint16_t EventTransferStart;
	uint16_t EventTransfer;
	uint16_t EventTransferEnd;
	
} xRxTransactionTransferAdapterT;
//==============================================================================
xResult xRxTransactionTransferAdapterInit(xTxTransferT* layer, xRxTransactionTransferAdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
