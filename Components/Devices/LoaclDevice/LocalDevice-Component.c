//==============================================================================
//includes:

#include "LocalDevice-Component.h"
#include "Components.h"

#include "Services/Temperature/Adapters/TemperatureService-Adapter.h"
#include "Adapters/LocalDevice-Adapter.h"
//==============================================================================
//defines:

#define TASK_STACK_SIZE 0x200
//==============================================================================
//import:


//==============================================================================
//variables:

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE];


static uint32_t timeStamp;

TemperatureServiceT TemperatureService1;
TemperatureServiceT TemperatureService2;

xDeviceT LocalDevice;
//==============================================================================
//functions:

static void PrivateEventListener(ObjectBaseT* object, int selector, void* arg)
{

}
//------------------------------------------------------------------------------
static void privateTask(void* arg)
{
	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
//------------------------------------------------------------------------------
void LocalDeviceComponentHandler()
{
	xDeviceHandler(&LocalDevice);
}
//------------------------------------------------------------------------------
void LocalDeviceComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:

static LocalDeviceAdapterT privateLocalDeviceAdapter;

static TemperatureServiceAdapterT privateTemperatureServiceAdapter1;
static TemperatureServiceAdapterT privateTemperatureServiceAdapter2;
//------------------------------------------------------------------------------
/*static TerminalObjectT privateTerminalObject =
{
	.Requests = RxRequests
};*/
//==============================================================================
//initialization:

xResult LocalDeviceComponentInit(void* parent)
{
	TemperatureServiceAdapterInitT temperatureServiceAdapterInit;
	TemperatureServiceAdapterInit(&TemperatureService1, &privateTemperatureServiceAdapter1, &temperatureServiceAdapterInit);
	TemperatureServiceAdapterInit(&TemperatureService2, &privateTemperatureServiceAdapter2, &temperatureServiceAdapterInit);

	TemperatureServiceInitT temperatureServiceInit;
	temperatureServiceInit.Base.Holder = &LocalDevice;
	TemperatureServiceInit(&TemperatureService1, &temperatureServiceInit);
	TemperatureServiceInit(&TemperatureService2, &temperatureServiceInit);

	LocalDeviceAdapterInitT deviceAdapterInit;
	LocalDeviceAdapterInit(&LocalDevice, &privateLocalDeviceAdapter, &deviceAdapterInit);

	xDeviceInitT deviceInit = { 0 };
	xDeviceInit(&LocalDevice, &deviceInit);

	xDeviceAddService(&LocalDevice, (xServiceT*)&TemperatureService1);
	xDeviceAddService(&LocalDevice, (xServiceT*)&TemperatureService2);
	/*taskHandle =
			xTaskCreateStatic(privateTask, // Function that implements the task.
								"CAN example task", // Text name for the task.
								TASK_STACK_SIZE, // Number of indexes in the xStack array.
								NULL, // Parameter passed into the task.
								osPriorityNormal, // Priority at which the task is created.
								taskStack, // Array to use as the task's stack.
								&taskBuffer);*/

	return xResultAccept;
}
//==============================================================================
