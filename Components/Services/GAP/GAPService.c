//==============================================================================
//includes:

#include "GAPService.h"
//==============================================================================
//defines:

#define UID xSERVICE_UID
//==============================================================================
//variables:

/*static const ObjectDescriptionT privateObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = UID,
	.Type = nameof(GAPServiceT)
};*/
//==============================================================================
//functions:

static void privateHandler(GAPServiceT* service)
{
	service->Adapter.Interface->Handler((void*)service);
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(GAPServiceT* service, xServiceAdapterRequestSelector selector, void* arg)
{
	if ((int)selector > xServiceBaseRequestOffset && service->Base.Info.Type != xServiceTypeTemperatureControl)
	{
		return xResultInvalidRequest;
	}

	switch ((int)selector)
	{
		case xServiceAdapterRequestDispose:
		{
			service->Base.Adapter.Interface->RequestListener(service, GAPServiceAdapterRequestDispose, NULL);
			break;
		}

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

xResult GAPServiceInit(GAPServiceT* service, GAPServiceInitT* init)
{
	xServiceInit((xServiceT*)service, (xServiceInitT*)init);

	service->Base.Info.Type = xServiceTypeGAP;

	service->Base.Adapter.Interface = &privateInterface;
	service->Base.Adapter.Description = nameof(GAPServiceT);

	return xResultAccept;
}
//==============================================================================
