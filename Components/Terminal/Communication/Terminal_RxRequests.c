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
void TerminalRequestsReceiver(xRxT* rx, uint8_t* data, uint32_t size)
{
	//if the received data is a transaction:
	//Packet header: [#][Description][:][DeviceKey];
	//Packet info: [RequestId][ActionKey][ContentSize]
	//Packet content: [uint8_t Content[info.ContentSize]]
	//Packet end packet marker: [\r]; - not included in "uint16_t size"
	
	//casting data to package structure
	PacketT* request = (PacketT*)data;
	
	//whether the package is a transaction
  if(size >= sizeof(PacketIdentificatorT)
	  && (request->Header.Identificator.Value & PACKET_HEADER_IDENTIFICATOR_MASK) == PACKET_HEADER_IDENTIFICATOR)
  {
		//size check for minimum transaction length. size - does not include the length of the separator character - '\r'
    if (size < sizeof(PacketT))
		{
			return;
		}
		
		//content size calculation
    int content_size = (int)size - sizeof(PacketT);
		
		//checking if the package content size matches the actual size, if the size is short
    if(content_size < request->Info.ContentSize)
		{
			return;
		}
		
		//reset size when content exceeds size specified in packet.Info
    if(content_size > request->Info.ContentSize)
		{
			return;
		}
  }
	
	//command identification
	if (xRxRequestIdentify(rx, &Terminal, (xRxRequestT*)TerminalRxRequests, data, size)) { goto end; }

	end:;
	xRxRequestListener(rx, xRxRequestClearBuffer, 0, 0);
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
