//=============================================================================
//includes:

#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/apps/mqtt.h"
#include "MqttClient-Component.h"

#include "TCPServer/LWIP/LWIP-NetTcpServer-Component.h"
#include "Components/USART-Ports/USART-Ports-Component.h"
#include "Adapters/MqttPort-Adapter.h"
//==============================================================================
//defines:

#define TASK_STACK_SIZE 0x100

#define MQTT_BROKER_IP_ADDR0 90
#define MQTT_BROKER_IP_ADDR1 156
#define MQTT_BROKER_IP_ADDR2 229
#define MQTT_BROKER_IP_ADDR3 205

#define MQTT_BROKER_IP      "90.156.229.205"
#define MQTT_BROKER_PORT    1883
#define MQTT_CLIENT_ID      "bro-123456"
#define MQTT_TOPIC_RX		"bro-rx"
#define MQTT_TOPIC_TX		"bro-tx"

#define MQTT_TX_BUFFER_SIZE	250
//==============================================================================
//import:

extern struct netif gnetif;
//==============================================================================
//variables:

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE] MQTT_CLIENT_COMPONENT_MAIN_TASK_STACK_SECTION;

static int RTOS_MqttClientTaskStackWaterMark;
static int privateTimeStamp;

static uint8_t privateMqttTxBuffer[MQTT_TX_BUFFER_SIZE];

mqtt_client_t* MqttClient = NULL;

xPortT MqttPort;

static struct
{
	uint32_t IsConnected : 1;

} MqttClientState;

uint32_t MqttTxTimeStamp = 0;
//==============================================================================
//functions:

static void privateMqttConnectionStateListener(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	switch((int)status)
	{
		case MQTT_CONNECT_ACCEPTED:
		{
			MqttPort.Adapter.Interface->RequestListener(&MqttPort, xPortAdapterRequestOpen, client);
			MqttClientState.IsConnected = true;
			break;
		}

		case MQTT_CONNECT_DISCONNECTED:
			MqttClientState.IsConnected = true;
			break;
	}
}
//------------------------------------------------------------------------------
static void Task(void* arg)
{
	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(10));

		uint32_t time = xSystemGetTime(NULL);

		if (!MqttClientState.IsConnected && LWIP_Net.SNTP_Complite)
		{
			ip_addr_t address;
			IP4_ADDR(&address,
					MQTT_BROKER_IP_ADDR0,
					MQTT_BROKER_IP_ADDR1,
					MQTT_BROKER_IP_ADDR2,
					MQTT_BROKER_IP_ADDR3);

			struct mqtt_connect_client_info_t clientInfo;

			/* Setup an empty client info structure */
			memset(&clientInfo, 0, sizeof(clientInfo));
			clientInfo.client_id = MQTT_CLIENT_ID;

			mqtt_client_connect(MqttClient,
					&address,
					MQTT_BROKER_PORT,
					privateMqttConnectionStateListener,
					NULL,
					&clientInfo);

			vTaskDelay(pdMS_TO_TICKS(500));
		}

		if (MqttClientState.IsConnected && (time - privateTimeStamp) > 5000)
		{
			privateTimeStamp = time;

			//xPortStartTransmission(&MqttPort);
			//xPortTransmitString(&MqttPort, "bro ebat zaebal!!! nado sushitsa\r");
			//xPortEndTransmission(&MqttPort);
		}

		xPortDirectlyHandler(MqttPort);

		RTOS_MqttClientTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);
	}
}
//------------------------------------------------------------------------------
void MqttClientComponentHandler()
{

}
//------------------------------------------------------------------------------
static void privateEventListener(xPortT* port, int selector, void* arg)
{
	switch((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------
static void privateLWIP_NetEventListener(xNetT* net, int selector, void* subscriber, void* arg)
{
	switch(selector)
	{
		case xNetEventPhyDisconnected:
		{
			MqttClientState.IsConnected = false;
			break;
		}
	}
}
//==============================================================================
//initializations:

static MqttPortAdapterT privateMqttPortAdapter =
{
	.RxTopic = MQTT_TOPIC_RX,
	.TxTopic = MQTT_TOPIC_TX
};
//==============================================================================
//initialization:

xResult MqttClientComponentInit(void* parent)
{
	MqttClient = mqtt_client_new();

	static xNetEventSubscriberT privateNetEventSubscriber;
	privateNetEventSubscriber.Object = MqttClient;
	privateNetEventSubscriber.EventListener = privateLWIP_NetEventListener;

	xNetAddEventListener(&LWIP_Net, &privateNetEventSubscriber);

	MqttPortAdapterInitT portAdapterInit =
	{
		.TxBuffer = privateMqttTxBuffer,
		.TxBufferSize = sizeof(privateMqttTxBuffer)
	};

	MqttPortAdapterInit(&MqttPort, &privateMqttPortAdapter, &portAdapterInit);

	xPortInitT portInit =
	{
		.Parent = parent,
		.EventListener = (void*)privateEventListener
	};

	xPortInit(&MqttPort, &portInit);

	taskHandle = xTaskCreateStatic(Task, // Function that implements the task.
									"Tcp server task", // Text name for the task.
									TASK_STACK_SIZE, // Number of indexes in the xStack array.
									NULL, // Parameter passed into the task.
									osPriorityNormal, // Priority at which the task is created.
									taskStack, // Array to use as the task's stack.
									&taskBuffer);

	return xResultAccept;
}
//==============================================================================
