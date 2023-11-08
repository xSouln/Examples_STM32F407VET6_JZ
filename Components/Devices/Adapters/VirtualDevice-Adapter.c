//==============================================================================
//includes:

#include "Common/xMemory.h"
#include "VirtualDevice-Adapter.h"
#include "TransferLayer/TransferLayer-Component.h"
#include "RequestControl/HostRequestControl-Component.h"
#include "Devices/Host/HostDevice-Component.h"
#include "Services/Temperature/Adapters/VirtualTemperatureService-Adapter.h"
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

			xServiceListElementT* element = xListStartEnumeration((void*)&device->Services);

			while (element)
			{
				if (element->Value->Id == responseContent.Id)
				{
					goto end;
				}

				element = element->Next;
			}

			xListStopEnumeration((void*)&device->Devices);

			if(element)
			{
				break;
			}

			switch (responseContent.Type)
			{
				case xServiceTypeTemperatureControl:
				{
					TemperatureServiceT* service = xMemoryAllocate(1, sizeof(TemperatureServiceT));
					service->Base.DynamicallyAllocated = true;

					VirtualTemperatureServiceAdapterT* adapter = xMemoryAllocate(1, sizeof(TemperatureServiceT));
					VirtualTemperatureServiceAdapterInit(service, adapter, NULL);

					TemperatureServiceInitT temperatureServiceInit;
					temperatureServiceInit.Base.Id = responseContent.Id;
					temperatureServiceInit.Base.EventListener = (void*)privateServicesEventListener;

					TemperatureServiceInit(service, &temperatureServiceInit);

					xDeviceAddService(device, (void*)service);
					break;
				}
			}
			break;
		}
		default: return;
	}

	end:;
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

	if (adapter->Content.Transfer)
	{
		return;
	}

	switch (adapter->Content.ServicesInitState)
	{
		case InitStateGetNumberOfServices:
		{
			adapter->Content.OperationTimeStamp = time;
			adapter->Content.OperationTimeOut = 1000;

			adapter->Content.Request = xRequestNew(&HostRequestControl);
			adapter->Content.Request->Base.EventListener = privateRequestEventListener;
			adapter->Content.Request->Base.Sender = (void*)&HostGAP;
			adapter->Content.Request->Action = GAPServiceRequestGetNumberOfServices;
			adapter->Content.Request->Recipient = (void*)&adapter->Content.GAP;

			xRequestControlAdd(&HostRequestControl, (void*)adapter->Content.Request);
			break;
		}

		case InitStateGetServices:
		{
			if (adapter->Content.TotalServiceNumber < adapter->Content.ServicesCount)
			{
				adapter->Content.OperationTimeStamp = time;
				adapter->Content.OperationTimeOut = 1000;

				adapter->Content.Request = xRequestNew(&HostRequestControl);
				adapter->Content.Request->Base.EventListener = privateRequestEventListener;
				adapter->Content.Request->Base.Sender = (void*)&HostGAP;
				adapter->Content.Request->Action = GAPServiceRequestGetService;
				adapter->Content.Request->Recipient = (void*)&adapter->Content.GAP;

				adapter->Content.Request->Data.Bytes[0] = adapter->Content.TotalServiceNumber;
				adapter->Content.Request->Base.DataSize = 1;

				xRequestControlAdd(&HostRequestControl, (void*)adapter->Content.Request);

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
	VirtualDeviceAdapterT* adapter = (VirtualDeviceAdapterT*)device->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xDeviceAdapterRequestDispose:
		{
			break;
		}

		case xDeviceAdapterRequestGetPort:
		{
			*(void**)arg = adapter->Port;
			break;
		}

		case xDeviceAdapterRequestGetTransferLayer:
		{
			*(void**)arg = adapter->TransferLayer;
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
		device->Adapter.Description = nameof(VirtualDeviceAdapterT);

		memset(&adapter->Content, 0, sizeof(adapter->Content));

		adapter->Port = init->Port;
		adapter->TransferLayer = init->TransferLayer;

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
