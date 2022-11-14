//==============================================================================
#ifndef _TERMINAL_RX_TRANSACTIONS_H
#define _TERMINAL_RX_TRANSACTIONS_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Common/xRxTransaction.h"
//==============================================================================
//types:

typedef enum
{
  TERMINAL_TRANSACTION_GET_FIRMWARE_VERSION = 100,
	TERMINAL_TRANSACTION_GET_TIME,

	TERMINAL_TRANSACTION_SET = 1000,
	TERMINAL_TRANSACTION_SET_TIME,

	TERMINAL_TRANSACTION_TRY = 2000,
	TERMINAL_TRANSACTION_TRY_RESET_TIME,

	TERMINAL_TRANSACTION_EVT = 10000,
	
} TERMINAL_TRANSACTIONS;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
