//==============================================================================
//includes:

#include "Terminal-RxTransferLayerAdapter.h"
//==============================================================================
//variables:


//==============================================================================
//functions:

static void PrivateHandler(xTransferLayerT* layer)
{

}
//------------------------------------------------------------------------------
static void PrivateIRQ(xTransferLayerT* layer, void* arg)
{

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xTransferLayerT* layer, xTransferLayerRequestSelector selector, void* arg)
{
	register xPortT* port = layer->Binding;

	xResult result = xResultAccept;

	switch ((uint32_t)selector)
	{
		case xTransferLayerRequestUpdateRxTxState:
			layer->TxIsBusy = port->Tx.IsTransmitting;
			break;

		case xTransferLayerRequestGetTxBufferSize:
			*(uint32_t*)arg = xPortGetTxBufferSize((xPortT*)layer->Binding);
			break;

		case xTransferLayerRequestGetTxBufferFreeSize:
			*(uint32_t*)arg = xPortGetTxBufferFreeSize((xPortT*)layer->Binding);
			break;

		case xTransferLayerRequestTransferBegin:
			return xResultWaitOperation;

		case xTransferLayerRequestTransferBeginConfirmation:
			break;

		case xTransferLayerRequestTransferConfirmation:
			return xResultWaitOperation;

		case xTransferLayerRequestTransferEnding:
			break;

		case xTransferLayerRequestTransferEndingConfirmation:
			break;

		case xTransferLayerRequestSetBinding:
			layer->Binding = arg;
			break;

		case xTransferLayerRequestResetBinding:
			layer->Binding = 0;
			break;

		default : return xResultRequestIsNotFound;
	}

	return result;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xTransferLayerT* layer, xPortEventSelector selector, void* arg)
{
	//register TerminalTransferLayerAdapterT* adapter = (TerminalTransferLayerAdapterT*)layer->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------
static int PrivateTransfer(xTransferLayerT* layer, void* data, uint32_t size)
{
	return 0;
}
//------------------------------------------------------------------------------
static xResult PrivateRxTransferStart(xRxRequestManagerT* manager, uint8_t* object, uint16_t size)
{
	register xTransferLayerT* layer = manager->Object;

	layer->TransferedData = 0;
	layer->TransferedPacketSize = 0;

	layer->Status = xTransferLayerStatusTransfer;

	layer->WaitData = false;

	return xResultAccept;
}
//------------------------------------------------------------------------------
static xResult PrivateRxTransfer(xRxRequestManagerT* manager, uint8_t* object, uint16_t size)
{
	register xTransferLayerT* layer = manager->Object;

	int i = 0;
	while (i < size && layer->TransferedPacketSize < layer->DataSize)
	{
		layer->Data[layer->TransferedPacketSize] = object[i];
		layer->TransferedPacketSize++;
		i++;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static xResult PrivateRxTransferEnd(xRxRequestManagerT* manager, uint8_t* object, uint16_t size)
{
	register xTransferLayerT* layer = manager->Object;

	layer->Status = xTransferLayerStatusEndingConfirmation;

	layer->WaitData = false;

	return xResultAccept;
}
//==============================================================================
//initializations:

static xTransferLayerInterfaceT PrivateInterface =
{
	.Handler = (xTransferLayerHandlerT)PrivateHandler,
	.IRQ = (xTransferLayerIRQT)PrivateIRQ,

	.RequestListener = (xTransferLayerRequestListenerT)PrivateRequestListener,
	.EventListener = (xTransferLayerEventListenerT)PrivateEventListener,

	.Transfer = (xTransferLayerTransferActionT)PrivateTransfer
};
//------------------------------------------------------------------------------
static const xRxRequestT PrivateRxRequests[] =
{
	NEW_RX_REQUEST0("HeaderTransferStart", PrivateRxTransferStart),
	NEW_RX_REQUEST0("HeaderTransfer :", PrivateRxTransfer),
	NEW_RX_REQUEST0("HeaderTransferEnd", PrivateRxTransferEnd),

	{ 0 }
};
//------------------------------------------------------------------------------
xResult TerminalRxTransferLayerAdapterInit(xTransferLayerT* layer, xTransferLayerAdapterInitT* init)
{
	if (layer && init)
	{
		TerminalRxTransferLayerAdapterInitT* adapterInit = init->Init;
		TerminalRxTransferLayerAdapterT* adapter = init->Adapter;

		layer->Adapter.Description = nameof(TerminalRxTransferLayerAdapterT);
		layer->Adapter.Content = adapter;
		layer->Adapter.Interface = &PrivateInterface;
		
		adapter->TerminalObject.Object = &layer->Base;
		adapter->TerminalObject.Requests = PrivateRxRequests;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
