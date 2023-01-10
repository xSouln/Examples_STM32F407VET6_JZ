/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright � 2013-2021 Sureflap Limited.
* All Rights Reserved.
*
* All information contained herein is, and remains the property of Sureflap
* Limited.
* The intellectual and technical concepts contained herein are proprietary to
* Sureflap Limited. and may be covered by U.S. / EU and other Patents, patents
* in process, and are protected by copyright law.
* Dissemination of this information or reproduction of this material is
* strictly forbidden unless prior written permission is obtained from Sureflap
* Limited.
*
* Filename: Surenet-Interface.c
* Author:   Chris Cowdery 23/07/2019
*
* This file handles the inter-task communication with SureNet. It contains weakly linked
* functions which can be overridden by the correct handlers in the application
* The weakly linked handlers exist in the context of the main applicaton.
*
*
**************************************************************************/

#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "hermes-time.h"

// SureNet
#include "SureNet-Interface.h"
#include "SureNet.h"
#include "devices.h"

#include "RegisterMap.h"


// local functions

// These mailboxes are used to transfer data into / out of the SureNet stack
extern QueueHandle_t xAssociationSuccessfulMailbox;
extern QueueHandle_t xRxMailbox;
extern QueueHandle_t xRxMailbox_resp;
extern QueueHandle_t xGetNumPairsMailbox_resp;
extern QueueHandle_t xSetPairingModeMailbox;
extern QueueHandle_t xUnpairDeviceMailbox;
extern QueueHandle_t xUnpairDeviceByIndexMailbox;
extern QueueHandle_t xGetPairmodeMailbox_resp;
extern QueueHandle_t xIsDeviceOnlineMailbox;
extern QueueHandle_t xIsDeviceOnlineMailbox_resp;
extern QueueHandle_t xGetNextMessageMailbox;
extern QueueHandle_t xGetNextMessageMailbox_resp;
extern QueueHandle_t xDeviceAwakeMessageMailbox;
extern QueueHandle_t xPairingModeHasChangedMailbox;
extern QueueHandle_t xGetChannelMailbox_resp;
extern QueueHandle_t xSetChannelMailbox;
extern QueueHandle_t xPingDeviceMailbox;
extern QueueHandle_t xPingDeviceMailbox_resp;
extern QueueHandle_t xDeviceRcvdSegsParametersMailbox;
extern QueueHandle_t xDeviceFirmwareChunkMailbox;
extern QueueHandle_t xGetLastHeardFromMailbox_resp;
extern QueueHandle_t xBlockingTestMailbox;

QueueHandle_t xSendDeviceStatusMailbox;

// EventGroup to communicate with SureNet
extern EventGroupHandle_t xSurenet_EventGroup;

/**************************************************************
 * Function Name   : Surenet_Interface_Handler
 * Description     : This function must be polled in the same context as the application
 *                 : It will emit callbacks for SureNet events.
 *                 : It handles the mailboxes and events into / out of the SureNet stack
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void Surenet_Interface_Handler(void)
{
    RECEIVED_PACKET rx_packet;
    bool result;
	PAIRING_REQUEST presult;
    uint64_t mac_address;
	uint32_t blocking_test_value;
    MESSAGE_PARAMS message_params;
    ASSOCIATION_SUCCESS_INFORMATION assoc_info;
    DEVICE_AWAKE_MAILBOX device_awake_mailbox;
    PING_STATS ping_result;
	DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX DeviceRcvdSegsParameters;

    // Receive Packet
    if( (uxQueueMessagesWaiting(xRxMailbox) > 0) &&
		(xQueueReceive(xRxMailbox, &rx_packet,0)==pdPASS) )
    {
        result = surenet_data_received_cb(&rx_packet);
        xQueueSend(xRxMailbox_resp,&result,0);
    }

    // check for a successful association
    if( (uxQueueMessagesWaiting(xAssociationSuccessfulMailbox) > 0) &&
		(pdPASS == xQueueReceive(xAssociationSuccessfulMailbox,&assoc_info,0)) )
    { // got successful association, so store in the pairing table
        surenet_device_pairing_success_cb(&assoc_info);    // call back to say association successful
    }

    // check for pairing mode change
    if( (uxQueueMessagesWaiting(xPairingModeHasChangedMailbox) > 0) &&
		(pdPASS == xQueueReceive(xPairingModeHasChangedMailbox,&presult,0)) )
    { // got a request, so service it
        surenet_pairing_mode_change_cb(presult);
    }

    // check for a request for the next message to send
    if( (uxQueueMessagesWaiting(xGetNextMessageMailbox) > 0) &&
		(pdPASS == xQueueReceive(xGetNextMessageMailbox,&mac_address,0)) )
    { // got a request, so service it
        message_params.new_message = surenet_get_next_message_cb(mac_address,&message_params.ptr, &message_params.handle);
        xQueueSend(xGetNextMessageMailbox_resp,&message_params,0);
    }

    // check for data from the most recent DEVICE_AWAKE message
    if( (uxQueueMessagesWaiting(xDeviceAwakeMessageMailbox) > 0) &&
		(pdPASS == xQueueReceive(xDeviceAwakeMessageMailbox,&device_awake_mailbox,0)) )
    { // got a request, so service it
        surenet_device_awake_notification_cb(&device_awake_mailbox);
    }

	if( (uxQueueMessagesWaiting(xPingDeviceMailbox_resp) > 0) &&
		(pdPASS == xQueueReceive(xPingDeviceMailbox_resp,&ping_result,0)) )
	{	// SureNet has sent us a ping reply!
		surenet_ping_response_cb(&ping_result);
	}

	if (pdPASS == xQueueReceive(xDeviceRcvdSegsParametersMailbox,&DeviceRcvdSegsParameters,0))
	{
		surenet_device_rcvd_segs_cb(&DeviceRcvdSegsParameters);
	}

	if( pdPASS == xQueueReceive(xBlockingTestMailbox,&blocking_test_value,0))
	{
		surenet_blocking_test_cb(blocking_test_value);
	}

	HubReg_Handle_Messages();	// Check for register updates.

}

/**************************************************************
 * Function Name   : surenet_init
 * Description     : Called before scheduler starts to initialise SureNet.
 *                 : Initialises some tasks, and also initialises the Surenet Driver.
 * Inputs          :
 * Outputs         :
 * Returns         : pdPASS if it was successful, pdFAIL if it could not complete (e.g. couldn't set up a task)
 **************************************************************/
BaseType_t surenet_init(uint64_t *mac, uint16_t panid, uint8_t channel)
{
    xSendDeviceStatusMailbox	= xQueueCreate(1, sizeof(DEVICE_STATUS_REQUEST));
    return sn_init(mac, panid, channel);
}

/**************************************************************
 * Function Name   : surenet_send_firmware_chunk
 * Description     : Sends a chunk of firmware (and it's destination) into SureNet
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_send_firmware_chunk(DEVICE_FIRMWARE_CHUNK *device_firmware_chunk)
{
	xQueueSend(xDeviceFirmwareChunkMailbox,device_firmware_chunk,pdMS_TO_TICKS(100));
}

/**************************************************************
 * Function Name   : surenet_is_device_online
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
bool surenet_is_device_online(uint64_t mac_addr)
{
    bool result;
    xQueueSend(xIsDeviceOnlineMailbox,&mac_addr,0);
    if( pdPASS == xQueueReceive(xIsDeviceOnlineMailbox_resp,&result,pdMS_TO_TICKS(100)) )
    { // got result, so return it
        return result;
    }
    return false;   // slightly dodgy - we should return 'unknown' here instead.

}

/**************************************************************
 * Function Name   : surenet_ping_device
 * Description     : Pings a remote device.
 * Inputs          : mac address of remote device, or 0 for the first one in the pairing table
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_ping_device(uint64_t mac_address, uint8_t value)
{
	PING_REQUEST_MAILBOX ping_request;
	ping_request.mac_address = mac_address;
	ping_request.value = value;
    xQueueSend(xPingDeviceMailbox,&ping_request,0);
}

/**************************************************************
 * Function Name   : surenet_unpair_device(index)
 * Description     : Detach one paired device by mac (or 0 for all devices)
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_unpair_device(uint64_t mac_addr)
{
    xQueueSend(xUnpairDeviceMailbox,&mac_addr,0);
}

/**************************************************************
 * Function Name   : surenet_unpair_device_by_index(index)
 * Description     : Detach one paired device
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_unpair_device_by_index(uint8_t index)
{
    xQueueSend(xUnpairDeviceByIndexMailbox,&index,0);
}

/**************************************************************
 * Function Name   : surenet_set_hub_pairing_mode()
 * Description     : Put Hub in Pairing Mode. This is called by all of the 'user'
 *                 : mechanisms, specifically via the Server (register write), the
 *                 : button on the bottom of the Hub, or via the CLI.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_set_hub_pairing_mode(PAIRING_REQUEST mode)
{
    xQueueSend(xSetPairingModeMailbox, &mode, 0);
}

/**************************************************************
 * Function Name   : surenet_get_hub_pairing_mode()
 * Description     : Get current pairing mode
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
PAIRING_REQUEST surenet_get_hub_pairing_mode(void)
{
	PAIRING_REQUEST result;
	xEventGroupSetBits(xSurenet_EventGroup,SURENET_GET_PAIRMODE);    // and set flag
	xQueueReceive(xGetPairmodeMailbox_resp,&result,pdMS_TO_TICKS(100));
	return result;
}

/**************************************************************
 * Function Name   : surenet_get_last_heard_from()
 * Description     : Get when we last heard from an RF device
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint32_t surenet_get_last_heard_from(void)
{
    uint32_t result;
    xEventGroupSetBits(xSurenet_EventGroup,SURENET_GET_LAST_HEARD_FROM);    // and set request
    if( pdPASS == xQueueReceive(xGetLastHeardFromMailbox_resp,&result,pdMS_TO_TICKS(100)) )
    { // got pairing_mode, so return it
        return result;
    }
    return 0xffffffff;
}

/**************************************************************
 * Function Name   : surenet_get_channel()
 * Description     : Get current channel by sending an event and waiting for the reply
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t surenet_get_channel(void)
{
    uint8_t result;
    xEventGroupSetBits(xSurenet_EventGroup,SURENET_GET_CHANNEL);    // and set flag
    if( pdPASS == xQueueReceive(xGetChannelMailbox_resp,&result,pdMS_TO_TICKS(100)) )
    { // got pairing_mode, so return it
        return result;
    }
    return 0xff;   // slightly dodgy - we should return 'unknown' here instead.
}

/**************************************************************
 * Function Name   : surenet_set_channel()
 * Description     : set current channel
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t surenet_set_channel(uint8_t channel)
{
	//Force channel number to one RF module will recognise
	if(channel<=RF_CHANNEL1)
		channel = RF_CHANNEL1;
	else if(channel<=RF_CHANNEL2)
	  	channel = RF_CHANNEL2;
	else
	  	channel = RF_CHANNEL3;

    xQueueSend(xSetChannelMailbox,&channel,0);
	return channel;
}

/**************************************************************
 * Function Name   : surenet_how_many_pairs
 * Description     : Count number of valid devices we are paired with
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t surenet_how_many_pairs(void)
{
    uint8_t result;
    xEventGroupSetBits(xSurenet_EventGroup,SURENET_GET_NUM_PAIRS);    // and set flag
    if( pdPASS == xQueueReceive(xGetNumPairsMailbox_resp,&result,pdMS_TO_TICKS(100)) )
    { // got pairing_mode, so return it
        return result;
    }
    return false;   // slightly dodgy - we should return 'unknown' here instead.
}

/**************************************************************
 * Function Name   : surenet_hub_clear_pairing_table
 * Description     : Wipes the pairing table and all device status information
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_hub_clear_pairing_table(void)
{
    xEventGroupSetBits(xSurenet_EventGroup,SURENET_CLEAR_PAIRING_TABLE);    // and set flag
}

/**************************************************************
 * Function Name   : surenet_trigger_channel_hop
 * Description     : Triggers a channel hop within SureNet.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_trigger_channel_hop(void)
{
    xEventGroupSetBits(xSurenet_EventGroup,SURENET_TRIGGER_CHANNEL_HOP);    // and set flag
}

/**************************************************************
 * Function Name   : surenet_update_device_table_line
 * Description     : Sends message to Application to update Device Table values.
 **************************************************************/
BaseType_t surenet_update_device_table_line(DEVICE_STATUS* status, uint32_t line, bool limited, bool wait)
{
	TickType_t wait_period;
	DEVICE_STATUS_REQUEST	request;
	request.device_status = *status;
	request.line = line;
	request.limited = limited;

	if( true == wait)
	{
		wait_period = pdMS_TO_TICKS(10000);
	}
	else
	{
		wait_period = 0;	// some callers don't want to wait, they want to retry
	}

	return xQueueSend(xSendDeviceStatusMailbox, &request, wait_period);
}


///// Callbacks
/**************************************************************
 * Function Name   : surenet_device_rcvd_segs_cb
 * Description     : Called when a DEVICE_RCVD_SEGS message is received from
 *                 : the device. Will prompt the delivery of a new chunk of
 *                 : firmware to the device
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
__weak void surenet_device_rcvd_segs_cb(DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX *params)
{

}
/**************************************************************
* Function Name   : surenet_get_next_message_cb
* Description     : Called by SureNet when it wants the next message to send to mac
*                 : mac is the mac address of the destination
*                 : current_request_ptr is a pointer to a T_MESSAGE to send
*                 : current_request_handle is a pointer to a message handle, which is returned in surenet_clear_message_cb
*                 : so we know which message has been successfully sent.
*                 : of the message to be used as a parameter to surenet_clear_request_cb()
*                 : to tell the host code that SureNet has processed that message.
* Inputs          :
* Outputs         :
* Returns         : true if there is a message to go (and parameters have been set), false if no message
 **************************************************************/
__weak bool surenet_get_next_message_cb(uint64_t mac, T_MESSAGE **current_request_ptr, uint32_t *current_request_handle)
{
    return false;   // no new message at the moment.
}

/**************************************************************
 * Function Name   : surenet_clear_message_cb
 * Description     : Marks a message_for_device as invalid, i.e. consumed by SureNet
 * Inputs          : index of message, as set when calling surenet_get_next_message_cb()
 * Outputs         :
 * Returns         :
 **************************************************************/
__weak void surenet_clear_message_cb(uint32_t current_request_handle)
{
    return;
}

/**************************************************************
 * Function Name   : surenet_data_received_cb
 * Description     : Process messages received over SureNet from a device
 * Inputs          : Pointer to a packet.
 * Outputs         :
 * Returns         : TRUE if the message could be processed (SureNet will return an ACK), FALSE if not (NACK).
 **************************************************************/
__weak SN_DATA_RECEIVED_RESPONSE surenet_data_received_cb(RECEIVED_PACKET *rx_packet)
{
// We could do some additional sanity checking to be sure that the data is not corrupted. For example:
// if((rx_packet.packet.header.packet_length-24)!=rx_packet.packet.payload[2])  //24 = IEEE header length
// which is valid for T_MESSAGE formats is an additional check. If this fails, we should return SN_CORRUPTED
// which will eventually cause the stack to renegotiate the security key.
    zprintf(LOW_IMPORTANCE, "surenet_data_received_cb():\r\n");
    return SN_ACCEPTED;
}

/**************************************************************
 * Function Name   : surenet_pairing_mode_change_cb
 * Description     : Called when Pairing Mode has changed, including the automatic
 *                 : cancellation of Pairing Mode when a successful association has completed.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
__weak void surenet_pairing_mode_change_cb(PAIRING_REQUEST new_state)
{

}

/**************************************************************
 * Function Name   : surenet_device_pairing_success_cb()
 * Description     : Called when a pairing process has completed
 *                 : Pairing Mode is turned off automatically
 * Inputs          : Data about new device
 * Outputs         :
 * Returns         :
 **************************************************************/
__weak void surenet_device_pairing_success_cb(ASSOCIATION_SUCCESS_INFORMATION *assoc_info)
{
    uint64_t mac_addr;
    mac_addr = assoc_info->association_addr;
    zprintf(LOW_IMPORTANCE,"surenet_device_pairing_success_cb() Remote Device :%08x", (uint32_t)((mac_addr&0xffffffff00000000)>>32));
    zprintf(LOW_IMPORTANCE,"%08x\r\n", (uint32_t)(mac_addr&0xffffffff));
    return;
}

/**************************************************************
 * Function Name   : surenet_device_awake_notification()
 * Description     : Called when a device awake message has been received
 *                 : from an on-line device
 * Inputs          :
 * Outputs         : Source address and data. Note not all elements are useful!
 * Returns         :
 **************************************************************/
bool SDAN_QUIET_MODE		= true;			// display a muted notification if there is no data waiting
bool SDAN_VERY_QUIET_MODE	= true;			// don't show the muted notification
__weak void surenet_device_awake_notification_cb(DEVICE_AWAKE_MAILBOX *device_awake_mailbox)
{
	const char* data_status[] = {
		{"[no data]"},
		{"[has data]"},
		{"[send key]"},
		{"[don't send key]"},
		{"[received segs]"},
		{"[segs complete]"},
	};

	if( (false == SDAN_QUIET_MODE) ) //|| (DEVICE_HAS_NO_DATA != device_awake_mailbox->payload.device_data_status) )
	{
		zprintf(LOW_IMPORTANCE,"surenet_device_awake_notification_cb() from %08x", (uint32_t)((device_awake_mailbox->src_addr & 0xffffffff00000000) >> 32));
		zprintf(LOW_IMPORTANCE,"%08x ", (uint32_t)(device_awake_mailbox->src_addr & 0xffffffff));
		// if the minutes count has bit 7 set, then it wants a time update from the server
		// which it's not going to get, but we'll lose the top bit anyway
		zprintf(LOW_IMPORTANCE,"time: %02d:%02d voltage=%dmV rssi=%d ",device_awake_mailbox->payload.device_hours, \
															device_awake_mailbox->payload.device_minutes & 0x7f, \
															device_awake_mailbox->payload.battery_voltage*32, \
															device_awake_mailbox->payload.device_rssi);
		zprintf(LOW_IMPORTANCE,"%s\r\n", data_status[device_awake_mailbox->payload.device_data_status]);
	} else
	{
		if( false == SDAN_VERY_QUIET_MODE )
		{
			// to try and keep track of what's going on, we print a different character for each of the entries:
			// 0-9, a-z, A-X in order; ? if no entry
			int8_t k = convert_mac_to_index(device_awake_mailbox->src_addr);
			if( -1 == k )
			{
				zprintf(LOW_IMPORTANCE, "?");
			} else if( k < 10 )
			{
				zprintf(LOW_IMPORTANCE, "%c", '0' + k);
			} else if( k < 36 )
			{
				zprintf(LOW_IMPORTANCE, "%c", 'a' + (k - 10));
			} else
			{
				zprintf(LOW_IMPORTANCE, "%c", 'A' + (k - 36));
			}
		}
	}
}

/**************************************************************
 * Function Name   : surenet_ping_response_cb
 * Description     : Called when a ping response is received from a device.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
__weak void surenet_ping_response_cb(PING_STATS *ping_result)
{
	uint64_t mac_address = ping_result->mac_address;
	zprintf(MEDIUM_IMPORTANCE, "Ping response from Device: %08X%08X\r\n",(uint32_t)((mac_address&0xffffffff00000000)>>32),	mac_address&0xffffffff);
}

/**************************************************************
 * Function Name   : surenet_blocking_test_cb
 * Description     : Called when a PACKET_BLOCKING_TEST is received over RF
 *                 : This is part of the EMC testing
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
__weak void surenet_blocking_test_cb(uint32_t blocking_test_value)
{
	zprintf(LOW_IMPORTANCE,"Blocking test sequence number = 0x%08x\r\n",blocking_test_value);
}
