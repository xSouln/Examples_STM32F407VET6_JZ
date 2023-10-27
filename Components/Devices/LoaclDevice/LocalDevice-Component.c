//==============================================================================
//includes:

#include "LocalDevice-Component.h"
#include "Components.h"

#include "Abstractions/xDevice/Communication/xDevice-RxTransactions.h"
#include "Abstractions/xDevice/Communication/xService-RxTransactions.h"
#include "Services/Temperature/Communication/TemperatureService-RxTransactions.h"
#include "Services/Relay/Communication/RelayService-RxTransactions.h"

#include "Services/Temperature/Adapters/TemperatureService-Adapter.h"
#include "Services/Relay/Adapters/RelayService-Adapter.h"

#include "Devices/Adapters/LocalDevice-Adapter.h"
//==============================================================================
//defines:

#define TASK_STACK_SIZE 0x180

#define LOCAL_DEVICE_ID 2000
#define TEMPERATURE_SERVICE1_ID 22
#define TEMPERATURE_SERVICE2_ID 23

#define RELAY_SERVICE1_ID 50
//==============================================================================
//import:


//==============================================================================
//variables:

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE];

static TemperatureServiceT TemperatureService1;
static TemperatureServiceT TemperatureService2;

RelayServiceT RelayService;

xDeviceT LocalDevice;
//==============================================================================
//functions:

static void privateServiceEventListener(xServiceT* service, xServiceAdapterEventSelector selector, void* arg)
{
	switch ((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------
static void privateDeviceEventListener(xDeviceT* object, xDeviceEventSelector selector, void* arg)
{
	switch ((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------
static void privateRelayServiceEventListener(xServiceT* service, xServiceAdapterEventSelector selector, void* arg)
{
	switch ((int)selector)
	{
		default: break;
	}
}
//------------------------------------------------------------------------------
static void privateTask(void* arg)
{
	vTaskDelay(pdMS_TO_TICKS(1000));

	while (true)
	{
		xDeviceHandler(&LocalDevice);

		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
//------------------------------------------------------------------------------
void LocalDeviceComponentHandler()
{

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

static RelayServiceAdapterT privateRelayServiceAdapter1;
//------------------------------------------------------------------------------
static xTerminalObjectT privateServiceTerminalObject =
{
	.Requests = xServiceRxRequests,
	.Object = (void*)&LocalDevice
};
//------------------------------------------------------------------------------
static xTerminalObjectT privateDeviceTerminalObject =
{
	.Requests = xDeviceRxRequests,
	.Object = (void*)&LocalDevice
};
//------------------------------------------------------------------------------
static xTerminalObjectT privateTemperatureServiceTerminalObject =
{
	.Requests = TemperatureServiceRxRequests,
	.Object = (void*)&LocalDevice
};
//------------------------------------------------------------------------------
static xTerminalObjectT privateRelayServiceTerminalObject =
{
	.Requests = RelayServiceRxRequests,
	.Object = (void*)&LocalDevice
};
//==============================================================================
//initialization:

xResult LocalDeviceComponentInit(void* parent)
{
	LocalDeviceAdapterInitT deviceAdapterInit;
	deviceAdapterInit.Port = &CAN_Local1;
	LocalDeviceAdapterInit(&LocalDevice, &privateLocalDeviceAdapter, &deviceAdapterInit);

	xDeviceInitT deviceInit = { 0 };
	deviceInit.Parent = parent;
	deviceInit.Id = LOCAL_DEVICE_ID;
	deviceInit.EventListener = (void*)privateDeviceEventListener;
	xDeviceInit(&LocalDevice, &deviceInit);

	TemperatureServiceAdapterInitT temperatureServiceAdapterInit;
	temperatureServiceAdapterInit.Port = &CAN_Local1;
	TemperatureServiceAdapterInit(&TemperatureService1, &privateTemperatureServiceAdapter1, &temperatureServiceAdapterInit);
	TemperatureServiceAdapterInit(&TemperatureService2, &privateTemperatureServiceAdapter2, &temperatureServiceAdapterInit);

	TemperatureServiceInitT temperatureServiceInit;
	temperatureServiceInit.Base.EventListener = (void*)privateServiceEventListener;

	temperatureServiceInit.Base.Id = TEMPERATURE_SERVICE1_ID;
	TemperatureServiceInit(&TemperatureService1, &temperatureServiceInit);

	temperatureServiceInit.Base.Id = TEMPERATURE_SERVICE2_ID;
	TemperatureServiceInit(&TemperatureService2, &temperatureServiceInit);

	xDeviceAddService(&LocalDevice, (xServiceT*)&TemperatureService1);
	xDeviceAddService(&LocalDevice, (xServiceT*)&TemperatureService2);

	RelayServiceAdapterInitT relayServiceAdapterInit;
	RelayServiceAdapterInit(&RelayService, &privateRelayServiceAdapter1, &relayServiceAdapterInit);

	RelayServiceInitT relayServiceInit;
	relayServiceInit.Base.EventListener = (void*)privateRelayServiceEventListener;
	relayServiceInit.Base.Id = RELAY_SERVICE1_ID;
	RelayServiceInit(&RelayService, &relayServiceInit);

	xDeviceAddService(&LocalDevice, (xServiceT*)&RelayService);

	xServiceSubscribe(&TemperatureService1, &RelayService);
	xServiceSubscribe(&TemperatureService2, &RelayService);
	xServiceSubscribe(&TemperatureService2, &TemperatureService1);

	TerminalAddObject(&privateDeviceTerminalObject);
	TerminalAddObject(&privateServiceTerminalObject);
	TerminalAddObject(&privateTemperatureServiceTerminalObject);
	TerminalAddObject(&privateRelayServiceTerminalObject);

	taskHandle =
				xTaskCreateStatic(privateTask, // Function that implements the task.
									"device control task", // Text name for the task.
									TASK_STACK_SIZE, // Number of indexes in the xStack array.
									NULL, // Parameter passed into the task.
									osPriorityNormal, // Priority at which the task is created.
									taskStack, // Array to use as the task's stack.
									&taskBuffer);

	return xResultAccept;
}
//==============================================================================
