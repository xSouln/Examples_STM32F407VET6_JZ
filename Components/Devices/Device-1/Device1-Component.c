//==============================================================================
//includes:

#include "Device1-Component.h"
#include "CAN_Local/CAN_Local-Component.h"
#include "TransferLayer/TransferLayer-Component.h"

#include "Services/Temperature/Adapters/TemperatureService-Adapter.h"
#include "Services/GAP/Adapters/GAPService-Adapter.h"
#include "Devices/Adapters/ClientDevice-Adapter.h"
//==============================================================================
//defines:

#define DEVICE_ID 2001
//==============================================================================
//import:


//==============================================================================
//variables:

static GAPServiceT GAPService;

TemperatureServiceT TemperatureService3;
TemperatureServiceT TemperatureService4;

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
static void privateDeviceEventListener(xDeviceT* device, xDeviceEventSelector selector, void* arg)
{
	switch ((int)selector)
	{
		case xDeviceEventIdChanged:
		{
			GAPService.Base.Id = device->Id;
			break;
		}

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
//------------------------------------------------------------------------------
static void privateDeviceGAPEventListener(xServiceT* service, int selector, void* arg)
{
	switch ((int)selector)
	{
		default: break;
	}
}
//==============================================================================
//initializations:

static GAPServiceAdapterT privateGAPServiceAdapter;

static ClientDeviceAdapterT privateDeviceAdapter;

static TemperatureServiceAdapterT privateTemperatureServiceAdapter1;
static TemperatureServiceAdapterT privateTemperatureServiceAdapter2;
//==============================================================================
//initialization:

xResult Device1ComponentInit(void* parent)
{
	ClientDeviceAdapterInitT deviceAdapterInit;
	deviceAdapterInit.Port = &CAN_Local2;
	deviceAdapterInit.TransferLayer = &ExternalTransferLayer;
	Device1.MAC = 0x1122334466880001;
	ClientDeviceAdapterInit(&Device1, &privateDeviceAdapter, &deviceAdapterInit);

	xDeviceInitT deviceInit = { 0 };
	deviceInit.Parent = parent;
	deviceInit.Id = DEVICE_ID;
	deviceInit.EventListener = (void*)privateDeviceEventListener;
	xDeviceInit(&Device1, &deviceInit);
	//----------------------------------------------------------------------------
	GAPServiceAdapterInitT gapServiceAdapterInit;
	gapServiceAdapterInit.Port = &CAN_Local2;
	GAPServiceAdapterInit(&GAPService, &privateGAPServiceAdapter, &gapServiceAdapterInit);

	GAPServiceInitT gapServiceInit;
	gapServiceInit.Base.EventListener = (void*)privateDeviceGAPEventListener;
	gapServiceInit.Base.Id = DEVICE_ID;
	GAPServiceInit(&GAPService, &gapServiceInit);

	xDeviceAddService(&Device1, (xServiceT*)&GAPService);
	//----------------------------------------------------------------------------
	TemperatureServiceAdapterInitT temperatureServiceAdapterInit;
	temperatureServiceAdapterInit.Port = &CAN_Local2;
	TemperatureServiceAdapterInit(&TemperatureService3, &privateTemperatureServiceAdapter1, &temperatureServiceAdapterInit);
	TemperatureServiceAdapterInit(&TemperatureService4, &privateTemperatureServiceAdapter2, &temperatureServiceAdapterInit);

	TemperatureServiceInitT temperatureServiceInit;
	temperatureServiceInit.Base.EventListener = (void*)privateServiceEventListener;

	temperatureServiceInit.Base.Id = TEMPERATURE_SERVICE3_ID;
	TemperatureServiceInit(&TemperatureService3, &temperatureServiceInit);

	temperatureServiceInit.Base.Id = TEMPERATURE_SERVICE4_ID;
	TemperatureServiceInit(&TemperatureService4, &temperatureServiceInit);

	xDeviceAddService(&Device1, (xServiceT*)&TemperatureService3);
	xDeviceAddService(&Device1, (xServiceT*)&TemperatureService4);

	return xResultAccept;
}
//==============================================================================
