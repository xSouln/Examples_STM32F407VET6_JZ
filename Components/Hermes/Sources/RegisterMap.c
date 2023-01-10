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
* Filename: RegisterMap.c
* Author:   Chris Cowdery 14/10/2019
* Purpose:  Handles the Hub Register Map.
*
*
*
**************************************************************************/

#include "hermes.h"
#include "BankManager.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
/* Other includes */
#include "hermes-app.h"
#include "hermes-time.h"
#include "devices.h"    // for DEVICE_TYPE_HUB and DEVICE_STATUS
#include "SureNet-Interface.h"
#include "RegisterMap.h"
#include "buildnumber.h"
#include "Server_Buffer.h"
#include "message_parser.h"
#include "leds.h"
#include "utilities.h"
#include "wolfssl/wolfcrypt/sha256.h"

// Global variables
uint8_t						hub_debug_mode;
extern QueueHandle_t		xSendDeviceStatusMailbox;
extern QueueHandle_t		xSendDeviceTableMailbox;
extern EventGroupHandle_t	xSurenet_EventGroup;
extern QueueHandle_t		xLedMailbox;

// Local #defines
#define INITIAL_NUMBER_OF_EXPOSED_DEVICES	60	// this is how many entries of the DEVICE_TABLE
												// are exposed to the server on reset.
// Local variables
uint8_t number_of_exposed_devices;

// Local Functions
static void HubReg_Handle_Device_Status(void);
static void Hub_Registers_Send_Range(uint32_t start_index, uint32_t count);

uint8_t	reg_read(uint16_t address);
void	null_write_fn(uint16_t address,uint8_t uint8_t);
uint8_t	null_read_fn(uint16_t address);
uint8_t	checksum_read(uint16_t address);
void	pairing_handler_write(uint16_t address,uint8_t value);
uint8_t	pairing_handler_read(uint16_t address);
uint8_t	get_debug_mode(uint16_t address);
void	led_mode_write(uint16_t address,uint8_t value);
void	remote_reboot(uint16_t address,uint8_t value);
void	clear_pairing_table(uint16_t address,uint8_t value);
void	delete_pairing_table_entry(uint16_t address,uint8_t value);
uint8_t	radio_channel_noise_floor(uint16_t address);
uint8_t	get_surenet_channel_number(uint16_t address);
void	set_surenet_channel_number(uint16_t address,uint8_t value);
uint8_t	app_error_read(uint16_t address);
uint8_t	hub_uptime_read(uint16_t address);
uint8_t	bank_desc_read(uint16_t address);
uint8_t	get_num_pairs(uint16_t address);
uint8_t	getHwVersion(uint16_t address);
void	initiate_channel_hop(uint16_t address,uint8_t value);
void	clearHubBank(void);
uint8_t	led_mode_read(uint16_t address);
void	change_device_table_size(uint16_t address,uint8_t value);
uint8_t	trigger_get_device_table(uint16_t address);
void	get_device_table(bool trigger);
void	update_register_map_device_table_size(uint8_t value);
uint8_t	HRM_Read_Image_Hash(uint16_t address);

// Register Name						Read Handler					Write Handler 				Value						Update Flag		End of Block	Hashed}
T_HUB_REGISTER_ENTRY hubRegisterBank[HR_LAST_ELEMENT] = {
  [HR_DEVICE_TYPE] 						= {	reg_read,					null_write_fn,				DEVICE_TYPE_HUB,			false,			false,			true},	// 0
  [HR_NUM_REGISTERS_LOW]				= {	reg_read,					null_write_fn,				0,							false,			false,			true},
  [HR_NUM_REGISTERS_HIGH]				= {	reg_read,					null_write_fn,				0,							false,			false,			true},
  [HR_HUB_FIRMWARE_MAJOR_LOW]			= {	reg_read,					null_write_fn,				(SVN_REVISION & 0xff),		false,			false,			true},
  [HR_HUB_FIRMWARE_MAJOR_HIGH]			= {	reg_read,					null_write_fn,				((SVN_REVISION>>8) & 0xff),	false,			false,			true},
  [HR_HUB_FIRMWARE_MINOR_LOW]			= {	reg_read,					null_write_fn,				(BUILD_MARK & 0xff),		false,			false,			true},
  [HR_HUB_FIRMWARE_MINOR_HIGH]			= {	reg_read,					null_write_fn,				((BUILD_MARK>>8) & 0xff),	false,			false,			true},
  [HR_HUB_HARDWARE_MAJOR_LOW]			= {	getHwVersion,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_HARDWARE_MAJOR_HIGH]			= {	reg_read,					null_write_fn,				(HUB_HARDWARE_REVISION_MAJOR & 0xff),	false,	false,		true},
  [HR_HUB_BOOTLOADER_MAJOR]				= {	reg_read,					null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BOOTLOADER_MINOR]				= {	reg_read,					null_write_fn,				0,							false,			false,			true},	// 10
  [HR_HUB_MAX_MESSAGE_SIZE_LOW] 		= {	null_read_fn,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_MAX_MESSAGE_SIZE_HIGH]		= {	null_read_fn,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_FIRMWARE_CHECKSUM_LOW]		= {	checksum_read,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_FIRMWARE_CHECKSUM_HIGH] 		= {	checksum_read,				null_write_fn,				0,							false,			true,			true},	// 14
  [HR_PAIRING_STATE] 					= {	pairing_handler_read,		pairing_handler_write,		0,							false,			false,			false},
  [HR_DEBUG_MODE]						= {	get_debug_mode,				HubReg_SetDebugMode,		0,							false,			false,			false},
  [HR_PAIRING_TIMER_VALUE]				= {	null_read_fn,				null_write_fn,				0,							false,			false,			false},
  [HR_LEDS_MODE]						= {	reg_read,					led_mode_write,				9,							false,			false,			false},
  [HR_REMOTE_REBOOT]					= {	null_read_fn,				remote_reboot,				0,							false,			false,			false},
  [HR_CLEAR_PAIRING_TABLE]				= {	null_read_fn,				clear_pairing_table,		0,							false,			false,			false},	// 20
  [HR_INITIATE_CHANNEL_HOP]				= {	null_read_fn,				initiate_channel_hop,		0,							false,			false,			false},
  [HR_DELETE_CONNECTION_TABLE_ENTRY]	= {	null_read_fn,				delete_pairing_table_entry,	0,							false,			false,			false},
  [HR_SURENET_CHANNEL_NOISE_FLOOR]		= {	radio_channel_noise_floor,	null_write_fn,				0,							false,			false,			false},
  [HR_SURENET_CHANNEL_NUMBER]			= {	get_surenet_channel_number,	set_surenet_channel_number,	0,							false,			false,			false},
  [HR_APPLICATION_ERROR_LOW]			= {	app_error_read,				null_write_fn,				0,							false,			false,			true},
  [HR_APPLICATION_ERROR_HIGH]			= {	app_error_read,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_UPTIME_LOW]					= {	hub_uptime_read,			null_write_fn,				1,							false,			false,			false},
  [HR_HUB_UPTIME_BITS_15_8]				= {	hub_uptime_read,			null_write_fn,				2,							false,			false,			false},
  [HR_HUB_UPTIME_BITS_23_16]			= {	hub_uptime_read,			null_write_fn,				3,							false,			false,			false},
  [HR_HUB_UPTIME_HIGH]					= {	hub_uptime_read,			null_write_fn,				4,							false,			false,			false},	// 30
  [HR_HUB_BANK_A_MARK]					= {	bank_desc_read,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_A_WDOG_RESETS]			= {	bank_desc_read,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_B_MARK]					= {	bank_desc_read,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_B_WDOG_RESETS]			= {	bank_desc_read,				null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_A_HASH_LOW]				= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_A_HASH_BITS_15_8]		= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_A_HASH_BITS_23_16]		= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_A_HASH_HIGH]				= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_B_HASH_LOW]				= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_B_HASH_BITS_15_8]		= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},	// 40
  [HR_HUB_BANK_B_HASH_BITS_23_16]		= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},
  [HR_HUB_BANK_B_HASH_HIGH]				= {	HRM_Read_Image_Hash,		null_write_fn,				0,							false,			false,			true},
  [HR_DEVICE_TABLE_SIZE]				= {	reg_read,					change_device_table_size,	0,							false,			false,			true},
  [HR_DEVICE_TABLE_NUMBER_CONNECTIONS]	= {	get_num_pairs,				null_write_fn,				0,							false,			true,			true},
// we'll stuff the connection table entries programatically in clearHubBank()
};

// We use this enum to understand pairing mode requests from the Server
typedef enum
{
	PAIRING_OFF,		// used to be PAIRING_NEAR_FIELD and is used to indicate turning off pairing mode
	PAIRING_DISTANCE,	// NOT USED
	PAIRING_MANUAL_SERVER_INITIATED = 0x02,	// Server has sent down command to put unit into pairing mode
	PAIRING_MANUAL_HUB_INITIATED = 0x82,	// User has pushed button on the bottom of the Hub
} HUB_PAIRING_MODE;

/**************************************************************
 * Function Name   : trigger_get_device_table
 * Description     : Gets an up to date copy of the Device Table.
 *				   : Triggered by a read of the first entry in the Device Table
 **************************************************************/
uint8_t trigger_get_device_table(uint16_t address)
{
	get_device_table(true);	// update register map
	return hubRegisterBank[address].value;
}

void HubReg_Handle_Messages(void)
{
	if( uxQueueMessagesWaiting(xSendDeviceTableMailbox) > 0 )
	{	// Device Table waiting for us. Handle without requesting again.
		get_device_table(false);
	}

	if( uxQueueMessagesWaiting(xSendDeviceStatusMailbox) > 0 )
	{
		HubReg_Handle_Device_Status();
	}
}

/**************************************************************
 * Function Name   : get_device_table
 * Description     : Send a message to SureNet to get the Device Table
 **************************************************************/
void get_device_table(bool trigger)
{
	bool dummy = true;
	uint32_t i;
	uint8_t dev_table[sizeof(DEVICE_STATUS) * MAX_NUMBER_OF_DEVICES];

	xQueueReceive(xSendDeviceTableMailbox, &dev_table, 0); // ensure mailbox is empty

	// send a message to SureNet to get a message back with the Device Table in it
	if( true == trigger ){ xEventGroupSetBits(xSurenet_EventGroup, SURENET_GET_DEVICE_TABLE); }

	// Give it plenty of time as SureNet may be busy (e.g. after power-up with a large fleet of Devices)
    if( pdPASS == xQueueReceive(xSendDeviceTableMailbox, &dev_table, pdMS_TO_TICKS(10000)) )
	{	// a copy of the DEVICE_TABLE is now in dev_table, so we copy it into the register map
		for (i = 0; i < (sizeof(DEVICE_STATUS)*MAX_NUMBER_OF_DEVICES); i++)
		{
			hubRegisterBank[i + HR_DEVICE_TABLE].value = dev_table[i];
		}
	} else
	{ // This might happen if SureNet is too busy to deal with the request above.
		zprintf(MEDIUM_IMPORTANCE, "Device Table Read Failed.\r\n");
	}
}

/**************************************************************
 * Function Name   : HubReg_Handle_Device_Status
 * Description     : Checks to see if a line in device_status differs
 *                 : from it's corresponding line in hubRegisterBank.
 *                 : If so, copy the entire line across
 *                 : If request.limited == false, update every updateWebFlag
 *                 : If request.limited == true, only update the
 *                 : updateWebFlags for the first DEVICE_STATUS_LAST_HEARD_POS
 *                 : bytes which excludes .last_heard_from
 **************************************************************/
static void HubReg_Handle_Device_Status(void)
{
	DEVICE_STATUS_REQUEST	request;
	bool require_update = false;
	if( pdPASS == xQueueReceive(xSendDeviceStatusMailbox, &request, 0) )
	{
		uint8_t* ptr = (uint8_t*)&request.device_status;
		int i, j = HR_DEVICE_TABLE + request.line * sizeof(DEVICE_STATUS);

		for( i = 0; i < sizeof(DEVICE_STATUS); i++ )
		{
			if( hubRegisterBank[j+i].value != ptr[i] )
			{
				require_update = true;	// the two lines differ.
			}
		}

		if (true == require_update)	// the lines differ so update hubRegisterBank
		{
			for( i = 0; i < sizeof(DEVICE_STATUS); i++)
			{
				hubRegisterBank[j+i].value = ptr[i];
				if( (false == request.limited) || (i < DEVICE_STATUS_LAST_HEARD_POS) )
				{
					hubRegisterBank[j+i].updateWebFlag = true; // In limited mode, only update the non-time parts.
				}
			}
		}
		get_num_pairs(HR_DEVICE_TABLE_NUMBER_CONNECTIONS);  //ensure register 44 is properly initialised
		hubRegisterBank[HR_DEVICE_TABLE_NUMBER_CONNECTIONS].updateWebFlag = true; // need to tell the server about this
	} else
	{
		zprintf(MEDIUM_IMPORTANCE, "HubReg_Handle_Device_Status() failed to read mailbox\r\n");
	}
}

/**************************************************************
 * Function Name   : change_device_table_size
 * Description     : Called when the server wants to change the amount of the device
 *                 : table exposed via the register map.
 **************************************************************/
void change_device_table_size(uint16_t address,uint8_t value)
{
	update_register_map_device_table_size(value);
}

/**************************************************************
 * Function Name   : update_register_map_device_table_size
 * Description     : Handle changes to the register map as a result
 *                 : of changing the number of device table entries exposed to the server
 **************************************************************/
void update_register_map_device_table_size(uint8_t number_of_devices)
{
	uint8_t old_number_of_devices;
	uint32_t i;
	uint16_t old_num_registers, num_registers;

	if (number_of_devices>MAX_NUMBER_OF_DEVICES)
		return;	// just bail out if request is larger than the maximum size
	// First update the registers that depend on the number of devices we are exposing
	old_number_of_devices = hubRegisterBank[HR_DEVICE_TABLE_SIZE].value;
	num_registers = HR_DEVICE_TABLE+(sizeof(DEVICE_STATUS)*number_of_devices);
	old_num_registers = (hubRegisterBank[HR_NUM_REGISTERS_HIGH].value<<8) + hubRegisterBank[HR_NUM_REGISTERS_LOW].value;

	hubRegisterBank[HR_DEVICE_TABLE_SIZE].value = number_of_devices;
	hubRegisterBank[HR_NUM_REGISTERS_LOW].value = (num_registers & 0xff);
	hubRegisterBank[HR_NUM_REGISTERS_HIGH].value = ((num_registers >> 8)&0xff);

	hubRegisterBank[HR_DEVICE_TABLE_SIZE].updateWebFlag = true;
	hubRegisterBank[HR_NUM_REGISTERS_LOW].updateWebFlag = true;
	hubRegisterBank[HR_NUM_REGISTERS_HIGH].updateWebFlag = true;

	// Bearing in mind that the data already exists in the register map, we just need
	// to set the Update flags for the newly exposed data.
	if (number_of_devices>old_number_of_devices)
	{	// set the update flags for the newly exposed part of the register map
		for (i=(old_num_registers+1); i<num_registers; i++)
		{
			hubRegisterBank[i].updateWebFlag = false;
		}
	}
}

/**************************************************************
 * Function Name   : HubReg_Init
 * Description     : Initialise our register map
 **************************************************************/
void HubReg_Init(void)
{
	LED_MODE 	led_brightness;
	LED_DISPLAY	led_display;
	led_brightness = get_led_brightness(); // read value from NVM

	switch(led_brightness)	// translate from internal to external representation
	{
		case LED_MODE_OFF:
			led_display = LED_DISPLAY_OFF;
			break;
		case LED_MODE_DIM:
			led_display = LED_DISPLAY_DIM;
			break;
		case LED_MODE_NORMAL:
			led_display = LED_DISPLAY_NORMAL;
			break;
		default:
			zprintf(LOW_IMPORTANCE,"LED brightness from NVM has invalid value %d\r\n",led_brightness);
			led_display = LED_DISPLAY_NORMAL;
			break;
	}
	hubRegisterBank[HR_LEDS_MODE].value = led_display;

	// set HR_DEVICE_TABLE_SIZE and HUB_NUM_REGISTERS, and their updateWebFlags.
	// Also set updateWebFlags on the registers in the Device Table.
	update_register_map_device_table_size(INITIAL_NUMBER_OF_EXPOSED_DEVICES);

	// populate the device table structure entires with function pointers for read/write, updateWebFlag
	// etc, but not the actual register values
    clearHubBank();

	// This gets the actual device table values via a mailbox from SureNet
	get_device_table(true);
}

/**************************************************************
 * Function Name   : HubReg_Check_Full
 * Description     : Scan through the Hub registers, if any have updateWebFlag set to true
 *                 : then form a MSG_SET_REG_RANGE message and send it all to the outgoing message cache.
 *                 : Note that these messages are not prefixed with a subtopic (messages
 *                 : from the Devices are prefixed with their MAC address)
 **************************************************************/
void HubReg_Check_Full(void)
{
	uint32_t		i, j, base_index, next_entry;
	bool			sending = false;

	// Work out if we need to transmit anything from the general map.
	for( i = 0; i < HR_DEVICE_TABLE; i++ )
	{
		if( false != hubRegisterBank[i].updateWebFlag )
		{
			sending = true;
		}
	}

	if( true == sending )
	{
		Hub_Registers_Send_Range(0, HR_DEVICE_TABLE);
	}

	sending = false;

	// Now check the pair table, in large blocks.
	for( j = 0; j < MAX_NUMBER_OF_DEVICES; j++ )
	{
		sending = false;
		base_index = HR_DEVICE_TABLE + j * sizeof(DEVICE_STATUS);
		for( i = 0; i < sizeof(DEVICE_STATUS); i++ )
		{
			if( false != hubRegisterBank[base_index + i].updateWebFlag )
			{
				sending = true;
			}
		}

		if( true == sending )
		{
			next_entry = (j / MAX_PAIR_ENTRIES_PER_MESSAGE) * MAX_PAIR_ENTRIES_PER_MESSAGE;
			base_index = HR_DEVICE_TABLE + sizeof(DEVICE_STATUS) * next_entry;
			Hub_Registers_Send_Range(base_index, sizeof(DEVICE_STATUS) * MAX_PAIR_ENTRIES_PER_MESSAGE);
			j = next_entry + MAX_PAIR_ENTRIES_PER_MESSAGE;
		}
	}
}

static void Hub_Registers_Send_Range(uint32_t start_index, uint32_t count)
{
	static uint8_t 	server_set_reg_count	= 0;	// Will wrap around. Server has limit of 8 bit value

	uint8_t			outgoing_message[MAX_MESSAGE_SIZE_SERVER_BUFFER];
	SERVER_MESSAGE	server_message = {outgoing_message, 0};
	uint32_t		pos, i;

	if( start_index + count >= HR_GET_DYNAMIC_SIZE() )
	{
		count = HR_GET_DYNAMIC_SIZE() - start_index;
	}

	if( count == 0 ){ return; }

	pos = snprintf((char*)outgoing_message, sizeof(outgoing_message), "%d %d %d %d", MSG_REG_VALUES_INDEX_ADDED, server_set_reg_count, start_index, count);
	server_set_reg_count++;

	for( i = 0; i < count; i++ )
	{
		pos += snprintf((char*)&outgoing_message[pos], sizeof(outgoing_message) - pos, " %02x", hubRegisterBank[start_index + i].value);
	}

	if( true == server_buffer_add(&server_message) )
	{
		for( i = 0; i < count; i++ )
		{
			hubRegisterBank[start_index + i].updateWebFlag = false;
		}
	}
}

void HubReg_Send_All(void)
{
	uint32_t i;
	for( i = 0; i < HR_LAST_ELEMENT; i++ )
	{
		hubRegisterBank[i].updateWebFlag = true;
	}
}

/**************************************************************
 * Function Name   : reg_read
 * Description     : returns the value of a hub register
 **************************************************************/
uint8_t reg_read(uint16_t address)
{
	return hubRegisterBank[address].value;
}

uint8_t getHwVersion(uint16_t address)
{
	return readHwVersion();
}

/**************************************************************
 * Function Name   : reg_write
 * Description     : Updates a hub register. Also takes appropriate action. Used for old style registers.
 **************************************************************/
void reg_write(uint16_t address,uint8_t value)
{
	if (address>HR_LAST_ELEMENT) return;  // just ignore writes to bum addresses
	regmap_printf("reg_write() - address %d = %d\r\n",address,value);
	hubRegisterBank[address].value = value;
	hubRegisterBank[address].updateWebFlag = true;
}

/**************************************************************
 * Function Name   : null_write_fn
 * Description     : Function that does nothing, called when a read only register is written
 **************************************************************/
void null_write_fn(uint16_t address,uint8_t uint8_t)
{
    // do nothing
}

/**************************************************************
 * Function Name   : null_read_fn
 * Description     : Function that does nothing, called when a write only register is read
 **************************************************************/
uint8_t null_read_fn(uint16_t address)
{
    return 0;
}

/**************************************************************
 * Function Name   : checksum_read
 * Description     : Calculates the checksum crc16 of active flash bank
 **************************************************************/
extern uint32_t		m_bank_a_start;
extern uint32_t		m_bank_a_size;
uint8_t checksum_read(uint16_t address)
{
  	uint32_t 			i;
	static uint16_t 	crc=0;

	if( HR_HUB_FIRMWARE_CHECKSUM_LOW == address)
  {
		crc = CRC16((uint8_t *)&m_bank_a_start, (uint32_t)&m_bank_a_size, 0xcccc);
  }

	if( address == HR_HUB_FIRMWARE_CHECKSUM_LOW ) return crc & 0xFF;
	return (crc >> 8) & 0xFF;
}

/**************************************************************
 * Function Name   : pairing_handler_write
 * Description     : Handle writes to pairing registers.
 *                 : Note that if Pairing Mode is entered 'locally', i.e.
 *                 : by a button push, then the value in this register is 0x82
 *                 : which is 'immediately' bounced back down by the server
 *                 : as 0x02.
 **************************************************************/
void pairing_handler_write(uint16_t address,uint8_t value)
{
	PAIRING_REQUEST request = {0,false,PAIRING_REQUEST_SOURCE_SERVER};
	PAIRING_REQUEST current_pairing_mode;

	if( value == PAIRING_MANUAL_SERVER_INITIATED)
		zprintf(LOW_IMPORTANCE,"pairing_handler_write(PAIRING_MANUAL_SERVER_INITIATED)\r\n");
	else	// should be 0 for off
		zprintf(LOW_IMPORTANCE,"pairing_handler_write(%d)\r\n",value);

    hubRegisterBank[address].value = value;   // somewhat pointless as we read from SureNet rather than this register

	current_pairing_mode = surenet_get_hub_pairing_mode();

	if( value == PAIRING_MANUAL_SERVER_INITIATED) request.enable = true;	// should be 0x02
	{
		if( false == request.enable)
		{	// we always honour the server disabling pairing mode
			zprintf(LOW_IMPORTANCE,"Changing pairing mode to disabled\r\n");
			surenet_set_hub_pairing_mode(request);
		}
		else
		{	// request.enable == true
			if( false == current_pairing_mode.enable)
			{	// only honour server requests to go into pairing mode if we are not already in pairing mode
				zprintf(LOW_IMPORTANCE,"Changing pairing mode to enabled\r\n");
				surenet_set_hub_pairing_mode(request);
			}
			// else drop the request
		}
	}
}

/**************************************************************
 * Function Name   : pairing_handler_read
 * Description     : Handle reads from pairing registers
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t pairing_handler_read(uint16_t address)
{
	// This register should always be up to date as the callback from the stack when pairing mode
	// changes always ends up in HubReg_SetPairingMode()
    return hubRegisterBank[address].value;
}

/**************************************************************
 * Function Name   : set_pairing_mode_register
 * Description     : Sets the pairing mode register in the hub register bank
 **************************************************************/
void HubReg_SetPairingMode(PAIRING_REQUEST request)
{
	uint8_t value = PAIRING_OFF;
	if( true == request.enable )
	{
		if( PAIRING_REQUEST_SOURCE_SERVER == request.source )
			value = PAIRING_MANUAL_SERVER_INITIATED;	// This value does NOT elicit a server response.
		else
			value = PAIRING_MANUAL_HUB_INITIATED;	// Note the server will respond by writing PAIRING_MANUAL_SERVER_INITIATED
	}												// to the pairing mode register.

    hubRegisterBank[HR_PAIRING_STATE].value = value;
	hubRegisterBank[HR_PAIRING_STATE].updateWebFlag = true;

}

/**************************************************************
 * Function Name   : get_debug_mode
 * Description     : Gets a global 'debug mode' variable
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t get_debug_mode(uint16_t address)
{
  hubRegisterBank[address].value = hub_debug_mode;
  return hubRegisterBank[address].value;
}

/**************************************************************
 * Function Name   : HubReg_SetDebugMode
 * Description     : Sets a global 'debug mode' variable
 **************************************************************/
void HubReg_SetDebugMode(uint16_t address, uint8_t value)
{
	hub_debug_mode = value;
	hubRegisterBank[address].value = value;
}

/**************************************************************
 * Function Name   : led_mode_write(uint16_t address,INT8 value)
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void led_mode_write(uint16_t address, uint8_t value)
{
	// Check if bit 7 is set, if so, send an event to trigger a 'success' LED flash
	if ((value & LED_MODE_SUCCESS_BIT) != 0)
	{
		process_system_event(STATUS_DISPLAY_SUCCESS);
	}
	zprintf(LOW_IMPORTANCE,"Server setting brightness to %d\r\n",value);
	switch (value & LED_MODE_DISPLAY_MASK)
	{
		case LED_DISPLAY_OFF:
			set_led_brightness(LED_MODE_OFF, true);
			break;
		case LED_DISPLAY_DIM:
			set_led_brightness(LED_MODE_DIM, true);
			break;
		case LED_DISPLAY_NORMAL:
			set_led_brightness(LED_MODE_NORMAL, true);
			break;
		default:
			zprintf(LOW_IMPORTANCE,"Server sent invalid LED brightness - 0x%02X. Defaulting to NORMAL\r\n",value);
			set_led_brightness(LED_MODE_NORMAL, true);
			break;
	}

    hubRegisterBank[address].value = value;
    hubRegisterBank[address].updateWebFlag = true;    // suspect this is a debug feature
}

/**************************************************************
 * Function Name   : remote_reboot
 * Description     : Reset the hub cpu if 0xdd written to it.
 **************************************************************/
extern PRODUCT_CONFIGURATION product_configuration;
void remote_reboot(uint16_t address, uint8_t value)
{
	switch( value )
	{
		case RESET_PROCESSOR:
			NVIC_SystemReset(); // never returns
			break;
		case RESET_PROCESSOR_AND_USE_8_BYTE_RF_MAC:
			product_configuration.rf_mac_mangle = false;
			write_product_configuration();
			NVIC_SystemReset(); // never returns
			break;
		case RESET_PROCESSOR_AND_USE_6_BYTE_RF_MAC:
			product_configuration.rf_mac_mangle = true;
			write_product_configuration();
			NVIC_SystemReset(); // never returns
			break;
	}
}

/**************************************************************
 * Function Name   : clear_pairing_table
 **************************************************************/
void clear_pairing_table(uint16_t address,uint8_t value)  // only value 0xff will cause table to be cleared
{
	regmap_printf("Called clear_pairing_table value=%x\r\n", value);
	if (value==0xff)  // for safety only allow one specific value to trigger clearing
	{
		surenet_hub_clear_pairing_table();
	} else
	{
		regmap_printf("Invalid value (require 255)\r\n");
	}
}

/**************************************************************
 * Function Name   : clear_pairing_table
 **************************************************************/
void delete_pairing_table_entry(uint16_t address, uint8_t value)  //value is index to be cleared
{
	regmap_printf("Called delete_pairing_table entry %x\r\n", value);
	zprintf(LOW_IMPORTANCE,"delete_pairing_table entry(%x)\r\n", value);
	if( value < MAX_NUMBER_OF_DEVICES ){ surenet_unpair_device_by_index(value); }
}


/**************************************************************
 * Function Name   : radio_channel_noise_floor
 * Description     : Read the noise floor of the radio receiver (currently unimplemented)
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t radio_channel_noise_floor(uint16_t address)
{
    return hubRegisterBank[address].value;
}

/**************************************************************
 * Function Name   :surenet_channel_number
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t get_surenet_channel_number(uint16_t address)
{
    hubRegisterBank[address].value = surenet_get_channel();
    return hubRegisterBank[address].value;
}

/**************************************************************
 * Function Name   : set_surenet_channel_number
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void set_surenet_channel_number(uint16_t address,uint8_t channel)
{
	regmap_printf("In set_surenet_channel_number %d\r\n", channel);
	zprintf(LOW_IMPORTANCE,"RF Channel changed to %d\r\n",channel);
	channel = surenet_set_channel(channel);
	hubRegisterBank[address].value = channel; // Ought to call surenet_get_channel() but that uses mailboxes and may not be up to date.
    hubRegisterBank[address].updateWebFlag = true;
}

/**************************************************************
 * Function Name   : app_error_read
 * Description     : Unsupported mechanism for reporting application errors
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t app_error_read(uint16_t address)
{
    return 0;   // we don't have application errors
}

/**************************************************************
 * Function Name   : hub_uptime_read
 * Description     : Returns time in seconds since hub was last reset
 *                 : Note that the first read should be HR_HUB_UPTIME_LOW to trigger an atomic read of the timer
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t hub_uptime_read(uint16_t address)
{
	uint32_t t;
	t = get_UpTime();  // get local copy of time so if interrupt changes it partway through, it doesn't matter
	if( address == HR_HUB_UPTIME_LOW )
	{
		hubRegisterBank[HR_HUB_UPTIME_LOW].value = t & 0xff;
		hubRegisterBank[HR_HUB_UPTIME_BITS_15_8].value = (t>>8) & 0xff;
		hubRegisterBank[HR_HUB_UPTIME_BITS_23_16].value = (t>>16) & 0xff;
		hubRegisterBank[HR_HUB_UPTIME_HIGH].value = (t>>24) & 0xff;

		hubRegisterBank[HR_HUB_UPTIME_LOW].updateWebFlag = true;
		hubRegisterBank[HR_HUB_UPTIME_BITS_15_8].updateWebFlag = true;
		hubRegisterBank[HR_HUB_UPTIME_BITS_23_16].updateWebFlag = true;
		hubRegisterBank[HR_HUB_UPTIME_HIGH].updateWebFlag = true;
	}

	return hubRegisterBank[address].value;
}

/**************************************************************
 * Function Name   : bank_desc_read
 * Description     : Returns various elements of the Bank Descriptors
 *                 : Note that the source data are uint32_t, so we only
 *                 : output a truncated form.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
extern const BANK_DESCRIPTOR	BankA_Descriptor;
extern const BANK_DESCRIPTOR	BankB_Descriptor;
uint8_t bank_desc_read(uint16_t address)
{
	uint8_t retval = 0;

	switch( address )
	{
		case HR_HUB_BANK_A_MARK:
			retval = (uint8_t)BankA_Descriptor.bank_mark;
			break;
		case HR_HUB_BANK_A_WDOG_RESETS:
			retval = (uint8_t)BankA_Descriptor.watchdog_resets;
			break;
		case HR_HUB_BANK_B_MARK:
			retval = (uint8_t)BankB_Descriptor.bank_mark;
			break;
		case HR_HUB_BANK_B_WDOG_RESETS:
			retval = (uint8_t)BankB_Descriptor.watchdog_resets;
			break;
	}
	hubRegisterBank[address].value = retval;
	hubRegisterBank[address].updateWebFlag = true;

	return retval;
}

/**************************************************************
 * Function Name   : get_num_pairs
 * Description     : Find out how many Devices are paired with us.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t get_num_pairs(uint16_t address)
{
	hubRegisterBank[address].value = surenet_how_many_pairs();
	return hubRegisterBank[address].value;
}

/**************************************************************
 * Function Name   : initiate_channel_hop
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void initiate_channel_hop(uint16_t address,uint8_t value)
{
	surenet_trigger_channel_hop();
}

/**************************************************************
 * Function Name   : clearHubBank
 * Description     : Sets up the part of the Hub bank that isn't initialised at boot time
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void clearHubBank(void)
{
    int i,j;
    for (i=0; i<MAX_NUMBER_OF_DEVICES; i++)
    {
        for (j=0; j<sizeof(DEVICE_STATUS); j++)
        {
            hubRegisterBank[(i*sizeof(DEVICE_STATUS))+j+HR_DEVICE_TABLE].read_handler_function = reg_read;
            hubRegisterBank[(i*sizeof(DEVICE_STATUS))+j+HR_DEVICE_TABLE].write_handler_function = null_write_fn;
            //hubRegisterBank[(i*sizeof(DEVICE_STATUS))+j+HR_DEVICE_TABLE].updateWebFlag = true;
            hubRegisterBank[(i*sizeof(DEVICE_STATUS))+j+HR_DEVICE_TABLE].value = 0;
            hubRegisterBank[(i*sizeof(DEVICE_STATUS))+j+HR_DEVICE_TABLE].end_of_block = false;
        }
        // now set the final entry for an DEVICE_STATUS to be the end of that block
        hubRegisterBank[((i+1)*sizeof(DEVICE_STATUS))-1+HR_DEVICE_TABLE].end_of_block = true;
    }
	// special case for reading the first entry - it gets an uptodate copy atomically from SureNet
	hubRegisterBank[HR_DEVICE_TABLE].read_handler_function = trigger_get_device_table;
}

/**************************************************************
 * Function Name   : HubReg_SetValues
 * Description     : Updates Hub registers with values received from the server
 **************************************************************/
void HubReg_SetValues(uint16_t address,uint16_t numValues,uint8_t *values,bool set_update_flag)
{
	uint32_t i;
    if( (address >= HR_GET_DYNAMIC_SIZE()) || \
		((address + numValues - 1) >= HR_GET_DYNAMIC_SIZE()) )
	{
		return;
	}

	if( (address + numValues) > (HR_LAST_ELEMENT + 1) )
	{
		zprintf(CRITICAL_IMPORTANCE,"Server attempting to set out of range register %d %d\r\n",address,numValues);
		numValues = HR_LAST_ELEMENT + 1 - address;
	}

	for( i = 0; i < numValues; i++)
	{
		if( set_update_flag == true )
		{
			hubRegisterBank[address+i].updateWebFlag = true;
		}
        if( hubRegisterBank[address+i].write_handler_function != NULL )
		{
			hubRegisterBank[address+i].write_handler_function(address+i, values[i]);
		}
	}
}

/**************************************************************
 * Function Name   : HubReg_GetValues
 * Description     : Refreshes Hub Register values and markes them to be transmitted to the server
 **************************************************************/
void HubReg_GetValues(uint16_t address, uint16_t numValues)
{
    uint32_t i;
    if( (address >= HR_GET_DYNAMIC_SIZE()) || \
		((address + numValues - 1) >= HR_GET_DYNAMIC_SIZE()) )
	{
		return;
	}

	if( (address + numValues) > (HR_LAST_ELEMENT + 1) )
	{
		zprintf(CRITICAL_IMPORTANCE,"Server requested out of range register %d %d\r\n",address,numValues);
		numValues = HR_LAST_ELEMENT + 1 - address;
	}

    for( i = 0; i < numValues; i++ )
    {
       if( hubRegisterBank[address].read_handler_function != NULL )
       {
          hubRegisterBank[address].value = hubRegisterBank[address].read_handler_function(address);
          hubRegisterBank[address].updateWebFlag = true;
       }
       address++;
    }
    return;
}

/**************************************************************
 * Function Name   : HubReg_Refresh_All
 * Description     : Sets the update flags so the register map is sent to the server
 **************************************************************/
void HubReg_Refresh_All(void)
{
	HubReg_GetValues(0, HR_GET_DYNAMIC_SIZE());
}

/**************************************************************
 * Function Name   : HubReg_Dump
 * Description     : debug utility
 **************************************************************/
void hub_reg_dump(char* s, HUB_REGISTER_TYPE reg, uint32_t num_regs)
{
	uint32_t i;
	zprintf(CRITICAL_IMPORTANCE, "\t[ 0x%02x | %-31s| 0x%02x", reg, s, hubRegisterBank[reg].read_handler_function(reg));
	for( i = 1; i < num_regs; i++ )
	{
		zprintf(CRITICAL_IMPORTANCE, "%02x", hubRegisterBank[reg+i].read_handler_function(reg+i));
	}
	zprintf(CRITICAL_IMPORTANCE, "%*s| ", 10-2*num_regs, "");
	for( i = 0; i < num_regs; i++ )
	{
		if( true == hubRegisterBank[reg+i].updateWebFlag )
		{
			zprintf(CRITICAL_IMPORTANCE, "+");
		} else
		{
			zprintf(CRITICAL_IMPORTANCE, "-");
		}
	}
	zprintf(CRITICAL_IMPORTANCE, "%*s]\r\n", 7-num_regs, "");
}

void HubReg_Dump(void)
{
    uint32_t i,j;
	char mapHeader[] = "\t[--Reg-|------------- Name -------------|----Value----| +-Flag ]\r\n";
	int32_t	table_width = strlen(mapHeader)-3;

    zprintf(CRITICAL_IMPORTANCE, mapHeader);

    hub_reg_dump("DEVICE_TYPE",			HR_DEVICE_TYPE, 1);
    hub_reg_dump("NUM_REGISTERS",		HR_NUM_REGISTERS_LOW, 2);
    hub_reg_dump("FIRMWARE_MAJOR",		HR_HUB_FIRMWARE_MAJOR_LOW, 2);
    hub_reg_dump("FIRMWARE_MINOR",		HR_HUB_FIRMWARE_MINOR_LOW, 2);
    hub_reg_dump("HARDWARE_MAJOR",		HR_HUB_HARDWARE_MAJOR_LOW, 2);
    hub_reg_dump("BOOTLOADER_MAJOR",	HR_HUB_BOOTLOADER_MAJOR, 2);
    hub_reg_dump("MAX_MESSAGE_SIZE",	HR_HUB_MAX_MESSAGE_SIZE_LOW, 2);
    hub_reg_dump("FIRMWARE_CHECKSUM",	HR_HUB_FIRMWARE_CHECKSUM_LOW, 2);
    hub_reg_dump("PAIRING_STATE",		HR_PAIRING_STATE, 1);
    hub_reg_dump("DEBUG_MODE",			HR_DEBUG_MODE, 1);
    hub_reg_dump("PAIRING_TIMER",		HR_PAIRING_TIMER_VALUE, 1);
    hub_reg_dump("LEDS_MODE",			HR_LEDS_MODE, 1);
    hub_reg_dump("REMOTE_REBOOT",		HR_REMOTE_REBOOT, 1);
    hub_reg_dump("CLEAR_PAIRING_TABLE",	HR_CLEAR_PAIRING_TABLE, 1);
    hub_reg_dump("INITIATE_CHANNEL_HOP",HR_INITIATE_CHANNEL_HOP, 1);
    hub_reg_dump("DELETE_CONNECTION_TABLE_ENTRY",HR_DELETE_CONNECTION_TABLE_ENTRY, 1);
    hub_reg_dump("CHANNEL_NOISE_FLOOR",	HR_SURENET_CHANNEL_NOISE_FLOOR, 1);
    hub_reg_dump("CHANNEL_NUMBER",		HR_SURENET_CHANNEL_NUMBER, 1);
    hub_reg_dump("APPLICATION_ERROR",	HR_APPLICATION_ERROR_LOW, 2);
    hub_reg_dump("HUB_UPTIME",			HR_HUB_UPTIME_LOW, 4);
    hub_reg_dump("BANK_A_MARK",			HR_HUB_BANK_A_MARK, 1);
    hub_reg_dump("BANK_A_WATCHDOG_RESETS",	HR_HUB_BANK_A_WDOG_RESETS, 1);
    hub_reg_dump("BANK_B_MARK",			HR_HUB_BANK_B_MARK, 1);
    hub_reg_dump("BANK_B_WATCHDOG_RESETS",	HR_HUB_BANK_B_WDOG_RESETS, 1);
    hub_reg_dump("BANK_A_HASH",			HR_HUB_BANK_A_HASH_LOW, 4);
    hub_reg_dump("BANK_B_HASH",			HR_HUB_BANK_B_HASH_LOW, 4);
    hub_reg_dump("DEVICE_TABLE_SIZE",	HR_DEVICE_TABLE_SIZE, 1);
    hub_reg_dump("NUMBER_CONNECTIONS",	HR_DEVICE_TABLE_NUMBER_CONNECTIONS, 1);

	zprintf(CRITICAL_IMPORTANCE, "\t[--- End Hub Register Map ---]\r\n");

	zprintf(CRITICAL_IMPORTANCE, "\t[--- Device Table |%*s]\r\n", table_width - 20, "| " xstrfy(MAX_NUMBER_OF_DEVICES) " Entries ");
    for( i = 0; i < MAX_NUMBER_OF_DEVICES; i++ )
    {
		zprintf(CRITICAL_IMPORTANCE, "\t[ Device #%02d | ", i);
        for( j = 0; j < sizeof(DEVICE_STATUS); j++ )
        {
            zprintf(CRITICAL_IMPORTANCE, "%02X ",hubRegisterBank[HR_DEVICE_TABLE + (i * sizeof(DEVICE_STATUS)) + j].value);
        }
        zprintf(CRITICAL_IMPORTANCE, "]\r\n");
    }
	zprintf(CRITICAL_IMPORTANCE, "\t[--- End Device Table ---]\r\n");
}

uint8_t HRM_Read_Image_Hash(uint16_t address)
{
	char					result[WC_SHA256_DIGEST_SIZE];
	uint16_t				base_address;
	int						i, j;
	wc_Sha256				hash_context;
	const BANK_DESCRIPTOR*	descriptor = NULL;

	if( address == HR_HUB_BANK_B_HASH_LOW )
	{
		base_address = HR_HUB_BANK_B_HASH_LOW;
		descriptor = &BankB_Descriptor;
	} else if( address == HR_HUB_BANK_A_HASH_LOW )
	{
		base_address = HR_HUB_BANK_A_HASH_LOW;
		descriptor = &BankA_Descriptor;
	}

	if( NULL == descriptor ){ return hubRegisterBank[address].value; }

	wc_InitSha256(&hash_context);
	wc_Sha256Update(&hash_context, (const byte*)descriptor->vector_table_offset, descriptor->image_size);
	wc_Sha256Final(&hash_context, (byte*)result);

	for( i = 0; i < 4; i++ )
	{
		hubRegisterBank[base_address + i].value = 0;
		hubRegisterBank[base_address + i].updateWebFlag = true;
		for( j = i; j < WC_SHA256_DIGEST_SIZE; j += 4 )
		{
			hubRegisterBank[base_address + i].value ^= result[j];
		}
	}

	return hubRegisterBank[address].value;
}

uint32_t HubReg_Get_Integer(uint16_t address)
{
	uint32_t ret = hubRegisterBank[address].value;
	ret |= (uint32_t)hubRegisterBank[address + 1].value << 8;
	ret |= (uint32_t)hubRegisterBank[address + 2].value << 16;
	ret |= (uint32_t)hubRegisterBank[address + 3].value << 24;

	return ret;
}
