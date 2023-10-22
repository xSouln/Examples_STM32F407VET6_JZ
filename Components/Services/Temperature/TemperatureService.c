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

static void privateHandler(TemperatureServiceT* service)
{
	service->Adapter.Interface->Handler((void*)service);
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(TemperatureServiceT* service, xServiceAdapterRequestSelector selector, void* arg)
{
	if (selector > xServiceBaseRequestOffset && service->Base.Info.Type != xServiceTypeTemperatureControl)
	{
		return xResultInvalidRequest;
	}

	switch ((int)selector)
	{
		case TemperatureServiceRequestGetTemperature:
		{
			*(float*)arg = service->Temperature;
			break;
		}

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void privateEventListener(TemperatureServiceT* service, xServiceAdapterEventSelector selector, void* arg)
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
	.Handler = (xServiceAdapterHandlerT)privateHandler,

	.RequestListener = (xServiceAdapterRequestListenerT)privateRequestListener,
	.EventListener = (xServiceAdapterEventListenerT)privateEventListener,
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
