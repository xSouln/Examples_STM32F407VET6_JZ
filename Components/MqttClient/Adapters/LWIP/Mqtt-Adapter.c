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

static void PrivateHandler(xPortT* port)
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
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
	xPortT* port = arg;
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	uint8_t i = 0;
	while(adapter->RxTopic[i] != 0)
	{
		if (topic[i] != adapter->RxTopic[i])
		{
			return;
		}

		i++;
	}

	if (topic[i] != 0)
	{
		return;
	}

	adapter->RxTopicConfirmed = true;
}
//------------------------------------------------------------------------------
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	xPortT* port = arg;
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	if (adapter->RxTopicConfirmed)
	{
		adapter->RxTopicConfirmed = false;

		//xRxReceiverReceive(&adapter->RxReceiver, (void*)data, len);
		/*RxDataPacketT packet =
		{
			.Data = (void*)data,
			.Size = len - 1,
			.FullSize = len,
			.Content = NULL
		};*/

		//xPortEventListener(port, xPortObjectEventRxFoundEndLine, &packet);
		xCircleBufferAdd(&rxCircleBuffer, (uint8_t*)data, len);
	}
}
//------------------------------------------------------------------------------
static void mqtt_pub_request_cb(void *arg, err_t result)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)arg;

	adapter->TxResult = result == ERR_OK ? xResultAccept : xResultError;

	xSemaphoreGive(adapter->TxSemaphore);
}
//------------------------------------------------------------------------------
static void mqtt_sub_request_cb(void *arg, err_t result)
{

}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xPortT* port, xPortAdapterRequestSelector selector, void* arg)
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
				mqtt_set_inpub_callback(adapter->Client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, port);
				mqtt_subscribe(adapter->Client, adapter->RxTopic, 0, mqtt_sub_request_cb, port);

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

/*
			case xPortAdapterRequestGetRxCircleBuffer:
			{
				xCircleBufferT** out = arg;
				*out = &adapter->RxCircleBuffer;

				break;
			}
			*/
/*
			case xPortAdapterRequesExtendedTransmission:
			{
				xCircleBufferAddObject(&adapter->TxCircleBuffer, arg, 1, 0, 0);
#ifdef INC_FREERTOS_H
				xSemaphoreGive(adapter->TxSemaphore);
#endif
				break;
			}
*/
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

					adapter->TxResult = xResultError;

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

					xDataBufferClear(&adapter->TxDataBuffer);
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
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	if (!mqtt_client_is_connected(adapter->Client))
	{
		return -xResultError;
	}

	uint16_t freeSize = xDataBufferGetFreeSize(&adapter->TxDataBuffer);

	if (freeSize < size)
	{
		return -xResultError;
	}

	xDataBufferAdd(&adapter->TxDataBuffer, data, size);

	return size;
}
//------------------------------------------------------------------------------
static int PrivateReceive(xPortT* port, void* data, uint32_t size)
{
	return -xResultNotSupported;
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
