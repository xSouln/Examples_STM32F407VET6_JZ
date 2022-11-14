//==============================================================================
#include "xRxTransactionTransferAdapter.h"
#include "Common/xRxTransaction.h"
//==============================================================================
static void Handler(xTxTransferT* layer)
{
	static const int min_size = sizeof(PacketT) + sizeof_str(TRANSACTION_END_IDENTIFICATOR);
	
	xRxTransactionTransferAdapterT* adapter = layer->Adapter;
	int free_size;
	int buffer_size;
	int packet_size;
	int buffer_filling;
	
	if (layer->Tx->Status.Transmitter != xTxStatusIdle)
	{
		return;
	}
	
	free_size = xTxGetValue(layer->Tx, xTxValueFreeBufferSize);
	buffer_size = xTxGetValue(layer->Tx, xTxValueBufferSize);
	buffer_filling = buffer_size - buffer_size * layer->BufferFilling;
	
	free_size -= (buffer_filling + min_size);
	
	if (free_size < 0)
	{
		return;
	}
	
	switch ((uint8_t)layer->Status.Transfer)
	{
		case xTxTransferStatusBeginning:
			xRxTransactionTransmitEvent(layer->Tx,
																		adapter->DeviceId,
																		adapter->EventTransferStart,
																		0,
																		0);
			
			layer->Status.Transfer = xTxTransferStatusTransmits;
			break;
			
		case xTxTransferStatusTransmits:
			packet_size = layer->DataSize - layer->DataTransferred;
		
			if (packet_size)
			{
				if (free_size < layer->MinContentSize)
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
				
				xRxTransactionTransmitEvent(layer->Tx,
																		adapter->DeviceId,
																		adapter->EventTransfer,
																		layer->Data + layer->DataTransferred,
																		packet_size);
				
				layer->DataTransferred += packet_size;
				break;
			}
			
			layer->Status.Transfer = xTxTransferStatusEnding;
			break;
			
		case xTxTransferStatusEnding:
			xRxTransactionTransmitEvent(layer->Tx,
																	adapter->DeviceId,
																	adapter->EventTransferEnd,
																	0,
																	0);
		
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
xResult xRxTransactionTransferAdapterInit(xTxTransferT* layer, xRxTransactionTransferAdapterT* adapter)
{
	if (layer && adapter)
	{
		layer->Adapter = adapter;
		layer->Interface = (xTxTransferInterfaceT*)&Interface;
		
		return xResultAccept;
	}
	return xResultError;
}