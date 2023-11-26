//==============================================================================
//includes:

#include <malloc.h>

#include "Components.h"
#include "main.h"
#include "Peripherals/xTimer/xTimer.h"
#include "Common/xList.h"
#include "CAN_Local-Types.h"
//==============================================================================
//defines:

#define DEVICE_CONTROL_ENABLE 1

#define TASK_STACK_SIZE 0x200

#define HTTP_HOST "device-api.sintez.by"
#define HTTP_HOST_AUTHORIZATION "X-Sintez-Auth: 6b9f7b57e10f498e634204e77009e472"
#define HTTP_REPORT_DEVICE_ID "ce501046-f876-4bf6-8151-117b711340c5"
#define HTTP_GET_HEADER "GET /v0.4/devices/" HTTP_REPORT_DEVICE_ID " HTTP/1.1\r\n" HTTP_HOST_AUTHORIZATION "\r\n" "Host: " HTTP_HOST "\r\n\r\n"
//==============================================================================
//variables:

UniqueDeviceID_T* UniqueDeviceID = (void*)0x1FFF7A10;

struct mallinfo HeapInfo;

static uint32_t ledToggleTimeStamp;
static uint32_t startTransmittingEventTimeStamp;
static uint32_t eventTransmissionTime;

static uint32_t receivedEventsCount;

int RTOS_FreeHeapSize;
int RTOS_ComponentsTaskStackWaterMark;

//static TaskHandle_t taskHandle;
//==============================================================================
//functions:

static void privateTerminalComponentEventListener(xTerminalT* terminal, xTerminalEventSelector selector, void* arg)
{
	switch((int)selector)
	{
		case xTerminalEventTime_1000ms:
			break;

		default: break;
	}
}
//==============================================================================
//default functions:

xResult ComponentsRequestListener(ObjectBaseT* object, int selector, void* arg)
{
	return xResultNotSupported;
}
//------------------------------------------------------------------------------
void ComponentsEventListener(ObjectBaseT* object, int selector, void* arg)
{
	if (object->Description->Key != OBJECT_DESCRIPTION_KEY)
	{
		return;
	}

	switch(object->Description->ObjectId)
	{
		case xTERMINAL_UID:
			privateTerminalComponentEventListener((xTerminalT*)object, selector, arg);
			break;
	}
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void ComponentsHandler()
{
	UsartPortsComponentHandler();
	TerminalComponentHandler();
#if LWIP_ENABLE == 1
	LWIP_NetTcpServerComponentHandler();
#endif

#ifdef DEVICE_CONTROL_ENABLE

	LocalTransferLayerComponentHandler();
	HostTransferLayerComponentHandler();

	RequestControlComponentHandler();

	Device1ComponentHandler();
	//Device2ComponentHandler();
	//Device3ComponentHandler();

#endif
	uint32_t time = xSystemGetTime(ComponentsHandler);
	if (time - ledToggleTimeStamp > 999)
	{
		ledToggleTimeStamp = time;

		PortE->Output.LED1 ^= 1;
		PortE->Output.LED2 ^= PortE->Output.LED1;
		PortE->Output.LED3 ^= PortE->Output.LED1 && PortE->Output.LED2;

		/*CAN_LocalContentTemperatureSensoreEventT temperatureSensoreEventContent;
		temperatureSensoreEventContent.Temperature = (float)time / 100;

		CAN_LocalBaseEventPacketT eventPacket;
		eventPacket.ContentType = 0;
		eventPacket.Id = TEMPERATURE_SERVICE3_ID;
		eventPacket.Content = temperatureSensoreEventContent.Value;
		eventPacket.EventType = 0;
		eventPacket.Content = temperatureSensoreEventContent.Value;

		CAN_LocalSegmentT packet;
		packet.Header.MessageType = CAN_LocalMessageTypeNotification;
		packet.Header.ServiceType = xServiceTypeTemperatureControl;
		packet.ExtensionIsEnabled = false;
		packet.DataLength = sizeof(uint16_t) + sizeof(CAN_LocalContentTemperatureSensoreEventT);
		packet.Data.DoubleWord = eventPacket.Value;

		packet.DataLength = 8;

		startTransmittingEventTimeStamp = time;
		xPortExtendedTransmition(&CAN_Local1, &packet);*/
	}

	RTOS_FreeHeapSize = xPortGetFreeHeapSize();
	RTOS_ComponentsTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);
	HeapInfo = mallinfo();
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void ComponentsTimeSynchronization()
{
	TerminalComponentTimeSynchronization();
	UsartPortsComponentTimeSynchronization();

#ifdef DEVICE_CONTROL_ENABLE

	Device1ComponentTimeSynchronization();
	//Device2ComponentTimeSynchronization();
	//Device3ComponentTimeSynchronization();

#endif
}
//------------------------------------------------------------------------------
void Timer4_IRQ_Handler(xTimerT* timer, xTimerHandleT* handle)
{
	handle->Status.UpdateInterrupt = false;

	ComponentsTimeSynchronization();
}
//------------------------------------------------------------------------------
static void privateCustomSubscriberEventListener(xServiceT* service, xServiceSubscriberT* Subscriber, int selector, void* arg)
{
	CAN_LocalContentTemperatureSensoreEventT* content = arg;

	if (content->Temperature > 20.0f)
	{
		receivedEventsCount++;
	}

	eventTransmissionTime = xSystemGetTime(NULL) - startTransmittingEventTimeStamp;
}
//==============================================================================
//initialization:

static xServiceSubscriberT privateCustomSubscriber =
{
	.EventListener = privateCustomSubscriberEventListener
};
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return xResult
 */
xResult ComponentsInit(void* parent)
{
	xSystemLayersInit();

	TerminalComponentInit(parent);

	xSystemInit(parent);

	UsartPortsComponentInit(parent);

#if LWIP_ENABLE == 1
	LWIP_NetTcpServerComponentInit(parent);

#if MQTT_ENABLE == 1
	MqttClientComponentInit(parent);
#endif

#endif

#ifdef DEVICE_CONTROL_ENABLE

	CAN_PortsComponentInit(parent);

	LocalTransferLayerComponentInit(parent);
	HostTransferLayerComponentInit(parent);

	HostDeviceComponentInit(parent);

	RequestControlComponentInit(parent);
	Device1ComponentInit(parent);
	//Device2ComponentInit(parent);
	//Device3ComponentInit(parent);

	xServiceSubscribe((void*)&TemperatureService3, &privateCustomSubscriber);

#endif

	xTimerCoreBind(xTimer4, Timer4_IRQ_Handler, rTimer4, 0);
	rTimer4->DMAOrInterrupts.UpdateInterruptEnable = true;
	rTimer4->Control1.CounterEnable = true;

	/*xTaskCreate(privateTask, // Function that implements the task.
				"CAN local task", // Text name for the task.
				TASK_STACK_SIZE, // Number of indexes in the xStack array.
				NULL, // Parameter passed into the task.
				osPriorityHigh, // Priority at which the task is created.
				&taskHandle);*/

	return xResultAccept;
}
//==============================================================================
