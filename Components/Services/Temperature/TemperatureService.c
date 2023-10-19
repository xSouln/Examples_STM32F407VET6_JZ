//==============================================================================
//includes:

#include "TemperatureService.h"
//==============================================================================
//defines:

#define UID xSERVICE_UID
//==============================================================================
//variables:

static const ObjectDescriptionT privateObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = UID,
	.Type = nameof(TemperatureServiceT)
};
//==============================================================================
//functions:

static void PrivateHandler(TemperatureServiceT* service)
{
	service->Adapter.Interface->Handler((void*)service);
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(TemperatureServiceT* service, xServiceAdapterRequestSelector selector, void* arg)
{
	switch ((uint32_t)selector)
	{
		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(TemperatureServiceT* service, xServiceAdapterEventSelector selector, void* arg)
{
	switch((int)selector)
	{
		default: return;
	}
}
//==============================================================================
//initialization:

static xServiceAdapterInterfaceT privateInterface =
{
	.Handler = (xServiceAdapterHandlerT)PrivateHandler,

	.RequestListener = (xServiceAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xServiceAdapterEventListenerT)PrivateEventListener,
};
//------------------------------------------------------------------------------

xResult TemperatureServiceInit(TemperatureServiceT* service, TemperatureServiceInitT* init)
{
	xServiceInit((xServiceT*)service, (xServiceInitT*)init);

	service->Base.Base.Description = &privateObjectDescription;
	service->Base.Info.Type = xServiceTypeTemperatureControl;

	service->Base.Adapter.Interface = &privateInterface;
	service->Base.Adapter.Description = nameof(TemperatureServiceT);

	return xResultAccept;
}
//==============================================================================
