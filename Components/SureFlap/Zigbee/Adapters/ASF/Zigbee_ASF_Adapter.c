//==============================================================================
//module enable:

#include "SureFlap/Adapters/SureFlap_AdapterConfig.h"
#ifdef SUREFLAP_ZIGBEE_ASF_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "Zigbee_ASF_Adapter.h"
#include "pal.h"
#include "ieee_const.h"
#include "Components.h"
//==============================================================================
//defines:

//==============================================================================
//export:

extern void trx_irq_handler(void);
//==============================================================================
//variables:

static bool requested_pairing_mode;
static bool pairing_mode;

static uint64_t paired_device_mac;

static uint16_t paring_time;
static uint16_t update_time;

/**
 * used to indicate whether an update to the beacon payload is part of the
 * initialisation (false) (and therefore the callback it triggers performs the next
 * part of the sequence), or whether it's a 'run time' update of the payload (true)
 * and should NOT perform the next part of the sequence.
 */
static bool beacon_payload_update;
//==============================================================================
//functions:

static void trace(void* context, ...)
{
	UNUSED(context);

	xTxTransmitString(Terminal.Tx, context);
	xTxTransmitString(Terminal.Tx, "\r");
}

/**************************************************************
 * Function Name   : set_beacon_payload
 * Description     : Sets the beacon payload.
 * Inputs          : Payload, and update flag.
 *                 : update == true is for when just the payload needs changing
 *                 : update == false is for when the payload is being initialised
 *                 : as part of the initialisation sequence.
 *                 : This matters because setting the payload triggers a callback
 *                 : from the stack, which triggers the next step in the
 *                 : initialisation sequence. We need to suppress triggering the
 *                 : next step if we are just updating the payload.
 * Outputs         :
 * Returns         :
 **************************************************************/
void set_beacon_payload(void *payload, bool update)
{
	wpan_mlme_set_req(macBeaconPayload,payload); // will call back to usr_mlme_set_conf()
	beacon_payload_update = update;
}
//------------------------------------------------------------------------------
// This gets called when a data message is received over the RF Interface.
// Note that the buffer is freed immediately after this function returns, so
// we have to copy the data out.
// putting this variable outside the function means the debugger can see it out of context.
//RX_BUFFER rx_buffer;
void usr_mcps_data_ind(wpan_addr_spec_t *SrcAddrSpec,
		wpan_addr_spec_t *DstAddrSpec,
		uint8_t msduLength,
		uint8_t *msdu,
		uint8_t mpduLinkQuality,
		uint8_t DSN)
{
	static const char note[] = "usr_mcps_data_ind: data=";

	// Copy data into rx_buffer
	SureFlap.Zigbee.RxBuffer.uiSrcAddr = SrcAddrSpec->Addr.long_address;
	SureFlap.Zigbee.RxBuffer.uiDstAddr = DstAddrSpec->Addr.long_address;
	SureFlap.Zigbee.RxBuffer.ucBufferLength = msduLength;
	SureFlap.Zigbee.RxBuffer.ucRSSI = mpduLinkQuality;

	 // copy data out of stack buffer into ours
	if(msduLength < sizeof(SureFlap.Zigbee.RxBuffer.ucRxBuffer))
	{
		memcpy(SureFlap.Zigbee.RxBuffer.ucRxBuffer, msdu, msduLength);

		SureFlapZigbeePacketReceiver(&SureFlap.Zigbee, &SureFlap.Zigbee.RxBuffer);
	}
	else
	{
		//zprintf(HIGH_IMPORTANCE, "Received message too large...\r\n");
	}
/*
	xTxTransmitString(Terminal.Tx, note);
	xTxTransmitData(Terminal.Tx, msdu, msduLength);
	xTxTransmitString(Terminal.Tx, "\r");
	*/
}
//------------------------------------------------------------------------------
// What follows are callback etc. from the MAC layer
// These handle the various protocol sequences via the method of:
// Callback come out of the stack, which trigger new calls to the stack.
// Repeated many times.
void usr_mcps_data_conf(uint8_t msduHandle, uint8_t status) // Called back when a transmit message has gone
{
	static const char note[] = "usr_mcps_data_conf \r";

	SureFlapZigbeeRequestAccept(&SureFlap.Zigbee);

	xTxTransmitString(Terminal.Tx, note);
}
//------------------------------------------------------------------------------
//Note this gets called for a variety of reasons, indicated by STATUS.
//Status is probably set to one of retval_t
void usr_mlme_comm_status_ind(wpan_addr_spec_t *SrcAddrSpec,
		wpan_addr_spec_t *DstAddrSpec,
		uint8_t status)
{
	static const char note[] = "usr_mlme_comm_status_ind ";

	xTxTransmitString(Terminal.Tx, note);

	if (status == MAC_SUCCESS)
	{
		bool paring = true;
		wpan_mlme_set_req(macAssociationPermit, &paring);

		//wpan_addr_spec_t xDestinationAddress;
		//xDestinationAddress.Addr.long_address = DstAddrSpec->Addr.long_address;
		//xDestinationAddress.AddrMode = WPAN_ADDRMODE_LONG;
		//
		//xDestinationAddress.PANId = SureFlap.Zigbee.PanId;

		SureFlapZigbeeAcceptPairing(&SureFlap.Zigbee, DstAddrSpec->Addr.long_address);

		xTxTransmitString(Terminal.Tx, "MAC_SUCCESS");
	}
	else
	{
		xTxTransmitString(Terminal.Tx, "MAC_ERROR");
	}

	xTxTransmitString(Terminal.Tx, "\r");
}
//------------------------------------------------------------------------------
// called when the MAC receives an association request
// Note that devices that have already been paired to us in the past will continue to send us
// Association Requests even if they are not in pairing mode. So we need to reject them here
// if they are not in our pairing table.
void usr_mlme_associate_ind(uint64_t DeviceAddress, uint8_t CapabilityInformation, uint8_t dev_type, uint8_t dev_rssi)
{
	/*
	 * Any device is allowed to join the network
	 * Use: bool wpan_mlme_associate_resp(uint64_t DeviceAddress,
	 *                                    uint16_t AssocShortAddress,
	 *                                    uint8_t status);
	 *
	 * This response leads to comm status indication ->
	 * usr_mlme_comm_status_ind
	 * Get the next available short address for this device
	 */
	static const char note[] = "usr_mlme_associate_ind \r";
	uint16_t associate_short_addr = macShortAddress_def;

	if (SureFlapZigbeeAssociate(&SureFlap.Zigbee,
			DeviceAddress,
			dev_type,
			dev_rssi,
			&associate_short_addr) == xResultAccept)
    {
		wpan_mlme_associate_resp(DeviceAddress, associate_short_addr, ASSOCIATION_SUCCESSFUL);
	}
    else
    {
		wpan_mlme_associate_resp(DeviceAddress, associate_short_addr, PAN_AT_CAPACITY); // PAN_ACCESS_DENIED
	}

	trace("usr_mlme_associate_ind");
	xTxTransmitString(Terminal.Tx, note);
}
//------------------------------------------------------------------------------
// called as a callback from wpan_mlme_start_req(pan_id,current_channel,current_channel_page,15, 15,true, false, false);
void usr_mlme_start_conf(uint8_t status)
{
	if (status != MAC_SUCCESS)
	{
		trace("MAC_ERROR");
	}

	trace("usr_mlme_start_conf");
}
//------------------------------------------------------------------------------
// callback from wpan_mlme_set_req()
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{
	if (status != MAC_SUCCESS)
	{
		wpan_mlme_reset_req(true);
		goto end;
	}

	uint8_t beacon_payload_len = sizeof(SureFlap.Zigbee.BeaconPayload);

	switch(PIBAttribute)
	{
		case macShortAddress:
			// Set length of Beacon Payload  - have to do this before setting beacon payload
			wpan_mlme_set_req(macBeaconPayloadLength, &beacon_payload_len); // will call back to usr_mlme_set_conf()
			break;

		case macBeaconPayloadLength:
			/* Set Beacon Payload */
			set_beacon_payload(SureFlap.Zigbee.BeaconPayload, false);
			break;

		case macBeaconPayload:
			if(beacon_payload_update == false)
			{
				bool rx_on_when_idle = true;
				wpan_mlme_set_req(macRxOnWhenIdle, &rx_on_when_idle); // will call back to usr_mlme_set_conf()
			}
			break;

		case macRxOnWhenIdle:
			/*
			 * Start a nonbeacon-enabled network
			 * Use: bool wpan_mlme_start_req(uint16_t PANId,
			 *                               uint8_t LogicalChannel,
			 *                               uint8_t ChannelPage,
			 *                               uint8_t BeaconOrder,
			 *                               uint8_t SuperframeOrder,
			 *                               bool PANCoordinator,
			 *                               bool BatteryLifeExtension,
			 *                               bool CoordRealignment)
			 *
			 * This request leads to a start confirm message ->
			 * usr_mlme_start_conf
			 */
			// In theory we don't need to set the channel here as we already set it in the TAL initialisation
			// where it is set via TAL_CURRENT_CHANNEL_DEFAULT. But we do it here to ensure consistency
			// between SureNetDriver and the MAC. The 'master' value is now current_channel which is
			// in SureNetDriver, and this is manipulated via snd_set_channel() and snd_get_channel()
			wpan_mlme_start_req(SureFlap.Zigbee.PanId,
					SureFlap.Zigbee.CurrentChannel,
					SureFlap.Zigbee.CurrentChannelPage,
					15, 15, true, false, false);

			wpan_mlme_set_req(phyTransmitPower, &SureFlap.Zigbee.TxPowerPerChannel[SureFlap.Zigbee.CurrentChannel]);
			break;

		case macAssociationPermit:
			// this callback occurs whenever AssociationPermit is changed via a call to wpan_mlme_set_req()
			pairing_mode = requested_pairing_mode;
			break;

		case phyCurrentChannel:
			break;

		case phyTransmitPower:
			break;

		default:
			/* something went wrong; restart */
			//zprintf(HIGH_IMPORTANCE, "Unexpected attribute change, restarting stack\r\n");
			wpan_mlme_reset_req(true);
			break;
	}

	end:;
	trace("usr_mlme_set_conf");
}
//------------------------------------------------------------------------------
// This is a callback from wpan_mlme_get_req()
void usr_mlme_get_conf(uint8_t status, uint8_t PIBAttribute, void *PIBAttributeValue)
{
	if((status == MAC_SUCCESS) && (PIBAttribute == phyCurrentPage))
	{
		SureFlap.Zigbee.CurrentChannelPage = *(uint8_t *)PIBAttributeValue;
		wpan_mlme_get_req(phyChannelsSupported); // will cause a callback back to this function.
	}
	else if((status == MAC_SUCCESS) && (PIBAttribute == phyChannelsSupported))
	{
		uint8_t short_addr[2];
		short_addr[0] = (uint8_t)MAC_NO_SHORT_ADDR_VALUE; /* low byte */
		short_addr[1] = (uint8_t)(MAC_NO_SHORT_ADDR_VALUE >> 8); /*high byte */
		wpan_mlme_set_req(macShortAddress, short_addr);
	}
	else if( (status == MAC_SUCCESS) && (PIBAttribute == phyCurrentChannel) )
	{
		// response from call to get current channel. So put it in a mailbox and set event group
		SureFlap.Zigbee.CurrentChannel = *(uint8_t *)PIBAttributeValue;
	}
	else
	{
		/* Something went wrong; restart */
		//zprintf(HIGH_IMPORTANCE, "Unexpected attribute get callback - restarting stack\r\n");
		wpan_mlme_reset_req(true);
	}

	trace("usr_mlme_get_conf");
}
//------------------------------------------------------------------------------
void usr_mlme_reset_conf(uint8_t status) // this is a callback from wpan_mlme_reset_req
{
	if(status == MAC_SUCCESS)
	{
		wpan_mlme_get_req(phyCurrentPage);
	}
	else
	{	/* something went wrong; restart */
		wpan_mlme_reset_req(true);
	}

	trace("usr_mlme_reset_conf");
}
//------------------------------------------------------------------------------
void usr_mlme_associate_conf(uint16_t AssocShortAddress, uint8_t status)
{
	trace("usr_mlme_associate_conf");
}
//------------------------------------------------------------------------------
void usr_mlme_sync_loss_ind(uint8_t LossReason,
		uint16_t PANId,
		uint8_t LogicalChannel,
		uint8_t ChannelPage)
{
	trace("usr_mlme_sync_loss_ind");
}
//------------------------------------------------------------------------------
void usr_mlme_beacon_notify_ind(uint8_t BSN,
		wpan_pandescriptor_t *PANDescriptor,
		uint8_t PendAddrSpec,
		uint8_t *AddrList,
		uint8_t sduLength,
		uint8_t *sdu)
{
	trace("usr_mlme_beacon_notify_ind");
}
//------------------------------------------------------------------------------
/*
 * Callback function usr_mlme_disassociate_conf
 *
 * @param status             Result of requested disassociate operation.
 * @param DeviceAddrSpec     Pointer to wpan_addr_spec_t structure for device
 *                           that has either requested disassociation or been
 *                           instructed to disassociate by its coordinator.
 *
 * @return void
 */
void usr_mlme_disassociate_conf(uint8_t status, wpan_addr_spec_t *DeviceAddrSpec)
{
	trace("usr_mlme_disassociate_conf");
}
//------------------------------------------------------------------------------
/*
 * Callback function usr_mlme_disassociate_ind
 *
 * @param DeviceAddress        Extended address of device which initiated the
 *                             disassociation request.
 * @param DisassociateReason   Reason for the disassociation. Valid values:
 *                           - @ref WPAN_DISASSOC_BYPARENT,
 *                           - @ref WPAN_DISASSOC_BYCHILD.
 *
 * @return void
 */
void usr_mlme_disassociate_ind(uint64_t DeviceAddress, uint8_t DisassociateReason)
{
	trace("usr_mlme_disassociate_ind");
}
//------------------------------------------------------------------------------
/*
 * Callback function usr_mlme_orphan_ind
 *
 * @param OrphanAddress     Address of orphaned device.
 *
 * @return void
 *
 */
void usr_mlme_orphan_ind(uint64_t OrphanAddress)
{
	trace("usr_mlme_orphan_ind");
}
//------------------------------------------------------------------------------
/*
 * Callback function that must be implemented by application (NHLE) for MAC
 * service
 * MLME-POLL.confirm.
 *
 * @param status           Result of requested poll operation.
 *
 * @return void
 *
 */
void usr_mlme_poll_conf(uint8_t status)
{
	trace("usr_mlme_poll_conf");
}
//------------------------------------------------------------------------------
/*
 * @brief Callback function usr_mlme_scan_conf
 *
 * @param status            Result of requested scan operation
 * @param ScanType          Type of scan performed
 * @param ChannelPage       Channel page on which the scan was performed
 * @param UnscannedChannels Bitmap of unscanned channels
 * @param ResultListSize    Number of elements in ResultList
 * @param ResultList        Pointer to array of scan results
 */
void usr_mlme_scan_conf(uint8_t status,
		uint8_t ScanType,
		uint8_t ChannelPage,
		uint32_t UnscannedChannels,
		uint8_t ResultListSize,
		void *ResultList)
{
	trace("usr_mlme_scan_conf");
}
//------------------------------------------------------------------------------
// This is called from mac_beacon.c when a BEACON_REQUEST has arrived and has been parsed.
// It gives us an opportunity to do three things:
// 1. Record whether the beacon request was from a Thalamus device or pre-Thalamus device.
// 2. Record the channel (although surely we know that anyway?!)
// 3. Set whether we are going to allow Association from this device.
// We also decide here if we are going to allow a BEACON to be sent out in response.
// We return true to indicate that a beacon should be sent.
// We send a BEACON if:
// - We are in pairing mode
// - The MAC of device sending the BEACON_REQUEST is in our device table
// Note that if the MAC of the source device is in our device table, then
// we set the stack into pairing mode to allow the rest of the association
// process to complete.
bool usr_mac_process_beacon_request(uint64_t mac_address, uint8_t src_address_mode, uint8_t data)
{
	bool send_beacon = false;

	if ((data & 0x20) == 0)
	{
		SureFlap.Zigbee.BeaconPayload[SUREFLAP_BEACON_PAYLOAD_VERSION] = SUREFLAP_BEACON_PAYLOAD_HUB_DOES_NOT_SUPPORT_THALAMUS;
		set_beacon_payload(SureFlap.Zigbee.BeaconPayload, true);
	}
	else
	{
		SureFlap.Zigbee.BeaconPayload[SUREFLAP_BEACON_PAYLOAD_VERSION] = SUREFLAP_BEACON_PAYLOAD_HUB_SUPPORTS_THALAMUS;
		set_beacon_payload(SureFlap.Zigbee.BeaconPayload, true);
	}

	// We are already paired with this device, so set ASSOCIATION_PERMIT
	// Note it might already be set if we are in pairing mode.
	if (!paring_time)
	{
		paring_time = 10000;
		send_beacon = true;
		wpan_mlme_set_req(macAssociationPermit, &send_beacon);
	}

	return !send_beacon;
}
//------------------------------------------------------------------------------
/**
 * Checks to see if the supplied mac address is the same as that seen on the most recent beacon request
 */
bool usr_mac_process_associate_request(uint64_t mac_address)
{
	if (paired_device_mac != mac_address)
	{
		paired_device_mac = mac_address;

		return false;
	}
	return true;
}
//==============================================================================

static void PrivateHandler(SureFlapZigbeeT* network)
{
	extern void trx_irq_handler(void);

	SureFlapZigbee_ASF_AdapterT* adapter = network->Adapter.Child;

	//trx_irq_handler();
	if (adapter->Values.IRQ_Event)
	{
		adapter->Values.IRQ_Event = false;

		trx_irq_handler();
	}
/*
	if (!update_time)
	{
		update_time = 100;

		wpan_task();
	}
	*/
	if (!update_time)
	{
		update_time = 5;

		wpan_task();
	}
}
//------------------------------------------------------------------------------

static void PrivateTimeSynchronization(SureFlapZigbeeT* network)
{
	if (paring_time)
	{
		paring_time--;
	}

	if (update_time)
	{
		update_time--;
	}
}
//------------------------------------------------------------------------------

static void PrivateEXTI_Listener(SureFlapZigbeeT* network)
{
	extern volatile bool tal_awake_end_flag;

	SureFlapZigbee_ASF_AdapterT* adapter = network->Adapter.Child;

	adapter->Values.IRQ_Event = true;
	tal_awake_end_flag = true;
}
//------------------------------------------------------------------------------

static xResult PrivateStartNetwork(SureFlapZigbeeT* network)
{
	sw_timer_init();
	wpan_init();
	wpan_mlme_reset_req(true);

	bool pairing = network->ParingEnable;

	wpan_mlme_set_req(phyCurrentChannel, &network->CurrentChannel);
	wpan_mlme_set_req(macAssociationPermit, &pairing);

	return xResultAccept;
}
//------------------------------------------------------------------------------

static xResult PrivateTransmit(SureFlapZigbeeT* network, SureFlapZigbeeAdapterRequestTransmitT* request)
{
	wpan_addr_spec_t wpan_addr_spec;
	uint8_t tx_options;

	wpan_addr_spec.PANId = network->PanId;
	wpan_addr_spec.AddrMode = WPAN_ADDRMODE_LONG;
	wpan_addr_spec.Addr.long_address = request->MAC_Address;

	tx_options = request->AckEnable ? WPAN_TXOPT_ACK : WPAN_TXOPT_OFF;

	return wpan_mcps_data_req(WPAN_ADDRMODE_LONG,
			&wpan_addr_spec,
			request->Size,
			(uint8_t*)request->Data,
			0xcc,
			tx_options) ? xResultAccept : xResultError;
}
//==============================================================================
//interfaces:

static SureFlapZigbeeInterfaceT SureFlapZigbeeAdapterInterface =
{
	.Handler = (SureFlapZigbeeHandlerT)PrivateHandler,
	.EXTI_Listener = (SureFlapZigbeeEXTI_ListenerT)PrivateEXTI_Listener,
	.TimeSynchronization = (SureFlapZigbeeTimeSynchronizationT)PrivateTimeSynchronization,
	.StartNetwork = (SureFlapZigbeeStartNetworkT)PrivateStartNetwork,
	.Transmit = (SureFlapZigbeeTransmitT)PrivateTransmit
};
//==============================================================================
//initialization:

xResult SureFlapZigbee_ASF_AdapterInit(SureFlapZigbeeT* network, SureFlapZigbee_ASF_AdapterT* adapter)
{
	if (network && adapter)
	{
		network->Adapter.Object.Description = nameof(SureFlapZigbee_ASF_AdapterT);
		network->Adapter.Object.Parent = network;

		network->Adapter.Child = adapter;
		network->Adapter.Interface = &SureFlapZigbeeAdapterInterface;

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //ZIGBEE_ASF_ADAPTER_ENABLE
