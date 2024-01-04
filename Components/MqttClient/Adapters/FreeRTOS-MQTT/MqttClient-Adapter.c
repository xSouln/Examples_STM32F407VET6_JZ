//==============================================================================
//includes:

#include "MqttClient-Adapter.h"
#include "Abstractions/xSystem/xSystem.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//prototypes:

static int32_t privateTransportReceive(NetworkContext_t * pNetworkContext, void * pBuffer, size_t bytesToRecv);
static int32_t privateTransportSend(NetworkContext_t * pNetworkContext, const void * pBuffer, size_t bytesToSend);
static uint32_t privateMQTTGetTime(void);
static void privateMQTTCallback(struct MQTTContext * pContext,
        struct MQTTPacketInfo * pPacketInfo,
        struct MQTTDeserializedInfo * pDeserializedInfo);
//==============================================================================
//functions:


static void privateHandler(xMqttT* mqtt)
{
	MqttClientAdapterT* adapter = (MqttClientAdapterT*)mqtt->Adapter.Content;

	MQTT_ProcessLoop(&adapter->Internal.MQTTContext);
	//MQTT_ReceiveLoop(&adapter->Internal.MQTTContext);
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(xMqttT* mqtt, xMqttRequestSelector selector, uint32_t mode, void* in, void* out)
{
	MqttClientAdapterT* adapter = (MqttClientAdapterT*)mqtt->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xMqttRequestCreate:
		{
			xMqttRequestCreateClientT* request = in;

			MQTTStatus_t result = MQTT_Init(&adapter->Internal.MQTTContext,
											&adapter->Internal.TransportInterface,
											privateMQTTGetTime,
											privateMQTTCallback,
											&adapter->MQTTFixedBuffer);

			if (result == MQTTSuccess)
			{
				adapter->Id = request->ClientId;
				adapter->Address = request->Address;
				adapter->Port = request->Port;

				mqtt->Handle = &adapter->Internal.MQTTContext;
			}

			return result == MQTTSuccess ? xResultAccept : xResultError;
		}

		case xMqttRequestConnect:
		{
			if (adapter->Internal.Socket == NULL)
			{
				xSocket_t socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

				if (socket == NULL)
				{
					return xResultError;
				}

				adapter->Internal.Socket = socket;
			}

			if (!FreeRTOS_issocketconnected(adapter->Internal.Socket))
			{
				struct freertos_sockaddr sAddr;
				sAddr.sin_port = FreeRTOS_htons(adapter->Port);
				sAddr.sin_addr = adapter->Address;

				int result = FreeRTOS_connect(adapter->Internal.Socket, &sAddr, sizeof(sAddr));

				if (result < 0)
				{
					return xResultError;
				}
			}

			MQTTConnectInfo_t connectInfo;
			memset(&connectInfo, 0, sizeof(connectInfo));
			connectInfo.keepAliveSeconds = 0;
			connectInfo.cleanSession = true;
			connectInfo.pClientIdentifier = adapter->Id;
			connectInfo.clientIdentifierLength = strlen(adapter->Id);

			bool sessionPresent = false;

			mqtt->State = xMqttClientConnecnting;

			MQTTStatus_t result = MQTT_Connect(&adapter->Internal.MQTTContext,
					&connectInfo,
					NULL,
					100,
					&sessionPresent);

			if (result != MQTTSuccess)
			{
				return xResultError;
			}

			mqtt->State = xMqttClientConnected;

			break;
		}

		case xMqttRequestSubscribe:
		{
			xMqttRequestSubscribeToTopicT* request = in;

			MQTTSubscribeInfo_t info;
			info.qos = MQTTQoS0;
			info.pTopicFilter = request->Topic;
			info.topicFilterLength = strlen(request->Topic);

			uint16_t packetId = MQTT_GetPacketId(&adapter->Internal.MQTTContext);

			MQTTStatus_t result = MQTT_Subscribe(&adapter->Internal.MQTTContext,
					&info,
					1,
					packetId);

			if (result != MQTTSuccess)
			{
				return xResultError;
			}

			break;
		}

		default : return xResultNotSupported;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static xResult privateEventListener(xMqttT* mqtt, xMqttEventSelector selector, uint32_t mode, void* in, void* out)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return xResultAccept;
	}
}
//------------------------------------------------------------------------------
static int32_t privateTransportReceive(NetworkContext_t* pNetworkContext, void* pBuffer, size_t bytesToRecv)
{
	xMqttT* mqtt = pNetworkContext->Context;
	MqttClientAdapterT* adapter = (MqttClientAdapterT*)mqtt->Adapter.Content;

	if (mqtt->State == xMqttClientIdle || adapter->Internal.Socket == NULL)
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
		mqtt->State = xMqttClientIdle;

		return -1;
	}

	return bytesRead;
}
//------------------------------------------------------------------------------
static int32_t privateTransportSend(NetworkContext_t* pNetworkContext, const void* pBuffer, size_t bytesToSend)
{
	xMqttT* mqtt = pNetworkContext->Context;
	MqttClientAdapterT* adapter = (MqttClientAdapterT*)mqtt->Adapter.Content;

	if (mqtt->State == xMqttClientIdle || adapter->Internal.Socket == NULL)
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
			mqtt->State = xMqttClientIdle;

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
	MQTTPublishInfo_t publishInfo;

	static const char responseTopic[] = "bro-tx";
	static const char response[] = "1111-2222-3333-4444-5555\r";

	publishInfo.qos = MQTTQoS0;
	publishInfo.pTopicName = responseTopic;
	publishInfo.topicNameLength = sizeof_str(responseTopic);
	publishInfo.pPayload = response;
	publishInfo.payloadLength = sizeof_str(response);

	MQTT_Publish(pContext, &publishInfo, 0);
}
//------------------------------------------------------------------------------
static uint32_t privateMQTTGetTime(void)
{
	return xSystemGetTime();
}
//==============================================================================
//initializations:

static xMqttInterfaceT privateInterface =
{
	.Handler = (xMqttHandlerT)privateHandler,

	.RequestListener = (xMqttRequestListenerT)privateRequestListener,
	.EventListener = (xMqttEventListenerT)privateEventListener,
};
//------------------------------------------------------------------------------
xResult MqttClientAdapterInit(xMqttT* mqtt, MqttClientAdapterT* adapter, MqttClientAdapterInitT* init)
{
	if (mqtt && init)
	{
		mqtt->Adapter.Description = nameof(MqttPortAdapterT);
		mqtt->Adapter.Content = adapter;
		mqtt->Adapter.Interface = &privateInterface;

#ifdef INC_FREERTOS_H

#endif
		adapter->Internal.NetworkContext.Context = mqtt;

		memset(&adapter->Internal.MQTTContext, 0, sizeof(adapter->Internal.MQTTContext));
		adapter->Internal.TransportInterface.send = privateTransportSend;
		adapter->Internal.TransportInterface.recv = privateTransportReceive;
		adapter->Internal.TransportInterface.pNetworkContext = &adapter->Internal.NetworkContext;

		adapter->MQTTFixedBuffer.pBuffer = init->Buffer;
		adapter->MQTTFixedBuffer.size = init->BufferSize;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
