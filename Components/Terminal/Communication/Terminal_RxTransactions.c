//==============================================================================
#include <string.h>
#include "Common/xTx.h"
#include "Terminal_RxTransactions.h"
//==============================================================================
extern const xRxTransactionT TerminalRxTransactions[];

#define TRANSACTIONS TerminalRxTransactions
//==============================================================================
void TransactionGetTime(xRxRequestManagerT* manager)
{
	extern uint32_t WorkTime;
	
	xRxPutInResponseBuffer(manager->RxLine, &WorkTime, sizeof(WorkTime));
}
//------------------------------------------------------------------------------
void TransactionTryResetTime(xRxRequestManagerT* manager)
{
	extern uint32_t WorkTime;
	int16_t result = xResultAccept;
	
	WorkTime = 0;
	
	xRxPutInResponseBuffer(manager->RxLine, &result, sizeof(result));
	xRxPutInResponseBuffer(manager->RxLine, &WorkTime, sizeof(WorkTime));
}
//------------------------------------------------------------------------------
void TransactionSetTime(xRxRequestManagerT* manager, int* request, uint16_t request_size)
{
	extern uint32_t WorkTime;
	int16_t result = xResultAccept;
	
	WorkTime = *request;
	
	xRxPutInResponseBuffer(manager->RxLine, &result, sizeof(result));
	xRxPutInResponseBuffer(manager->RxLine, &WorkTime, sizeof(WorkTime));
}
//==============================================================================
const xRxTransactionT TerminalRxTransactions[] =
{
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//GET
  {
		.Id = TERMINAL_TRANSACTION_GET_FIRMWARE_VERSION,
	},
	//----------------------------------------------------------------------------
	{
		.Id = TERMINAL_TRANSACTION_GET_TIME,
		.Action = (xRxTransactionAction)TransactionGetTime,
	},
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//SET
	{
		.Id = TERMINAL_TRANSACTION_SET_TIME,
		.Action = (xRxTransactionAction)TransactionSetTime,
	},
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	//TRY
	{
		.Id = TERMINAL_TRANSACTION_TRY_RESET_TIME,
		.Action = (xRxTransactionAction)TransactionTryResetTime,
	},
	//----------------------------------------------------------------------------
	//----------------------------------------------------------------------------
	// end of transactions marker
  { .Id = -1 }
};
//==============================================================================
