//==============================================================================
//includes:

#include "MqttPort-Adapter.h"
#include "Components.h"
#include "Common/xCircleBuffer.h"
#include "Abstractions/xMQTT/xMQTT.h"
#include "Abstractions/xReflection/xReflection.h"
//==============================================================================
//defines:


//==============================================================================
//types:

enum
{
	ConncetionStateCreateSocket,
	ConncetionStateConnectSocket,
	ConncetionStateConnectToBroker,
	ConncetionStateSubscribe,
	ConncetionStateComplited
};
//------------------------------------------------------------------------------

enum
{
	xMqttOptionsBrokerAddressProperty,
	xMqttOptionsNetPortProperty,
	xMqttOptionsClientIdProperty,
	xMqttOptionsRxTopicProperty,
	xMqttOptionsTxTopicProperty,
};
//------------------------------------------------------------------------------

typedef struct
{
	xPropertyProviderHandleT Base;

	xPortT* Port;
	MqttPortAdapterT* Adapter;

} PropertyProviderHandleT;
//==============================================================================
//variables:

static const xPropertyDescriptionT privatePropertiesInfo[] =
{
	{ .PropertyId = xMqttOptionsBrokerAddressProperty, .Info = xPropertyCustomType, .SizeInfo = xPropertyTypeSizeWord },
	{ .PropertyId = xMqttOptionsNetPortProperty, .Info = xPropertyBaseType, .SizeInfo = xPropertyTypeSizeHalfWord },
	{ .PropertyId = xMqttOptionsClientIdProperty, .Info = xPropertyStringType, .SizeInfo = xPropertyTypeSizeByte },
	{ .PropertyId = xMqttOptionsRxTopicProperty, .Info = xPropertyStringType, .SizeInfo = xPropertyTypeSizeByte },
	{ .PropertyId = xMqttOptionsTxTopicProperty, .Info = xPropertyStringType, .SizeInfo = xPropertyTypeSizeByte },
};
//==============================================================================
//prototypes:

static uint32_t privateMQTTGetTime(void);

static void privateMQTTCallback(struct MQTTContext * pContext,
        struct MQTTPacketInfo * pPacketInfo,
        struct MQTTDeserializedInfo * pDeserializedInfo);
//==============================================================================
//functions:

static void PrivateHandler(xPortT* port)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	MQTT_ReceiveLoop(&adapter->Internal.MQTTContext);
	MQTT_ProcessLoop(&adapter->Internal.MQTTContext);
}
//------------------------------------------------------------------------------

static xResult privateConnectHandler(xPortT* port, MqttPortAdapterT* adapter)
{
	if (adapter->Internal.Socket == NULL)
	{
		adapter->Internal.ConnectState = ConncetionStateCreateSocket;
	}

	switch (adapter->Internal.ConnectState)
	{
		case ConncetionStateCreateSocket:
		{
			xSocket_t socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

			if (socket == NULL)
			{
				break;
			}

			adapter->Internal.Socket = socket;
			adapter->Internal.ConnectState = ConncetionStateConnectSocket;
		}

		case ConncetionStateConnectSocket:
		{
			struct freertos_sockaddr sAddr;
			sAddr.sin_port = FreeRTOS_htons(adapter->NetPort);
			sAddr.sin_addr = adapter->NetAddress.Value;

			if (FreeRTOS_connect(adapter->Internal.Socket, &sAddr, sizeof(sAddr)) < 0)
			{
				break;
			}

			adapter->Internal.ConnectState = ConncetionStateConnectToBroker;
		}

		case ConncetionStateConnectToBroker:
		{
			MQTTConnectInfo_t connectInfo;
			memset(&connectInfo, 0, sizeof(connectInfo));
			connectInfo.keepAliveSeconds = 0;
			connectInfo.cleanSession = true;
			connectInfo.pClientIdentifier = adapter->Id;
			connectInfo.clientIdentifierLength = strlen(adapter->Id);

			bool sessionPresent = false;

			MQTTStatus_t result = MQTT_Connect(&adapter->Internal.MQTTContext,
					&connectInfo,
					NULL,
					100,
					&sessionPresent);

			if (result != MQTTSuccess)
			{
				break;
			}

			adapter->Internal.ConnectState = ConncetionStateSubscribe;
		}

		case ConncetionStateSubscribe:
		{
			MQTTSubscribeInfo_t info;
			info.qos = MQTTQoS0;
			info.pTopicFilter = adapter->RxTopic;
			info.topicFilterLength = strlen(adapter->RxTopic);

			uint16_t packetId = MQTT_GetPacketId(&adapter->Internal.MQTTContext);

			MQTTStatus_t result = MQTT_Subscribe(&adapter->Internal.MQTTContext,
					&info,
					1,
					packetId);

			if (result != MQTTSuccess)
			{
				return xResultError;
			}

			adapter->Internal.ConnectState = ConncetionStateComplited;
			port->IsConnected = true;
		}

		default: break;
	}

	return adapter->Internal.ConnectState == ConncetionStateComplited ? xResultAccept : xResultError;
}
//------------------------------------------------------------------------------

static xResult privateOptionsGetter(PropertyProviderHandleT* handle, xPropertyDescriptionT descriptions)
{
	switch (descriptions.PropertyId)
	{
		case xMqttOptionsNetPortProperty:
			xDataBufferAdd(handle->Base.Out, &handle->Adapter->NetPort, sizeof(handle->Adapter->NetPort));
			break;

		case xMqttOptionsBrokerAddressProperty:
			xDataBufferAdd(handle->Base.Out, &handle->Adapter->NetAddress, sizeof(handle->Adapter->NetAddress));
			break;

		case xMqttOptionsClientIdProperty:
			xDataBufferAdd(handle->Base.Out, handle->Adapter->Id, strlen(handle->Adapter->Id) + 1);
			break;

		case xMqttOptionsRxTopicProperty:
			xDataBufferAdd(handle->Base.Out, handle->Adapter->RxTopic, strlen(handle->Adapter->RxTopic) + 1);
			break;

		case xMqttOptionsTxTopicProperty:
			xDataBufferAdd(handle->Base.Out, handle->Adapter->TxTopic, strlen(handle->Adapter->TxTopic) + 1);
			break;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult privateOptionsSetter(PropertyProviderHandleT* handle,
		xPropertyDescriptionT descriptions,
		xMemoryReaderT* memoryReader)
{
	switch (descriptions.PropertyId)
	{
		case xMqttOptionsNetPortProperty:
			return xMemoryReaderRead(memoryReader, &handle->Adapter->NetPort, sizeof(handle->Adapter->NetPort));

		case xMqttOptionsBrokerAddressProperty:
			return xMemoryReaderRead(memoryReader, &handle->Adapter->NetAddress, sizeof(handle->Adapter->NetAddress));

		case xMqttOptionsClientIdProperty:
			return xMemoryReaderReadString(memoryReader, handle->Adapter->Id, strlen(handle->Adapter->Id));

		case xMqttOptionsRxTopicProperty:
			return xMemoryReaderReadString(memoryReader, handle->Adapter->RxTopic, strlen(handle->Adapter->RxTopic));

		case xMqttOptionsTxTopicProperty:
			return xMemoryReaderReadString(memoryReader, handle->Adapter->TxTopic, strlen(handle->Adapter->TxTopic));
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateRequestListener(xPortT* port,
		xPortAdapterRequestSelector selector,
		uint32_t description,
		void* arg,
		void* out)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xPortAdapterRequestGetTxBufferSize:
			*(uint32_t*)arg = adapter->Internal.TxDataBuffer.MaxLength;
			break;

		case xPortAdapterRequestGetTxBufferFreeSize:
			*(uint32_t*)arg = xDataBufferGetFreeSize(&adapter->Internal.TxDataBuffer);
			break;

		case xPortRequestGetOptions:
		{
			xPropertyProviderArgT* request = arg;

			if (request == null || request->SizeOfParameters == 0)
			{
				return xResultError;
			}

			PropertyProviderHandleT providerHandle;
			providerHandle.Base.Provider = (void*)privateOptionsGetter;
			providerHandle.Base.Out = request->Out;
			providerHandle.Adapter = adapter;
			providerHandle.Port = port;

			xReflectionGetterProvider(&providerHandle.Base,
					request->Info,
					request->SizeOfParameters,
					privatePropertiesInfo,
					sizeof_array(privatePropertiesInfo));
			break;
		}

		case xPortRequestSetOptions:
		{
			xPropertyProviderArgT* request = arg;

			if (request == null || request->SizeOfParameters == 0)
			{
				return xResultError;
			}

			PropertyProviderHandleT providerHandle;
			providerHandle.Base.Provider = (void*)privateOptionsSetter;
			providerHandle.Base.Out = request->Out;
			providerHandle.Adapter = adapter;
			providerHandle.Port = port;

			xReflectionSetterProvider(&providerHandle.Base,
					request->Info,
					request->SizeOfParameters,
					privatePropertiesInfo,
					sizeof_array(privatePropertiesInfo));

			break;
		}

		case xPortAdapterRequestOpen:
		{
			//xMqttRequestCreateClientT* request = arg;

			MQTTStatus_t result = MQTT_Init(&adapter->Internal.MQTTContext,
											&adapter->Internal.TransportInterface,
											privateMQTTGetTime,
											privateMQTTCallback,
											&adapter->Internal.MQTTFixedBuffer);

			if (result == MQTTSuccess)
			{
				port->IsOpen = true;
			}


			break;
		}

		case xPortAdapterRequestConnect:
		{
			return privateConnectHandler(port, adapter);
		}

		case xPortAdapterRequestClose:
		{
			port->IsOpen = false;

			break;
		}

		case xPortRequestGetInfo:
		{
			break;
		}

		case xPortAdapterRequestClearRxBuffer:
		{
			//xRxReceiverClear(&adapter->Internal.RxReceiver);
			break;
		}

		case xPortAdapterRequestStartTransmission:
#ifdef INC_FREERTOS_H
			xSemaphoreTake(adapter->Internal.TransactionMutex, portMAX_DELAY);
#endif
			break;

		case xPortAdapterRequestEndTransmission:
		{
			if (adapter->Internal.TxDataBuffer.Length)
			{
				MQTTPublishInfo_t publishInfo = { 0 };
				publishInfo.qos = MQTTQoS0;
				publishInfo.pTopicName = adapter->TxTopic;
				publishInfo.topicNameLength = adapter->Internal.TxTopicLength;
				publishInfo.pPayload = adapter->Internal.TxDataBuffer.Data;
				publishInfo.payloadLength = adapter->Internal.TxDataBuffer.Length;

				MQTT_Publish(&adapter->Internal.MQTTContext, &publishInfo, 0);

				//xSemaphoreTake(adapter->Internal.TxSemaphore, 2000);

				xDataBufferClear(&adapter->Internal.TxDataBuffer);
			}

#ifdef INC_FREERTOS_H

			xSemaphoreGive(adapter->Internal.TransactionMutex);
#endif
			break;
		}

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

static int32_t privateTransportReceive(NetworkContext_t* pNetworkContext, void* pBuffer, size_t bytesToRecv)
{
	xPortT* port = (void*)pNetworkContext;
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	if (!port->IsOpen || adapter->Internal.Socket == NULL)
	{
		return -1;
	}

	BaseType_t flags = 0;//FREERTOS_MSG_DONTWAIT;

	adapter->Internal.RxAttempts++;

	BaseType_t bytesRead = FreeRTOS_recv(adapter->Internal.Socket, pBuffer, bytesToRecv, flags);

	if (bytesRead < 0)
	{
		FreeRTOS_closesocket(adapter->Internal.Socket);
		adapter->Internal.Socket = NULL;
		port->IsConnected = false;

		return -1;
	}

	return bytesRead;
}
//------------------------------------------------------------------------------
static int32_t privateTransportSend(NetworkContext_t* pNetworkContext, const void* pBuffer, size_t bytesToSend)
{
	xPortT* port = (void*)pNetworkContext;
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	if (!port->IsOpen || adapter->Internal.Socket == NULL)
	{
		return -1;
	}

	int sended = 0;
	uint8_t* mem = (uint8_t*)pBuffer;

	adapter->Internal.TxAttempts++;

	while (sended < bytesToSend)
	{
		int len = FreeRTOS_send(adapter->Internal.Socket, mem + sended, bytesToSend - sended, 0);

		if(len < 0)
		{
			FreeRTOS_closesocket(adapter->Internal.Socket);
			adapter->Internal.Socket = NULL;
			port->IsConnected = false;

			return -1;
		}

		sended += len;
	}

	return sended;
}
//------------------------------------------------------------------------------

static void privateMQTTCallback(struct MQTTContext * pContext,
        struct MQTTPacketInfo * pPacketInfo,
        struct MQTTDeserializedInfo * pDeserializedInfo)
{
	if (pDeserializedInfo->pPublishInfo)
	{
		xPortT* port = (void*)pContext->transportInterface.pNetworkContext;

		RxDataPacketT rxPacket;
		rxPacket.Data = (void*)pDeserializedInfo->pPublishInfo->pPayload;
		rxPacket.FullSize = pDeserializedInfo->pPublishInfo->payloadLength;
		rxPacket.Size = rxPacket.FullSize - 1;

		TerminalReceiveData(port, &rxPacket);
	}
}
//------------------------------------------------------------------------------
static uint32_t privateMQTTGetTime(void)
{
	return xSystemGetTime();
}
//------------------------------------------------------------------------------
static int PrivateTransmit(xPortT* port, void* data, uint32_t size)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;

	uint16_t freeSize = xDataBufferGetFreeSize(&adapter->Internal.TxDataBuffer);

	if (freeSize < size)
	{
		return -xResultError;
	}

	xDataBufferAdd(&adapter->Internal.TxDataBuffer, data, size);

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

		port->Interface = xPortInterfaceEthernet;
		port->TransferLayer = xPortTransferLayerMQTT;

		memset(&adapter->Internal, 0, sizeof(adapter->Internal));

#ifdef INC_FREERTOS_H
		adapter->Internal.TransactionMutex = xSemaphoreCreateMutex();
		adapter->Internal.TxSemaphore = xSemaphoreCreateBinary();
#endif

		adapter->TxTopic = init->TxTopic;
		adapter->Internal.TxTopicLength = strlen(init->TxTopic);

		adapter->RxTopic = init->RxTopic;
		adapter->Internal.RxTopicLength = strlen(init->RxTopic);

		adapter->Internal.TxDataBuffer.Memory = init->TxBuffer;
		adapter->Internal.TxDataBuffer.MaxLength = init->TxBufferSize;

		/*adapter->Internal.RxReceiver.Base.Parent = port;
		adapter->Internal.RxReceiver.Buffer = init->RxBuffer;
		adapter->Internal.RxReceiver.BufferSize = init->RxBufferSize;
		adapter->Internal.RxReceiver.EventListener = RxReceiverEventListener;*/

		adapter->Internal.NetworkContext.Context = port;

		memset(&adapter->Internal.MQTTContext, 0, sizeof(adapter->Internal.MQTTContext));
		adapter->Internal.TransportInterface.send = privateTransportSend;
		adapter->Internal.TransportInterface.recv = privateTransportReceive;
		adapter->Internal.TransportInterface.pNetworkContext = (void*)port;

		adapter->Internal.MQTTFixedBuffer.pBuffer = init->MqttBuffer;
		adapter->Internal.MQTTFixedBuffer.size = init->MqttBufferSize;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
