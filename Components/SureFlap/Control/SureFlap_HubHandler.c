//==============================================================================
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap.h"
//==============================================================================
//types:

typedef enum
{
    ACK_NOT_ARRIVED,
    ACK_ARRIVED,
    NACK_ARRIVED

} ACK_StatusT;
//==============================================================================
//defines:

#define MAX_SECURITY_KEY_USES	16
#define MAX_BAD_KEY 8

#define	TIMEOUT_SECURITY_ACK_MS			100
#define	TIMEOUT_WAIT_FOR_DATA_ACK_MS	25
#define	TIMEOUT_DEVICE_STAYS_AWAKE_MS	100
#define	TIMEOUT_PING_WAIT_MS			2000
#define	TIMEOUT_PET_DOOR_PAUSE_MS		50
//==============================================================================
//variables:


//==============================================================================
//functions:

//------------------------------------------------------------------------------
// Function Name   : has_ack_arrived()
// Description     : Check the acknowledge queue to see if an ack
//                 : with that sequence number has arrived recently
// Inputs          :
// Outputs         :
// Returns         :
static ACK_StatusT SureFlapDeviceAckArrived(int16_t seq)
{
	//zprintf(CRITICAL_IMPORTANCE,"{%02x} ",seq);
	/*
	for(uint8_t i = 0; i < ACKNOWLEDGE_QUEUE_SIZE; i++)
	{
		if ((acknowledge_queue[i].valid != 0) && (acknowledge_queue[i].seq_acknowledged==seq))
		{
			acknowledge_queue[i].valid=0;
			return !cknowledge_queue[i].ack_nack ? ACK_ARRIVED : NACK_ARRIVED;
		}
	}
	*/
	return ACK_NOT_ARRIVED;
}
//------------------------------------------------------------------------------

static void SureFlapDeviceInitHandler(SureFlapDeviceControlT* control, SureFlapDeviceT* device)
{
	control->DeviceHandlerIndex = 0;
	control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
}
//------------------------------------------------------------------------------
static void SureFlapDeviceConversationStartHandler(SureFlapDeviceControlT* control, SureFlapDeviceT* device)
{
	SureFlapT* hub = control->Object.Parent;

	//uint32_t time = SureFlapGetTime(control->Object.Parent);

	if(device->StatusExtra.SendSecurityKey != SUREFLAP_DEVICE_SECURITY_KEY_OK)
	{
		uint8_t SecurityKeyScramble[SUREFLAP_DEVICE_SECURITY_KEY_SIZE];
		uint8_t scramble[17] = "IrViNeCeNtReDrYd";

		for(uint8_t i = 0; i < 16; i++)
		{
			SecurityKeyScramble[i] = device->SecurityKey.InByte[i] ^ scramble[i];
		}

		//send the encryption key
		SureFlapZigbeeTransmit(&hub->Zigbee,
				SUREFLAP_ZIGBEE_PACKET_ENCRYPTION_KEY,
				device,
				(uint8_t*)SecurityKeyScramble,
				SUREFLAP_DEVICE_SECURITY_KEY_SIZE,
				-1,
				false);

		device->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_SECURITY_ACK;
	}
	else
	{
		device->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_SEND;
	}

	device->HubOperationTimeout = 100;
	device->HubOperationTimeStamp = SureFlapGetTime(control->Object.Parent);
/*
	while(
		!control->Devices[control->DeviceHandlerIndex].Status.Online
		//((control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.State == SUREFLAP_DEVICE_ASLEEP)
		//|| (time - control->Devices[control->DeviceHandlerIndex].Status.PacketReceptionTimeStamp) > TIMEOUT_DEVICE_STAYS_AWAKE_MS)
		&& (control->DeviceHandlerIndex < SUREFLAP_NUMBER_OF_DEVICES))
	{
		control->DeviceHandlerIndex++;
	}

	if (control->DeviceHandlerIndex >= SUREFLAP_NUMBER_OF_DEVICES)
	{
		// No devices are awake, or we have serviced them all, so skip the conversation phase
		control->HubOperation = SUREFLAP_HUB_OPERATION_HOP;
		for(uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
		{
			if(control->Devices[i].StatusExtra.SecurityKeyUses > MAX_SECURITY_KEY_USES)
			{
				control->Devices[i].StatusExtra.SendSecurityKey = SUREFLAP_DEVICE_SECURITY_KEY_RENEW;
			}
		}
	}
	else
	{
		control->ProcessedDevice = &control->Devices[control->DeviceHandlerIndex];
		//surenet_log_add(SURENET_LOG_STATE_MACHINE_RESPONDED, current_conversee);
		//hub_conv_timestamp = get_microseconds_tick();

		if((control->ProcessedDevice->StatusExtra.SendSecurityKey != SUREFLAP_DEVICE_SECURITY_KEY_OK)
		|| (control->ProcessedDevice->StatusExtra.SecureKeyInvalid >= MAX_BAD_KEY))
		{
			// get new security key
#if USE_RANDOM_KEY
			sn_GenerateSecurityKey(current_conversee);
			surenet_printf("Generating new key for Device %0d\r\n",current_conversee);
#endif
			// trivial obfuscation of security key top avoid sending in clear
			uint8_t SecurityKeyScramble[SUREFLAP_DEVICE_SECURITY_KEY_SIZE];
			uint8_t scramble[17] = "IrViNeCeNtReDrYd";
			for(uint8_t i = 0; i < 16; i++)
			{
				SecurityKeyScramble[i] = control->ProcessedDevice->SecurityKey.InByte[i] ^ scramble[i];
			}

			//send the encryption key
			SureFlapZigbeeTransmit(&hub->Zigbee,
					SUREFLAP_ZIGBEE_PACKET_ENCRYPTION_KEY,
					control->ProcessedDevice,
					(uint8_t*)SecurityKeyScramble,
					SUREFLAP_DEVICE_SECURITY_KEY_SIZE,
					-1,
					false);

			//surenet_log_add(SURENET_LOG_SENT_SECURITY_KEY,current_conversee);
			//zprintf(CRITICAL_IMPORTANCE,"[%02X] ",hub_conv_seq);
			//control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_SECURITY_ACK;
			device->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_SECURITY_ACK;
		}
		else
		{
			//surenet_log_add(SURENET_LOG_STATE_MACHINE_SWITCHING_TO_SEND,current_conversee);
#if PET_DOOR_DELAY
			if(control->Devices[control->DeviceHandlerIndex].Status.DeviceType == DEVICE_TYPE_CAT_FLAP)
			{
				pet_door_timestamp = get_microseconds_tick();
				control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_PET_DOOR_PAUSE;
			}
			else
			{
				control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_SEND;
			}
#else
			//control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_SEND;
			device->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_SEND;
#endif
		}
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceConversationPetDoorPauseHandler(SureFlapDeviceControlT* control)
{
	uint32_t time = SureFlapGetTime(control->Object.Parent);
/*
	if((time - pet_door_timestamp) > TIMEOUT_PET_DOOR_PAUSE_MS)
	{
		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_SEND;
		return;
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceConversationWaitForSecurityAckHandler(SureFlapDeviceControlT* control, SureFlapDeviceT* device)
{
	/*
	//ACK_StatusT ack_reply = has_ack_arrived(hub_conv_seq);
	uint32_t time = SureFlapGetTime(control->Object.Parent);

	if(!control->ProcessedDevice)
	{
		return;
	}

	// we got an ack for the PACKET_ENCRYPTION_KEY we just sent
	if(ack_reply == ACK_ARRIVED)
	{
		control->Devices[control->DeviceHandlerIndex].StatusExtra.SendSecurityKey = SUREFLAP_DEVICE_SECURITY_KEY_OK;
		control->Devices[control->DeviceHandlerIndex].StatusExtra.SecureKeyInvalid = 0;
		control->Devices[control->DeviceHandlerIndex].StatusExtra.SecurityKeyUses = 0;
		//surenet_log_add(SURENET_LOG_SECURITY_KEY_ACKNOWLEDGED,current_conversee);
		//now can send data if required
		hub_state = SUREFLAP_HUB_OPERATION_CONVERSATION_SEND;
	}
	else if((ack_reply == NACK_ARRIVED) || (time - hub_conv_timestamp) > TIMEOUT_SECURITY_ACK))
	{
		if( NACK_ARRIVED == ack_reply)
		{
			//surenet_log_add(SURENET_LOG_SECURITY_NACK,current_conversee);
		}
		else
		{
			//surenet_log_add(SURENET_LOG_SECURITY_ACK_TIMEOUT,current_conversee);
		}

		// loop around for next awake device
		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;

		control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.Mode = SUREFLAP_DEVICE_ASLEEP;
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceConversationSendHandler(SureFlapDeviceControlT* control)
{
	/*
	if(control->Devices[control->DeviceHandlerIndex].StatusExtra.SendDetach == 1)
	{
		tx_retries=0;
		hub_conv_timestamp = get_microseconds_tick();
		//surenet_printf("Sending Detach\r\n");
		//send the detach command
		hub_conv_seq = sn_transmit_packet(PACKET_DETACH,
				control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
				0, 0, -1, false);

		//wait for DETACH_ACK.
		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_DETACH_ACK;
		return;
	}

	// firmware to send, and device has nothing for us
	i = is_there_a_firmware_chunk(current_conversee);
	if((i>=0)
	&& (control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.Data == SUREFLAP_DEVICE_STATUS_HAS_NO_DATA))
	{
		sn_send_chunk(current_conversee, &firmware_chunk[i]);
	}

	// this mailbox ping-pong usually takes about 70us.
	// flush mailbox
	xQueueReceive(xGetNextMessageMailbox_resp,&message_params,0);

	// request polling of application to see if there is a new message
	xQueueSend(xGetNextMessageMailbox,&device_status[current_conversee].mac_address,0);

	// wait for other process to tell us if it has data to send
	result = xQueueReceive(xGetNextMessageMailbox_resp,&message_params,pdMS_TO_TICKS(20));

	// Note we can't wait too long as it will increase power consumption on the devices as it is awake whilst
	// waiting. Also the device will time out and go to sleep after 100ms approx anyway.
	if(pdPASS != result)
	{
		surenet_printf("%08d: Didn't get message from Application\r\n",get_microseconds_tick());
	}

	// we have a new message to send
	if((pdPASS == result) && (message_params.new_message == true))
	{
		hub_conv_timestamp = get_microseconds_tick();
		tx_retries = 0;
		hub_conv_seq = sn_transmit_packet(PACKET_DATA,
				control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
				(uint8_t*)message_params.ptr,
				message_params.ptr->length,
				-1, false);

		surenet_log_add(SURENET_LOG_DATA_SEND,current_conversee);
		hub_state = HUB_STATE_CONVERSATION_WAIT_FOR_DATA_ACK;  // wait for DATA_ACK.
	}
	// we've nothing more to send, so have three possible next steps.
	// Send a ping
	// Send an END_SESSION
	// switch to receive mode
	else
	{
		if (control->Devices[control->DeviceHandlerIndex].send_ping == true)
		{
			ping_stats.ping_attempts = 0;
			control->HubOperation = SUREFLAP_HUB_OPERATION_SEND_PING;
		}
		else
		{
			control->HubOperation = SUREFLAP_HUB_OPERATION_END_OR_RECEIVE;	// decide next step
		}
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceSendEndSessionHandler(SureFlapDeviceControlT* control)
{
	//surenet_printf("Sending PACKET_END_SESSION\r\n");
	//surenet_log_add(SURENET_LOG_END_OF_CONVERSATION,current_conversee);
/*
	control->Devices[control->DeviceHandlerIndex].StatusExtra.TxEndCount++;

	sn_transmit_packet(PACKET_END_SESSION,
			control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
			&control->Devices[control->DeviceHandlerIndex].StatusExtra.TxEndCount,
			1, -1, false);

	control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.Mode = SUREFLAP_DEVICE_ASLEEP;

	// loop around for the next awake device
	control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceSendPingHandler(SureFlapDeviceControlT* control)
{
	/*
	ping_stats.ping_attempts++;

	ping_tx_result = sn_transmit_packet(PACKET_PING,
			control->ProcessedDevice,
			&control->Devices[control->DeviceHandlerIndex].StatusExtra.PingValue,
			1, -1, false);

	// PING was successfully transmitted
	if(ping_tx_result > -1)
	{
		ping_stats.transmission_timestamp = get_microseconds_tick();
		ping_seq = (uint8_t)ping_tx_result;

		//surenet_printf( "Sent Ping with seq=%x value=%x\r\n", ping_seq, control->Devices[control->DeviceHandlerIndex].ping_value);
		control->Devices[control->DeviceHandlerIndex].StatusExtra.SendPing = false;

		// now wait for the ping to come back
		control->HubOperation = SUREFLAP_HUB_OPERATION_SEND_PING_WAIT;
		hub_conv_timestamp = get_microseconds_tick();
	}
	//else just try again - it's unlikely to fail to transmit so we shouldn't get stuck here for long.
	else
	{
		surenet_printf( "!! Ping failed to send: %d\r\n", ping_tx_result);
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceSendPingWaitHandler(SureFlapDeviceControlT* control)
{
	/*
	// never got the reply, so try again
	if((get_microseconds_tick() - hub_conv_timestamp) > usTICK_MILLISECONDS * 2000)
	{
		if(ping_stats.ping_attempts < 5)
		{
			ping_stats.num_bad_pings++;
			control->Devices[control->DeviceHandlerIndex].StatusExtra.SendPing = true;

			// try again
			control->HubOperation = SUREFLAP_HUB_OPERATION_SEND_PING;
		}
		// send the ping too many times, so move on
		else
		{
			control->HubOperation = SUREFLAP_HUB_OPERATION_END_OR_RECEIVE;
		}
	}
	else if(ping_stats.found_ping == true)
	{
		ping_stats.num_good_pings++;
		control->HubOperation = SUREFLAP_HUB_OPERATION_END_OR_RECEIVE;
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceEndOrReceiveHandler(SureFlapDeviceControlT* control)
{
	/*
	if (control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.Data == SUREFLAP_DEVICE_STATUS_HAS_NO_DATA)
	{
		control->HubOperation = SUREFLAP_HUB_OPERATION_SEND_END_SESSION;
	}
	// Note that in Hub1 code, the 'if' part of the above else if is not present, so this
	// block catches everything other than NO_DATA, i.e. DEVICE_HAS_DATA,DEVICE_DETACH,DEVICE_SEND_KEY,
	// DEVICE_DONT_SEND_KEY,DEVICE_RCVD_SEGS,DEVICE_SEGS_COMPLETE
	else if (control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.Data == SUREFLAP_DEVICE_STATUS_HAS_DATA)
	{

		dtx_payload.transmit_end_count = control->Devices[control->DeviceHandlerIndex].StatusExtra.TxEndCount++;
		dtx_payload.encryption_type = control->Devices[control->DeviceHandlerIndex].StatusExtra.EncryptionType;

		// zprintf(LOW_IMPORTANCE,"PDTX\r\n");

		sn_transmit_packet(PACKET_DEVICE_TX,
				control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
				(uint8_t *)&dtx_payload,
				sizeof(DEVICE_TX_PAYLOAD),
				-1, false);

		// this will get updated when a PACKET_DATA is received
		most_recent_rx_time = get_microseconds_tick();

		// this will get set to true when a PACKET_END_SESSION is received
		conv_end_session_flag = false;

		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_RECEIVE;
	}
	else
	{
		//surenet_printf("UNHANDLED REQUEST FROM DEVICE - aborting conversation\r\n");
		//surenet_printf("device_awake_status.data=%d\r\n", control->Devices[control->DeviceHandlerIndex].device_awake_status.data);

		// end session normally
		control->HubOperation = SUREFLAP_HUB_OPERATION_SEND_END_SESSION;
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceConversationWaitForDetachAckHandler(SureFlapDeviceControlT* control)
{
	/*
	ack_reply = has_ack_arrived(hub_conv_seq);

	// we got an ack for the PACKET_DATA we just sent
	if (ack_reply == ACK_ARRIVED)
	{
		//surenet_printf("Got a detach ack %x\r\n", hub_conv_seq);
		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;

		// loop around for next awake device
		control->Devices[control->DeviceHandlerIndex].StatusExtra.SendDetach = 0;
	}
	else if((get_microseconds_tick() - hub_conv_timestamp) > usTICK_SECONDS * 2)
	{
		tx_retries++;

		// retry by sending the exact same packet again
		if (tx_retries < HUB_TX_MAX_RETRIES)
		{
			hub_conv_timestamp = get_microseconds_tick();
			//surenet_printf("Detach retry ");
			//zprintf(LOW_IMPORTANCE,"PD: Sending PACKET_DETACH #2\r\n");

			// send the detach command
			hub_conv_seq = sn_transmit_packet(PACKET_DETACH,
					control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
					0, 0
					hub_conv_seq, false);
		}
		else
		{
			//surenet_printf("Failed to get ACK for detach command but delete device anyway\r\n");

			// loop around for next awake device
			control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
			control->Devices[control->DeviceHandlerIndex].StatusExtra.SendDetach = 0;
		}
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceConversationWaitForDataAckHandler(SureFlapDeviceControlT* control)
{
	/*
	ack_reply = has_data_ack_arrived(hub_conv_seq);

	// we got an ack for the PACKET_DATA we just sent
	if (ack_reply == ACK_ARRIVED)
	{
		// tell the main code that this request has gone
		//surenet_clear_message_cb(message_params.handle);
		//surenet_log_add(SURENET_LOG_DATA_ACK,current_conversee);
		hub_conv_timestamp = get_microseconds_tick();

		// see if more data has arrived.
		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_SEND;
	}
	// we got a nack for the PACKET_DATA we just sent
	// so we need to stop transmitting and switch to receive mode
	else if(ack_reply == NACK_ARRIVED)
	{
		//surenet_printf("Got an NACK - device full - switching to receive %x\r\n", hub_conv_seq);
		dtx_payload.transmit_end_count = control->Devices[control->DeviceHandlerIndex].StatusExtra.TxEndCount++;
		dtx_payload.encryption_type = control->Devices[control->DeviceHandlerIndex].StatusExtra.EncryptionType;

		sn_transmit_packet(PACKET_DEVICE_TX,
				control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
				(uint8_t *)&dtx_payload,
				sizeof(DEVICE_TX_PAYLOAD),
				-1, false);

		//surenet_log_add(SURENET_LOG_DATA_NACK,current_conversee);
		// this will get updated when a PACKET_DATA is received
		most_recent_rx_time = get_microseconds_tick();

		// this will get set to true when a PACKET_END_SESSION is received
		conv_end_session_flag = false;

		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_RECEIVE;
	}
	// didn't get the ack, so we need to resend
	else if((get_microseconds_tick() - hub_conv_timestamp) > TIMEOUT_WAIT_FOR_DATA_ACK)
	{
		// retry
		if(((tx_retries++) < HUB_TX_MAX_RETRIES)
		&& ((get_microseconds_tick() - device_status[current_conversee].last_heard_from) < (usTICK_MILLISECONDS * 100)))
		{
			//surenet_printf("Retrying send\r\n");
			sn_transmit_packet(PACKET_DATA,
					control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
					(uint8_t*)message_params.ptr,
					message_params.ptr->length,
					hub_conv_seq, false);

			//surenet_log_add(SURENET_LOG_DATA_RETRY_ATTEMPT,current_conversee);
			hub_conv_timestamp = get_microseconds_tick();
			// remain in this state
		}
		// give up
		else
		{
			//surenet_printf("Retried DATA too many times, giving up %x, sending PACKET_END_SESSION\r\n", hub_conv_seq);

			// this will probably fail
			//if we haven't managed to send the data
			sn_transmit_packet(PACKET_END_SESSION,
					control->Devices[control->DeviceHandlerIndex].Status.MAC_Address,
					0, 0,
					-1, false);

			control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.Mode = SUREFLAP_DEVICE_ASLEEP;
			//surenet_log_add(SURENET_LOG_DATA_RETRY_FAIL,current_conversee);

			// loop around for next awake device
			control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
		}
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceConversationReceiveHandler(SureFlapDeviceControlT* control)
{
	/*
	// timeout if we've not received a new message in 1 sec.
	if((conv_end_session_flag == true)
	|| ((get_microseconds_tick() - most_recent_rx_time) > usTICK_SECONDS))
	{
		if(true == conv_end_session_flag)
		{
			//surenet_log_add(SURENET_LOG_END_OF_SESSION,current_conversee);
		}
		else
		{
			//surenet_log_add(SURENET_LOG_TIMEOUT,current_conversee);
		}

		control->Devices[control->DeviceHandlerIndex].StatusExtra.AwakeStatus.Mode = SUREFLAP_DEVICE_ASLEEP;

		// conversation over
		control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
	}
	*/
}
//------------------------------------------------------------------------------

static void SureFlapDeviceHopHandler(SureFlapDeviceControlT* control)
{
	/*
	if(trigger_channel_hop == true)
	{
		surenet_printf("Channel hopping by hub not implemented\r\n");
	}
	*/
	control->DeviceHandlerIndex = 0;
	control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
}
//------------------------------------------------------------------------------
//BUSY_STATE hub_functions(void) : SureNet.c

void SureFlapDeviceHubHandler(SureFlapDeviceControlT* control)
{
	SureFlapT* hub = control->Object.Parent;
	SureFlapDeviceT* device = 0;
	/*
	for(uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{

	}
*/
	if (hub->Zigbee.RequestState != SF_ZigbeeRequestIdle)
	{
		return;
	}

	for (uint8_t i = 0; i < SUREFLAP_NUMBER_OF_DEVICES; i++)
	{
		if (control->Devices[i].Status.Online && control->Devices[i].Transmission.State == SureFlapDeviceTransmissionIdle)
		{
			device = &control->Devices[i];
			break;
		}
	}

	if(!device)
	{
		return;
	}

	switch(device->HubOperation)
	{
		case SUREFLAP_HUB_OPERATION_INIT:
			SureFlapDeviceInitHandler(control, device);
			break;

		// Need to discover an awake device. Don't bother attempting
		// to talk to devices who's DEVICE_AWAKE message happened so long ago that they will
		// have gone back to sleep. Were we to attempt that, we'd just incur a pointless delay
		case SUREFLAP_HUB_OPERATION_CONVERSATION_START:
			SureFlapDeviceConversationStartHandler(control, device);
			break;

		// This state introduces a pause into the conversation (i.e. slows it down)
		// to match Hub1 timing. It only applies to Pet Doors as there is a concern
		// that they occasionally crash with Hub2 timings.
		case SUREFLAP_HUB_OPERATION_CONVERSATION_PET_DOOR_PAUSE:
			SureFlapDeviceConversationPetDoorPauseHandler(control);
			break;

		// wait for the ack to arrive, or timeout and resend
		case SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_SECURITY_ACK:
			SureFlapDeviceConversationWaitForSecurityAckHandler(control, device);
			break;

		// Remember that the device could have gone back to sleep
		// now decide what to send the device. Options are:
		// PACKET_DATA if we have data to send
		// PACKET_DEVICE_TX if we have no data, but device_awake_status[].data=DEVICE_HAS_DATA
		// PACKET_END_SESSION if neither of us have any data
		// OR, we might have a ping to send to the device, so we divert the state machine to send that before PACKET_END_SESSION.
		case SUREFLAP_HUB_OPERATION_CONVERSATION_SEND:
			SureFlapDeviceConversationSendHandler(control);
			break;

		case SUREFLAP_HUB_OPERATION_SEND_END_SESSION:
			SureFlapDeviceSendEndSessionHandler(control);
			break;

		//send a ping to the device
		case SUREFLAP_HUB_OPERATION_SEND_PING:
			SureFlapDeviceSendPingHandler(control);
			break;

		// wait for either the ping reply, or a short timeout to expire
		case SUREFLAP_HUB_OPERATION_SEND_PING_WAIT:
			SureFlapDeviceSendPingWaitHandler(control);
			break;

		case SUREFLAP_HUB_OPERATION_END_OR_RECEIVE:
			SureFlapDeviceEndOrReceiveHandler(control);
			break;

		// wait for the Detach ack to arrive, or timeout and resend the detach command
		case SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_DETACH_ACK:
			SureFlapDeviceConversationWaitForDetachAckHandler(control);
			break;

		// wait for the ack to arrive, or timeout and resend
		case SUREFLAP_HUB_OPERATION_CONVERSATION_WAIT_FOR_DATA_ACK:
			SureFlapDeviceConversationWaitForDataAckHandler(control);
			break;

		// receive and acknowledge data from the device, until timeout or PACKET_END_SESSION
		case SUREFLAP_HUB_OPERATION_CONVERSATION_RECEIVE:
			SureFlapDeviceConversationReceiveHandler(control);
			break;

		// conversation over - look at whether we need a frequency hop......
		case SUREFLAP_HUB_OPERATION_HOP:
			SureFlapDeviceHopHandler(control);
			break;

		default:
			control->HubOperation = SUREFLAP_HUB_OPERATION_CONVERSATION_START;
			break;
	}

	// decide if we are busy (i.e. mid activity), or about to start again.

	control->HubHandlerState = control->HubOperation == SUREFLAP_HUB_OPERATION_CONVERSATION_START ?
			SUREFLAP_HUB_HANDLER_IDLE : SUREFLAP_HUB_HANDLER_BUSY;
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE
