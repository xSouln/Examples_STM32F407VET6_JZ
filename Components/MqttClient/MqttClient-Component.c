//=============================================================================
//header:

#include "MqttClient-Component.h"

#if defined(_MQTT_CLIENT_COMPONENT_H_)
//=============================================================================
//includes:

#include "MqttClient-Component.h"
#include "Net/Net-Component.h"
#include "Components/USART-Ports/USART-Ports-Component.h"
#include "Adapters/FreeRTOS-MQTT/MqttClient-Adapter.h"
#include "Adapters/Ports/FreeRTOS-MQTT/MqttPort-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//import:


//==============================================================================
//variables:

#if MQTT_CLIENT_COMPONENT_TASK_ENABLE == 1 && OS_TYPE == OS_TYPE_FREERTOS

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[MQTT_TASK_STACK_SIZE] MQTT_CLIENT_COMPONENT_TASK_STACK_SECTION;

static int RTOS_MqttClientTaskStackWaterMark;

#endif //MQTT_CLIENT_COMPONENT_TASK_ENABLE
//------------------------------------------------------------------------------

static uint8_t privateMqttTxBuffer[MQTT_CLIENT_RX_BUFFER_SIZE];
static uint8_t privateMqttPortTxBuffer[MQTT_CLIENT_PORT_TX_BUFFER_SIZE];
//------------------------------------------------------------------------------

xPortT MqttPort;
xMqttT MqttClient;

uint32_t MqttTxTimeStamp = 0;
//==============================================================================
//functions:

#if MQTT_CLIENT_COMPONENT_TASK_ENABLE == 1
static void privateTask(void* arg)
{
	while (true)
	{
		RTOS_MqttClientTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);

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
#endif //privateTask
//------------------------------------------------------------------------------

#ifndef MqttClientComponentHandler
void MqttClientComponentHandler()
{

}
#endif //MqttClientComponentHandler

//------------------------------------------------------------------------------

#ifndef MqttClientComponentTimeSynchronization
void MqttClientComponentTimeSynchronization()
{

}
#endif
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
	MqttPortAdapterInitT portAdapterInit = { 0 };
	portAdapterInit.ClientId = MQTT_CLIENT_ID;
	portAdapterInit.TxTopic = MQTT_CLIENT_TX_TOPIC;
	portAdapterInit.RxTopic = MQTT_CLIENT_RX_TOPIC;
	portAdapterInit.MqttBuffer = privateMqttTxBuffer;
	portAdapterInit.MqttBufferSize = sizeof(privateMqttTxBuffer);
	portAdapterInit.TxBuffer = privateMqttPortTxBuffer;
	portAdapterInit.TxBufferSize = sizeof(privateMqttPortTxBuffer);

	MqttPortAdapterInit(&MqttPort, &privateMqttPortAdapter, &portAdapterInit);

	xPortInitT portInit = { 0 };
	portInit.Parent = parent;
	portInit.EventListener = (void*)privateEventListener;

	xPortInit(&MqttPort, &portInit);

#if MQTT_CLIENT_COMPONENT_TASK_ENABLE == 1 && OS_TYPE == OS_TYPE_FREERTOS
	taskHandle = xTaskCreateStatic(privateTask, // Function that implements the task.
			MQTT_TASK_NAME, // Text name for the task.
			MQTT_TASK_STACK_SIZE, // Number of indexes in the xStack array.
			NULL, // Parameter passed into the task.
			MQTT_TASK_PRIORITY, // Priority at which the task is created.
			taskStack, // Array to use as the task's stack.
			&taskBuffer);
#endif

	return xResultAccept;
}
//==============================================================================
#endif //_MQTT_CLIENT_COMPONENT_H_
