//==============================================================================
//module enable:

#include "Zigbee/Adapters/Zigbee_AdapterConfig.h"
#ifdef ZIGBEE_ASF_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "Zigbee_ASF_Adapter.h"
#include "pal.h"
#include "ieee_const.h"
//==============================================================================
//defines:

#define SUREFLAP_HUB                    0x7e
#define HUB_SUPPORTS_THALAMUS           0x02
#define HUB_DOES_NOT_SUPPORT_THALAMUS   0x01
#define HUB_IS_SOLE_PAN_COORDINATOR     0x00

#define SUREFLAP_PAN_ID	0x3421

#define RF_CHANNEL1 15
#define RF_CHANNEL2 20
#define RF_CHANNEL3 26
#define INITIAL_CHANNEL RF_CHANNEL1
//==============================================================================
//export:

extern void trx_irq_handler(void);
//==============================================================================
//variables:

static uint8_t current_channel_page;
static uint8_t current_channel = INITIAL_CHANNEL;
static uint16_t pan_id = SUREFLAP_PAN_ID;
static bool requested_pairing_mode;
static bool pairing_mode;
static uint8_t beacon_payload[] = { SUREFLAP_HUB, HUB_SUPPORTS_THALAMUS, HUB_IS_SOLE_PAN_COORDINATOR };
//==============================================================================
//functions:

static void trace(void* context, ...)
{
	UNUSED(context);
}
//------------------------------------------------------------------------------
// This gets called when a data message is received over the RF Interface.
// Note that the buffer is freed immediately after this function returns, so
// we have to copy the data out.
//RX_BUFFER rx_buffer;	// putting this variable outside the function means the debugger can see it out of context.
void usr_mcps_data_ind(wpan_addr_spec_t *SrcAddrSpec,
		wpan_addr_spec_t *DstAddrSpec,
		uint8_t msduLength,
		uint8_t *msdu,
		uint8_t mpduLinkQuality,
		uint8_t DSN)
{
	trace(usr_mcps_data_ind);
}
//------------------------------------------------------------------------------
//Note this gets called for a variety of reasons, indicated by STATUS.
//Status is probably set to one of retval_t
void usr_mlme_comm_status_ind(wpan_addr_spec_t *SrcAddrSpec,
		wpan_addr_spec_t *DstAddrSpec,
		uint8_t status)
{
	trace(usr_mlme_comm_status_ind);
}
//------------------------------------------------------------------------------
// called when the MAC receives an association request
// Note that devices that have already been paired to us in the past will continue to send us
// Association Requests even if they are not in pairing mode. So we need to reject them here
// if they are not in our pairing table.
void usr_mlme_associate_ind(uint64_t DeviceAddress, uint8_t CapabilityInformation)
{
	trace(usr_mlme_associate_ind);
}
//------------------------------------------------------------------------------
// called as a callback from wpan_mlme_start_req(pan_id,current_channel,current_channel_page,15, 15,true, false, false);
void usr_mlme_start_conf(uint8_t status)
{
	if (status != MAC_SUCCESS)
	{
		trace("MAC_ERROR");
	}

	trace(usr_mlme_start_conf);
}
//------------------------------------------------------------------------------
// callback from wpan_mlme_set_req()
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{
	if((status == MAC_SUCCESS) && (PIBAttribute == macShortAddress))
	{
		// Set length of Beacon Payload  - have to do this before setting beacon payload
		uint8_t beacon_payload_len = sizeof(beacon_payload);
		wpan_mlme_set_req(macBeaconPayloadLength, &beacon_payload_len);    // will call back to usr_mlme_set_conf()
	}
	else if((status == MAC_SUCCESS) && (PIBAttribute == macBeaconPayloadLength))
	{
		/* Set Beacon Payload */
		wpan_mlme_set_req(macBeaconPayload, beacon_payload);
	}
	else if((status == MAC_SUCCESS) && (PIBAttribute == macBeaconPayload))
	{
		// Set RX on when idle to enable the receiver as default.
		// Use: bool wpan_mlme_set_req(uint8_t PIBAttribute, void *PIBAttributeValue);
		bool rx_on_when_idle = true;
		wpan_mlme_set_req(macRxOnWhenIdle, &rx_on_when_idle); // will call back to usr_mlme_set_conf()
	}
	else if((status == MAC_SUCCESS) && (PIBAttribute == macRxOnWhenIdle))
	{
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
		wpan_mlme_start_req(pan_id, current_channel, current_channel_page, 15, 15, true, false, false);
	}
	else if((status == MAC_SUCCESS) && (PIBAttribute == macAssociationPermit))
	{
		// this callback occurs whenever AssociationPermit is changed via a call to wpan_mlme_set_req()
		pairing_mode = requested_pairing_mode;
	}
	else if( (status == MAC_SUCCESS) && (PIBAttribute == phyCurrentChannel) )
	{
		// do nothing
	}
	else
	{
		/* something went wrong; restart */
		//zprintf(HIGH_IMPORTANCE, "Unexpected attribute change, restarting stack\r\n");
		wpan_mlme_reset_req(true);
	}

	trace(usr_mlme_set_conf);
}
//------------------------------------------------------------------------------
// This is a callback from wpan_mlme_get_req()
void usr_mlme_get_conf(uint8_t status, uint8_t PIBAttribute, void *PIBAttributeValue)
{
	if((status == MAC_SUCCESS) && (PIBAttribute == phyCurrentPage))
	{
		current_channel_page = *(uint8_t *)PIBAttributeValue;
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
		current_channel = *(uint8_t *)PIBAttributeValue;
	}
	else
	{
		/* Something went wrong; restart */
		//zprintf(HIGH_IMPORTANCE, "Unexpected attribute get callback - restarting stack\r\n");
		wpan_mlme_reset_req(true);
	}

	trace(usr_mlme_get_conf);
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

	trace(usr_mlme_reset_conf);
}
//------------------------------------------------------------------------------
// What follows are callback etc. from the MAC layer
// These handle the various protocol sequences via the method of:
// Callback come out of the stack, which trigger new calls to the stack.
// Repeated many times.
void usr_mcps_data_conf(uint8_t msduHandle, uint8_t status) // Called back when a transmit message has gone
{
	trace(usr_mcps_data_conf);
}
//------------------------------------------------------------------------------
void usr_mlme_associate_conf(uint16_t AssocShortAddress, uint8_t status)
{
	trace(usr_mlme_associate_conf);
}
//------------------------------------------------------------------------------
void usr_mlme_sync_loss_ind(uint8_t LossReason,
		uint16_t PANId,
		uint8_t LogicalChannel,
		uint8_t ChannelPage)
{
	trace(usr_mlme_sync_loss_ind);
}
//------------------------------------------------------------------------------
void usr_mlme_beacon_notify_ind(uint8_t BSN,
		wpan_pandescriptor_t *PANDescriptor,
		uint8_t PendAddrSpec,
		uint8_t *AddrList,
		uint8_t sduLength,
		uint8_t *sdu)
{
	trace(usr_mlme_beacon_notify_ind);
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
	trace(usr_mlme_disassociate_conf);
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
	trace(usr_mlme_disassociate_ind);
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
	trace(usr_mlme_orphan_ind);
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
	trace(usr_mlme_poll_conf);
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
	trace(usr_mlme_scan_conf);
}
//==============================================================================

static void PrivateHandler(ZigbeeT* network)
{
	extern void trx_irq_handler(void);

	Zigbee_ASF_AdapterT* adapter = network->Adapter.Child;

	//trx_irq_handler();
	if (adapter->Values.IRQ_Event)
	{
		adapter->Values.IRQ_Event = false;

		trx_irq_handler();
	}

	wpan_task();
}
//------------------------------------------------------------------------------

static void PrivateEXTI_Listener(ZigbeeT* network)
{
	extern volatile bool tal_awake_end_flag;

	Zigbee_ASF_AdapterT* adapter = network->Adapter.Child;

	adapter->Values.IRQ_Event = true;
	tal_awake_end_flag = true;
}
//==============================================================================
//interfaces:

static ZigbeeAdapterInterfaceT ZigbeeAdapterInterface =
{
	INITIALIZATION_HANDLER(ZigbeeAdapter, PrivateHandler),
	.EXTI_Listener = (ZigbeeAdapterEXTI_ListenerT)PrivateEXTI_Listener
};
//==============================================================================
//initialization:

xResult Zigbee_ASF_AdapterInit(ZigbeeT* network, Zigbee_ASF_AdapterT* adapter)
{
	if (network && adapter)
	{
		network->Adapter.Object.Description = nameof(Zigbee_ASF_AdapterT);
		network->Adapter.Object.Parent = network;

		network->Adapter.Child = adapter;
		network->Adapter.Interface = &ZigbeeAdapterInterface;
		
		sw_timer_init();
		wpan_init();
		wpan_mlme_reset_req(true);

		uint8_t channel = 15;
		bool pairing = true;

		wpan_mlme_set_req(phyCurrentChannel, &channel);

		wpan_mlme_set_req(macAssociationPermit, &pairing);

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //ZIGBEE_ASF_ADAPTER_ENABLE
