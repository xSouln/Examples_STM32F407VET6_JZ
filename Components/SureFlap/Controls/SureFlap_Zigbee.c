//==============================================================================
//includes:

#include "SureFlap_Zigbee.h"
#include "SureFlap.h"
//==============================================================================
//defines:

#define SUREFLAP_HUB                    0x7e
#define HUB_SUPPORTS_THALAMUS           0x02
#define HUB_DOES_NOT_SUPPORT_THALAMUS   0x01
#define HUB_IS_SOLE_PAN_COORDINATOR     0x00
//==============================================================================
//types:

//------------------------------------------------------------------------------
typedef enum
{
    DETACH_IDLE,
    DETACH_FIND_NEXT_PAIR,
    DETACH_SEND_DETACH_COMMAND,
    DETACH_WAIT_FOR_ACK,
    DETACH_FINISH,

} DETACH_STATE;
//==============================================================================
//variables:

//==============================================================================
//functions:

/**************************************************************
 * Function Name   : CRC16
 * Description     : Calculates 16bit CRC
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint16_t CRC16(uint8_t *ptr, uint32_t count, uint16_t initCRC)
{
    uint16_t crc;
    uint8_t i;
    uint32_t j;

    crc = initCRC;
    for (j=0; j<count; j++)
    {
      crc = crc ^ ((uint16_t) ptr[j] << 8);
      for(i = 0; i < 8; i++)
      {
        if( crc & 0x8000 )
        {
          crc = (crc << 1) ^ 0x1021;
        }
        else
        {
          crc = crc << 1;
        }
      }
    }
    return crc;
}
/**************************************************************
 * Function Name   : CRC32
 * Description     : Calculates 32bit CRC
 * Inputs          : ptr = pointer to data, count = number of bytes, initCRC is initial value of CRC
 * Outputs         :
 * Returns         :
 **************************************************************/
uint32_t CRC32(uint8_t *ptr, uint32_t count, uint32_t initCRC)
{
    uint32_t crc;
    uint8_t i;
    uint32_t j;

    crc = initCRC;
    for (j=0; j<count; j++)
    {
      crc = crc ^ ((uint32_t) ptr[j] << 24);
      for(i = 0; i < 8; i++)
      {
        if( crc & 0x80000000 )
        {
          crc = (crc << 1) ^ 0x04C11DB7;
        }
        else
        {
          crc = crc << 1;
        }
      }
    }
    return crc;
}
/**************************************************************
 * Function Name   : SureFlapZigbeeDecodeFrom_IEEE
 * Description     : Builds a SureFlap format frame for subsequent processing
 *                 : Source information is in rx_buffer which is a module level variable
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void SureFlapZigbeeDecodeFrom_IEEE(SureFlapZigbeeT *network, SureFlapZigbeeRxBufferT *rx_buffer)
{
    // received packet has two SureNet 'header bytes' at the start of the payload, which need to be
    // moved into the header. The first byte of the payload is the PACKET_TYPE and the 2nd is the sequence number.
    // Everything else for the header is included in other structure members of rx_buffer.
	uint16_t calculated_crc;

	// the -2 is because the first two bytes of
	// the payload will be moved to the header
	network->RxPacket.Packet.Header.PacketLength = rx_buffer->ucBufferLength + sizeof(SureFlapZigbeePacketHeaderT) - 2;

	network->RxPacket.Packet.Header.SequenceNumber = rx_buffer->ucRxBuffer[1]; // 2nd byte of payload is sequence number
	network->RxPacket.Packet.Header.PacketType = rx_buffer->ucRxBuffer[0]; // 1st byte of payload is packet type
	network->RxPacket.Packet.Header.SourceAddress = rx_buffer->uiSrcAddr;
	network->RxPacket.Packet.Header.DestinationAddress = rx_buffer->uiDstAddr;
	network->RxPacket.Packet.Header.Crc = 0; // will get overwritten
	network->RxPacket.Packet.Header.Rss = 0; // will get overwritten
	network->RxPacket.Packet.Header.Spare = 0;

	if (rx_buffer->ucBufferLength < 2)
	{
		//zprintf(CRITICAL_IMPORTANCE,"CORRUPTED RX BUFFER - HALTED RF STACK - NEED TO INVESTIGATE WHY\r\n");
		//DbgConsole_Flush();
		// if ucBufferLength is less than 2, then the memcpy() line below will try to copy
		// a negative number of bytes, which will translate to a huge positive number and so will
		// run out of memory!!
		while(1);
	}
    memcpy(&network->RxPacket.Packet.Payload, &rx_buffer->ucRxBuffer[2], rx_buffer->ucBufferLength - 2); // copy payload

    // calculate CRC on header & payload, probably not needed by SureNet
    calculated_crc = CRC16(network->RxPacket.Buffer, network->RxPacket.Packet.Header.PacketLength, 0xcccc);

    network->RxPacket.Packet.Header.Crc = calculated_crc;
    network->RxPacket.Packet.Header.Rss = rx_buffer->ucRSSI;
}
//------------------------------------------------------------------------------

xResult _SureFlapZigbeeTransmit(SureFlapZigbeeT* network,
		SureFlapZigbeePacketTypes type,
		SureFlapDeviceT* dest,
		uint8_t *payload_ptr,
		uint16_t length,
		int16_t seq,
		bool request_ack)
{
	SureFlapT* hub = network->Object.Parent;
	SureFlapZigbeeAdapterRequestTransmitT request;

	static struct
	{
		SureFlapZigbeePacketTypes Types;
		uint8_t Sequence;
		uint8_t Payload[SUREFLAP_ZIGBEE_MAX_PACKET_SIZE];

	} Packet;

	//max 802.15.4 packet size
	static uint8_t data[SUREFLAP_ZIGBEE_MAX_PACKET_SIZE];

	// use next number in sequence
	if(seq == -1)
	{
	  seq = network->TxSequenceNumber++;
	}

	if(length > sizeof(data) - 2)
	{
		return xResultError;
	}

	memcpy(Packet.Payload, payload_ptr, length);

	if(type == SUREFLAP_ZIGBEE_PACKET_DATA)
	{
		Packet.Types = SureFlapDeviceEncrypt(dest,
				Packet.Payload,
				length,
				Packet.Sequence);
	}
	else
	{
		Packet.Types = type;
	}

	request.AckEnable = request_ack;
	request.Data = data;
	request.Size = length + 2;
	request.MAC_Address = dest->Status.MAC_Address;

	SureFlapZigbeeAdapterTransmit(network, &request);

	return xResultAccept;
}
//------------------------------------------------------------------------------

xResult _SureFlapZigbeeStartNetwork(SureFlapZigbeeT* network)
{
	xResult result = xResultError;

	if (!network->IsStarted)
	{
		result = SureFlapZigbeeAdapterStartNetwork(network);

		network->IsStarted = result == xResultAccept;
	}

	return result;
}
//==============================================================================

void _SureFlapZigbeeHandler(SureFlapZigbeeT* network)
{
	SureFlapZigbeeAdapterHandler(network);
}
//------------------------------------------------------------------------------

void _SureFlapZigbeeTimeSynchronization(SureFlapZigbeeT* network)
{
	network->Adapter.Interface->TimeSynchronization(network);
}
//==============================================================================
//initialization:

xResult _SureFlapZigbeeInit(SureFlapT* hub)
{
	if(hub)
	{
		hub->Zigbee.Object.Description = nameof(SureFlapZigbeeT);
		hub->Zigbee.Object.Parent = hub;

		hub->Zigbee.PanId = SUREFLAP_ZIGBEE_PAN_ID;
		hub->Zigbee.CurrentChannel = SUREFLAP_ZIGBEE_INITIAL_CHANNEL;

		hub->Zigbee.BeaconPayload[0] = SUREFLAP_HUB;
		hub->Zigbee.BeaconPayload[1] = HUB_SUPPORTS_THALAMUS;
		hub->Zigbee.BeaconPayload[2] = HUB_IS_SOLE_PAN_COORDINATOR;

		for(uint8_t i = 0; i < sizeof(hub->Zigbee.TxPowerPerChannel); i++)
		{
			hub->Zigbee.TxPowerPerChannel[i] = SUREFLAP_ZIGBEE_DEFAULT_TRANSMITTER_POWER;
		}

		hub->Zigbee.TxPowerPerChannel[SUREFLAP_ZIGBEE_CHANNELS_COUNT - 1] = -8;

		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
