//==============================================================================
//includes:

#include "Terminal.h"
#include "Terminal/Communication/Terminal_RxRequests.h"
#include "Components.h"
//==============================================================================
//defines:

#define RESPONSE_DATA_BUFFER_SIZE 1024
//==============================================================================
//variables:


//------------------------------------------------------------------------------

static uint8_t response_data_buffer_memory[RESPONSE_DATA_BUFFER_SIZE];
//------------------------------------------------------------------------------

TerminalT Terminal;

uint64_t WorkTime;

static uint16_t time_1000ms;
//==============================================================================
//functions:

void _TerminalReceiveData(xRxT* rx, uint8_t* data, uint32_t size)
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
			goto end;
		}
	}

	//command identification
	if (xRxRequestIdentify(rx, &Terminal, (xRxRequestT*)TerminalRxRequests, data, size)) { goto end; }

	xRxTransactionError(rx->Tx, request, 0, 0);

	end:;
	xRxRequestListener(rx, xRxRequestClearBuffer, 0);
}
//------------------------------------------------------------------------------

void _TerminalRequestListener(TerminalT* terminal, TerminalRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		case TerminalRequestReceiveData:

			break;

		default: return;
	}
}
//------------------------------------------------------------------------------

void _TerminalEventListener(TerminalT* terminal, TerminalEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void TerminalHandler()
{
	if (!time_1000ms)
	{
		time_1000ms = 1000;

		TerminalComponentEventListener((ComponentObjectBaseT*)&Terminal, TerminalEventTime_1000ms, 0);
	}

	xTxTransferHandler(&Terminal.Transfer);
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void TerminalTimeSynchronization()
{
	WorkTime++;

	if (time_1000ms)
	{
		time_1000ms--;
	}
}
//==============================================================================
//initialization:

static const ObjectDescriptionT TerminalObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = TERMINAL_OBJECT_ID,
	.Type = nameof(TerminalT)
};
//------------------------------------------------------------------------------
TerminalTransferAdapterT TerminalTransferAdapter =
{
	.HeaderTransferStart = "Start",
	.HeaderTransfer = "Data:",
	.HeaderTransferEnd = "End"
};
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return xResult
 */
xResult TerminalInit(void* parent)
{
	Terminal.Object.Description = &TerminalObjectDescription;
	Terminal.Object.Parent = parent;

	Terminal.ResponseBuffer.Data = response_data_buffer_memory;
	Terminal.ResponseBuffer.Size = RESPONSE_DATA_BUFFER_SIZE;

	TerminalTransferAdapterInit(&Terminal.Transfer, &TerminalTransferAdapter);

	xTxTransferInit(&Terminal.Transfer, 1, 255, 0.5);

	return xResultAccept;
}
//==============================================================================
