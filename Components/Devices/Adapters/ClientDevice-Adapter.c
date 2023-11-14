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

	while (adapter->Content.RxPacketHandlerIndex != adapter->Content.PortRxCircleBuffer->TotalIndex)
	{
		CAN_LocalSegmentT* segment = xCircleBufferGetElement(adapter->Content.PortRxCircleBuffer, adapter->Content.RxPacketHandlerIndex);

		if (segment->DHCP_Header.IsEnabled && segment->DHCP_Header.PacketType == CAN_LocalBroadcastPacketTypeDHCPResponseGetId)
		{
			CAN_LocalResponseContentDHCPGetIdT content = { .Value = segment->Data.Content };
			CAN_LocalCharacteristicDHCPApplyIdT characteristic = { .Value = segment->DHCP_Header.Characteristic };

			if (content.MAC == device->MAC)
			{
				device->NetworkState = xDeviceNetworkStateRegistered;
				xDeviceSetId(device, characteristic.Id);
			}
		}

		adapter->Content.RxPacketHandlerIndex++;
		adapter->Content.RxPacketHandlerIndex &= adapter->Content.PortRxCircleBuffer->SizeMask;
	}

	uint32_t time = xSystemGetTime(NULL);
	if (device->NetworkState == xDeviceNetworkStateUnregistered
		&& (time - adapter->Content.OperationTimeStamp) > 3000)
	{
		adapter->Content.OperationTimeStamp = time;

		CAN_LocalRequestContentDHCPGetIdT content;
		content.MAC = device->MAC;

		CAN_LocalCharacteristicDHCPGetIdT characteristic;
		characteristic.Type = device->Info.Type;
		characteristic.Extansion = device->Info.Extansion;

		CAN_LocalSegmentT segment;
		segment.DHCP_Header.MessageType = CAN_LocalMessageTypeBroadcast;
		segment.DHCP_Header.PacketType = CAN_LocalBroadcastPacketTypeDHCPRequestGetId;
		segment.DHCP_Header.ServiceType = xServiceTypeDHCP;
		segment.DHCP_Header.IsEnabled = true;
		segment.DHCP_Header.Characteristic = characteristic.Value;

		segment.Data.Value = content.Value;
		segment.DataLength = sizeof(CAN_LocalRequestContentDHCPGetIdT);

		xPortExtendedTransmition(adapter->Port, &segment);
	}

	if (device->NetworkState == xDeviceNetworkStateRegistered
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
		/*case xDeviceAdapterRequestExecuteCommand:
		{
			xSemaphoreTake(adapter->Content.CommandExecutionMutex, portMAX_DELAY);

			adapter->Content.Command = arg;

#ifdef INC_FREERTOS_H
			xSemaphoreTake(adapter->Content.CommandAccomplishSemaphore, portMAX_DELAY);
#endif
			xResult commandResult = adapter->Content.CommandExecutionResult;
			xSemaphoreGive(adapter->Content.CommandExecutionMutex);

			return commandResult;
		}*/
		case xDeviceAdapterRequestDispose:
		{
			break;
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
		//device->Adapter.Description = nameof(ClientDeviceAdapterT);

		memset(&adapter->Content, 0, sizeof(adapter->Content));

		adapter->Port = init->Port;
		adapter->TransferLayer = init->TransferLayer;

		adapter->Content.PortRxCircleBuffer = xPortGetRxCircleBuffer(adapter->Port);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
