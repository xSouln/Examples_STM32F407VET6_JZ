//==============================================================================
#include <string.h>
#include "Terminal_RxRequests.h"
#include "Terminal_RxTransactions.h"
#include "Terminal/Controls/Terminal.h"
#include "Components.h"

#include "Terminal/Communication/Terminal_RxRequests.h"
//==============================================================================
void TerminalRequestGetFirmware(xRxT* rx, xRxRequestT* request, uint8_t* object, uint16_t size)
{
	const char response[] = "firmware: qwerty\r";
	
	if (rx->Tx)
	{
		xTxTransmitData(rx->Tx, (void*)response, SIZE_STRING(response));
	}
}
//==============================================================================
extern const xRxTransactionT TerminalRxTransactions[];
//------------------------------------------------------------------------------
static const PacketHeaderT TransactionRequestHeader =
{
	.Identificator = { .Value = TRANSACTION_REQUEST_IDENTIFICATOR },
	.DeviceKey = TERMINAL_DEVICE_KEY
};
//------------------------------------------------------------------------------
const xRxRequestT TerminalRxRequests[] =
{
	{
		.Header = (void*)&TransactionRequestHeader,
		.HeaderLength = sizeof(TransactionRequestHeader),
		.Action = (xRxRequestReceiverT)xRxTransactionRequestReceiver,
		.Content = (void*)&TerminalRxTransactions
	},
	
	NEW_RX_REQUEST0("get fitmware", TerminalRequestGetFirmware),
  { 0 }
};
//==============================================================================
