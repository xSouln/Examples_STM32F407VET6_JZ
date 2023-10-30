//==============================================================================
//includes:

#include "DeviceService.h"
//==============================================================================
//defines:

#define UID xSERVICE_UID
//==============================================================================
//variables:

/*static const ObjectDescriptionT privateObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = UID,
	.Type = nameof(DeviceServiceT)
};*/
//==============================================================================
//functions:

static void privateHandler(DeviceServiceT* service)
{
	service->Adapter.Interface->Handler((void*)service);
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(DeviceServiceT* service, xServiceAdapterRequestSelector selector, void* arg)
{
	if ((int)selector > xServiceBaseRequestOffset && service->Base.Info.Type != xServiceTypeTemperatureControl)
	{
		return xResultInvalidRequest;
	}

	switch ((int)selector)
	{
		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//==============================================================================
//initialization:

static xServiceAdapterInterfaceT privateInterface =
{
	.Handler = (xServiceAdapterHandlerT)privateHandler,
	.RequestListener = (xServiceAdapterRequestListenerT)privateRequestListener
};
//------------------------------------------------------------------------------

xResult DeviceServiceInit(DeviceServiceT* service, DeviceServiceInitT* init)
{
	xServiceInit((xServiceT*)service, (xServiceInitT*)init);

	service->Base.Info.Type = xServiceTypeDeviceControl;

	service->Base.Adapter.Interface = &privateInterface;
	service->Base.Adapter.Description = nameof(DeviceServiceT);

	return xResultAccept;
}
//==============================================================================
