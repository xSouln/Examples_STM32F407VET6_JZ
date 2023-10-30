//==============================================================================
//includes:

#include <stdlib.h>
#include "Common/xCircleBuffer.h"
#include "DeviceService-Adapter.h"
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

static void privateOpenTransferHandler(DeviceServiceT* service,
		DeviceServiceAdapterT* adapter,
		volatile CAN_LocalSegmentT* segment)
{
	CAN_LocalPacketOpenTransferRequestT request = { .Value = segment->Data.DoubleWord };

	if (request.ServiceId == service->Base.Id)
	{
		CAN_LocalPacketOpenTransferResponseT response;
		response.ServiceId = segment->ExtensionHeader.ServiceId;
		response.Action = request.Action;
		response.Token = request.Token;
		response.Result = 0;

		CAN_LocalSegmentT packet;
		packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeTransfer;
		packet.ExtensionHeader.PacketType = CAN_LocalTransferPacketTypeApproveTransfer;
		packet.ExtensionHeader.ServiceType = service->Base.Info.Type;
		packet.ExtensionHeader.ServiceId = service->Base.Id;
		packet.ExtensionIsEnabled = true;

		packet.Data.DoubleWord = response.Value;
		packet.DataLength = sizeof(response);

		xPortExtendedTransmition(adapter->Port, &packet);
	}
}
//------------------------------------------------------------------------------
static void privateNotificationHandler(DeviceServiceT* service,
		DeviceServiceAdapterT* adapter,
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
static void privateHandler(DeviceServiceT* service)
{
	DeviceServiceAdapterT* adapter = service->Adapter.Content;

	xCircleBufferT* circleBuffer = xPortGetRxCircleBuffer(adapter->Port);

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
						case CAN_LocalTransferPacketTypeOpenTransfer:
							privateOpenTransferHandler(service, adapter, segment);
							break;
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
static xResult privateRequestListener(DeviceServiceT* service, int selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//==============================================================================
//initializations:

static DeviceServiceAdapterInterfaceT privateInterface =
{
	.Handler = (DeviceServiceAdapterHandlerT)privateHandler,
	.RequestListener = (DeviceServiceAdapterRequestListenerT)privateRequestListener
};
//------------------------------------------------------------------------------
xResult DeviceServiceAdapterInit(DeviceServiceT* service,
		DeviceServiceAdapterT* adapter,
		DeviceServiceAdapterInitT* init)
{
	if (service && init)
	{
		service->Adapter.Content = adapter;
		service->Adapter.Interface = &privateInterface;
		service->Adapter.Description = nameof(DeviceServiceAdapterT);

		adapter->Port = init->Port;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
