//=============================================================================
//header:


//=============================================================================
//includes:

#include "MqttClient-Component.h"
#include "Net/Net-Component.h"
#include "Components/USART-Ports/USART-Ports-Component.h"
#include "Adapters/FreeRTOS-MQTT/MqttClient-Adapter.h"
#include "Adapters/Ports/FreeRTOS-MQTT/MqttPort-Adapter.h"
//==============================================================================
//defines:

#define MQTT_PORT_TX_BUFFER 200
//==============================================================================
//import:


//==============================================================================
//variables:

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[MQTT_TASK_STACK_SIZE] MQTT_CLIENT_COMPONENT_MAIN_TASK_STACK_SECTION;

static int RTOS_MqttClientTaskStackWaterMark;
//static int privateTimeStamp;

static uint8_t privateMqttTxBuffer[MQTT_TX_BUFFER_SIZE];
static uint8_t privateMqttPortTxBuffer[MQTT_PORT_TX_BUFFER];

xPortT MqttPort;
xMqttT MqttClient;

uint32_t MqttTxTimeStamp = 0;
//==============================================================================
//functions:


//------------------------------------------------------------------------------
static void privateTask(void* arg)
{
	while (true)
	{
		//vTaskDelay(pdMS_TO_TICKS(10));
		RTOS_MqttClientTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);
		//uint32_t time = xSystemGetTime(NULL);

		/*if (!MqttClient.Handle && Net.SNTP_Complite)
		{
			xMqttRequestCreateClientT request;
			request.ClientId = MQTT_CLIENT_ID;
			request.Address = privateBrokerNetAddress.Value;
			request.Port = MQTT_BROKER_PORT;

			xMqttRequestListener(&MqttClient, xMqttRequestCreate, 0, &request, NULL);
		}

		if (MqttClient.Handle && MqttClient.State == xMqttClientIdle)
		{
			MqttComponentFlags.IsSubscribed = false;

			xMqttRequestListener(&MqttClient, xMqttRequestConnect, 0, NULL, NULL);
		}

		if (MqttClient.State == xMqttClientConnected && !MqttComponentFlags.IsSubscribed)
		{
			xMqttRequestSubscribeToTopicT request;
			request.Topic = MQTT_TOPIC_RX;

			xMqttRequestListener(&MqttClient, xMqttRequestSubscribe, 0, &request, NULL);

			MqttComponentFlags.IsSubscribed = true;
		}

		if (MqttClient.Handle)
		{
			xMqttHandler(&MqttClient);
		}*/

		/*if (MqttClientState.IsConnected && (time - privateTimeStamp) > 5000)
		{
			privateTimeStamp = time;

			//xPortStartTransmission(&MqttPort);
			//xPortTransmitString(&MqttPort, "bro ebat zaebal!!! nado sushitsa\r");
			//xPortEndTransmission(&MqttPort);
		}

		xPortDirectlyHandler(MqttPort);*/



		if (!MqttPort.IsOpen)
		{
			xPortRequestListener(&MqttPort, xPortAdapterRequestOpen, 0, NULL);

			continue;
		}

		xPortDirectlyHandler(MqttPort);

		if (!MqttPort.IsConnected)
		{
			xPortRequestListener(&MqttPort, xPortAdapterRequestConnect, 0, NULL);
		}
	}
}
//------------------------------------------------------------------------------
void MqttClientComponentHandler()
{

}
//------------------------------------------------------------------------------
static void privateEventListener(xPortT* port, int selector, uint32_t description, void* arg)
{
	switch((int)selector)
	{
		default: break;
	}
}
//==============================================================================
//initializations:

static MqttPortAdapterT privateMqttPortAdapter =
{
	.Id = MQTT_CLIENT_ID,
	.NetPort = MQTT_BROKER_PORT,
	.NetAddress =
	{
		.Octet1 = MQTT_BROKER_IP_ADDR3,
		.Octet2 = MQTT_BROKER_IP_ADDR2,
		.Octet3 = MQTT_BROKER_IP_ADDR1,
		.Octet4 = MQTT_BROKER_IP_ADDR0,
	}
};
//==============================================================================
//initialization:

xResult MqttClientComponentInit(void* parent)
{
	/*
	MqttClientAdapterInitT adapterInit;
	adapterInit.Buffer = privateMqttTxBuffer;
	adapterInit.BufferSize = sizeof(privateMqttTxBuffer);

	MqttClientAdapterInit(&MqttClient, &privateMqttClientAdapter, &adapterInit);

	xMqttInitT mqttInit;
	mqttInit.Parent = parent;

	xMqttInit(&MqttClient, &mqttInit);*/

	MqttPortAdapterInitT portAdapterInit = { 0 };
	portAdapterInit.TxTopic = MQTT_TOPIC_TX;
	portAdapterInit.RxTopic = MQTT_TOPIC_RX;
	portAdapterInit.MqttBuffer = privateMqttTxBuffer;
	portAdapterInit.MqttBufferSize = sizeof(privateMqttTxBuffer);
	portAdapterInit.TxBuffer = privateMqttPortTxBuffer;
	portAdapterInit.TxBufferSize = sizeof(privateMqttPortTxBuffer);

	MqttPortAdapterInit(&MqttPort, &privateMqttPortAdapter, &portAdapterInit);

	xPortInitT portInit = { 0 };
	portInit.Parent = parent;
	portInit.EventListener = (void*)privateEventListener;

	xPortInit(&MqttPort, &portInit);


	taskHandle = xTaskCreateStatic(privateTask, // Function that implements the task.
									"mqtt task", // Text name for the task.
									MQTT_TASK_STACK_SIZE, // Number of indexes in the xStack array.
									NULL, // Parameter passed into the task.
									osPriorityNormal, // Priority at which the task is created.
									taskStack, // Array to use as the task's stack.
									&taskBuffer);

	return xResultAccept;
}
//==============================================================================
