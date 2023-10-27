//==============================================================================
//includes:

#include <malloc.h>

#include "Components.h"
#include "main.h"
#include "Peripherals/xTimer/xTimer.h"
#include "Common/xList.h"
#include "Abstractions/xTxRequest/xTxRequest.h"
//==============================================================================
//defines:

#define DEVICE_CONTROL_ENABLE 1

#define TASK_STACK_SIZE 0x180

#define HTTP_HOST "device-api.sintez.by"
#define HTTP_HOST_AUTHORIZATION "X-Sintez-Auth: 6b9f7b57e10f498e634204e77009e472"
#define HTTP_REPORT_DEVICE_ID "ce501046-f876-4bf6-8151-117b711340c5"
#define HTTP_GET_HEADER "GET /v0.4/devices/" HTTP_REPORT_DEVICE_ID " HTTP/1.1\r\n" HTTP_HOST_AUTHORIZATION "\r\n" "Host: " HTTP_HOST "\r\n\r\n"
//==============================================================================
//variables:

struct mallinfo HeapInfo;

static uint32_t ledToggleTimeStamp;

int RTOS_FreeHeapSize;
int RTOS_ComponentsTaskStackWaterMark;

static TaskHandle_t taskHandle;
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
	//LWIP_NetTcpServerComponentHandler();

#ifdef DEVICE_CONTROL_ENABLE

	TransferLayerComponentHandler();

	Device1ComponentHandler();
	//Device2ComponentHandler();
	//Device3ComponentHandler();

#endif

	if (xSystemGetTime(ComponentsHandler) - ledToggleTimeStamp > 999)
	{
		ledToggleTimeStamp = xSystemGetTime(ComponentsHandler);

		PortE->Output.LED1 ^= 1;
		PortE->Output.LED2 ^= PortE->Output.LED1;
		PortE->Output.LED3 ^= PortE->Output.LED1 && PortE->Output.LED2;
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
static void privateTask(void* arg)
{
	while (true)
	{
		CAN_LocalComponentHandler();
	}
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
	xSystemLayersInit();

	TerminalComponentInit(parent);

	xSystemInit(parent);

	UsartPortsComponentInit(parent);
	//LWIP_NetTcpServerComponentInit(parent);

#ifdef DEVICE_CONTROL_ENABLE

	CAN_LocalComponentInit(parent);
	TransferLayerComponentInit(parent);
	LocalDeviceComponentInit(parent);

	Device1ComponentInit(parent);
	//Device2ComponentInit(parent);
	//Device3ComponentInit(parent);

#endif

	xTimerCoreBind(xTimer4, Timer4_IRQ_Handler, rTimer4, 0);
	rTimer4->DMAOrInterrupts.UpdateInterruptEnable = true;
	rTimer4->Control1.CounterEnable = true;

	xTaskCreate(privateTask, // Function that implements the task.
				"CAN local task", // Text name for the task.
				TASK_STACK_SIZE, // Number of indexes in the xStack array.
				NULL, // Parameter passed into the task.
				osPriorityNormal, // Priority at which the task is created.
				&taskHandle);

	return xResultAccept;
}
//==============================================================================
