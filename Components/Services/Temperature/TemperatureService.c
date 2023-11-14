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
	/*xServiceRequestBaseT* request = arg;

	xServiceRequestManagerT requestManager;
	requestManager.Service = service;
	requestManager.Operation = selector;
	requestManager.OperationResult = xResultAccept;
	requestManager.Content = request->Content;

	if ((int)selector > xServiceBaseRequestOffset && service->Base.Info.Type != xServiceTypeTemperatureControl)
	{
		return xResultInvalidRequest;
	}

	switch ((int)selector)
	{
		case TemperatureServiceRequestGetTemperature:
		{
			//*(float*)arg = service->Temperature;
			//requestManager.Result = &service->Temperature;
			service->Adapter.Interface->RequestListener(service, TemperatureServiceRequestGetTemperature, &requestManager);
			break;
		}

		default : return xResultRequestIsNotFound;
	}

	if(request->Callback)
	{
		request->Callback(&requestManager);
	}*/

	service->Adapter.Interface->RequestListener(service, TemperatureServiceRequestGetTemperature, arg);

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

xResult TemperatureServiceInit(TemperatureServiceT* service, TemperatureServiceInitT* init)
{
	xServiceInit((xServiceT*)service, (xServiceInitT*)init);

	service->Base.Base.Description = &privateObjectDescription;
	service->Base.Info.Type = xServiceTypeTemperatureControl;

	service->Base.Adapter.Interface = &privateInterface;
	//service->Base.Adapter.Description = nameof(TemperatureServiceT);

	return xResultAccept;
}
//==============================================================================
