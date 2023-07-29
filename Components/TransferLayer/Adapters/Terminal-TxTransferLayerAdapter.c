//==============================================================================
//includes:

#include "Terminal-TxTransferLayerAdapter.h"
//==============================================================================
//variables:

const static char end_of_packet[] = "\r";
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
	register TerminalTxTransferLayerAdapterT* adapter = (TerminalTxTransferLayerAdapterT*)layer->Adapter.Content;
	register xPortT* port = layer->Binding;

	xResult result = xResultAccept;
	uint32_t len;

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
			len = strlen(adapter->HeaderTransferStart);

			if (layer->AvailableBufferSize < len + sizeof_str(end_of_packet))
			{
				return xResultError;
			}

			xPortStartTransmission(port);
			xPortTransmitData(port, adapter->HeaderTransferStart, len);
			xPortTransmitData(port, end_of_packet, sizeof_str(end_of_packet));
			xPortEndTransmission(port);
			break;

		case xTransferLayerRequestTransferBeginConfirmation:

			break;

		case xTransferLayerRequestTransferConfirmation:
			break;

		case xTransferLayerRequestTransferEnding:
			len = strlen(adapter->HeaderTransferEnd);

			if (layer->AvailableBufferSize < len + sizeof_str(end_of_packet))
			{
				return xResultError;
			}

			xPortStartTransmission(port);
			xPortTransmitData(port, adapter->HeaderTransferEnd, len);
			xPortTransmitData(port, end_of_packet, sizeof_str(end_of_packet));
			xPortEndTransmission(port);
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
	register TerminalTxTransferLayerAdapterT* adapter = (TerminalTxTransferLayerAdapterT*)layer->Adapter.Content;
	int len = strlen(adapter->HeaderTransfer);

	if(layer->AvailableBufferSize >= size + sizeof_str(end_of_packet) + len)
	{
		xPortStartTransmission(layer->Binding);
		xPortTransmitData(layer->Binding, adapter->HeaderTransfer, len);
		xPortTransmitData(layer->Binding, data, size);
		xPortTransmitData(layer->Binding, end_of_packet, sizeof_str(end_of_packet));
		xPortEndTransmission(layer->Binding);

		return size;
	}

	return 0;
}
//------------------------------------------------------------------------------
static xResult PrivateRxTransferStartResponce(xRxRequestManagerT* manager, uint8_t* object, uint16_t size)
{

	return xResultAccept;
}
//==============================================================================
//initializations:

//------------------------------------------------------------------------------
/*static const xRxRequestT PrivateRxRequests[] =
{
	NEW_RX_REQUEST0("HeaderTransferStart:", PrivateRxTransferStartResponce),

	{ 0 }
};*/
//------------------------------------------------------------------------------
static xTransferLayerInterfaceT PrivateInterface =
{
	.Handler = (xTransferLayerHandlerT)PrivateHandler,
	.IRQ = (xTransferLayerIRQT)PrivateIRQ,

	.RequestListener = (xTransferLayerRequestListenerT)PrivateRequestListener,
	.EventListener = (xTransferLayerEventListenerT)PrivateEventListener,

	.Transfer = (xTransferLayerTransferActionT)PrivateTransfer
};
//------------------------------------------------------------------------------
xResult TerminalTxTransferLayerAdapterInit(xTransferLayerT* layer, xTransferLayerAdapterInitT* init)
{
	if (layer && init)
	{
		TerminalTxTransferLayerAdapterInitT* adapterInit = init->Init;
		TerminalTxTransferLayerAdapterT* adapter = init->Adapter;

		layer->Adapter.Description = nameof(TerminalTxTransferLayerAdapterT);
		layer->Adapter.Content = adapter;
		layer->Adapter.Interface = &PrivateInterface;
		
		adapter->HeaderTransfer = adapterInit->HeaderTransfer;
		adapter->HeaderTransferStart = adapterInit->HeaderTransferStart;
		adapter->HeaderTransferEnd = adapterInit->HeaderTransferEnd;
		
		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
