//==============================================================================
//includes:

#include <stdlib.h>
#include "Common/xCircleBuffer.h"
#include "GAPService-Adapter.h"
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

static void privateTransferEventListener(xTransferLayerT* layer, xTransferT* transfer, int selector, void* arg)
{
	transfer->State = xTransferStateIdle;


}
//------------------------------------------------------------------------------
static void privateOpenTransferHandler(GAPServiceT* service,
		GAPServiceAdapterT* adapter,
		volatile CAN_LocalSegmentT* segment)
{
	CAN_LocalRequestContentOpenTransferT request = { .Value = segment->Data.DoubleWord };

	if (request.ServiceId == service->Base.Id)
	{
		CAN_LocalResponseContentOpenTransferT response;
		response.ServiceId = segment->ExtensionHeader.ServiceId;
		response.Action = request.Action;
		response.Token = -1;
		response.Result = xResultError;

		xTransferT* transfer = xTransferLayerNewTransfer(&ExternalTransferLayer);
		CAN_LocalTransferT* extansion = (void*)transfer;

		if (transfer == NULL)
		{
			goto error;
		}

		transfer->Holder = service;
		transfer->Id = segment->TransferHeader.ServiceId;
		transfer->EventListener = privateTransferEventListener;
		transfer->Type = request.Type == xTransferTypeReceive ? xTransferTypeTransmite : xTransferTypeReceive;
		transfer->ValidationIsEnabled = request.ValidationIsEnabled;
		transfer->MasterModeIsEnabled = false;
		extansion->Action = request.Action;

		response.Token = random();
		response.Result = xResultAccept;

		error:;
		transfer->State = xTransferStateIdle;
		transfer = NULL;

		CAN_LocalSegmentT packet;
		packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeTransfer;
		packet.ExtensionHeader.PacketType = CAN_LocalTransferPacketTypeApproveTransfer;
		packet.ExtensionHeader.ServiceType = service->Base.Info.Type;
		packet.ExtensionHeader.ServiceId = service->Base.Id;
		packet.ExtensionIsEnabled = true;

		packet.Data.Value = response.Value;
		packet.DataLength = sizeof(response);

		if (transfer)
		{
			xTransferLayerAdd(&ExternalTransferLayer, transfer);
		}

		xPortExtendedTransmition(adapter->Port, &packet);
	}
}
//------------------------------------------------------------------------------
static void privateRequestHandler(GAPServiceT* service,
		GAPServiceAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalRequestContentT content = { .Value = segment->Data.Value };
	xDeviceT* device = xServiceGetDevice(service);

	if (content.Description.Recipient == service->Base.Id)
	{
		CAN_LocalResponseContentT response;
		response.Description.Sequence = content.Description.Sequence;
		response.Description.Sender = segment->ExtensionHeader.ServiceId;
		response.Description.Action = content.Description.Action;

		CAN_LocalSegmentT segment = { 0 };
		segment.ExtensionHeader.IsEnabled = true;
		segment.ExtensionHeader.MessageType = CAN_LocalMessageTypeResponse;
		segment.ExtensionHeader.PacketType = CAN_LocalRequestPacketTypeCommon;
		segment.ExtensionHeader.ServiceId = service->Base.Id;
		segment.ExtensionHeader.ServiceType = service->Base.Info.Type;

		switch (content.Description.Action)
		{
			case GAPServiceRequestGetNumberOfServices:
			{
				response.Data.Bytes[0] = device->Services.Count;
				segment.DataLength = sizeof(CAN_LocalResponseContentT) - sizeof(response.Data) + 1;

				break;
			}

			case GAPServiceRequestGetService:
			{
				volatile CAN_LocalRequestGATGetServiceT requestContent = { .Value = content.Data.Content };

				xServiceT* service = xListGetObjectByIndex((void*)&device->Services, requestContent.Id);

				CAN_LocalResponseGATGetServiceT responseContent;
				responseContent.Id = service->Id;
				responseContent.Type = service->Info.Type;
				responseContent.Extension = service->Info.Extension;

				response.Data.Content = responseContent.Value;
				segment.DataLength = sizeof(CAN_LocalResponseContentT) - sizeof(response.Data) + sizeof(CAN_LocalResponseGATGetServiceT);

				break;
			}

			default: return;
		}

		segment.Data.Content = response.Value;
		xPortExtendedTransmition(adapter->Port, &segment);
	}
}
//------------------------------------------------------------------------------
static void privateNotificationHandler(GAPServiceT* service,
		GAPServiceAdapterT* adapter,
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
	GAPServiceAdapterT* adapter = service->Adapter.Content;

	xCircleBufferT* circleBuffer = xPortGetRxCircleBuffer(adapter->Port);

	while (adapter->Internal.RxPacketHandlerIndex != circleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(circleBuffer, adapter->Internal.RxPacketHandlerIndex);

		if (segment->ExtensionIsEnabled)
		{
			if (segment->ExtensionHeader.MessageType == CAN_LocalMessageTypeTransfer)
			{
				switch((uint8_t)segment->ExtensionHeader.PacketType)
				{
					case CAN_LocalTransferPacketTypeOpenTransfer:
						privateOpenTransferHandler(service, adapter, segment);
						break;
				}
			}
			else if (segment->ExtensionHeader.MessageType == CAN_LocalMessageTypeRequest)
			{
				privateRequestHandler(service, adapter, segment);
			}

			/*switch((uint8_t)segment->ExtensionHeader.MessageType)
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

				case 3:
				{
					privateRequestHandler(service, adapter, segment);
					break;
				}
			}*/
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
xResult GAPServiceAdapterInit(GAPServiceT* service,
		GAPServiceAdapterT* adapter,
		GAPServiceAdapterInitT* init)
{
	if (service && init)
	{
		service->Adapter.Content = adapter;
		service->Adapter.Interface = &privateInterface;
		service->Adapter.Description = nameof(GAPServiceAdapter
				T);

		adapter->Port = init->Port;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
