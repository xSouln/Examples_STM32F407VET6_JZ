//==============================================================================
//includes:

#include "Device2-Component.h"

#include "Services/Temperature/Adapters/TemperatureService-Adapter.h"
#include "Devices/Adapters/LocalDevice-Adapter.h"
//==============================================================================
//defines:

#define DEVICE_ID 35476
#define SERVICE1_ID 10010
#define SERVICE2_ID 10011
//==============================================================================
//import:


//==============================================================================
//variables:

xDeviceT Device2;
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
void Device2ComponentHandler()
{
	xDeviceHandler(&Device2);
}
//------------------------------------------------------------------------------
void Device2ComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:

static LocalDeviceAdapterT privateLocalDeviceAdapter;
//==============================================================================
//initialization:

xResult Device2ComponentInit(void* parent)
{
	LocalDeviceAdapterInitT deviceAdapterInit;
	LocalDeviceAdapterInit(&Device2, &privateLocalDeviceAdapter, &deviceAdapterInit);

	xDeviceInitT deviceInit = { 0 };
	deviceInit.Parent = parent;
	deviceInit.Id = DEVICE_ID;
	deviceInit.EventListener = (void*)privateDeviceEventListener;
	xDeviceInit(&Device2, &deviceInit);

	return xResultAccept;
}
//==============================================================================
