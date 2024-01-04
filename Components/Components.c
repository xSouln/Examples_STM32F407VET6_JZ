//==============================================================================
//includes:

#include "Components.h"
#include "main.h"
#include "Peripherals/xTimer/xTimer.h"
#include "Common/xList.h"
#include "CAN_Local-Types.h"
//==============================================================================
//defines:

#if FREERTOS_ENABLE == 1
#define COMPONENTS_MAIN_TASK_STACK_SIZE 0x200
#endif
//------------------------------------------------------------------------------
#define HTTP_HOST "device-api.sintez.by"
#define HTTP_HOST_AUTHORIZATION "X-Sintez-Auth: 6b9f7b57e10f498e634204e77009e472"
#define HTTP_REPORT_DEVICE_ID "ce501046-f876-4bf6-8151-117b711340c5"
#define HTTP_GET_HEADER "GET /v0.4/devices/" HTTP_REPORT_DEVICE_ID " HTTP/1.1\r\n" HTTP_HOST_AUTHORIZATION "\r\n" "Host: " HTTP_HOST "\r\n\r\n"
//==============================================================================
//variables:

UniqueDeviceID_T* UniqueDeviceID = (void*)0x1FFF7A10;

static uint32_t ledToggleTimeStamp;
//------------------------------------------------------------------------------
#if FREERTOS_ENABLE == 1

int RTOS_FreeHeapSize;
int RTOS_ComponentsTaskStackWaterMark;

//static TaskHandle_t taskHandle;
#endif

//==============================================================================
//functions:


//==============================================================================
//default functions:

xResult ComponentsRequestListener(ObjectBaseT* object, int selector, uint32_t description, void* arg)
{
	return xResultNotSupported;
}
//------------------------------------------------------------------------------
void ComponentsEventListener(ObjectBaseT* object, int selector, uint32_t description, void* arg)
{

}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
void ComponentsHandler()
{
	UsartPortsComponentHandler();
	TerminalComponentHandler();

#if NET_ENABLE == 1
	NetComponentHandler();
#endif

#if DEVICE_CONTROL_ENABLE == 1

	LocalTransferLayerComponentHandler();
	HostTransferLayerComponentHandler();

	RequestControlComponentHandler();

	Device1ComponentHandler();

#endif
	uint32_t time = xSystemGetTime();
	if (time - ledToggleTimeStamp > 999)
	{
		ledToggleTimeStamp = time;

		PortE->Output.LED1 ^= 1;
		PortE->Output.LED2 ^= PortE->Output.LED1;
		PortE->Output.LED3 ^= PortE->Output.LED1 && PortE->Output.LED2;
	}

#if FREERTOS_ENABLE == 1
	RTOS_FreeHeapSize = xPortGetFreeHeapSize();
	RTOS_ComponentsTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);
#endif
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void ComponentsTimeSynchronization()
{
	TerminalComponentTimeSynchronization();
	UsartPortsComponentTimeSynchronization();

#if DEVICE_CONTROL_ENABLE == 1

	Device1ComponentTimeSynchronization();

#endif
}
//------------------------------------------------------------------------------
void Timer4_IRQ_Handler(xTimerT* timer, xTimerHandleT* handle)
{
	handle->Status.UpdateInterrupt = false;

	ComponentsTimeSynchronization();
}
//==============================================================================
//initialization:

/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return xResult
 */
xResult ComponentsInit(void* parent)
{
	xSystemInit(parent);

	TerminalComponentInit(parent);

	UsartPortsComponentInit(parent);

#if NET_ENABLE == 1
	NetComponentInit(parent);

#if MQTT_ENABLE == 1
	MqttClientComponentInit(parent);
#endif

#endif

#if DEVICE_CONTROL_ENABLE == 1

	CAN_PortsComponentInit(parent);

	LocalTransferLayerComponentInit(parent);
	HostTransferLayerComponentInit(parent);

	HostDeviceComponentInit(parent);

	RequestControlComponentInit(parent);
	Device1ComponentInit(parent);

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
