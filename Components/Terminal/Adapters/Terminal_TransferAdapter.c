//==============================================================================
#include <string.h>
#include "Terminal/Adapters/Terminal_Adapters.h"
#include "Common/xTxTransfer.h"
//==============================================================================
static void Handler(xTxTransferT* layer)
{
	static const char end[] = "\r";
	
	static const int min_size = sizeof_str(end);
	
	TerminalTransferAdapterT* adapter = layer->Adapter;
	int free_size;
	int buffer_size;
	int packet_size;
	int buffer_filling;
	
	if (!layer->Tx
		|| layer->Status.Transfer == xTxTransferStatusComplite
		|| layer->Status.Transfer == xTxTransferStatusIdle)// || xTxGetValue(layer->Tx, xTxValueTransmitterStatus) != xTxStatusIdle)
	{
		return;
	}
	
	free_size = xTxGetValue(layer->Tx, xTxValueFreeBufferSize);
	buffer_size = xTxGetValue(layer->Tx, xTxValueBufferSize);
	buffer_filling = buffer_size - buffer_size * layer->BufferFilling;
	
	free_size -= (buffer_filling + min_size);
	
	switch ((uint8_t)layer->Status.Transfer)
	{
		case xTxTransferStatusBeginning:
			
			free_size -= strlen(adapter->HeaderTransferStart);
			
			if (free_size < 0)
			{
				return;
			}
			
			xTxTransmitData(layer->Tx, (uint8_t*)adapter->HeaderTransferStart, strlen(adapter->HeaderTransferStart));
			xTxTransmitData(layer->Tx, (uint8_t*)end, sizeof_str(end));
			layer->Status.Transfer = xTxTransferStatusTransmits;
			break;
			
		case xTxTransferStatusTransmits:
			packet_size = layer->DataSize - layer->DataTransferred;
		
			if (packet_size)
			{
				free_size -= strlen(adapter->HeaderTransfer);
				
				if (free_size < 0 || free_size < layer->MinContentSize)
				{
					return;
				}
				
				if (packet_size > free_size)
				{
					packet_size = free_size;
				}
				
				if (packet_size > layer->MaxContentSize)
				{
					packet_size = layer->MaxContentSize;
				}
				
				xTxTransmitData(layer->Tx, (uint8_t*)adapter->HeaderTransfer, strlen(adapter->HeaderTransfer));
				xTxTransmitData(layer->Tx, layer->Data + layer->DataTransferred, packet_size);
				xTxTransmitData(layer->Tx, (uint8_t*)end, sizeof_str(end));
				
				layer->DataTransferred += packet_size;
				break;
			}
			
			layer->Status.Transfer = xTxTransferStatusEnding;
			break;
			
		case xTxTransferStatusEnding:
			free_size -= strlen(adapter->HeaderTransferEnd);
		
			if (free_size < 0)
			{
				return;
			}
			
			xTxTransmitData(layer->Tx, (uint8_t*)adapter->HeaderTransferEnd, strlen(adapter->HeaderTransferEnd));
			xTxTransmitData(layer->Tx, (uint8_t*)end, sizeof_str(end));
			layer->Status.Transfer = xTxTransferStatusComplite;
			break;
			
		default: return;
	}
}
//------------------------------------------------------------------------------
static xResult RequestListener(xTxTransferT* layer, xTxTransferRequestSelector selector, uint32_t args, uint32_t count)
{
	switch ((uint8_t)selector)
	{
		default : return xResultNotSupported;
	}
	
	return xResultAccept;
}
//------------------------------------------------------------------------------
static int GetValue(xTxTransferT* layer, xTxTransferValueSelector selector)
{
	switch ((uint8_t)selector)
	{
		default : return 0;
	}
}
//==============================================================================
static const xTxTransferInterfaceT Interface = 
{
	.Handler = (xTxTransferHandlerT)Handler,
	.RequestListener = (xTxTransferRequestListenerT)RequestListener,
	.GetValue = (xTxTransferActionGetValueT)GetValue
};
//==============================================================================
xResult TerminalTransferAdapterInit(xTxTransferT* layer, TerminalTransferAdapterT* adapter)
{
	if (layer && adapter)
	{
		layer->Adapter = adapter;
		layer->Interface = (xTxTransferInterfaceT*)&Interface;
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
