//==============================================================================
//header:

#include "NetPort-Adapter.h"

#ifdef _NET_PORT_ADAPTER_H_
//==============================================================================
//includes:


//==============================================================================
//functions:

static void privateRxTask(xPortT* port)
{
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	xNetSocketT* socket = port->Binding;

	while (true)
	{
		if (socket != NULL && socket->State == xNetSocketEstablished)
		{
			int len = xNetReceive(socket, adapter->RxOperationBuffer, adapter->RxOperationBufferSize);

			if (len > 0)
			{
				xCircleBufferAdd(&adapter->Internal.RxCircleBuffer, adapter->RxOperationBuffer, len);
			}
		}
	}
}
//------------------------------------------------------------------------------
static void PrivateHandler(xPortT* port)
{
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	xNetSocketT* socket = port->Binding;

	xNetSocketHandler(socket);

	xRxReceiverRead(&adapter->RxReceiver, &adapter->Internal.RxCircleBuffer);

	if (socket != NULL && socket->State == xNetSocketEstablished)
	{
		if (adapter->TxBuffer.Length)
		{
			xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
			xNetTransmit(socket, adapter->TxBuffer.Data, adapter->TxBuffer.Length);
			adapter->TxBuffer.Length = 0;
			xSemaphoreGive(adapter->TransactionMutex);
		}
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xPortT* port, xPortAdapterRequestSelector selector, uint32_t description, void* arg)
{
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	xNetSocketT* socket = port->Binding;

	switch ((uint32_t)selector)
	{
		case xPortAdapterRequestUpdateTxStatus:
			port->Tx.IsEnable = socket != 0 && (int)socket->Handle != -1;
			break;

		case xPortAdapterRequestUpdateRxStatus:
			port->Rx.IsEnable = socket != 0 && (int)socket->Handle != -1;
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
			*(uint32_t*)arg = socket != 0 && (int)socket->Handle != -1 ? 1000 : 0;
			break;

		case xPortAdapterRequestGetTxBufferFreeSize:
			*(uint32_t*)arg = socket != 0 && (int)socket->Handle != -1 ? 1000 : 0;
			break;

		case xPortAdapterRequestSetBinding:
			port->Binding = arg;
			break;

		case xPortAdapterRequestStartTransmission:
			xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
			break;

		case xPortAdapterRequestEndTransmission:
			if (socket->State == xNetSocketEstablished)
			{
				xNetTransmit(socket, adapter->TxBuffer.Data, adapter->TxBuffer.Length);
				adapter->TxBuffer.Length = 0;
			}
			xSemaphoreGive(adapter->TransactionMutex);
			break;

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xPortT* port, xPortAdapterEventSelector selector, uint32_t description, void* arg)
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
	NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	//xNetSocketT* socket = port->Binding;

	xDataBufferAdd(&adapter->TxBuffer, data, size);

	//return xNetTransmit(socket, data, size);
	return size;
}
//------------------------------------------------------------------------------
static int PrivateReceive(xPortT* port, void* data, uint32_t size)
{
	xNetSocketT* socket = port->Binding;

	return xNetReceive(socket, data, size);
}
//------------------------------------------------------------------------------
static void PrivateRxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector event, void* arg)
{
	register xPortT* port = receiver->Base.Parent;

	switch ((uint8_t)event)
	{
		case xRxReceiverEventEndLine:
			xPortEventListener(port, xPortEventRxFoundEndLine, 0, arg);
			break;

		case xRxReceiverEventBufferIsFull:
			xPortEventListener(port, xPortEventRxBufferIsFull, 0, arg);
			break;

		default: return;
	}
}
//==============================================================================
//initializations:

static xPortAdapterInterfaceT privatePortInterface =
{
	.Handler = (xPortAdapterHandlerT)PrivateHandler,

	.RequestListener = (xPortAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xPortAdapterEventListenerT)PrivateEventListener,

	.Transmit = (xPortAdapterTransmitActionT)PrivateTransmit,
	.Receive = (xPortAdapterReceiveActionT)PrivateReceive
};
//------------------------------------------------------------------------------
xResult NetPortAdapterInit(xPortT* port, NetPortAdapterT* adapter, NetPortAdapterInitT* adapterInit)
{
	if (port)
	{
		port->Adapter.Description = nameof(UsartPortAdapterT);
		port->Adapter.Content = adapter;
		port->Adapter.Interface = &privatePortInterface;

		adapter->RxOperationBuffer = adapterInit->RxOperationBuffer;
		adapter->RxOperationBufferSize = adapterInit->RxOperationBufferSize;

		adapter->Internal.RxCircleBuffer.SizeMask = 0x1ff;
		adapter->Internal.RxCircleBuffer.Memory = adapter->Internal.RxCircleBufferMemory;

		xRxReceiverInit(&adapter->RxReceiver, 
						port,
						PrivateRxReceiverEventListener,
						adapterInit->RxBuffer,
						adapterInit->RxBufferSize);

		xDataBufferInit(&adapter->TxBuffer,
						adapterInit->TxBuffer,
						adapterInit->TxBufferSize);

		adapter->TransactionMutex = xSemaphoreCreateMutex();
		
		xTaskCreate((void*)privateRxTask, // Function that implements the task.
						"net port task", // Text name for the task.
						0x100, // Number of indexes in the xStack array.
						port, // Parameter passed into the task.
						osPriorityNormal, // Priority at which the task is created.
						&adapter->Internal.RxTaskHandle);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //_NET_PORT_ADAPTER_H_
