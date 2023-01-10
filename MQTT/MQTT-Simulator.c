/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright © 2013-2021 Sureflap Limited.
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
* Filename: MQTT-Simulator.c   
* Author:   Nailed Barnacle
* Purpose:   
*   
*            
**************************************************************************/
#include "MQTT.h"	// Can hold the MQTT_SERVER_SIMULATED #define, if it's not Project-defined.

#ifdef MQTT_SERVER_SIMULATED

#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <compiler.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "MQTT.h"
#include "task.h"
#include "queue.h"

/*Other includes*/
//#include "hermes-app.h"
#include "fsl_debug_console.h"
#include "server_buffer.h"
#include "CLI-Commands-Simulator.h"
#include "hermes-time.h"
#include "FreeRTOS_CLI.h"
#include "DeviceFirmwareUpdate.h"


/* MQTT Debug message control */

#define MQTT_ECHO_RX		false		/* false to stop echoing received_message to terminal*/
#define MQTT_CHATTY			false		/* lots of status and debug info is vomited forth */

void MQTT_handle_set_one_reg (char * str);
void MQTT_handle_set_reg_range (char * str);
void MQTT_handle_send_reg_range (char * str);
void print_MQTT_message_type (uint32_t type, char * str, char * header);
void MQTT_handle_reg_values_index_added (char * str);
void MQTT_handle_thalamus_message (int8_t * str, char * header);
void MQTT_handle_multiple_thalamus_message (char * str, char * header);


extern QueueHandle_t xIncomingMQTTMessageMailbox;  // has multiple entires
extern QueueHandle_t xOutgoingMQTTMessageMailbox;       // has a single item

extern const char * rfid_types[RFID_TYPE_BACKSTOP+1];	// defined in MQTT-Synapse.c

char		msg[256];

// The correlation ID is an odd beast...
// When a message is sent, it contains a correlation ID. If the message is a response to a previous message
// (e.g. an ack, or a message with requested data) then it should contain the same ID as the original message.
// However, there is no requirement for the messages to be either serial or sequential.
// For messages incoming from a paired device, this is handled automatically because they always arrive - even if
// singular - as '126 MESSAGE_HUB_THALAMUS_MULTIPLE' which are handled as individual '127 MSG_HUB_THALAMUS' messages.
// The response to a 127 message includes an automatic use of the associated ID; the ID is not further stored.
// Outgoing messages should theoretically check for a matching ID on their replies, but to do that requires maintaining
// a log of messages targets and IDs. There seems to be no actual requirement to handle this at this point, however,
// so for now we simply increment a global value to ensure we don't send the same ID twice.
int 		corrID = 0;

void process_MQTT_message_for_server(MQTT_MESSAGE * message);
void MQTT_ack_message (uint16_t corrID, uint16_t ack, char * header);


void MQTT_Simulator_task(void *pvParameters)
{
	MQTT_MESSAGE om;		// outgoing message buffer
	
	zprintf(LOW_IMPORTANCE,"MQTT Simulator task started here\r\n");
	
	// send a single message
	while(1)
    {
        // if something to send to Application, populate MQTT_message and call
		
        // Check for messages from the Hub to send to the 'server'
        if (pdPASS == xQueueReceive(xOutgoingMQTTMessageMailbox, &om, 0))
        {
            process_MQTT_message_for_server(&om);
        }        
        vTaskDelay(pdMS_TO_TICKS( 100 ));   // give up CPU for 100ms
    }
}

void MQTT_set_register_message (MSG_TYPE msg_type, HUB_REGISTER_TYPE reg, uint8_t val)
{
    // no correlation ID because it does to the hub
	MQTT_MESSAGE mqtt_message;
	U_MESSAGES	u;

	sprintf (u.set_one_reg.utc, "%08d ", get_UTC());			// time of day
	sprintf (u.set_one_reg.hdr, "1000 ");						// all messages from server are '1000'
	sprintf (u.set_one_reg.msg, "%d ", msg_type);
	sprintf (u.set_one_reg.addval, "%d %x", reg, val);
	strcpy (mqtt_message.message, u.set_one_reg.utc);
	strcat (mqtt_message.message, u.set_one_reg.hdr);
	strcat (mqtt_message.message, u.set_one_reg.msg);
	strcat (mqtt_message.message, u.set_one_reg.addval);		// I'm sure there's a better way to put this message together...

	sprintf (mqtt_message.subtopic, "messages");	

	xQueueSend(xIncomingMQTTMessageMailbox, &mqtt_message, 0);
}


void MQTT_ack_message (uint16_t corrID, uint16_t ack, char * header)
{
    // uses global corrID
	MQTT_MESSAGE mqtt_message;
	U_MESSAGES	u;

	sprintf (u.thal_ack.server_time, "%08x ", get_UTC());
	sprintf (u.thal_ack.hdr, "1000 ");						// all messages from server are '1000'
	sprintf (u.thal_ack.tmsg, "127 ");						// and they're single thalamus messages
	sprintf (u.thal_ack.type, "00 00 ");					// we're always an ack (duh)
	sprintf (u.thal_ack.corr, "%02x %02x ", corrID & 0xff, 
			 					(corrID & 0xff00) >> 8); 	
	sprintf (u.thal_ack.utc, "%02x %02x %02x %02x ", get_UTC() & 0xff,
			 								(get_UTC() & 0xff00) >> 8,
											(get_UTC() & 0xff0000) >> 16,
											(get_UTC() & 0xff000000) >> 24);
	sprintf (u.thal_ack.msg, "%02x %02x ", ack & 0xff, (ack & 0xff) >> 8);
	sprintf (u.thal_ack.err, "00");							// assume error is always no_error

	strcpy (mqtt_message.message, u.thal_ack.server_time);
	strcat (mqtt_message.message, u.thal_ack.hdr);
	strcat (mqtt_message.message, u.thal_ack.tmsg);
	strcat (mqtt_message.message, u.thal_ack.type);
	strcat (mqtt_message.message, u.thal_ack.corr);
	strcat (mqtt_message.message, u.thal_ack.utc);
	strcat (mqtt_message.message, u.thal_ack.msg);
	strcat (mqtt_message.message, u.thal_ack.err);

	// header/subtopic currently looks like "messages/<mac address>"; looks like we just need the mac address
	// this could of course be complete bollocks...
	sprintf (mqtt_message.subtopic, "%s", header);	

	xQueueSend(xIncomingMQTTMessageMailbox, &mqtt_message,0);
#if (true == MQTT_CHATTY)
	zprintf(LOW_IMPORTANCE,"%s - %s\r\n", mqtt_message.subtopic, mqtt_message.message);
#endif
}

/**********************************************************************
* General CLI commands to a hub or device
*
* Earlier routines in this group call later routines until either
* MQTT_generic_device_msg, MQTT_petdoor_device_msg, or MQTT_generic_hub_msg
* are executed, which add the correct metadata to create a legal MQTT
* message which is sent to the message queue
*
* this avoids having to remember lots of code sequences for common
* functions
*
* it also blocks sending nonsense commands to some devices
*
***********************************************************************/

void MQTT_ping_device_message (uint64_t mac, int8_t type)
{
	// simulate a ping request from the server to the specified device
	// this a message to the hub, which then does the work
  	// we need the mac address in little tiny pieces, following the count: 8

  	char arg[32];
	sprintf (arg, "14 %02x %02x %02x %02x %02x %02x %02x %02x %02x",
			 			  type,
						  (uint8_t)((mac >> 56) & 0xff),
						  (uint8_t)((mac >> 48) & 0xff),
						  (uint8_t)((mac >> 40) & 0xff),
						  (uint8_t)((mac >> 32) & 0xff),
						  (uint8_t)((mac >> 24) & 0xff),
						  (uint8_t)((mac >> 16) & 0xff),
						  (uint8_t)((mac >> 8) & 0xff),
						  (uint8_t)(mac & 0xff)
						  );
	MQTT_generic_hub_msg (arg);
}
						  

void MQTT_request_profiles (uint64_t mac)
{
	// request all the profiles
	// requests fastrf first to speed things up
	// to get the table from the pet door, we have to ask for each of 32 profiles directly
	// then we have to turn the data into something sensible - that we do when we receive the data

	if (SURE_PRODUCT_PET_DOOR != device_type_from_mac (mac))
	{
		MQTT_set_fastrf (mac);
		MQTT_generic_device_msg (mac, 1, "11 00 ff");
	}
	else 
	{
		// it's a pet door... ask for a whole load of data
		// fastrf not required here
		MQTT_petdoor_device_msg (mac, 3, "91 224");
	}
}

void MQTT_request_settings (uint64_t mac)
{
	// request all the settings
	// requests fastrf first to speed things up
	MQTT_set_fastrf (mac);
	switch (device_type_from_mac (mac))
	{
		case SURE_PRODUCT_IMPF:
		case SURE_PRODUCT_IDSCF:
		case SURE_PRODUCT_POSEIDON:
			MQTT_generic_device_msg (mac, 1, "09 00 ff");
			break;
		case	SURE_PRODUCT_PET_DOOR:
			zprintf(LOW_IMPORTANCE,"Doesn't work for PetDoor\r\n");
			break;
		case	SURE_PRODUCT_UNKNOWN:
		case	SURE_PRODUCT_HUB:
		case	SURE_PRODUCT_REPEATER:
		case	SURE_PRODUCT_PROGRAMMER:
		case	SURE_PRODUCT_FEEDER_LITE:
			zprintf(LOW_IMPORTANCE,"Unknown\r\n");
			break;
	}
}

void MQTT_request_curfews (uint64_t mac)
{
	// request all the curfews
	// requests fastrf first to speed things up
	MQTT_set_fastrf (mac);
	switch (device_type_from_mac (mac))
	{
		case SURE_PRODUCT_IMPF:
		case SURE_PRODUCT_IDSCF:
		case SURE_PRODUCT_POSEIDON:
			MQTT_generic_device_msg (mac, 1, "12 00 ff");
			break;
		case	SURE_PRODUCT_PET_DOOR:
			MQTT_petdoor_device_msg (mac, 3, "315 208");
			break;
		case	SURE_PRODUCT_UNKNOWN:
		case	SURE_PRODUCT_HUB:
		case	SURE_PRODUCT_REPEATER:
		case	SURE_PRODUCT_PROGRAMMER:
		case	SURE_PRODUCT_FEEDER_LITE:
			zprintf(LOW_IMPORTANCE,"Unknown\r\n");
			break;
	}
}

void MQTT_set_training_mode_3 (uint64_t mac)
{
    // set training mode 3 - special case of MQTT_set_setting
	// only makes sense on an IMPF
	if (SURE_PRODUCT_IMPF == device_type_from_mac (mac))
	{
		MQTT_set_setting (mac, 0x05, 3);
	}
	else
	{
		zprintf(LOW_IMPORTANCE,"Only available for IMPF\r\n");
	}
}

void MQTT_set_fastrf (uint64_t mac)
{
	// set the rf response time to 500ms - special case of MQTT_set_setting
	// but it's different for each device type (different settings)
	switch (device_type_from_mac (mac))
	{
		case SURE_PRODUCT_IMPF:
			MQTT_set_setting (mac, 0x12, 500);
			break;
		case SURE_PRODUCT_IDSCF:
			MQTT_set_setting (mac, 0x0b, 500);
			break;
		case SURE_PRODUCT_POSEIDON:
			MQTT_set_setting (mac, 0x12, 500);
			break;
		case	SURE_PRODUCT_PET_DOOR:
			MQTT_petdoor_device_msg (mac, 1, "39 1");
			break;
		case	SURE_PRODUCT_UNKNOWN:
		case	SURE_PRODUCT_HUB:
		case	SURE_PRODUCT_REPEATER:
		case	SURE_PRODUCT_PROGRAMMER:
		case	SURE_PRODUCT_FEEDER_LITE:
			zprintf(LOW_IMPORTANCE,"Unknown\r\n");
			MQTT_set_setting (mac, 0x12, 500);		// and assume it's a feeder to avoid unpointers later
			break;
	}
	set_unit ((SURE_PRODUCT)device_type_from_mac (mac));
}

void MQTT_get_info (uint64_t mac)
{
	// request the product info
	// requests fastrf first to speed things up
	MQTT_set_fastrf (mac);
	switch (device_type_from_mac (mac))
	{
		case SURE_PRODUCT_IMPF:
		case SURE_PRODUCT_IDSCF:
		case SURE_PRODUCT_POSEIDON:
			MQTT_generic_device_msg (mac, 1, "0b 00 ff");
			break;
		case	SURE_PRODUCT_PET_DOOR:
			MQTT_petdoor_device_msg (mac, 3, "0 13");
			break;
		case	SURE_PRODUCT_UNKNOWN:
		case	SURE_PRODUCT_HUB:
		case	SURE_PRODUCT_REPEATER:
		case	SURE_PRODUCT_PROGRAMMER:
		case	SURE_PRODUCT_FEEDER_LITE:
			zprintf(LOW_IMPORTANCE,"Unknown\r\n");
			break;
	}
}

void MQTT_set_setting (uint64_t mac, uint16_t setting, int32_t value)
{
	// set a device setting to a given value

	char msg[255];

	sprintf (msg, "%02x %02x %02x %02x %02x", (uint8_t) setting,
			 								value & 0xff,
											(value & 0xff00) >> 8,
											(value & 0xff0000) >> 16,
											(value & 0xff000000) >> 24);
	MQTT_generic_device_msg (mac, MESSAGE_CHANGE_SETTING, msg);
}

void MQTT_generic_device_msg (uint64_t mac, uint16_t type, char * pcParameter)
{
	// send a generic message to a paired device, e.g. 16 00 ... 00 00 00 00 00 00 00 00 00 00 to release the lid button
	// the mac address is the full 64-bit address of the paired device
	// the type is the action required (SYNAPSE_MESSAGE_TYPE)
	// pcParameter is a space separated sequence of hex bytes (as pairs); the data which should be sent to suit the action
	// (see the structures in MQTT-Synapse.h for details)

    // we need to be careful about the MAC - we need it the 'right' way round 0x70b3d5f9c0000000 for compatibility with
	// CLI-commands and friends, but we need it byte reversed to talk to the device (this is new code for server compatibility)

	// uses global corrID
	MQTT_MESSAGE mqtt_message;
	U_MESSAGES	u;
	uint64_t	backmac;		// a byte reversed mac

	sprintf (u.thal_msg.server_time, "%08x ", get_UTC());
	sprintf (u.thal_msg.hdr, "1000 ");						// all messages from server are '1000'
	if ((SURE_PRODUCT_IMPF == device_type_from_mac (mac)) ||
		(SURE_PRODUCT_IDSCF == device_type_from_mac (mac)) ||
		(SURE_PRODUCT_POSEIDON == device_type_from_mac (mac)))
	{
		sprintf (u.thal_msg.tmsg, "127 ");				// and they're single thalamus messages, unless they're not...
		sprintf (u.thal_msg.type, "%02x %02x ", type & 0xff, (type & 0xff00) >> 8);
		sprintf (u.thal_msg.corr, "%02x %02x ", corrID & 0xff, (corrID & 0xff00) >> 8); 	
		sprintf (u.thal_msg.utc, "%02x %02x %02x %02x ", get_UTC() & 0xff,
												(get_UTC() & 0xff00) >> 8,
												(get_UTC() & 0xff0000) >> 16,
												(get_UTC() & 0xff000000) >> 24);
		strcpy (mqtt_message.message, u.thal_msg.server_time);
		strcat (mqtt_message.message, u.thal_msg.hdr);
		strcat (mqtt_message.message, u.thal_msg.tmsg);
		strcat (mqtt_message.message, u.thal_msg.type);
		strcat (mqtt_message.message, u.thal_msg.corr);
		strcat (mqtt_message.message, u.thal_msg.utc);
		strcat (mqtt_message.message, pcParameter);
		corrID++;
		// the mac address so we go to the right place
		backmac = swap64 (mac);
		sprintf (mqtt_message.subtopic, "%08x%08x", (uint32_t)(backmac >> 32), (uint32_t)(backmac & 0xffffffff));	

		xQueueSend(xIncomingMQTTMessageMailbox, &mqtt_message,0);
#if (true == MQTT_CHATTY)
		zprintf(LOW_IMPORTANCE,"%s - %s\r\n", mqtt_message.subtopic, mqtt_message.message);
		zprintf(LOW_IMPORTANCE,"Got all generic with %08x %08x, kthxbye\r\n", (uint32_t)(mac >> 32), (uint32_t)(mac & 0xffffffff));
#endif
	}
	else
	{
		zprintf(LOW_IMPORTANCE,"This instruction only works with thalamus devices!\r\n");
	}
}

void MQTT_petdoor_device_msg (uint64_t mac, uint16_t type, char * pcParameter)
{
	MQTT_MESSAGE mqtt_message;
	U_MESSAGES	u;
	uint64_t	backmac;		// a byte reversed mac

	sprintf (u.thal_msg.server_time, "%08x ", get_UTC());
	sprintf (u.thal_msg.hdr, "1000 ");						// all messages from server are '1000'
	if (SURE_PRODUCT_PET_DOOR == (device_type_from_mac (mac)))
	{
		// for a pet door, we just use the first parameter
		sprintf (u.thal_msg.type, "%02x ", type & 0xff);
		strcpy (mqtt_message.message, u.thal_msg.server_time);
		strcat (mqtt_message.message, u.thal_msg.hdr);
		strcat (mqtt_message.message, u.thal_msg.type);
		strcat (mqtt_message.message, pcParameter);
		// the mac address so we go to the right place
		backmac = swap64 (mac);
		sprintf (mqtt_message.subtopic, "%08x%08x", (uint32_t)(backmac >> 32), (uint32_t)(backmac & 0xffffffff));	

		xQueueSend(xIncomingMQTTMessageMailbox, &mqtt_message,0);

#if (true == MQTT_CHATTY)
		zprintf(LOW_IMPORTANCE,"%s - %s\r\n", mqtt_message.subtopic, mqtt_message.message);
		zprintf(LOW_IMPORTANCE,"Got all generic with petdoor %08x %08x, kthxbye\r\n", (uint32_t)(mac >> 32), (uint32_t)(mac & 0xffffffff));
#endif
	}
	else
	{
		zprintf(LOW_IMPORTANCE,"This instruction only works with pet doors!\r\n");
	}
}

void MQTT_generic_hub_msg (char * pcParameter)
{
	// send a generic message to the hub	
	// hub messages have the subtopic 'messages' and no mac address or correlation ID
	// pcParameter is a string containing all the necessary data
	
	MQTT_MESSAGE mqtt_message;

	sprintf (mqtt_message.message, "%08d ", get_UTC());	// time of day
	strcat (mqtt_message.message, "1000 ");				// all messages from server are '1000'
	strcat (mqtt_message.message, pcParameter);			// I'm sure there's a better way to put this message together...

	sprintf (mqtt_message.subtopic, "messages");	

	xQueueSend(xIncomingMQTTMessageMailbox, &mqtt_message,0);
#if (true == MQTT_CHATTY)
	zprintf(LOW_IMPORTANCE,"%s - %s\r\n", mqtt_message.subtopic, mqtt_message.message);
#endif
}

void process_MQTT_message_for_server(MQTT_MESSAGE * message)
{
 	// the main purpose of this is to reflect a received message to the application,
	// informing the application that the message can be removed from the message queue

	char *		ptr;
	uint64_t	mac;
	uint32_t	msg_type;
	char *		token;
	char		cmac[20];		// room for the subtopic
	
#if	MQTT_ECHO_RX 	
	zprintf(LOW_IMPORTANCE,"MQTT received xmessage: %s - %s\r\n", message->subtopic, message->message);
#endif
	
	xQueueSend(xIncomingMQTTMessageMailbox, message, 0);	// echo it

	// now see whether the message was from a device or the hub itself
	// is there a mac address present?
	// if the subtopic length is zero, it's from the hub; if 17, it's a mac with a leading /
	
	ptr = &message->subtopic[0];
	if (0 != *ptr)
	{
		// not a null
		if ('/' == *ptr)
		{
			// starts with a / so skip it
			ptr++;	
		}
		strcpy (cmac, ptr);
		
		mac = strtoull (cmac, NULL, 16);						// get the mac
		set_unit ((SURE_PRODUCT)device_type_from_mac (swap64(mac)));	// and device type
		// skip past timestamp and correlation bytes
		ptr = message->message;
		ptr = strchr (++ptr, ' ');								// timestamp
		ptr = strchr (++ptr, ' ');								// correlation byte
		sscanf (ptr, "%d", &msg_type);							// read the message type
		ptr = strchr (++ptr, ' ');								// skip the message type
		ptr++;													// and the following space
		print_MQTT_message_type (msg_type, ptr, cmac);			// tell the world about it
	}
}

void print_MQTT_message_type (uint32_t type, char * str, char * header)
{
	// meh, non-contigious list
	switch (type)
	{
		case MSG_NONE:
			zprintf(LOW_IMPORTANCE,"MSG_NONE");
			break;
		case MSG_SET_ONE_REG:
			MQTT_handle_set_one_reg (str);
			break;
		case MSG_SET_REG_RANGE:
			MQTT_handle_set_reg_range (str);
			break;
		case MSG_SEND_REG_RANGE:
			MQTT_handle_send_reg_range (str);
			break;
		case MSG_REG_VALUES:
			zprintf(LOW_IMPORTANCE,"MSG_REG_VALUES");
			break;
		case MSG_SEND_REG_DUMP:
			zprintf(LOW_IMPORTANCE,"MSG_SEND_REG_DUMP");
			break;
		case MSG_REPROGRAM_HUB:
			zprintf(LOW_IMPORTANCE,"MSG_REPROGRAM_HUB");
			break;
		case MSG_FULL_REG_SET:
			zprintf(LOW_IMPORTANCE,"MSG_FULL_REG_SET");
			break;
		case MSG_MOVEMENT_EVENT:
			zprintf(LOW_IMPORTANCE,"MSG_MOVEMENT_EVENT");
			break;
		case MSG_GET_REG_RANGE:
			zprintf(LOW_IMPORTANCE,"MSG_GET_REG_RANGE");
			break;
		case MSG_HUB_UPTIME:
			zprintf(LOW_IMPORTANCE,"MSG_HUB_UPTIME");
			break;
		case MSG_FLASH_SET:
			zprintf(LOW_IMPORTANCE,"MSG_FLASH_SET");
			break;
		case MSG_HUB_REBOOT:
			zprintf(LOW_IMPORTANCE,"MSG_HUB_REBOOT");
			break;
		case MSG_HUB_VERSION_INFO:
			zprintf(LOW_IMPORTANCE,"MSG_HUB_VERSION_INFO");
			break;
		case MSG_WEB_PING:
			zprintf(LOW_IMPORTANCE,"MSG_WEB_PING");
			break;
		case MSG_HUB_PING:
			zprintf(LOW_IMPORTANCE,"MSG_HUB_PING");
			break;
		case MSG_LAST_WILL:
			zprintf(LOW_IMPORTANCE,"MSG_LAST_WILL");
			break;
		case MSG_SET_DEBUG_MODE:
			zprintf(LOW_IMPORTANCE,"MSG_SET_DEBUG_MODE");
			break;
		case MSG_DEBUG_DATA:
			zprintf(LOW_IMPORTANCE,"MSG_DEBUG_DATA");
			break;
		case MSG_HUB_THALAMUS_MULTIPLE:
			MQTT_handle_multiple_thalamus_message (str, header);
			break;
		case MSG_HUB_THALAMUS:
			zprintf(LOW_IMPORTANCE,"MSG_HUB_THALAMUS");
			break;
		case MSG_REG_VALUES_INDEX_ADDED:
			MQTT_handle_reg_values_index_added (str);
			break;
		default:
			zprintf(HIGH_IMPORTANCE,"WTF? This shouldn't happen!");
			break;
	}
}

void MQTT_handle_set_one_reg (char * str)
{
	zprintf(LOW_IMPORTANCE,"MSG_SET_ONE_REG\r\n");
	// from pet door
	// expect: reg address, value... all decimal
	int		reg;
	int		val;
	
	// get the parameters
	sscanf(str, "%d %d", &reg, &val);
	zprintf(LOW_IMPORTANCE,"\tRegister:  %d\r\n", reg);
	zprintf(LOW_IMPORTANCE,"\tValue:     %d\r\n", val);		// maybe a '1' before this and a parity after?
}	

void MQTT_handle_set_reg_range (char * str)
{
	zprintf(LOW_IMPORTANCE,"MSG_SET_REG_RANGE\r\n");
	// from pet door
	// expect: reg address, multiple values, all decimal
	int		reg;
	char * 	space;
	
	// get the parameters
	sscanf(str, "%d", &reg);
	zprintf(LOW_IMPORTANCE,"\tRegister:  %d\r\n", reg);
	// now hope that the message is well-formed...
	// move past the first space 
	space = strchr (str, ' ');
	space++;
	zprintf(LOW_IMPORTANCE,"\tData:      [%s]\r\n", space);
}	

void MQTT_handle_send_reg_range (char * str)
{
	zprintf(LOW_IMPORTANCE,"MSG_SEND_REG_RANGE");
	// from pet door
	// expect: reg address, number of values, all decimal
	int		reg;
	char * 	space;

	// get the parameters
	sscanf(str, "%d", &reg);
	zprintf(LOW_IMPORTANCE,"\tRegister:  %d\r\n", reg);
	// now hope that the message is well-formed...
	// move past the first space 
	space = strchr (str, ' ');
	space++;
	zprintf(LOW_IMPORTANCE,"\tData:      [%s]\r\n", space);
}


static uint8_t reverse (uint8_t b) 
{
	// nicked this of stack overflow
	// reverses the order of the bits in a byte
	// probably by magic
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

void MQTT_handle_reg_values_index_added (char * str)
{
	// this is a pain because of the individual register entries
	// some have magical powers...
#define STORED_TYPE_1	91
#define STORED_TYPE_2	98
#define STORED_TYPE_3	105
#define STORED_TYPE_4	112
#define STORED_TYPE_5	119
#define STORED_TYPE_6	126
#define STORED_TYPE_7	133
#define STORED_TYPE_8	140
#define STORED_TYPE_9	147
#define STORED_TYPE_10	154
#define STORED_TYPE_11	161
#define STORED_TYPE_12	168
#define STORED_TYPE_13	175
#define STORED_TYPE_14	182
#define STORED_TYPE_15	189
#define STORED_TYPE_16	196
#define STORED_TYPE_17	203
#define STORED_TYPE_18	210
#define STORED_TYPE_19	217
#define STORED_TYPE_20	224
#define STORED_TYPE_21	231
#define STORED_TYPE_22	238
#define STORED_TYPE_23	245
#define STORED_TYPE_24	252
#define STORED_TYPE_25	259
#define STORED_TYPE_26	266
#define STORED_TYPE_27	273
#define STORED_TYPE_28	280
#define STORED_TYPE_29	287
#define STORED_TYPE_30	294
#define STORED_TYPE_31	301
#define STORED_TYPE_32	308
	
#define DEFAULT_ENTRY_CTL_BLK		315
#define GLOBAL_ENTRY_CTL_BLK		321
#define PER_ANIMAL_ENTRY_CTL_BLK_1	327
#define PER_ANIMAL_ENTRY_CTL_BLK_2	333
#define PER_ANIMAL_ENTRY_CTL_BLK_3	339
#define PER_ANIMAL_ENTRY_CTL_BLK_4	345
#define PER_ANIMAL_ENTRY_CTL_BLK_5	351
#define PER_ANIMAL_ENTRY_CTL_BLK_6	357
#define PER_ANIMAL_ENTRY_CTL_BLK_7	363
#define PER_ANIMAL_ENTRY_CTL_BLK_8	369
#define PER_ANIMAL_ENTRY_CTL_BLK_9	375
#define PER_ANIMAL_ENTRY_CTL_BLK_10	381
#define PER_ANIMAL_ENTRY_CTL_BLK_11	387
#define PER_ANIMAL_ENTRY_CTL_BLK_12	393
#define PER_ANIMAL_ENTRY_CTL_BLK_13	399
#define PER_ANIMAL_ENTRY_CTL_BLK_14	405
#define PER_ANIMAL_ENTRY_CTL_BLK_15	411
#define PER_ANIMAL_ENTRY_CTL_BLK_16	417
#define PER_ANIMAL_ENTRY_CTL_BLK_17	423
#define PER_ANIMAL_ENTRY_CTL_BLK_18	429
#define PER_ANIMAL_ENTRY_CTL_BLK_19	435
#define PER_ANIMAL_ENTRY_CTL_BLK_20 441
#define PER_ANIMAL_ENTRY_CTL_BLK_21	447
#define PER_ANIMAL_ENTRY_CTL_BLK_22	453
#define PER_ANIMAL_ENTRY_CTL_BLK_23	459
#define PER_ANIMAL_ENTRY_CTL_BLK_24	465
#define PER_ANIMAL_ENTRY_CTL_BLK_25	471
#define PER_ANIMAL_ENTRY_CTL_BLK_26	477
#define PER_ANIMAL_ENTRY_CTL_BLK_27	483
#define PER_ANIMAL_ENTRY_CTL_BLK_28	489
#define PER_ANIMAL_ENTRY_CTL_BLK_29	495
#define PER_ANIMAL_ENTRY_CTL_BLK_30	501
#define PER_ANIMAL_ENTRY_CTL_BLK_31	507
#define PER_ANIMAL_ENTRY_CTL_BLK_32	513
#define EXIT_CTL_BLK				519
	

	
	zprintf(LOW_IMPORTANCE,"MSG_REG_VALUES_INDEX_ADDED\r\n");
	// from pet door
	// expect: index, reg address, num of values, value... all decimal
	int				index;
	int				reg;
	int				count;
	char * 			space;
	RFID_MICROCHIP	chip;
	
	// get the parameters
	sscanf(str, "%d %d %d", &index, &reg, &count);
	// now hope that the message is well-formed...
	// move past the first three spaces (try and pretend you haven't seen this!)
	space = strchr (str, ' ');
	space++;
	space = strchr (space, ' ');
	space++;
	space = strchr (space, ' ');
	space++;
	// now we have some data type specific stuff... this is going to be 'orrible...
	switch (reg)
	{
		case STORED_TYPE_1:
		case STORED_TYPE_2:
		case STORED_TYPE_3:
		case STORED_TYPE_4:
		case STORED_TYPE_5:
		case STORED_TYPE_6:
		case STORED_TYPE_7:
		case STORED_TYPE_8:
		case STORED_TYPE_9:
		case STORED_TYPE_10:
		case STORED_TYPE_11:
		case STORED_TYPE_12:
		case STORED_TYPE_13:
		case STORED_TYPE_14:
		case STORED_TYPE_15:
		case STORED_TYPE_16:
		case STORED_TYPE_17:
		case STORED_TYPE_18:
		case STORED_TYPE_19:
		case STORED_TYPE_20:
		case STORED_TYPE_21:
		case STORED_TYPE_22:
		case STORED_TYPE_23:
		case STORED_TYPE_24:
		case STORED_TYPE_25:
		case STORED_TYPE_26:
		case STORED_TYPE_27:
		case STORED_TYPE_28:
		case STORED_TYPE_29:
		case STORED_TYPE_30:
		case STORED_TYPE_31:
		case STORED_TYPE_32:
			// we're looking at one of the profiles, so get the chip data from 'space' <<-- silly name, never mind
			// profiles are delivered as 7-character packages; we're already pointing at the first
			// petdoor profiles are numbered 1-32
			mqtt_synapse_printf ("\tChip %d - number: ", ((reg - STORED_TYPE_1) / 7) + 1); // magic 7 is size of type and number
			chip.type = (RFID_TYPE) atoi (space);		
			for (uint32_t q = 0; q < RFID_MICROCHIP_NUMBER_LENGTH; q++)
			{
				space = strchr (space, ' ');			// advance to next space
				space++;
				chip.number[q] = (uint8_t) atoi (space);
			}
			for (uint32_t q = 0; q < RFID_MICROCHIP_NUMBER_LENGTH; q++)
			{
				mqtt_synapse_printf ("%02x", reverse (chip.number[q]));
			}
			mqtt_synapse_printf (", %s\r\n", rfid_types[chip.type]);
			break;
		
		case DEFAULT_ENTRY_CTL_BLK:
		case GLOBAL_ENTRY_CTL_BLK:
		case PER_ANIMAL_ENTRY_CTL_BLK_1:
		case PER_ANIMAL_ENTRY_CTL_BLK_2:
		case PER_ANIMAL_ENTRY_CTL_BLK_3:
		case PER_ANIMAL_ENTRY_CTL_BLK_4:
		case PER_ANIMAL_ENTRY_CTL_BLK_5:
		case PER_ANIMAL_ENTRY_CTL_BLK_6:
		case PER_ANIMAL_ENTRY_CTL_BLK_7:
		case PER_ANIMAL_ENTRY_CTL_BLK_8:
		case PER_ANIMAL_ENTRY_CTL_BLK_9:
		case PER_ANIMAL_ENTRY_CTL_BLK_10:
		case PER_ANIMAL_ENTRY_CTL_BLK_11:
		case PER_ANIMAL_ENTRY_CTL_BLK_12:
		case PER_ANIMAL_ENTRY_CTL_BLK_13:
		case PER_ANIMAL_ENTRY_CTL_BLK_14:
		case PER_ANIMAL_ENTRY_CTL_BLK_15:
		case PER_ANIMAL_ENTRY_CTL_BLK_16:
		case PER_ANIMAL_ENTRY_CTL_BLK_17:
		case PER_ANIMAL_ENTRY_CTL_BLK_18:
		case PER_ANIMAL_ENTRY_CTL_BLK_19:
		case PER_ANIMAL_ENTRY_CTL_BLK_20:
		case PER_ANIMAL_ENTRY_CTL_BLK_21:
		case PER_ANIMAL_ENTRY_CTL_BLK_22:
		case PER_ANIMAL_ENTRY_CTL_BLK_23:
		case PER_ANIMAL_ENTRY_CTL_BLK_24:
		case PER_ANIMAL_ENTRY_CTL_BLK_25:
		case PER_ANIMAL_ENTRY_CTL_BLK_26:
		case PER_ANIMAL_ENTRY_CTL_BLK_27:
		case PER_ANIMAL_ENTRY_CTL_BLK_28:
		case PER_ANIMAL_ENTRY_CTL_BLK_29:
		case PER_ANIMAL_ENTRY_CTL_BLK_30:
		case PER_ANIMAL_ENTRY_CTL_BLK_31:
		case PER_ANIMAL_ENTRY_CTL_BLK_32:
		case EXIT_CTL_BLK:
			// these are the entry control blocks aka curfews
			if (EXIT_CTL_BLK != reg)
			{
				mqtt_synapse_printf ("Entry ");
			}
			else
			{
				mqtt_synapse_printf ("Exit ");
			}
			mqtt_synapse_printf ("control block ");
			switch (reg)
			{
				case DEFAULT_ENTRY_CTL_BLK:
					mqtt_synapse_printf ("- Default\r\n");
					break;
				case GLOBAL_ENTRY_CTL_BLK:
					mqtt_synapse_printf ("- Global\r\n");
					break;
				case EXIT_CTL_BLK:
					mqtt_synapse_printf ("\r\n");
					break;
				default:
					mqtt_synapse_printf ("- Animal %d\r\n", ((reg - PER_ANIMAL_ENTRY_CTL_BLK_1) / 6) + 1);
					break;
			}
			mqtt_synapse_printf ("\tPermission:  %d\r\n", (uint8_t) atoi (space));		
			space = strchr (space, ' ');			// advance to next space
			space++;
			mqtt_synapse_printf ("\tLock time:   %02d:", (uint8_t) atoi (space));		
			space = strchr (space, ' ');			// advance to next space
			space++;
			mqtt_synapse_printf ("%02d\r\n", (uint8_t) atoi (space));		
			space = strchr (space, ' ');			// advance to next space
			space++;
			mqtt_synapse_printf ("\tUnlock time: %02d:", (uint8_t) atoi (space));		
			space = strchr (space, ' ');			// advance to next space
			space++;
			mqtt_synapse_printf ("%02d\r\n", (uint8_t) atoi (space));		
			space = strchr (space, ' ');			// advance to next space
			space++;
			mqtt_synapse_printf ("\tDelay time:  %d\r\n", (uint8_t) atoi (space));		
			space = strchr (space, ' ');			// advance to next space
			space++;
			break;

		default:
			mqtt_synapse_printf ("\tIndex:     %d\r\n", index);
			mqtt_synapse_printf ("\tRegister:  %d\r\n", reg);
			mqtt_synapse_printf ("\tCount:     %d\r\n", count);
			mqtt_synapse_printf ("\tData:      [%s]\r\n", space);
			break;
	}
}	

void MQTT_handle_thalamus_message (int8_t * str, char * header)
{
	// decode a single thalamus message
	// the first character is the message length; the data thereafter is message dependent
	//zprintf(LOW_IMPORTANCE,"MSG_HUB_THALAMUS\r\n");	
	
	int8_t		length = *str++;				// we don't include the count
	int16_t 	type = (uint8_t)*str + ((uint8_t)*(str+1) * 256);		// make sure we save the type and the correlation words
	uint16_t	corr = (uint8_t)*(str+2) +((uint8_t)*(str+3) * 256);
	
	set_header (str);
	set_payload (str + 8, length - 8);
	message_handler (type);						// print the message info
	MQTT_ack_message (corr, type, header);		// we reply with matching correlation word
}

void MQTT_handle_multiple_thalamus_message (char * str, char * header)
{
	// decode a multiple thalamus message
	// this contains sequential single thalamus messages in ascii format
	// pointed to by str
	
	// however, we need to acknowledge each message seperately, which means
	// we need to maintain the correlation type and ack type
	
	int8_t *	ptr;
	int8_t		binary_msg[MAX_INCOMING_MQTT_MESSAGE_SIZE_SMALL];	// three times as big as it needs to be
	char * 		token;
	char *		next;
	//zprintf(LOW_IMPORTANCE,"MSG_HUB_THALAMUS_MULTIPLE\r\n");
	// we want to convert this string of text into sequential hex bytes
	// there should always be at least one entry, but...
	
	ptr = binary_msg;
	next = str;

	// we should be looking at an ascii hex length byte
	long temp = 0;
	long count = 0;
	while (0 != (count = strtol (next, &next, 16)))
	{
		// for each length byte, read that many parameters
		*ptr++ = (int8_t) count;
		for (uint8_t q = 0; q < (uint8_t)count; q++)
		{
			temp = strtol (next, &next, 16);
			*ptr++ = (int8_t) temp;
		}
	}
	
	*ptr = 0;		// mark the end of the binary list
					// there is no count of entries in e.g. multiple thalamus messages
	ptr = binary_msg;
	while (*ptr != '\0')
	{
		// the first byte is the message length
		MQTT_handle_thalamus_message (ptr, header);
		ptr += *ptr;		// add the message length
		ptr ++;				// which doesn't include the length byte so compensate
	}
}

#endif	// MQTT_SERVER_SIMULATED
