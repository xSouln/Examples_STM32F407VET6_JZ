//==============================================================================
#include "xTxTransfer.h"
//==============================================================================
void xTxTransferHandler(xTxTransferT* layer)
{
	if (layer && layer->Interface && layer->Interface->Handler)
	{
		layer->Interface->Handler(layer);
	}
}
//------------------------------------------------------------------------------
xResult xTxTransferStart(xTxTransferT* layer, void* data, uint32_t data_size)
{
	if (!layer || !layer->Tx || !layer->Adapter)
	{
		return xResultLinkError;
	}
	
	if (!data || !data_size)
	{
		return xResultLinkError;
	}
	
	if (layer->Status.Transfer == xTxTransferStatusIdle
		|| layer->Status.Transfer == xTxTransferStatusComplite
		|| layer->Status.Transfer == xTxTransferStatusError)
	{
		layer->Data = (uint8_t*)data;
		layer->DataSize = data_size;
		layer->DataTransferred = 0;
		
		layer->Status.Transfer = xTxTransferStatusBeginning;
		
		return xResultAccept;
	}
	
	return xResultError;
}
//------------------------------------------------------------------------------
xResult xTxTransferSetTxLine(xTxTransferT* layer, xTxT* tx)
{
	if (layer && tx)
	{
		if (layer->Status.Transfer == xTxTransferStatusIdle
		|| layer->Status.Transfer == xTxTransferStatusComplite)
		{
			layer->Tx = tx;
			
			return xResultAccept;
		}
	}
	
	return xResultLinkError;
}
//------------------------------------------------------------------------------
void xTransferAbort(xTxTransferT* layer)
{
	
}
//==============================================================================
xResult xTxTransferInit(xTxTransferT* layer,
												uint32_t min_content_size,
												uint32_t max_content_size,
												float buffer_filling)
{
	if (layer && max_content_size && buffer_filling)
	{
		layer->Description = "xTxTransferT";
		layer->MinContentSize = min_content_size;
		layer->MaxContentSize = max_content_size;
		layer->BufferFilling = buffer_filling;
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
