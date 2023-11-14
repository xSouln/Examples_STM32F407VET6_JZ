//==============================================================================
//includes:

#include "HostDevice-Component.h"
#include "Components.h"

#include "Abstractions/xDevice/Communication/xDevice-RxTransactions.h"
#include "Abstractions/xDevice/Communication/xService-RxTransactions.h"
#include "Services/Temperature/Communication/TemperatureService-RxTransactions.h"
#include "Services/Relay/Communication/RelayService-RxTransactions.h"

#include "Services/Temperature/Adapters/TemperatureService-Adapter.h"
#include "Services/Relay/Adapters/RelayService-Adapter.h"
#include "Services/GAP/Adapters/GAPService-Adapter.h"

#include "Devices/Adapters/HostDevice-Adapter.h"
//==============================================================================
//defines:

#define TASK_STACK_SIZE 0x100

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

int RTOS_HostDeviceComponentTaskStackWaterMark;

GAPServiceT HostGAP;

static TemperatureServiceT TemperatureService1;
static TemperatureServiceT TemperatureService2;

RelayServiceT RelayService;

xDeviceT HostDevice;
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
static void privateGAPServiceEventListener(xServiceT* service, int selector, void* arg)
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
		HostRequestControlComponentHandler();

		xDeviceHandler(&HostDevice);

		RTOS_HostDeviceComponentTaskStackWaterMark = uxTaskGetStackHighWaterMark(NULL);

		//vTaskDelay(pdMS_TO_TICKS(1));
	}
}
//------------------------------------------------------------------------------
void HostDeviceComponentHandler()
{

}
//------------------------------------------------------------------------------
void HostDeviceComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:

static HostDeviceAdapterT privateHostDeviceAdapter;

static TemperatureServiceAdapterT privateTemperatureServiceAdapter1;
static TemperatureServiceAdapterT privateTemperatureServiceAdapter2;

static GAPServiceAdapterT privateGAPServiceAdapter;

static RelayServiceAdapterT privateRelayServiceAdapter1;
//------------------------------------------------------------------------------
static xTerminalObjectT privateServiceTerminalObject =
{
	.Requests = xServiceRxRequests,
	.Object = (void*)&HostDevice
};
//------------------------------------------------------------------------------
static xTerminalObjectT privateDeviceTerminalObject =
{
	.Requests = xDeviceRxRequests,
	.Object = (void*)&HostDevice
};
//------------------------------------------------------------------------------
static xTerminalObjectT privateTemperatureServiceTerminalObject =
{
	.Requests = TemperatureServiceRxRequests,
	.Object = (void*)&HostDevice
};
//------------------------------------------------------------------------------
static xTerminalObjectT privateRelayServiceTerminalObject =
{
	.Requests = RelayServiceRxRequests,
	.Object = (void*)&HostDevice
};
//------------------------------------------------------------------------------
static xServiceSubscriberT privateRelayServiceSubscriber =
{
	.Object = &RelayService
};
//------------------------------------------------------------------------------
static xServiceSubscriberT privateTemperatureService1Subscriber =
{
	.Object = &TemperatureService1
};
//==============================================================================
//initialization:

xResult HostDeviceComponentInit(void* parent)
{
	HostRequestControlComponentInit(parent);

	HostDeviceAdapterInitT deviceAdapterInit;
	deviceAdapterInit.Port = &CAN_Local1;
	deviceAdapterInit.TransferLayer = &LocalTransferLayer;
	HostDeviceAdapterInit(&HostDevice, &privateHostDeviceAdapter, &deviceAdapterInit);

	xDeviceInitT deviceInit = { 0 };
	deviceInit.Parent = parent;
	deviceInit.Id = LOCAL_DEVICE_ID;
	deviceInit.EventListener = (void*)privateDeviceEventListener;
	xDeviceInit(&HostDevice, &deviceInit);

	HostDevice.MAC = UniqueDeviceID->MAC;
	//----------------------------------------------------------------------------
	GAPServiceAdapterInitT gapServiceAdapterInit;
	gapServiceAdapterInit.Port = &CAN_Local1;
	GAPServiceAdapterInit(&HostGAP, &privateGAPServiceAdapter, &gapServiceAdapterInit);

	GAPServiceInitT gapServiceInit;
	gapServiceInit.Base.EventListener = (void*)privateGAPServiceEventListener;
	gapServiceInit.Base.Id = HostDevice.Id;
	GAPServiceInit(&HostGAP, &gapServiceInit);

	xDeviceAddService(&HostDevice, (xServiceT*)&HostGAP);
	//----------------------------------------------------------------------------
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

	xDeviceAddService(&HostDevice, (xServiceT*)&TemperatureService1);
	xDeviceAddService(&HostDevice, (xServiceT*)&TemperatureService2);
	//----------------------------------------------------------------------------
	RelayServiceAdapterInitT relayServiceAdapterInit;
	RelayServiceAdapterInit(&RelayService, &privateRelayServiceAdapter1, &relayServiceAdapterInit);

	RelayServiceInitT relayServiceInit;
	relayServiceInit.Base.EventListener = (void*)privateRelayServiceEventListener;
	relayServiceInit.Base.Id = RELAY_SERVICE1_ID;
	RelayServiceInit(&RelayService, &relayServiceInit);

	xDeviceAddService(&HostDevice, (xServiceT*)&RelayService);
	HostDevice.IsEnable = true;
	HostDevice.IsAvailable = true;
	//----------------------------------------------------------------------------

	xServiceSubscribe((void*)&TemperatureService1, &privateRelayServiceSubscriber);
	xServiceSubscribe((void*)&TemperatureService2, &privateRelayServiceSubscriber);
	xServiceSubscribe((void*)&TemperatureService2, &privateTemperatureService1Subscriber);

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
