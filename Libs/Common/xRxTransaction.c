//==============================================================================
#include <string.h>
#include "xRxTransaction.h"
//==============================================================================
xRxTransactionT* xRxTransactionIdentify(xRxTransactionT* transaction, uint16_t key)
{
  while(transaction && transaction->Id != (uint16_t)-1)
  {
    if(transaction->Id == key)
    {
      return transaction;
    }
    transaction++;
  }
  
  return 0;
}
//------------------------------------------------------------------------------
xResult xRxTransactionTransmitEvent(xTxT* tx, uint32_t device_id, uint16_t transaction, void* data, uint16_t data_size)
{
  //event array:
	//Header: [#][Description][:][DeviceKey];
	//Info: [RequestId][ActionKey][ContentSize]
	//Content: [uint8_t Content[ContentSize]]
	//End packet marker: [\r]
	
	if (!tx)
	{
		return xResultLinkError;
	}
	
	//filling in the package information
	PacketInfoT packet_info =
	{
		.ActionKey = transaction,
		.ContentSize = data_size
	};
	
	//start transmission logic implementation selected "tx" line
	xTxEventListener(tx, xTxEventStartTransmission, 0);
	
	//Packet header start
	xTxTransmitWord(tx, TRANSACTION_EVENT_IDENTIFICATOR);
	xTxTransmitData(tx, &device_id, sizeof(device_id));
	//Packet header end
	
	//Packet info: command "id" and "content_size"
	xTxTransmitData(tx, &packet_info, sizeof(packet_info));
	
	//sending content
	xTxTransmitData(tx, data, data_size);
	
	//Packet end
	xTxTransmitData(tx, TRANSACTION_END_IDENTIFICATOR, SIZE_STRING(TRANSACTION_END_IDENTIFICATOR));
	
	//transmission logic implementation selected "tx" line
	xTxEventListener(tx, xTxEventStopTransmission, 0);
	
	//generate event
	xTxEventListener(tx, xTxEventTransmissionComplete, 0);
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
xResult xRxTransactionError(xTxT* tx, PacketT* request_header, void* data, uint16_t data_size)
{
  //event array:
	//Header: [#][Description][:][DeviceKey];
	//Info: [RequestId][ActionKey][ContentSize]
	//Content: [uint8_t Content[ContentSize]]
	//End packet marker: [\r]

	if (!tx)
	{
		return xResultLinkError;
	}

	//filling in the packet header information
	PacketHeaderT packet_header =
	{
		.Identificator =
		{
			.Value = TRANSACTION_ERROR_IDENTIFICATOR
		},

		.DeviceKey = request_header->Header.DeviceKey
	};

	//filling in the package information
	PacketInfoT packet_info =
	{
		.RequestId = request_header->Info.RequestId,
		.ActionKey = request_header->Info.ActionKey,
		.ContentSize = data_size
	};

	//start transmission logic implementation selected "tx" line
	xTxEventListener(tx, xTxEventStartTransmission, 0);

	//Packet header start
	xTxTransmitData(tx, &packet_header, sizeof(packet_header));
	//Packet header end

	//Packet info: command "id" and "content_size"
	xTxTransmitData(tx, &packet_info, sizeof(packet_info));

	//sending content
	xTxTransmitData(tx, data, data_size);

	//Packet end
	xTxTransmitData(tx, TRANSACTION_END_IDENTIFICATOR, SIZE_STRING(TRANSACTION_END_IDENTIFICATOR));

	//transmission logic implementation selected "tx" line
	xTxEventListener(tx, xTxEventStopTransmission, 0);

	//generate event
	xTxEventListener(tx, xTxEventTransmissionComplete, 0);

	return xResultAccept;
}
//------------------------------------------------------------------------------
xResult xRxTransactionRequestReceiver(xRxRequestManagerT* manager, uint8_t* object, uint16_t size)
{
	//request array:
	//Packet header: [#][Description][:][DeviceKey]; - the "uint8_t*
	//object" is offset by the length of the header because the function is called
	//when the header "PacketHeaderT" matches
	//Packet info: [RequestId][ActionKey][ContentSize]
	//Packet content: [uint8_t Content[ContentSize]]
	//Packet end packet marker: [\r]; - not included in "uint16_t size"
	
	//received packet info
	PacketInfoT* info = (PacketInfoT*)object;
	
	//received transaction header
	PacketHeaderT* transaction_header = manager->FoundRequest->Header;
	
	//finde result by ActionKey
  xRxTransactionT* result = xRxTransactionIdentify((xRxTransactionT*)manager->FoundRequest->Content, info->ActionKey);
  
  if(result)
  {
		xTxT* tx = manager->RxLine->Tx;
		
		//receiving the transmitter corresponding to the selected receiver
		PacketInfoT packet_info;
    
    object += sizeof(PacketInfoT);
    size -= sizeof(PacketInfoT);
    
    if(result->Action)
    {
		//clearing the data buffer
    	xRxClearResponseBuffer(manager->RxLine);

		//function call corresponding to the request ActionKey
    	result->Action(manager, object, size);

    	uint8_t* content_data = xRxGetResponseBuffer(manager->RxLine);
    	int content_data_size = xRxGetBytesCountInResponseBuffer(manager->RxLine);

		//xRxGetValue(manager->RxLine, xRxValueResponseBuffer, &content_data);
		//xRxGetValue(manager->RxLine, xRxValueResponseBufferSize, &content_data_size);


		if (tx)
		{
			//filling in the package information structure
			packet_info.RequestId = info->RequestId; //must match the request RequestId
			packet_info.ActionKey = result->Id;
			//setting content size according to added in call function "result->Action"
			packet_info.ContentSize = content_data_size;

			//response array:
			//Packet header: [#][Description][:][DeviceKey]
			//Packet info: [RequestId][ActionKey][ContentSize]
			//Packet content: [uint8_t Content[ContentSize]]
			//Packet end packet marker: [\r]

			//start transmission logic implementation selected "tx" line
			xTxEventListener(tx, xTxEventStartTransmission, 0);

			//Packet header start
			xTxTransmitWord(tx, TRANSACTION_RESPONSE_IDENTIFICATOR);
			xTxTransmitData(tx, &transaction_header->DeviceKey, sizeof(transaction_header->DeviceKey));
			//Packet header end

			//Packet info: unique request key: "RequestId", command key: "ActionKey" and "content_size"
			xTxTransmitData(tx, &packet_info, sizeof(packet_info));

			//Packet content:
			xTxTransmitData(tx, content_data, content_data_size);
			
			//Packet end
			xTxTransmitData(tx, TRANSACTION_END_IDENTIFICATOR, SIZE_STRING(TRANSACTION_END_IDENTIFICATOR));
			
			//transmission logic implementation selected "tx" line
			xTxEventListener(tx, xTxEventStopTransmission, 0);
			
			//generate event
			xTxEventListener(tx, xTxEventTransmissionComplete, 0);
		}
    }
  }
	
	return xResultAccept;
}
//==============================================================================
