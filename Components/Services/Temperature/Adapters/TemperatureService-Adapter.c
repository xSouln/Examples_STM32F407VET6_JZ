//==============================================================================
//includes:

#include <stdlib.h>
#include "Abstractions/xSystem/xSystem.h"
#include "TemperatureService-Adapter.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
#include "TransferLayer/TransferLayer-Component.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:

extern xTransferLayerT ExternalTransferLayer;
static uint32_t privateCount;

static xTransferT* privateSlaveTransfer;
//==============================================================================
//functions:

static void privateTransferEventListener(xTransferLayerT* layer, xTransferT* transfer, int selector, void* arg)
{
	transfer->State = xTransferStateIdle;

	/*if (memcmp(transferTxData, transferRxData, sizeof(transferRxData)) != 0)
	{
		while (true)
		{

		}
	}*/
}
//------------------------------------------------------------------------------
static void privateOpenTransferHandler(TemperatureServiceT* service,
		TemperatureServiceAdapterT* adapter,
		CAN_LocalSegmentT* segment)
{
	CAN_LocalRequestContentOpenTransferT request = { .Value = segment->Data.Value };

	if (request.ServiceId == service->Base.Id)
	{
		privateCount++;

		CAN_LocalResponseContentOpenTransferT response;
		response.ServiceId = segment->TransferHeader.ServiceId;
		response.Action = request.Action;
		response.Result = xResultError;
		response.Token = -1;

		privateSlaveTransfer = xTransferLayerNewTransfer(&ExternalTransferLayer);

		if (privateSlaveTransfer)
		{
			response.Result = xResultAccept;
			response.Token = random();

			privateSlaveTransfer->Holder = service;
			privateSlaveTransfer->Id = segment->TransferHeader.ServiceId;
			privateSlaveTransfer->Type = request.Type == xTransferTypeReceive ? xTransferTypeTransmite : xTransferTypeReceive;
			privateSlaveTransfer->ValidationIsEnabled = request.ValidationIsEnabled;
			privateSlaveTransfer->MasterModeIsEnabled = false;

			if (privateSlaveTransfer->Type == xTransferTypeReceive)
			{
				memset(transferRxData, 0, sizeof(transferRxData));
				privateSlaveTransfer->Data = transferRxData;
				privateSlaveTransfer->DataLength = sizeof(transferRxData);
			}
			else if (privateSlaveTransfer->Type == xTransferTypeTransmite)
			{
				privateSlaveTransfer->Data = (uint8_t*)transferTxData;
				privateSlaveTransfer->DataLength = sizeof_str(transferTxData);
			}

			privateSlaveTransfer->Token = response.Token;
			privateSlaveTransfer->TimeOut = 1000;
			privateSlaveTransfer->TransmittingAttempts = 1;

			privateSlaveTransfer->EventListener = privateTransferEventListener;

			memset(transferRxData, 0, sizeof(transferRxData));

			xTransferLayerAdd(&ExternalTransferLayer, privateSlaveTransfer);
		}

		CAN_LocalSegmentT packet;
		packet.TransferHeader.MessageType = CAN_LocalMessageTypeTransfer;
		packet.TransferHeader.PacketType = CAN_LocalTransferPacketTypeApproveTransfer;
		packet.TransferHeader.Characteristic = service->Base.Info.Type;
		packet.TransferHeader.ServiceId = service->Base.Id;
		packet.TransferHeader.IsEnabled = true;

		packet.Data.Value = response.Value;
		packet.DataLength = sizeof(response);

		xPortExtendedTransmition(adapter->Port, &packet);
	}
}
//------------------------------------------------------------------------------
static void privateNotificationHandler(TemperatureServiceT* service,
		TemperatureServiceAdapterT* adapter,
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
static void privateReceiver(TemperatureServiceT* service, TemperatureServiceAdapterT* adapter)
{
	xCircleBufferT* circleBuffer = xPortGetRxCircleBuffer(adapter->Port);

	while (adapter->Internal.RxPacketHandlerIndex != circleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(circleBuffer, adapter->Internal.RxPacketHandlerIndex);

		if (segment->ExtensionIsEnabled)
		{
			switch((uint8_t)segment->MessageType)
			{
				case CAN_LocalMessageTypeTransfer:
				{
					switch((uint8_t)segment->TransferHeader.PacketType)
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
static void privateHandler(TemperatureServiceT* service)
{
	TemperatureServiceAdapterT* adapter = service->Adapter.Content;

	uint32_t totalTime = xSystemGetTime(NULL);

	if (totalTime - adapter->Internal.TimeStamp > 500)
	{
		adapter->Internal.TimeStamp = totalTime;

		service->Temperature = 10.0f + (float)(rand() & 0x3fff) / 1000;

		CAN_LocalTemperatureNotificationUpdateTemperatureT content;
		content.Temperature = service->Temperature * 1000;
		content.TimeStamp = totalTime;

		CAN_LocalSegmentT segment;
		segment.ExtensionHeader.IsEnabled = true;
		segment.ExtensionHeader.MessageType = CAN_LocalMessageTypeNotification;
		segment.ExtensionHeader.PacketType = CAN_LocalTemperatureNotificationUpdateTemperature;
		segment.ExtensionHeader.ServiceId = service->Base.Id;
		segment.ExtensionHeader.ServiceType = service->Base.Info.Type;

		memcpy(segment.Data.Bytes, content.Data, sizeof(CAN_LocalTemperatureNotificationUpdateTemperatureT));
		segment.DataLength = sizeof(CAN_LocalTemperatureNotificationUpdateTemperatureT);

		xPortExtendedTransmition(adapter->Port, &segment);
	}

	privateReceiver(service, adapter);
}
//------------------------------------------------------------------------------
static xResult privateRequestListener(TemperatureServiceT* service, int selector, void* arg)
{
	switch ((uint32_t)selector)
	{

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//==============================================================================
//initializations:

static TemperatureServiceAdapterInterfaceT privateInterface =
{
	.Handler = (TemperatureServiceAdapterHandlerT)privateHandler,
	.RequestListener = (TemperatureServiceAdapterRequestListenerT)privateRequestListener
};
//------------------------------------------------------------------------------
xResult TemperatureServiceAdapterInit(TemperatureServiceT* service,
		TemperatureServiceAdapterT* adapter,
		TemperatureServiceAdapterInitT* init)
{
	if (service && init)
	{
		service->Adapter.Content = adapter;
		service->Adapter.Interface = &privateInterface;
		service->Adapter.Description = nameof(TemperatureServiceAdapterT);

		adapter->Port = init->Port;

		service->Base.IsEnable = true;
		service->Base.IsAvailable = true;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
