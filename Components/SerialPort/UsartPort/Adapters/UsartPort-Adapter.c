//==============================================================================
//includes:

#include "UsartPort-Adapter.h"
//==============================================================================
//functions:

static void PrivateHandler(xPortT* port)
{
	register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter.Content;

	if (adapter->RxDMA)
	{
		adapter->RxCircleBuffer.TotalIndex = (adapter->RxCircleBuffer.SizeMask + 1) -
													((DMA_Stream_TypeDef*)adapter->RxDMA->Instance)->NDTR;
	}

	if (!adapter->Usart->Control1.TxEmptyInterruptEnable
	&& adapter->TxCircleBuffer.TotalIndex != adapter->TxCircleBuffer.HandlerIndex)
	{
		adapter->Usart->Control1.TxEmptyInterruptEnable = true;
	}

	port->Tx.IsTransmitting = adapter->Usart->Control1.TxEmptyInterruptEnable;

	xRxReceiverRead(&adapter->RxReceiver, &adapter->RxCircleBuffer);
}
//------------------------------------------------------------------------------
static void PrivateIRQ(xPortT* port, void* arg)
{
	volatile register uint32_t clear;

	register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter.Content;

	if (adapter->Usart->Control1.TxEmptyInterruptEnable && adapter->Usart->InterruptAndStatus.TxEmpty)
	{
		if (adapter->TxCircleBuffer.HandlerIndex != adapter->TxCircleBuffer.TotalIndex)
		{
			adapter->Usart->Data = xCircleBufferGet(&adapter->TxCircleBuffer);
		}
		else
		{
			adapter->Usart->Control1.TxEmptyInterruptEnable = false;
		}
	}
	else if (adapter->Usart->InterruptAndStatus.ErrorOverRun)
	{
		clear = adapter->Usart->Data;
		(void)clear;
		adapter->Usart->InterruptAndStatus.ErrorOverRun = false;
	}
	else if (adapter->Usart->InterruptAndStatus.RxNotEmpty)
	{
		uint8_t byte = adapter->Usart->Data;
		xCircleBufferAdd(&adapter->TxCircleBuffer, &byte, 1);
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xPortT* port, xPortAdapterRequestSelector selector, void* arg)
{
	register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xPortAdapterRequestEnableTx:
			adapter->Usart->Control1.TransmitterEnable = true;
			break;

		case xPortAdapterRequestDisableTx:
			adapter->Usart->Control1.TransmitterEnable = false;
			break;

		case xPortAdapterRequestUpdateTxStatus:
			port->Tx.IsEnable = adapter->Usart->Control1.TransmitterEnable;
			port->Tx.IsTransmitting = adapter->Usart->Control1.TxEmptyInterruptEnable;
			break;

		case xPortAdapterRequestUpdateRxStatus:
			port->Rx.IsEnable = adapter->Usart->Control1.ReceiverEnable;
			break;

		case xPortAdapterRequestGetRxBuffer:
			*(uint8_t**)arg = adapter->RxReceiver.Buffer;
			break;

		case xPortAdapterRequestGetRxBufferSize:
			*(uint32_t*)arg = adapter->RxReceiver.BufferSize;
			break;

		case xPortAdapterRequestGetRxBufferFreeSize:
			*(uint32_t*)arg = adapter->RxReceiver.BufferSize - adapter->RxReceiver.BytesReceived;
			break;

		case xPortAdapterRequestClearRxBuffer:
			adapter->RxReceiver.BytesReceived = 0;
			break;

		case xPortAdapterRequestGetTxBufferSize:
			*(uint32_t*)arg = adapter->TxCircleBuffer.SizeMask + 1;
			break;

		case xPortAdapterRequestGetTxBufferFreeSize:
			*(uint32_t*)arg = xCircleBufferGetFreeSize(&adapter->TxCircleBuffer);
			break;

		case xPortAdapterRequestClearTxBuffer:
			adapter->TxCircleBuffer.HandlerIndex = adapter->TxCircleBuffer.TotalIndex;
			break;

		case xPortAdapterRequestSetBinding:
			port->Binding = arg;
			break;

		case xPortAdapterRequestStartTransmission:
			xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
			break;

		case xPortAdapterRequestEndTransmission:
			xSemaphoreGive(adapter->TransactionMutex);
			break;

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xPortT* port, xPortAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------
static int PrivateTransmit(xPortT* port, void* data, uint32_t size)
{
	register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter.Content;

	uint8_t* in = data;
	int result = size;

	while (size)
	{
		uint16_t packet_size = xCircleBufferGetFreeSize(&adapter->TxCircleBuffer);

		if (packet_size > size)
		{
			packet_size = size;
		}

		if (packet_size)
		{
			xCircleBufferAdd(&adapter->TxCircleBuffer, in, packet_size);
			adapter->Usart->Control1.TxEmptyInterruptEnable = true;

			in += packet_size;
			size -= packet_size;
		}
	}

	return result;
}
//------------------------------------------------------------------------------
static int PrivateReceive(xPortT* port, void* data, uint32_t size)
{
	return -xResultNotSupported;
}
//------------------------------------------------------------------------------
static void PrivateRxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, void* arg)
{
	register xPortT* port = receiver->Base.Parent;

	switch ((uint8_t)event)
	{
		case xRxReceiverEventEndLine:
			xPortEventListener(port, xPortObjectEventRxFoundEndLine, arg);
			break;

		case xRxReceiverEventBufferIsFull:
			xPortEventListener(port, xPortObjectEventRxBufferIsFull, arg);
			break;

		default: return;
	}
}
//==============================================================================
//initializations:

static xPortAdapterInterfaceT PrivatePortInterface =
{
	.Handler = (xPortAdapterHandlerT)PrivateHandler,
	.IRQ = (xPortAdapterIRQT)PrivateIRQ,

	.RequestListener = (xPortAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xPortAdapterEventListenerT)PrivateEventListener,

	.Transmit = (xPortAdapterTransmitActionT)PrivateTransmit,
	.Receive = (xPortAdapterReceiveActionT)PrivateReceive
};
//------------------------------------------------------------------------------
static xRxReceiverInterfaceT PrivateRxReceiverInterface =
{
	.EventListener = (xRxReceiverEventListenerT)PrivateRxReceiverEventListener
};
//------------------------------------------------------------------------------
xResult UsartPortAdapterInit(xPortT* port, xPortAdapterInitT* init)
{
	if (port && init)
	{
		UsartPortAdapterInitT* adapterInit = (UsartPortAdapterInitT*)init->Init;
		UsartPortAdapterT* adapter = init->Adapter;

		port->Adapter.Description = nameof(UsartPortAdapterT);
		port->Adapter.Content = adapter;
		port->Adapter.Interface = &PrivatePortInterface;

		adapter->Usart = adapterInit->Usart;
		adapter->RxDMA = adapterInit->RxDMA;
		adapter->RxReceiver.Interface = &PrivateRxReceiverInterface;
		adapter->TransactionMutex = xSemaphoreCreateMutex();

		xCircleBufferInit(&adapter->RxCircleBuffer, adapterInit->RxBuffer, adapterInit->RxBufferSizeMask, sizeof(uint8_t));
		xCircleBufferInit(&adapter->TxCircleBuffer, adapterInit->TxBuffer, adapterInit->TxBufferSizeMask, sizeof(uint8_t));
		xRxReceiverInit(&adapter->RxReceiver,
				port,
				&PrivateRxReceiverInterface,
				adapterInit->RxResponseBuffer,
				adapterInit->RxResponseBufferSize);

		adapter->Usart->Control1.USART_Enable = false;

		adapter->Usart->Control1.TxCompleteInterruptEnable = false;
		adapter->Usart->Control1.TxEmptyInterruptEnable = false;
		adapter->Usart->Control1.RxNotEmptyInterruptEnable = false;
		adapter->Usart->Control3.ErrorInterruptEnable = true;

		(void)adapter->Usart->Data;
		adapter->Usart->InterruptAndStatus.Value = 0;

		if (adapter->RxDMA)
		{
			adapter->Usart->Control3.RxDMA_Enable = true;

			if (adapter->RxDMA)
			{
				uint8_t dma_result = HAL_DMA_Start(adapter->RxDMA,
													(uint32_t)&adapter->Usart->Data,
													(uint32_t)adapter->RxCircleBuffer.Memory,
													adapter->RxCircleBuffer.SizeMask + 1);

				if (dma_result != HAL_OK)
				{
					return xResultError;
				}
			}
		}
		else
		{
			adapter->Usart->Control1.RxNotEmptyInterruptEnable = true;
		}
		
		adapter->Usart->Control1.TransmitterEnable = true;
		adapter->Usart->Control1.ReceiverEnable = true;
		
		adapter->Usart->Control1.USART_Enable = true;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
