//==============================================================================
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap_Zigbee.h"
#include "SureFlap.h"
#include "SureFlap_ZigbeePackets.h"
#include <stddef.h>
#include "Components.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

static bool SureFlapZigbeePacketIsNew(SureFlapZigbeeT* network, SureFlapDeviceT* device)
{
	/*
	for (uint8_t i = 0; i < SUREFLAP_ZIGBEE_RX_SEQUENCE_BUFFER_SIZE; i++)
	{
		if (device->RxSequences[i].Id == network->RxPacket.Header.SequenceNumber
			&& !device->RxSequences[i].TimeOut)
		{
			device->RxSequences[i].TimeOut = 100;
		}
	}
*/
	return false;
}
//------------------------------------------------------------------------------

static void SureFlapZigbeeReceiveDeviceStatus(SureFlapZigbeeT* network, SureFlapDeviceT* device)
{
	network->AckPayload.Sequence = network->RxPacket.Header.SequenceNumber;
	network->AckPayload.Value = 0;

	if(network->RxPacket.Payload[0] != device->Status.DeviceType)
	{
		//surenet_printf("Error, mismatched device type\r\n");
		network->AckPayload.Value = 0xFF; // send a NACK back
	}
	//device wants security key to be sent
	else if(network->RxPacket.Payload[1] == SUREFLAP_DEVICE_STATUS_SEND_KEY)
	{
		device->StatusExtra.SendSecurityKey = SUREFLAP_DEVICE_SECURITY_KEY_RENEW;
	}
	//currently just send an ACK/NACK back but could add other initialisation info if required
	//although not secure at this stage so better to send as a PACKET_DATA later
	//surenet_printf("Sending PACKET_DEVICE_CONFIRM\r\n");

	SureFlapZigbeeTransmit(network,
			SUREFLAP_ZIGBEE_PACKET_DEVICE_CONFIRM,
			device,
			network->AckPayload.Data,
			2,
			-1,
			false);
}
//------------------------------------------------------------------------------

static void SureFlapZigbeeReceiveDataAck(SureFlapZigbeeT* network, SureFlapDeviceT* device)
{
	/*
	uint8_t i = 0;

	// find the first free space in the queue
	while ((i < SUREFLAP_ZIGBEE_DATA_ACKNOWLEDGE_QUEUE_SIZE)
			&& (network->DataAcknowledgeQueue[i].TimeOut == 0))
	{
		network->DataAcknowledgeQueue[i].TimeOut = SUREFLAP_ZIGBEE_DATA_ACKNOWLEDGE_LIFETIME;
		network->DataAcknowledgeQueue[i].SequenceAcknowledged = network->RxPacket.Payload[0];
		network->DataAcknowledgeQueue[i].AckNack = network->RxPacket.Payload[1];

		i++;
		return;
	}
*/
	//surenet_printf("Data ack queue full, dropping data ack\r\n");
}
//------------------------------------------------------------------------------

static void SureFlapZigbeeReceiveAck(SureFlapZigbeeT* network, SureFlapDeviceT* device)
{
	/*
	uint8_t i = 0;

	// find the first free space in the queue
	while ((i < SUREFLAP_ZIGBEE_ACKNOWLEDGE_QUEUE_SIZE)
			&& (network->AcknowledgeQueue[i].TimeOut == 0))
	{
		network->AcknowledgeQueue[i].TimeOut = SUREFLAP_ZIGBEE_ACKNOWLEDGE_LIFETIME;
		network->AcknowledgeQueue[i].SequenceAcknowledged = network->RxPacket.Payload[0];
		network->AcknowledgeQueue[i].AckNack = network->RxPacket.Payload[1];
		//surenet_ack_printf("(1449) Packet ack %x %x\r\n", ack_payload[0], ack_payload[1]);

		i++;
		return;
	}
	*/

	xTxTransmitString(Terminal.Tx, "SureFlapZigbeeReceiveAck");
	xTxTransmitString(Terminal.Tx, "\r");
}
//------------------------------------------------------------------------------

static void SureFlapZigbeeReceiveData(SureFlapZigbeeT* network, SureFlapDeviceT* device)
{
	//exclude test packets
	if (!network->RxPacket.Header.Spare)
	{
		SureFlapDeviceDecrypt(device,
				network->RxPacket.Payload,
				network->RxPacket.Header.PacketLength,
				network->RxPacket.Header.SequenceNumber);

		device->StatusExtra.SecurityKeyUses++;
	}

	// size byte is wrong
	if((network->RxPacket.Payload[2] - 1) > sizeof(SF_ZigbeeRxPacketT))
	{
		if(device->StatusExtra.SecureKeyInvalid < SUREFLAP_ZIGBEE_MAX_BAD_KEY)
		{
			device->StatusExtra.SecureKeyInvalid++;
		}
		return;
	}

	//calculate parity and drop it in at the end of the array
	uint8_t pM = (uint8_t)SureFlapUtilitiesGetParity((int8_t*)(&network->RxPacket.Payload[0]),
			network->RxPacket.Payload[2] - 1);

	// if parity fails, we assume that is due to a bad security key.
	if(pM != network->RxPacket.Payload[network->RxPacket.Payload[2] - 1])
	{
		if(device->StatusExtra.SecureKeyInvalid < SUREFLAP_ZIGBEE_MAX_BAD_KEY)
		{
			device->StatusExtra.SecureKeyInvalid++;
		}
		return;
	}

	//this removes the parity byte as we no longer need it
	network->RxPacket.Header.PacketLength--;
	device->StatusExtra.SecureKeyInvalid = 0;

	// make a note of the current time to permit receive data timeouts
	//most_recent_rx_time = SureFlapGetTime();

	network->AckPayload.Sequence = network->RxPacket.Header.SequenceNumber;

	//  need to check if we are paired before accepting the packet
	if (device->Status.IsAPair)
	{
		// pass packet up the stack for consumption or rejection
		uint8_t sn_resp;// = sn_data_received(&rx_packet);
		switch(sn_resp)
		{
			case SUREFLAP_DEVICE_RESPONSE_ACCEPTED:
				network->AckPayload.Value = 0;  // message accepted, so send an ACK
				break;

			case SUREFLAP_DEVICE_RESPONSE_CORRUPTED:
				if(device->StatusExtra.SecureKeyInvalid < SUREFLAP_ZIGBEE_MAX_BAD_KEY)
				{
					// could be corrupted because of a bad security key
					device->StatusExtra.SecureKeyInvalid++;
				}
				// message rejected (corrupted), so send a NACK
				network->AckPayload.Value = 0xff;
				break;

			// stack rejected message for unknown reasons
			case SUREFLAP_DEVICE_RESPONSE_REJECTED:

			// stack failed to respond in a timely manner
			case SUREFLAP_DEVICE_RESPONSE_TIMEOUT:

			default:
				network->AckPayload.Value = 0xff; // message rejected, so send a NACK
				break;
		}
	}
	else
	{
		// send a NACK back - not paired
		network->AckPayload.Value = 0xff;
	}

	SureFlapZigbeeTransmit(network,
			SUREFLAP_ZIGBEE_PACKET_DATA_ACK,
			device,
			network->AckPayload.Data,
			2,
			-1,
			false);

	// record ack response in case we have a repeated packet and need to repeat the ack
	//store_ack_response(rx_packet.packet.header.source_address, rx_packet.packet.header.sequence_number, ack_payload[1]);
	//surenet_ack_printf("(1424) Packet data: %x %x\r\n", ack_payload[0], ack_payload[1]);
}
//------------------------------------------------------------------------------

static void SureFlapZigbeeReceiveDeviceAwake(SureFlapZigbeeT* network, SureFlapDeviceT* device)
{
	SureFlapT* hub = network->Object.Parent;
	SureFlapZigbeeAwakePacketT* packet;

	// only accept DEVICE_AWAKE messages if we are paired
	if (!device->Status.IsAPair)
	{
		return;
	}
	else
	{
		//tell device that we are not paired
		SureFlapZigbeeTransmit(network,
				SUREFLAP_ZIGBEE_PACKET_REFUSE_AWAKE,
				device,
				0, 0, -1, false);
	}

	//surenet_log_add(SURENET_LOG_DEVICE_AWAKE_RECIVED,convert_mac_to_index(rx_packet.packet.header.source_address));
	packet = (SureFlapZigbeeAwakePacketT*)(&network->RxPacket.Payload);

	//memcpy(&device_awake_mailbox.payload,pda_payload,sizeof(PACKET_DEVICE_AWAKE_PAYLOAD));
	//device_awake_mailbox.src_addr = rx_packet.packet.header.source_address;
	//xQueueSend(xDeviceAwakeMessageMailbox,&device_awake_mailbox,0);   // put result in mailbox for Surenet-Interface

	device->StatusExtra.AwakeStatus.State = SUREFLAP_DEVICE_AWAKE;

	switch ((int)packet->DeviceDataStatus)
	{
		case SUREFLAP_DEVICE_STATUS_SEND_KEY:
			device->StatusExtra.SendSecurityKey = SUREFLAP_DEVICE_SECURITY_KEY_RENEW;
			packet->DeviceDataStatus = SUREFLAP_DEVICE_STATUS_HAS_NO_DATA;
			break;

		case SUREFLAP_DEVICE_STATUS_DETACH:
			packet->DeviceDataStatus = SUREFLAP_DEVICE_STATUS_HAS_NO_DATA;
			device->Status.IsAPair = 0;
			device->Status.Associated = 0;
			//surenet_update_device_table_line(&device_status[device_index], device_index, true, true);
			//sn_unpair_device(device_status[device_index].mac_address);
			break;

		// copy device_rcvd_segs parameters into a mailbox and send to application
		case SUREFLAP_DEVICE_STATUS_RCVD_SEGS:
			// copy device_rcvd_segs parameters into a mailbox and send to application
			// First update encryption flag for this device
			if(network->RxPacket.Header.PacketLength - sizeof(SureFlapZigbeePacketHeaderT)
					> offsetof(SureFlapZigbeeAwakePacketT, EncryptionTypeExtended))
			{
				// Last byte is optional, so check it's actually there.
				device->StatusExtra.EncryptionType = packet->EncryptionTypeExtended;
			}
			else
			{
				// Not specified, so default to XTEA. Must be an older firmware, or a Pet Door Connect.
				device->StatusExtra.EncryptionType = SUREFLAP_DEVICE_ENCRYPTION_CRYPT_BLOCK_XTEA;
			}

			packet->DeviceDataStatus = SUREFLAP_DEVICE_STATUS_HAS_NO_DATA;
			//keep this information at SureNet level as it is used when deciding which segments of a chunk to send.
			device->ReceivedSegments = packet->RCVD_SEGS_Parameters.ReceivedSegments[0];
			/*
			memcpy(&device_rcvd_segs_parameters, &pda_payload->rcvd_segs_params, sizeof(device_rcvd_segs_parameters));
			// record which device made this request
			device_rcvd_segs_parameters.device_mac = rx_packet.packet.header.source_address;
			// put result in mailbox for Surenet-Interface
			xQueueSend(xDeviceRcvdSegsParametersMailbox,&device_rcvd_segs_parameters,0);
			// Now we are going to cheekily pause this task to give the DFU task
			// an opportunity to find the required f/w page and load it into
			// the mailbox for this task to consume. By doing this, it means that
			// the f/w chunk should be sent on the same Conversation that
			// requested it rather than the one after.
			xQueuePeek(xDeviceFirmwareChunkMailbox, &dummy_chunk, pdMS_TO_TICKS(10));
			*/
			break;

		// This indicates the successful reception of a chunk. Not sure we care
		case SUREFLAP_DEVICE_STATUS_SEGS_COMPLETE:
			packet->DeviceDataStatus = SUREFLAP_DEVICE_STATUS_HAS_NO_DATA;
			break;

		// there is data for us, so we need to establish what decryption to use
		case SUREFLAP_DEVICE_STATUS_HAS_DATA:
			// Last byte is optional, so check it's actually there.
			if(network->RxPacket.Header.PacketLength - sizeof(SureFlapZigbeePacketHeaderT)
					> offsetof(SureFlapZigbeeAwakePacketT, EncryptionType))
			{
				device->StatusExtra.EncryptionType = packet->EncryptionType;
			}
			// Not specified, so default to XTEA. Must be an older firmware, or a Pet Door Connect.
			else
			{
				device->StatusExtra.EncryptionType = SUREFLAP_DEVICE_ENCRYPTION_CRYPT_BLOCK_XTEA;
			}
			break;
	}

	device->StatusExtra.AwakeStatus.Data = packet->DeviceDataStatus;
	device->Status.DeviceRSSI = packet->DeviceRSSI;
	device->Status.HubRSSI = network->RxPacket.Header.RSS;

	//has lock_status changed
	if(device->Status.LockStatus != packet->LockStatus || !device->Status.IsAPair)
	{
		device->Status.LockStatus = packet->LockStatus;

		//call limted version so no time update flags set to limit message rate to Xively
		//surenet_update_device_table_line(&device_status[device_index], device_index, true, true);
	}

	//have minutes changed
	if (device->StatusExtra.DeviceMinutes != packet->DeviceMinutes)
	{
		// minutes changed
		device->StatusExtra.DeviceMinutes = packet->DeviceMinutes;
		/*
		//Cat Flap sends hourly messages itself
		if((hub->DebugMode & SUREFLAP_HUB_SEND_TIME_UPDATES_FROM_DEVICES_EVERY_MINUTE) ||
				(hub->Synchronization.UptimeMinCount == (device_index+1) ))
		{
			// minutes changed, and either hub_debug_mode==1 or hour changed
			//call limted version so no time update flags set to limit message rate to Xively
			////surenet_update_device_table_line(&device_status[device_index], device_index, false, true);
			// Now we have to synthesise a message to the Server, setting registers 33,34,35 which are
			// Battery voltage, hours and minutes. So we need to use COMMAND_SET_REG, length, reg addr,
			// number of registers, reg values, parity.
			// The fake message will end up in surenet_data_received_cb() so we have to ensure we
			// include all the sanity checks it applies
			T_MESSAGE *rx_message;
			rx_message=(T_MESSAGE *)((uint8_t *)&rx_packet+sizeof(HEADER)); //skip past SureNet Header
			rx_message->payload[4] = pda_payload->battery_voltage;	// we do the data copying first to ensure we don't overwrite it
			rx_message->payload[5] = pda_payload->device_hours;		// with any of the fixed values we write below.
			rx_message->payload[6] = pda_payload->device_minutes;
			rx_packet.packet.header.packet_type = PACKET_DATA;
			rx_packet.packet.header.packet_length = 12+23;	// rx_message->length (below) + sizeof(header) -1. Not sure why -1 is needed.
			rx_message->command = 2;	// COMMAND_SET_REG - This is not available as a #define in this part of the code
			rx_message->length = 12; // 2 for COMMAND_, 2 for length, 2 for reg address, 2 for num of reg, 3 for values, and 1 for parity
			rx_message->payload[0]=0;	// register number high byte
			rx_message->payload[1]=33;	// register number low byte
			rx_message->payload[2]=0;	// num of reg high byte
			rx_message->payload[3]=3;	// num of reg low byte
			// We don't need to bother with the parity calculation as it isn't checked
			sn_data_received(&rx_packet);
		}
		*/
	}

}
//------------------------------------------------------------------------------
void SureFlapZigbeePacketReceiver(SureFlapZigbeeT* network, SF_ZigbeeRxBufferT* rx_buffer)
{
	SureFlapT* hub = network->Object.Parent;

	SureFlapZigbeeDecodeFrom_IEEE(network, rx_buffer);

	SureFlapDeviceT* device = SureFlapDeviceGetFrom_MAC(&hub->DeviceControl,
			network->RxPacket.Header.SourceAddress);

	if (!device)
	{
		return;
	}

	device->Status.PacketReceptionTimeStamp = SureFlapGetTime();
	device->Status.Online = true;

	if(device->Transmission.State == SureFlapDeviceTransmissionIsEnable)
	{
		if (device->Transmission.Sequence == network->RxPacket.Header.SequenceNumber)
		{
			device->Transmission.Result = SureFlapDeviceTransmissionComplite;
			device->Transmission.State = SureFlapDeviceTransmissionIdle;
		}
	}

	switch((int)network->RxPacket.Header.PacketType)
	{
		case SUREFLAP_ZIGBEE_PACKET_DATA_ALT_ENCRYPTED:
			device->StatusExtra.EncryptionType = SUREFLAP_DEVICE_ENCRYPTION_CHACHA;
			// fall through to PACKET_DATA processing
		case SUREFLAP_ZIGBEE_PACKET_DATA:
			SureFlapZigbeeReceiveData(network, device);
			break;

		case SUREFLAP_ZIGBEE_PACKET_DATA_ACK:
			// call the host to indicate the acknowledgement
			SureFlapZigbeeReceiveDataAck(network, device);
			break;

		case SUREFLAP_ZIGBEE_PACKET_ACK:
			// here we need to store the information in the ack payload in a buffer
			SureFlapZigbeeReceiveAck(network, device);
			break;

		case SUREFLAP_ZIGBEE_PACKET_BEACON:
			//surenet_printf("Beacon received SHOULD NEVER SEE THIS\r\n");
			break;

		case SUREFLAP_ZIGBEE_PACKET_PAIRING_REQUEST:
			break;

		case SUREFLAP_ZIGBEE_PACKET_DEVICE_STATUS:
			SureFlapZigbeeReceiveDeviceStatus(network, device);
			break;

		case SUREFLAP_ZIGBEE_PACKET_PAIRING_CONFIRM:
			break;

		case SUREFLAP_ZIGBEE_PACKET_CHANNEL_HOP:
			break;

		case SUREFLAP_ZIGBEE_PACKET_DEVICE_AWAKE:
			// message received from device indicating that it is awake and wants a chat.
			SureFlapZigbeeReceiveDeviceAwake(network, device);
			break;

		case SUREFLAP_ZIGBEE_PACKET_DEVICE_TX:
			break;

		case SUREFLAP_ZIGBEE_PACKET_END_SESSION:
			network->EndSession = true;
			break;

		// not sure as a Hub we should ever get this
		case SUREFLAP_ZIGBEE_PACKET_DETACH:
			//surenet_printf( "Received PACKET_DETACH! - should never happen\r\n");
			network->AckPayload.Sequence = network->RxPacket.Header.SequenceNumber;
			// send an ACK back
			network->AckPayload.Value = 0;

			SureFlapZigbeeTransmit(network,
							SUREFLAP_ZIGBEE_PACKET_ACK,
							device,
							network->AckPayload.Data,
							sizeof(network->AckPayload),
							-1, false);
			break;

		case SUREFLAP_ZIGBEE_PACKET_DEVICE_SLEEP:
			break;

		case SUREFLAP_ZIGBEE_PACKET_DEVICE_P2P:
			break;

		case SUREFLAP_ZIGBEE_PACKET_ENCRYPTION_KEY:
			break;

		case SUREFLAP_ZIGBEE_PACKET_REPEATER_PING:
			break;

		case SUREFLAP_ZIGBEE_PACKET_PING_R:
			/*
			// curly brace to overcome weird 'C' limitation of not being able to declare variables inside a case statement
			uint8_t tmp = ~ping_seq;  //cant use ~ping_seq directly, treated as uint32_t!!!
			static uint8_t last_ping_val;
			ping_stats.ping_value = last_ping_val+1;  //force failure if sequence number doesn't match so increments nbad;

			if(rx_packet.packet.payload[0]==tmp)  //Device inverts sequence number as a check
			{
				ping_stats.reply_timestamp = get_microseconds_tick();  //May want to use this for timing ping response time
				ping_stats.found_ping = true;						// stop repeated pings
				ping_stats.report_ping = true;						// and report it's happened
				ping_stats.num_good_pings++;
				ping_stats.ping_rss = rx_packet.packet.payload[2];
				ping_stats.ping_value = ~rx_packet.packet.payload[0];
				memcpy(ping_stats.ping_res, rx_packet.packet.payload, 3);
				ping_stats.ping_res[3] = rx_packet.packet.header.rss;
				ping_stats.hub_rssi_sum += ping_stats.ping_res[3];
				ping_stats.device_rssi_sum += ping_stats.ping_rss;
			}

			//surenet_printf( "New ping counts %x %x\r\n", (unsigned int)ping_stats.num_good_pings, (unsigned int)ping_stats.num_bad_pings);
			last_ping_val = ping_stats.ping_value+1;
			*/
			break;
	}
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE

