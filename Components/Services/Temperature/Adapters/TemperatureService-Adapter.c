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

static uint32_t privateCount;
//==============================================================================
//functions:

static void privateHandler(TemperatureServiceT* service)
{
	TemperatureServiceAdapterT* adapter = service->Adapter.Content;

	uint32_t totalTime = xSystemGetTime(service);

	if (adapter->Internal.TimeStamp - totalTime > 500)
	{
		adapter->Internal.TimeStamp = totalTime;

		service->Temperature = 10.0f + (float)(rand() & 0x3fff) / 1000;
	}

	xCircleBufferT* circleBuffer = xPortGetRxCircleBuffer(adapter->Port);

	while (adapter->Internal.RxPacketHandlerIndex != circleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(circleBuffer, adapter->Internal.RxPacketHandlerIndex);

		if (segment->ExtensionIsEnabled && segment->ExtensionHeader.PacketType == CAN_LocalPacketTypeOpenTransaction)
		{
			volatile CAN_LocalPacketOpenTransactionRequestT request = { .Value = segment->Data.DoubleWord };
			xDeviceT* device = ((ObjectBaseT*)service)->Parent;

			if (request.DeviceId == device->Id && request.ServiceId == service->Base.Id)
			{
				privateCount++;

				CAN_LocalPacketOpenTransactionResponseT response;
				response.DeviceId = request.DeviceId;
				response.ServiceId = request.ServiceId;
				response.Action = request.Action;
				response.Token = request.Token;
				response.Result = 0;

				CAN_LocalSegmentT packet;
				packet.ExtensionHeader.MessageType = CAN_LocalMessageTypeResponse;
				packet.ExtensionHeader.PacketType = CAN_LocalPacketTypeApproveTransaction;
				packet.ExtensionHeader.DeviceType = 11;
				packet.ExtensionHeader.Address = 1;
				packet.ExtensionIsEnabled = true;

				packet.Data.DoubleWord = response.Value;
				packet.DataLength = sizeof(response);

				xPortExtendedTransmition(adapter->Port, &packet);
			}
		}

		adapter->Internal.RxPacketHandlerIndex++;
		adapter->Internal.RxPacketHandlerIndex &= circleBuffer->SizeMask;
	}
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
//------------------------------------------------------------------------------
static void privateEventListener(TemperatureServiceT* service, TemperatureServiceAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//==============================================================================
//initializations:

static TemperatureServiceAdapterInterfaceT privateInterface =
{
	.Handler = (TemperatureServiceAdapterHandlerT)privateHandler,

	.RequestListener = (TemperatureServiceAdapterRequestListenerT)privateRequestListener,
	.EventListener = (TemperatureServiceAdapterEventListenerT)privateEventListener,
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

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
