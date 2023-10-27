//==============================================================================
//includes:

#include <stdlib.h>

#include "LocalDevice-Adapter.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
#include "ServerDevice-Adapter.h"
#include "Common/xMemory.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static int privateConnectionHandler(xDeviceT* device, CAN_LocalSegmentT* segment)
{
	LocalDeviceAdapterT* adapter = (LocalDeviceAdapterT*)device->Adapter.Content;

	if (segment->Identifier == CAN_LocalPacketIdentifierNewDevice)
	{
		CAN_LocalPacketNewDeviceT packetContent = { .Value = segment->Data.DoubleWord };
		uint16_t id = random();

		xDeviceListElementT* element = xListStartEnumeration((void*)&device->Devices);
		uint8_t isContained = false;

		while (element)
		{
			if (element->Value->MAC == packetContent.MAC)
			{
				id = element->Value->Id;
				isContained = true;
				break;
			}

			element = element->Next;
		}

		xListStopEnumeration((void*)&device->Devices);

		if (!isContained)
		{
			xDeviceT* newDevice = xMemoryAllocate(1, sizeof(xDeviceT));
			ServerDeviceAdapterT* adapter = xMemoryAllocate(1, sizeof(ServerDeviceAdapterT));

			ServerDeviceAdapterInitT adapterInit;
			adapterInit.Port = adapter->Port;
			ServerDeviceAdapterInit(newDevice, adapter, &adapterInit);

			xDeviceInitT deviceInit;
			deviceInit.Parent = device;
			deviceInit.Id = id;
			deviceInit.EventListener = device->EventListener;

			newDevice->MAC = packetContent.MAC;
			xDeviceInit(newDevice, &deviceInit);

			xDeviceAddDevice(device, newDevice);
		}

		CAN_LocalPacketDeviceApplyIdT content;
		content.MAC = segment->Data.DoubleWord;

		CAN_LocalExtansionDeviceApplyIdT extansion;
		extansion.Id = id;

		CAN_LocalSegmentT request;
		request.Identifier = CAN_LocalPacketIdentifierDeviceApplyId;
		request.Extension = extansion.Value;
		request.ExtensionIsEnabled = true;
		request.Data.DoubleWord = content.Value;
		request.DataLength = sizeof(CAN_LocalPacketDeviceApplyIdT);

		xPortExtendedTransmition(adapter->Port, &request);

		return 1;
	}

	return 0;
}
//------------------------------------------------------------------------------
static void PrivateHandler(xDeviceT* device)
{
	LocalDeviceAdapterT* adapter = (LocalDeviceAdapterT*)device->Adapter.Content;

	if (adapter->Content.Command)
	{
		adapter->Content.CommandExecutionResult = adapter->Content.Command->Action(device, adapter->Content.Command->Arg);
		adapter->Content.Command = 0;
		xSemaphoreGive(adapter->Content.CommandAccomplishSemaphore);
	}

	while (adapter->Content.RxPacketHandlerIndex != adapter->Content.PortRxCircleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(adapter->Content.PortRxCircleBuffer, adapter->Content.RxPacketHandlerIndex);

		privateConnectionHandler(device, segment);

		adapter->Content.RxPacketHandlerIndex++;
		adapter->Content.RxPacketHandlerIndex &= adapter->Content.PortRxCircleBuffer->SizeMask;
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xDeviceT* device, xDeviceAdapterRequestSelector selector, void* arg)
{
	LocalDeviceAdapterT* adapter = (LocalDeviceAdapterT*)device->Adapter.Content;

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
xResult LocalDeviceAdapterInit(xDeviceT* device, LocalDeviceAdapterT* adapter, LocalDeviceAdapterInitT* init)
{
	if (device && init)
	{
		device->Adapter.Content = adapter;
		device->Adapter.Interface = &privateAdapterInterface;
		device->Adapter.Description = nameof(LocalDeviceAdapterT);

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
