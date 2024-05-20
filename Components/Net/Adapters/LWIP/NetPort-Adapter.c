//==============================================================================
//header:

#include "NetPort-Adapter.h"

#ifdef _NET_PORT_ADAPTER_H_
//==============================================================================
//includes:

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
//==============================================================================
//functions:

static void PrivateHandler(xPortT* port)
{
	register NetPortAdapterT* adapter = (NetPortAdapterT*)port->Adapter.Content;
	xNetSocketT* socket = port->Binding;

	xNetSocketHandler(socket);

	if (socket != NULL && socket->State == xNetSocketEstablished)
	{
		int rcvbuf = adapter->RxOperationBufferSize;
    	//socklen_t optlen = sizeof(rcvbuf);

		//if (getsockopt(socket->Number, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen) == 0 && rcvbuf > 0)
		{
			if (rcvbuf > adapter->RxOperationBufferSize)
			{
				rcvbuf = adapter->RxOperationBufferSize;
			}

			if (rcvbuf)
			{
				int len = xNetReceive(socket, adapter->RxOperationBuffer, rcvbuf);

				if (len > 0)
				{
					xRxReceiverReceive(&adapter->RxReceiver, adapter->RxOperationBuffer, len);
				}
			}
		}

		if (adapter->TxBuffer.DataSize)
		{
			xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
			xNetTransmit(socket, adapter->TxBuffer.Data, adapter->TxBuffer.DataSize);
			adapter->TxBuffer.DataSize = 0;
			xSemaphoreGive(adapter->TransactionMutex);
		}
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xPortT* port, xPortAdapterRequestSelector selector, void* arg)
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
				xNetTransmit(socket, adapter->TxBuffer.Data, adapter->TxBuffer.DataSize);
				adapter->TxBuffer.DataSize = 0;
			}
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

static xPortAdapterInterfaceT privatePortInterface =
{
	.Handler = (xPortAdapterHandlerT)PrivateHandler,

	.RequestListener = (xPortAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xPortAdapterEventListenerT)PrivateEventListener,

	.Transmit = (xPortAdapterTransmitActionT)PrivateTransmit,
	.Receive = (xPortAdapterReceiveActionT)PrivateReceive
};
//------------------------------------------------------------------------------
static xRxReceiverInterfaceT privateRxReceiverInterface =
{
	.EventListener = (xRxReceiverEventListenerT)PrivateRxReceiverEventListener
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

		xRxReceiverInit(&adapter->RxReceiver, 
						port,
						&privateRxReceiverInterface,
						adapterInit->RxBuffer,
						adapterInit->RxBufferSize);

		xDataBufferInit(&adapter->TxBuffer,
						port,
						0,
						adapterInit->TxBuffer,
						adapterInit->TxBufferSize);

		adapter->TransactionMutex = xSemaphoreCreateMutex();
		
		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //_NET_PORT_ADAPTER_H_
