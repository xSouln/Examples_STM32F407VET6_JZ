//==============================================================================
//includes:

#include "SureFlap_Zigbee.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

//------------------------------------------------------------------------------
void SureFlapZigbeePacketReceiver(SureFlapZigbeeT* network, SureFlapZigbeeRxBufferT *rx_buffer)
{
	SureFlapT* hub = network->Object.Parent;
	uint16_t packet_length;

	SureFlapZigbeeDecodeFrom_IEEE(network, rx_buffer);

	packet_length = network->RxPacket.Packet.Header.PacketLength;

	SureFlapDeviceT* device = SureFlapDeviceGetFrom_MAC(&hub->DeviceControl,
			network->RxPacket.Packet.Header.SourceAddress);

	if (!device)
	{
		return;
	}

	if (!device->Status.State.Online)
	{
		device->Status.State.Online = true;
	}


}
//==============================================================================
