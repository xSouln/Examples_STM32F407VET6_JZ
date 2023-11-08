//==============================================================================
//includes:

#include <stdlib.h>
#include "Common/xCircleBuffer.h"
#include "VirtualGAPService-Adapter.h"
#include "Abstractions/xSystem/xSystem.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
#include "TransferLayer/TransferLayer-Component.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void privateNotificationHandler(GAPServiceT* service,
		VirtualGAPServiceAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalBaseEventPacketT content = { .Value = segment->Data.DoubleWord };

	if (segment->Header.ServiceType == service->Base.Info.Type && content.Id == service->Base.Id)
	{
		xServiceSubscriberListElementT* element = service->Base.Subscribers.Head;

		while (element)
		{
			xServiceSubscriberT* subscriber = element->Value;

			if (subscriber->EventListener)
			{
				subscriber->EventListener((void*)service, subscriber, 0, &content.Content);
			}

			element = element->Next;
		}
	}
}
//------------------------------------------------------------------------------
static void privateHandler(GAPServiceT* service)
{
	VirtualGAPServiceAdapterT* adapter = service->Adapter.Content;
	xPortT* port = xServiceGetPort((void*)service);

	xCircleBufferT* circleBuffer = xPortGetRxCircleBuffer(port);

	while (adapter->Internal.RxPacketHandlerIndex != circleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(circleBuffer, adapter->Internal.RxPacketHandlerIndex);

		if (segment->ExtensionIsEnabled)
		{
			switch((uint8_t)segment->ExtensionHeader.MessageType)
			{
				case CAN_LocalMessageTypeTransfer:
				{
					switch((uint8_t)segment->ExtensionHeader.PacketType)
					{
						/*case CAN_LocalTransferPacketTypeOpenTransfer:
							privateOpenTransferHandler(service, adapter, segment);
							break;*/
					}
					break;
				}
			}
		}
		else
		{
			switch((uint8_t)segment->Header.MessageType)
			{
				case CAN_LocalMessageTypeNotification:
				{
					privateNotificationHandler(service, adapter, segment);
					break;
				}
			}
		}

		adapter->Internal.RxPacketHandlerIndex++;
		adapter->Internal.RxPacketHandlerIndex &= circleBuffer->SizeMask;
	}
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(GAPServiceT* service, int selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//==============================================================================
//initializations:

static GAPServiceAdapterInterfaceT privateInterface =
{
	.Handler = (GAPServiceAdapterHandlerT)privateHandler,
	.RequestListener = (GAPServiceAdapterRequestListenerT)privateRequestListener
};
//------------------------------------------------------------------------------
xResult VirtualGAPServiceAdapterInit(GAPServiceT* service,
		VirtualGAPServiceAdapterT* adapter,
		VirtualGAPServiceAdapterInitT* init)
{
	if (service && init)
	{
		service->Adapter.Content = adapter;
		service->Adapter.Interface = &privateInterface;
		service->Adapter.Description = nameof(VirtualGAPServiceAdapterT);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
