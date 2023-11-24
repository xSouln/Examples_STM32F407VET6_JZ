//=============================================================================
//includes:

#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/apps/mqtt.h"
#include "MqttClient-Component.h"

#include "TCPServer/LWIP/LWIP-NetTcpServer-Component.h"
#include "Components/USART-Ports/USART-Ports-Component.h"
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
//==============================================================================
//import:

extern struct netif gnetif;
//==============================================================================
//variables:

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE];

static int RTOS_MqttClientTaskStackWaterMark;
static int privateTimeStamp;

mqtt_client_t* MqttClient = NULL;

static struct
{
	uint32_t IsConnected : 1;

} MqttClientState;
//==============================================================================
//functions:

static void PrivateEventListener(ObjectBaseT* object, int selector, void* arg)
{

}
//------------------------------------------------------------------------------
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
	xPortStartTransmission(&SerialPort);
	xPortTransmitData(&SerialPort, topic, tot_len);
	xPortTransmitByte(&SerialPort, '\r');
	xPortEndTransmission(&SerialPort);
}
//------------------------------------------------------------------------------
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	xPortStartTransmission(&SerialPort);
	xPortTransmitData(&SerialPort, data, len);
	xPortTransmitByte(&SerialPort, '\r');
	xPortEndTransmission(&SerialPort);
}
//------------------------------------------------------------------------------
static void mqtt_sub_request_cb(void *arg, err_t result)
{

}
//------------------------------------------------------------------------------
static void privateMqttConnectionStateListener(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	if (status == MQTT_CONNECT_ACCEPTED)
	{
		/* Setup callback for incoming publish requests */
		mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
		mqtt_subscribe(client, MQTT_TOPIC_RX, 1, mqtt_sub_request_cb, arg);

		xPortStartTransmission(&SerialPort);
		xPortTransmitString(&SerialPort, "MQTT_CONNECT_ACCEPTED\r");
		xPortEndTransmission(&SerialPort);

		MqttClientState.IsConnected = true;
	}
}
//------------------------------------------------------------------------------
static void mqtt_pub_request_cb(void *arg, err_t result)
{

}
//------------------------------------------------------------------------------
static void Task(void* arg)
{
	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(1));

		uint32_t time = xSystemGetTime(NULL);

		if (!MqttClientState.IsConnected && LWIP_Net.DHCP_Complite)
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

			xPortStartTransmission(&SerialPort);
			xPortTransmitString(&SerialPort, "MQTT try connect\r");
			xPortEndTransmission(&SerialPort);
		}

		if (MqttClientState.IsConnected && (time - privateTimeStamp) > 1000)
		{
			privateTimeStamp = time;

			char* data = "bro ebat zaebal!!! nado sushitsa";
			uint8_t qos = 2; /* 0 1 or 2, see MQTT specification */
			uint8_t retain = 0; /* No don't retain such crappy payload... */

			mqtt_publish(MqttClient, MQTT_TOPIC_TX, data, strlen(data), qos, retain, mqtt_pub_request_cb, arg);
		}

		RTOS_MqttClientTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);
	}
}
//------------------------------------------------------------------------------
void MqttClientComponentHandler()
{

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

//==============================================================================
//initialization:

xResult MqttClientComponentInit(void* parent)
{
	MqttClient = mqtt_client_new();

	static xNetEventSubscriberT privateNetEventSubscriber;
	privateNetEventSubscriber.Object = MqttClient;
	privateNetEventSubscriber.EventListener = privateLWIP_NetEventListener;

	xNetAddEventListener(&LWIP_Net, &privateNetEventSubscriber);

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
