//==============================================================================
//includes:

#include "Common/xMemory.h"
#include "VirtualDevice-Adapter.h"
#include "TransferLayer/TransferLayer-Component.h"
#include "RequestControl/HostRequestControl-Component.h"
#include "Devices/Host/HostDevice-Component.h"
#include "Services/Temperature/Adapters/VirtualTemperatureService-Adapter.h"
#include "Components.h"
//==============================================================================
//defines:


//==============================================================================
//types:

typedef struct
{
	uint8_t ServicesCount;

	uint8_t TotalServiceNumber;

} ServicesInitHandle;
//------------------------------------------------------------------------------

typedef enum
{
	InitStateGetNumberOfServices,
	InitStateGetServices,
	InitStateComplite

} InitStates;
//==============================================================================
//variables:

static uint16_t id = 100;
//==============================================================================
//functions:

static void privateServicesEventListener(xServiceT* service, int selector, void* arg)
{

}
//------------------------------------------------------------------------------
static void privateRequestEventListener(xRequestControlT* control, int selector, xRequestT* request, ...)
{
	CAN_LocalRequestT* extansion = (void*)request;
	xDeviceT* device = xServiceGetDevice(extansion->Recipient);
	VirtualDeviceAdapterT* adapter = (VirtualDeviceAdapterT*)device->Adapter.Content;

	if (selector != xRequestEventComlite)
	{
		return;
	}

	switch (extansion->Action)
	{
		case GAPServiceRequestGetNumberOfServices:
		{
			adapter->Content.ServicesCount = *(uint32_t*)GetParameter(request, 1);
			adapter->Content.TotalServiceNumber = 0;
			adapter->Content.ServicesInitState = InitStateGetServices;
			break;
		}
		case GAPServiceRequestGetService:
		{
			volatile CAN_LocalResponseGATGetServiceT responseContent = { .Value = *(uint32_t*)GetParameter(request, 1) };
			adapter->Content.TotalServiceNumber++;
			(void)responseContent;

			if(xDeviceGetServiceById(device, responseContent.Id))
			{
				break;
			}

			switch (responseContent.Type)
			{
				case xServiceTypeTemperatureControl:
				{
					TemperatureServiceT* service = xMemoryAllocate(1, sizeof(TemperatureServiceT));
					service->Base.DynamicallyAllocated = true;

					VirtualTemperatureServiceAdapterT* serviceAdapter = xMemoryAllocate(1, sizeof(VirtualTemperatureServiceAdapterT));
					VirtualTemperatureServiceAdapterInit(service, serviceAdapter, NULL);

					TemperatureServiceInitT temperatureServiceInit;
					temperatureServiceInit.Base.Id = responseContent.Id;
					temperatureServiceInit.Base.EventListener = (void*)privateServicesEventListener;

					TemperatureServiceInit(service, &temperatureServiceInit);

					xDeviceAddService(device, (void*)service);

					GAPServiceRequestSetIdT parameter;
					parameter.ServiceId =  responseContent.Id;
					parameter.NewServiceId = id;

					/*xServiceAsyncRequestT asyncRequest = { 0 };
					asyncRequest.Callback = privateCallbackGetTemperature;
					asyncRequest.Content = content;*/

					//xServiceRequestListener((void*)&adapter->Content.GAP, xServiceRequestSetId, NULL, &parameter);

					id++;

					break;
				}
			}
			break;
		}
		default: return;
	}

	adapter->Content.OperationTimeOut = 0;
}
//------------------------------------------------------------------------------
static void privateHandler(xDeviceT* device)
{
	VirtualDeviceAdapterT* adapter = (VirtualDeviceAdapterT*)device->Adapter.Content;

	uint32_t time = xSystemGetTime(NULL);
	if (time - adapter->Content.OperationTimeStamp < adapter->Content.OperationTimeOut)
	{
		return;
	}

	switch (adapter->Content.ServicesInitState)
	{
		case InitStateGetNumberOfServices:
		{
			adapter->Content.OperationTimeStamp = time;
			adapter->Content.OperationTimeOut = 1000;

			CAN_LocalRequestT* request = xRequestNew(&HostRequestControl);
			request->Base.EventListener = privateRequestEventListener;
			request->Base.Sender = (void*)&HostGAP;
			request->Action = GAPServiceRequestGetNumberOfServices;
			request->Recipient = (void*)&adapter->Content.GAP;

			xRequestControlAdd(&HostRequestControl, (void*)request);
			break;
		}

		case InitStateGetServices:
		{
			if (adapter->Content.TotalServiceNumber < adapter->Content.ServicesCount)
			{
				adapter->Content.OperationTimeStamp = time;
				adapter->Content.OperationTimeOut = 1000;

				CAN_LocalRequestT* request = xRequestNew(&HostRequestControl);
				request->Base.EventListener = privateRequestEventListener;
				request->Base.Sender = (void*)&HostGAP;
				request->Action = GAPServiceRequestGetService;
				request->Recipient = (void*)&adapter->Content.GAP;

				request->Data.Bytes[0] = adapter->Content.TotalServiceNumber;
				request->Base.TxDataSize = 1;

				xRequestControlAdd(&HostRequestControl, (void*)request);

				break;
			}

			adapter->Content.ServicesInitState = InitStateComplite;
			break;
		}
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xDeviceT* device, xDeviceAdapterRequestSelector selector, void* arg)
{
	//VirtualDeviceAdapterT* adapter = (VirtualDeviceAdapterT*)device->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xDeviceAdapterRequestDispose:
		{
			break;
		}

		case xDeviceAdapterRequestGetPort:
		{
			//*(void**)arg = adapter->Port;
			*(void**)arg = &CAN_LocalPort;
			break;
		}

		case xDeviceAdapterRequestGetTransferLayer:
		{
			//*(void**)arg = adapter->TransferLayer;
			*(void**)arg = &LocalTransferLayer;
			break;
		}

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xDeviceT* device, xDeviceAdapterEventSelector selector, void* arg)
{
	VirtualDeviceAdapterT* adapter = (VirtualDeviceAdapterT*)device->Adapter.Content;

	switch((int)selector)
	{
		case xDeviceAdapterEventDeviceInit:
			adapter->Content.GAP.Base.Id = device->Id;

			break;

		default: return;
	}
}
//==============================================================================
//initializations:

static xDeviceAdapterInterfaceT privateAdapterInterface =
{
	.Handler = (xDeviceAdapterHandlerT)privateHandler,

	.RequestListener = (xDeviceAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xDeviceAdapterEventListenerT)PrivateEventListener,
};
//------------------------------------------------------------------------------
xResult VirtualDeviceAdapterInit(xDeviceT* device, VirtualDeviceAdapterT* adapter, VirtualDeviceAdapterInitT* init)
{
	if (device && init)
	{
		device->Adapter.Content = adapter;
		device->Adapter.Interface = &privateAdapterInterface;
		//device->Adapter.Description = nameof(VirtualDeviceAdapterT);

		memset(&adapter->Content, 0, sizeof(adapter->Content));

		//adapter->Port = init->Port;
		//adapter->TransferLayer = init->TransferLayer;

		VirtualGAPServiceAdapterInitT gapServiceAdapterInit;
		VirtualGAPServiceAdapterInit(&adapter->Content.GAP, &adapter->Content.GAPAdapter, &gapServiceAdapterInit);

		GAPServiceInitT gapServiceInit;
		gapServiceInit.Base.EventListener = (void*)privateServicesEventListener;
		gapServiceInit.Base.Id = device->Id;
		GAPServiceInit(&adapter->Content.GAP, &gapServiceInit);

		xDeviceAddService(device, (xServiceT*)&adapter->Content.GAP);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
