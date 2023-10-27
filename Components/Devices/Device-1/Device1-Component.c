//==============================================================================
//includes:

#include "Device1-Component.h"
#include "CAN_Local/CAN_Local-Component.h"

#include "Services/Temperature/Adapters/TemperatureService-Adapter.h"
#include "Devices/Adapters/ClientDevice-Adapter.h"
//==============================================================================
//defines:

#define DEVICE_ID 2001
#define TEMPERATURE_SERVICE1_ID 10000
#define TEMPERATURE_SERVICE2_ID 10001
//==============================================================================
//import:


//==============================================================================
//variables:

static TemperatureServiceT TemperatureService1;
static TemperatureServiceT TemperatureService2;

xDeviceT Device1;
//==============================================================================
//functions:

static void privateServiceEventListener(xServiceT* service, int selector, void* arg)
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
void Device1ComponentHandler()
{
	xDeviceHandler(&Device1);
}
//------------------------------------------------------------------------------
void Device1ComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:

static ClientDeviceAdapterT privateDeviceAdapter;

static TemperatureServiceAdapterT privateTemperatureServiceAdapter1;
static TemperatureServiceAdapterT privateTemperatureServiceAdapter2;
//==============================================================================
//initialization:

xResult Device1ComponentInit(void* parent)
{
	ClientDeviceAdapterInitT deviceAdapterInit;
	deviceAdapterInit.Port = &CAN_Local2;
	Device1.MAC = 0x1122334466880001;
	ClientDeviceAdapterInit(&Device1, &privateDeviceAdapter, &deviceAdapterInit);

	xDeviceInitT deviceInit = { 0 };
	deviceInit.Parent = parent;
	deviceInit.Id = DEVICE_ID;
	deviceInit.EventListener = (void*)privateDeviceEventListener;
	xDeviceInit(&Device1, &deviceInit);

	TemperatureServiceAdapterInitT temperatureServiceAdapterInit;
	TemperatureServiceAdapterInit(&TemperatureService1, &privateTemperatureServiceAdapter1, &temperatureServiceAdapterInit);
	TemperatureServiceAdapterInit(&TemperatureService2, &privateTemperatureServiceAdapter2, &temperatureServiceAdapterInit);

	TemperatureServiceInitT temperatureServiceInit;
	temperatureServiceInit.Base.EventListener = (void*)privateServiceEventListener;

	temperatureServiceInit.Base.Id = TEMPERATURE_SERVICE1_ID;
	TemperatureServiceInit(&TemperatureService1, &temperatureServiceInit);

	temperatureServiceInit.Base.Id = TEMPERATURE_SERVICE2_ID;
	TemperatureServiceInit(&TemperatureService2, &temperatureServiceInit);

	xDeviceAddService(&Device1, (xServiceT*)&TemperatureService1);
	xDeviceAddService(&Device1, (xServiceT*)&TemperatureService2);

	return xResultAccept;
}
//==============================================================================
