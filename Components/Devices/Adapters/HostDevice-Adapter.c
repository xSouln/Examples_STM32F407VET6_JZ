//==============================================================================
//includes:

#include <stdlib.h>

#include "HostDevice-Adapter.h"
#include "VirtualDevice-Adapter.h"
#include "CAN_Local/Control/CAN_Local-Types.h"
#include "Common/xMemory.h"
//==============================================================================
//defines:


//==============================================================================
//types:


//==============================================================================
//variables:


//==============================================================================
//functions:

static void privateRegisteredDivicesEvenListener(xDeviceT* device, int selector, void* arg)
{

}
//------------------------------------------------------------------------------
static void privateDHCPHandler(xDeviceT* device, HostDeviceAdapterT* adapter, CAN_LocalSegmentT* segment)
{
	switch (segment->DHCP_Header.PacketType)
	{
		case CAN_LocalBroadcastPacketTypeDHCPRequestGetId:
		{
			CAN_LocalRequestContentDHCPGetIdT reuqest = { .Value = segment->Data.Content };
			CAN_LocalCharacteristicDHCPGetIdT characteristic = { .Value = segment->DHCP_Header.Characteristic };
			uint16_t id = random() & 0x1FFF;

			xDeviceListElementT* element = xListStartEnumeration((void*)&device->Devices);
			uint8_t isContained = false;

			while (element)
			{
				if (element->Value->MAC == reuqest.MAC)
				{
					id = element->Value->Id;
					isContained = true;
					break;
				}

				element = element->Next;
			}

			xListStopEnumeration((void*)&device->Devices);

			//uint8_t isContained = xListEnumeration((void*)&device->Devices, privateFindDevice, reuqest) != NULL;

			if (!isContained)
			{
				xDeviceT* newDevice = xMemoryAllocate(1, sizeof(xDeviceT));
				VirtualDeviceAdapterT* newDeviceAdapter = xMemoryAllocate(1, sizeof(VirtualDeviceAdapterT));

				VirtualDeviceAdapterInitT adapterInit;
				adapterInit.Port = adapter->Port;
				adapterInit.TransferLayer = adapter->TransferLayer;
				VirtualDeviceAdapterInit(newDevice, newDeviceAdapter, &adapterInit);

				xDeviceInitT deviceInit;
				deviceInit.Parent = device;
				deviceInit.Id = id;
				deviceInit.EventListener = (void*)privateRegisteredDivicesEvenListener;

				newDevice->MAC = reuqest.MAC;
				newDevice->Info.Type = characteristic.Type;
				newDevice->Info.Extansion = characteristic.Extansion;
				newDevice->DynamicallyAllocated = true;
				newDevice->IsEnable = true;

				xDeviceInit(newDevice, &deviceInit);

				xDeviceAddDevice(device, newDevice);
			}

			CAN_LocalResponseContentDHCPGetIdT response;
			response.MAC = segment->Data.Content;

			CAN_LocalCharacteristicDHCPApplyIdT responseCharacteristic;
			responseCharacteristic.Id = id;

			CAN_LocalSegmentT request;
			request.DHCP_Header.MessageType = CAN_LocalMessageTypeBroadcast;
			request.DHCP_Header.PacketType = CAN_LocalBroadcastPacketTypeDHCPResponseGetId;
			request.DHCP_Header.ServiceType = xServiceTypeDHCP;
			request.DHCP_Header.Characteristic = responseCharacteristic.Value;
			request.DHCP_Header.IsEnabled = true;

			request.Data.Content = response.Value;
			request.DataLength = sizeof(response);

			xPortExtendedTransmition(adapter->Port, &request);

			break;
		}
	}
}
//------------------------------------------------------------------------------
static void PrivateHandler(xDeviceT* device)
{
	HostDeviceAdapterT* adapter = (HostDeviceAdapterT*)device->Adapter.Content;

	while (adapter->Content.RxPacketHandlerIndex != adapter->Content.PortRxCircleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(adapter->Content.PortRxCircleBuffer, adapter->Content.RxPacketHandlerIndex);

		if (segment->ExtensionIsEnabled)
		{
			if (segment->MessageType == CAN_LocalMessageTypeBroadcast && segment->ServiceType == xServiceTypeDHCP)
			{
				privateDHCPHandler(device, adapter, segment);
			}
		}

		adapter->Content.RxPacketHandlerIndex++;
		adapter->Content.RxPacketHandlerIndex &= adapter->Content.PortRxCircleBuffer->SizeMask;
	}
}
//------------------------------------------------------------------------------
static xResult PrivateRequestListener(xDeviceT* device, xDeviceAdapterRequestSelector selector, void* arg)
{
	HostDeviceAdapterT* adapter = (HostDeviceAdapterT*)device->Adapter.Content;

	switch ((uint32_t)selector)
	{
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
xResult HostDeviceAdapterInit(xDeviceT* device, HostDeviceAdapterT* adapter, HostDeviceAdapterInitT* init)
{
	if (device && init)
	{
		device->Adapter.Content = adapter;
		device->Adapter.Interface = &privateAdapterInterface;
		//device->Adapter.Description = nameof(HostDeviceAdapterT);

		memset(&adapter->Content, 0, sizeof(adapter->Content));

		adapter->Port = init->Port;
		adapter->TransferLayer = init->TransferLayer;

		adapter->Content.PortRxCircleBuffer = xPortGetRxCircleBuffer(adapter->Port);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
