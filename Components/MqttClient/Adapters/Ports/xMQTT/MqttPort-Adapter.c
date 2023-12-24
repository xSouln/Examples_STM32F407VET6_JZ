//==============================================================================
//includes:

#include "MqttPort-Adapter.h"
#include "Components.h"
#include "Common/xCircleBuffer.h"
//==============================================================================
//defines:

#define RX_CIRCLE_BUFFER_MASK 0xff
//==============================================================================
//types:


//==============================================================================
//variables:

static void RxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector selector, void* arg);

static uint8_t rxCircleBufferMem[RX_CIRCLE_BUFFER_MASK + 1];
static uint8_t rxReceiverBuffer[100];

static xRxReceiverInterfaceT RxReceiverInterface =
{
	.EventListener = (void*)RxReceiverEventListener
};

static xRxReceiverT rxReceiver =
{
	.Buffer = rxReceiverBuffer,
	.BufferSize = sizeof(rxReceiverBuffer),
	.Interface = &RxReceiverInterface
};

static xCircleBufferT rxCircleBuffer =
{
	.Memory = rxCircleBufferMem,
	.TypeSize = sizeof(uint8_t),
	.SizeMask = RX_CIRCLE_BUFFER_MASK
};
//==============================================================================
//functions:

static void privateHandler(xPortT* port)
{
	xRxReceiverRead(&rxReceiver, &rxCircleBuffer);
}
//------------------------------------------------------------------------------
static void RxReceiverEventListener(xRxReceiverT* receiver, xRxReceiverEventSelector selector, void* arg)
{
	TerminalReceiveData(receiver->Base.Parent, arg);

	extern uint32_t EthernetRxTimeStamp;
	EthernetRxTimeStamp = xSystemGetTime(NULL);
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(xPortT* port, xPortAdapterRequestSelector selector, void* arg)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xPortAdapterRequestGetTxBufferSize:
			*(uint32_t*)arg = adapter->TxDataBuffer.Size;
			break;

		case xPortAdapterRequestGetTxBufferFreeSize:
			*(uint32_t*)arg = xDataBufferGetFreeSize(&adapter->TxDataBuffer);
			break;

		case xPortAdapterRequestOpen:
		{
			adapter->Client = arg;
			/* Setup callback for incoming publish requests */
			/*
			mqtt_set_inpub_callback(adapter->Client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, port);
			mqtt_subscribe(adapter->Client, adapter->RxTopic, 0, mqtt_sub_request_cb, port);*/

			break;
		}

		case xPortAdapterRequestClose:
		{

			break;
		}

		case xPortAdapterRequestClearRxBuffer:
		{
			xRxReceiverClear(&rxReceiver);
			break;
		}

		case xPortAdapterRequestStartTransmission:
#ifdef INC_FREERTOS_H
			xSemaphoreTake(adapter->TransactionMutex, portMAX_DELAY);
#endif
			break;

		case xPortAdapterRequestEndTransmission:
		{
			if (adapter->TxDataBuffer.DataSize)
			{
				uint8_t qos = 0; /* 0 1 or 2, see MQTT specification */
				uint8_t retain = 0; /* No don't retain such crappy payload... */

				/*adapter->TxResult = xResultError;

				extern uint32_t MqttTxTimeStamp;
				MqttTxTimeStamp = xSystemGetTime(NULL);

				mqtt_publish(adapter->Client,
						adapter->TxTopic,
						adapter->TxDataBuffer.Data,
						adapter->TxDataBuffer.DataSize,
						qos,
						retain,
						mqtt_pub_request_cb,
						adapter);

				xSemaphoreTake(adapter->TxSemaphore, 2000);

				xDataBufferClear(&adapter->TxDataBuffer);*/
			}

			uint8_t result = adapter->TxResult;

#ifdef INC_FREERTOS_H

			xSemaphoreGive(adapter->TransactionMutex);
#endif
			return result;
		}

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void privateEventListener(xPortT* port, xPortAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------
static int privateTransmit(xPortT* port, void* data, uint32_t size)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	uint16_t freeSize = xDataBufferGetFreeSize(&adapter->TxDataBuffer);

	if (freeSize < size)
	{
		return -xResultError;
	}

	xDataBufferAdd(&adapter->TxDataBuffer, data, size);

	return size;
}
//------------------------------------------------------------------------------
static int privateReceive(xPortT* port, void* data, uint32_t size)
{
	return -xResultNotSupported;
}
//==============================================================================
//initializations:

static xPortAdapterInterfaceT privatePortInterface =
{
	.Handler = (xPortAdapterHandlerT)privateHandler,

	.RequestListener = (xPortAdapterRequestListenerT)privateRequestListener,
	.EventListener = (xPortAdapterEventListenerT)privateEventListener,

	.Transmit = (xPortAdapterTransmitActionT)privateTransmit,
	.Receive = (xPortAdapterReceiveActionT)privateReceive
};
//------------------------------------------------------------------------------
xResult MqttPortAdapterInit(xPortT* port, MqttPortAdapterT* adapter, MqttPortAdapterInitT* init)
{
	if (port && init)
	{
		port->Adapter.Description = nameof(MqttPortAdapterT);
		port->Adapter.Content = adapter;
		port->Adapter.Interface = &privatePortInterface;

#ifdef INC_FREERTOS_H
		adapter->TransactionMutex = xSemaphoreCreateMutex();
		adapter->TxSemaphore = xSemaphoreCreateBinary();
#endif
		adapter->TxDataBuffer.Memory = init->TxBuffer;
		adapter->TxDataBuffer.Size = init->TxBufferSize;

		rxReceiver.Base.Parent = port;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
