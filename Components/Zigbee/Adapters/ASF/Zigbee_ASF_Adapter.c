//==============================================================================
//module enable:

#include "Zigbee/Adapters/Zigbee_AdapterConfig.h"
#ifdef ZIGBEE_ASF_ADAPTER_ENABLE
//==============================================================================
//includes:

#include "Zigbee_ASF_Adapter.h"
//==============================================================================
//export:

extern void trx_irq_handler(void);
//==============================================================================
//functions:

static void PrivateHandler(ZigbeeT* network)
{
	Zigbee_ASF_AdapterT* adapter = network->Adapter.Child;

	wpan_task();
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

}
//------------------------------------------------------------------------------
//Note this gets called for a variety of reasons, indicated by STATUS.
//Status is probably set to one of retval_t
void usr_mlme_comm_status_ind(wpan_addr_spec_t *SrcAddrSpec,
		wpan_addr_spec_t *DstAddrSpec,
		uint8_t status)
{

}
//------------------------------------------------------------------------------
// called when the MAC receives an association request
// Note that devices that have already been paired to us in the past will continue to send us
// Association Requests even if they are not in pairing mode. So we need to reject them here
// if they are not in our pairing table.
void usr_mlme_associate_ind(uint64_t DeviceAddress, uint8_t CapabilityInformation)
{

}
//------------------------------------------------------------------------------
// called as a callback from wpan_mlme_start_req(pan_id,current_channel,current_channel_page,15, 15,true, false, false);
void usr_mlme_start_conf(uint8_t status)
{

}
//------------------------------------------------------------------------------
// callback from wpan_mlme_set_req()
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{

}
//------------------------------------------------------------------------------
// This is a callback from wpan_mlme_get_req()
void usr_mlme_get_conf(uint8_t status, uint8_t PIBAttribute, void *PIBAttributeValue)
{

}
//------------------------------------------------------------------------------
void usr_mlme_reset_conf(uint8_t status) // this is a callback from wpan_mlme_reset_req
{

}
//------------------------------------------------------------------------------
// What follows are callback etc. from the MAC layer
// These handle the various protocol sequences via the method of:
// Callback come out of the stack, which trigger new calls to the stack.
// Repeated many times.
void usr_mcps_data_conf(uint8_t msduHandle, uint8_t status) // Called back when a transmit message has gone
{

}
//------------------------------------------------------------------------------
void usr_mlme_associate_conf(uint16_t AssocShortAddress, uint8_t status)
{

}
//------------------------------------------------------------------------------
void usr_mlme_sync_loss_ind(uint8_t LossReason,
		uint16_t PANId,
		uint8_t LogicalChannel,
		uint8_t ChannelPage)
{

}
//------------------------------------------------------------------------------
void usr_mlme_beacon_notify_ind(uint8_t BSN,
		wpan_pandescriptor_t *PANDescriptor,
		uint8_t PendAddrSpec,
		uint8_t *AddrList,
		uint8_t sduLength,
		uint8_t *sdu)
{

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

}
//==============================================================================
//interfaces:

static ZigbeeAdapterInterfaceT ZigbeeAdapterInterface =
{
	INITIALIZATION_HANDLER(ZigbeeAdapter, PrivateHandler),
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

		return xResultAccept;
	}
  
  return xResultError;
}
//==============================================================================
#endif //ZIGBEE_ASF_ADAPTER_ENABLE
