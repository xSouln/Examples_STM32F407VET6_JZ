//==============================================================================
//includes:

#include "Device3-Component.h"

#include "Services/Temperature/Adapters/TemperatureService-Adapter.h"
#include "Devices/Adapters/LocalDevice-Adapter.h"
//==============================================================================
//defines:

#define DEVICE_ID 35477
#define SERVICE1_ID 10020
#define SERVICE2_ID 10021
//==============================================================================
//import:


//==============================================================================
//variables:

xDeviceT Device3;
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
void Device3ComponentHandler()
{
	xDeviceHandler(&Device3);
}
//------------------------------------------------------------------------------
void Device3ComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:

static LocalDeviceAdapterT privateLocalDeviceAdapter;
//==============================================================================
//initialization:

xResult Device3ComponentInit(void* parent)
{
	LocalDeviceAdapterInitT deviceAdapterInit;
	LocalDeviceAdapterInit(&Device3, &privateLocalDeviceAdapter, &deviceAdapterInit);

	xDeviceInitT deviceInit = { 0 };
	deviceInit.Parent = parent;
	deviceInit.Id = DEVICE_ID;
	deviceInit.EventListener = (void*)privateDeviceEventListener;
	xDeviceInit(&Device3, &deviceInit);

	return xResultAccept;
}
//==============================================================================
