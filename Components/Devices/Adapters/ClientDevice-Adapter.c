//==============================================================================
//includes:

#include "Abstractions/xSystem/xSystem.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
#include "ClientDevice-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void PrivateHandler(xDeviceT* device)
{
	ClientDeviceAdapterT* adapter = (ClientDeviceAdapterT*)device->Adapter.Content;

	if (adapter->Content.Command)
	{
		adapter->Content.CommandExecutionResult = adapter->Content.Command->Action(device, adapter->Content.Command->Arg);
		adapter->Content.Command = 0;

		xSemaphoreGive(adapter->Content.CommandAccomplishSemaphore);
	}

	while (adapter->Content.RxPacketHandlerIndex != adapter->Content.PortRxCircleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(adapter->Content.PortRxCircleBuffer, adapter->Content.RxPacketHandlerIndex);

		if (segment->Identifier == CAN_LocalPacketIdentifierDeviceApplyId)
		{
			CAN_LocalPacketDeviceApplyIdT content = { .Value = segment->Data.DoubleWord };
			CAN_LocalExtansionDeviceApplyIdT extansion = { .Value = segment->Extansion };

			if (content.MAC == device->MAC)
			{
				device->Id = extansion.Id;
				device->ConnectionState = xDeviceConnectionStateRegistration;
			}
		}

		adapter->Content.RxPacketHandlerIndex++;
		adapter->Content.RxPacketHandlerIndex &= adapter->Content.PortRxCircleBuffer->SizeMask;
	}

	uint32_t time = xSystemGetTime(NULL);
	if (device->ConnectionState == xDeviceConnectionStateDisconnected
		&& (time - adapter->Content.OperationTimeStamp) > 3000)
	{
		adapter->Content.OperationTimeStamp = time;

		CAN_LocalExtansionNewDeviceT extansion;
		extansion.Type = device->Info.Type;
		extansion.Extansion = device->Info.Extansion;

		CAN_LocalPacketNewDeviceT data;
		data.MAC = device->MAC;

		CAN_LocalSegmentT segment;
		segment.Identifier = CAN_LocalPacketIdentifierNewDevice;
		segment.Extansion = extansion.Value;
		segment.ExtansionIsEnabled = true;
		segment.Data.DoubleWord = data.Value;
		segment.DataLength = sizeof(CAN_LocalPacketNewDeviceT);

		xPortExtendedTransmition(adapter->Port, &segment);
	}

	if (device->ConnectionState == xDeviceConnectionStateRegistration
		&& (time - adapter->Content.OperationTimeStamp) > 1000)
	{
		adapter->Content.OperationTimeStamp = time;


	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xDeviceT* device, xDeviceAdapterRequestSelector selector, void* arg)
{
	ClientDeviceAdapterT* adapter = (ClientDeviceAdapterT*)device->Adapter.Content;

	switch ((uint32_t)selector)
	{
		case xDeviceAdapterRequestExecuteCommand:
		{
			xSemaphoreTake(adapter->Content.CommandExecutionMutex, portMAX_DELAY);

			adapter->Content.Command = arg;

#ifdef INC_FREERTOS_H
			xSemaphoreTake(adapter->Content.CommandAccomplishSemaphore, portMAX_DELAY);
#endif
			xResult commandResult = adapter->Content.CommandExecutionResult;
			xSemaphoreGive(adapter->Content.CommandExecutionMutex);

			return commandResult;
		}

		default : return xResultRequestIsNotFound;
	}

	return xResultAccept;
}
//------------------------------------------------------------------------------
static void PrivateEventListener(xDeviceT* device, xDeviceAdapterEventSelector selector, void* arg)
{
	//register UsartPortAdapterT* adapter = (UsartPortAdapterT*)port->Adapter;

	switch((int)selector)
	{
		default: return;
	}
}
//==============================================================================
//initializations:

static xDeviceAdapterInterfaceT privateAdapterInterface =
{
	.Handler = (xDeviceAdapterHandlerT)PrivateHandler,

	.RequestListener = (xDeviceAdapterRequestListenerT)PrivateRequestListener,
	.EventListener = (xDeviceAdapterEventListenerT)PrivateEventListener,
};
//------------------------------------------------------------------------------
xResult ClientDeviceAdapterInit(xDeviceT* device, ClientDeviceAdapterT* adapter, ClientDeviceAdapterInitT* init)
{
	if (device && init)
	{
		device->Adapter.Content = adapter;
		device->Adapter.Interface = &privateAdapterInterface;
		device->Adapter.Description = nameof(ClientDeviceAdapterT);

#ifdef INC_FREERTOS_H
		adapter->Content.CommandExecutionMutex = xSemaphoreCreateMutex();
		adapter->Content.CommandAccomplishSemaphore = xSemaphoreCreateBinary();
#endif

		adapter->Port = init->Port;
		adapter->Content.PortRxCircleBuffer = xPortGetRxCircleBuffer(adapter->Port);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
