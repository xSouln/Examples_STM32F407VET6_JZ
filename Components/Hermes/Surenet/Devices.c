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
* Filename: Devices.c
* Author:   Chris Cowdery 11/06/2019
* Purpose:  
* 
* This file handles the devices table (which is a table of
* previously associated devices). The table is persistent.
* The devices table is directly accessed from SureNet.c via an extern.
* This is because there is a lot of interaction, and handler functions would be cumbersome.
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

// SureNet
#include "Surenet-Interface.h"
#include "SureNetDriver.h"
#include "devices.h"
#include "SureNet.h"
#include "hermes-time.h"

// Shared variables
DEVICE_STATUS device_status[MAX_NUMBER_OF_DEVICES];
DEVICE_STATUS_EXTRA device_status_extra[MAX_NUMBER_OF_DEVICES];
extern QueueHandle_t xNvStoreMailboxSend;
extern QueueHandle_t xNvStoreMailboxResp;

// Private functions
bool remove_pairing_table_entry(uint64_t mac);
/**************************************************************
 * Function Name : sn_devicetable_init
 * Description	 : Read device table from NVM, and check it's sane.
 *				 : If it's not sane, initialise it to default values.
 *				 : Sanity check is to ensure that it is either old style
 *				 : or new style MAC address.
 *				 : Old style is to have 0xfe in bytes 3 & 4
 *				 : New style is to contain 0x70b3d5f9c
 * Inputs		 :
 * Outputs		 :
 * Returns		 :
 **************************************************************/
void sn_devicetable_init(void)
{	
	int			i;
	bool		sane = true;
	uint8_t* 	mac_8;
	uint32_t	notifyValue;
	
	memset(device_status_extra, 0, sizeof(device_status_extra)); // hose extra part of Device Statuses. Maybe should store this too?
	
	xQueueSend(xNvStoreMailboxSend, &notifyValue, 0); // get device table into RAM
	
	if( xTaskNotifyWait(0, 0, &notifyValue, 100 ) == pdTRUE ) // nominal time to wait for notification, or use portMAX_DELAY if blocking
	{
		if( notifyValue == 0)
		{
			for( i=0; i<MAX_NUMBER_OF_DEVICES; i++ )
			{
				mac_8 = (uint8_t *)&device_status[i].mac_address;
				//check first for old style extended address
				if ((device_status[i].status.valid==1) && (mac_8[3]!=MAC_BYTE_4) && (mac_8[4]!=MAC_BYTE_4)) //cope with device address in either endian
				{  //now check for SureFlap UID
				  if( mac_8[7] != UID_1 ) sane = false;
				  if( mac_8[6] != UID_2 ) sane = false;
				  if( mac_8[5] != UID_3 ) sane = false;
				  if( mac_8[4] != UID_4 ) sane = false;
				  if( (0xf0 & mac_8[3]) != UID_5 ) sane = false;
				}
			}
			if( sane == false )
			{
				memset(device_status,0,sizeof(device_status)); // hose Device Statuses.
				store_device_table();			
			}
		} else
		{
			zprintf(HIGH_IMPORTANCE, "\r\nUnable to retrieve Device Table from NVM\r\n");
		}
	} else
	{	// couldn't read from NVM
		zprintf(HIGH_IMPORTANCE, "Failed to read Device Status from NVM\r\n");
	}
	memset(device_status_extra, 0, sizeof(device_status_extra));	// clear the extra part always
	// apart from the SendSecurityKey because it's random and we have just started up, we must send it to the devices
	// when they try and talk to us.
	for( i=0; i<MAX_NUMBER_OF_DEVICES; i++ )
	{
		if( device_status[i].status.associated == 1 )
		{
			device_status_extra[i].SendSecurityKey = SECURITY_KEY_RENEW;	// send key to previously associated devices
		}
	}
}

// looks up the index of an entry in the pairing table, given it's address
bool convert_addr_to_index(uint64_t addr, uint8_t *index)
{
	if( NULL == index) {return false;}	// safety net
	uint8_t i;
	for( i = 0; i < MAX_NUMBER_OF_DEVICES; i++ )
	{
		if( device_status[i].mac_address == addr )
		{
			*index = i;
			return true;
		}
	}
	return false;
}

// handy utility to output the full pairing table to the console
void device_table_dump(void)
{
	uint8_t i;
	T_DEVICE_TYPE device_type;
	char* device_type_names[] = { "Unknown       ",
								  "Hub           ",
								  "Repeater      ",
								  "iMPD Pet Door ",
								  "iMPF Feeder   ",
								  "Programmer    ",
								  "iDSCF Cat Flap ",
								  "Feeder Lite   ",
								  "Posiedon      ",
								  "INVALID       " };
	
	
	zprintf(CRITICAL_IMPORTANCE, "Pairing table:\r\n");
	zprintf(CRITICAL_IMPORTANCE, "#\tValid Online Product Type  Web MAC Address             Dev RSSI Hub RSSI Last heard\r\n");
	for( i = 0; i < MAX_NUMBER_OF_DEVICES; i++ )
	{
		if ((device_status[i].status.valid==1) && \
			(device_status[i].mac_address!=0))		
		{
			zprintf(CRITICAL_IMPORTANCE, "%d\t", i); 
		  
			if( device_status[i].status.valid == 1 )
				zprintf(CRITICAL_IMPORTANCE, "YES   ");
			else
				zprintf(CRITICAL_IMPORTANCE, "NO    ");
			if( device_status[i].status.online == 1 )
				zprintf(CRITICAL_IMPORTANCE, "YES    ");
			else
				zprintf(CRITICAL_IMPORTANCE, "NO     ");
			device_type = (T_DEVICE_TYPE)device_status[i].status.device_type;
			
			if( device_type >= DEVICE_TYPE_BACKSTOP ){ device_type = DEVICE_TYPE_BACKSTOP; }
			zprintf(CRITICAL_IMPORTANCE, device_type_names[device_type]);

			if( device_status[i].status.associated == 1 )
				zprintf(CRITICAL_IMPORTANCE, "YES ");
			else
				zprintf(CRITICAL_IMPORTANCE, "NO  ");
			zprintf(CRITICAL_IMPORTANCE, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", \
				(uint8_t)(device_status[i].mac_address>>56),(uint8_t)(device_status[i].mac_address>>48), \
				(uint8_t)(device_status[i].mac_address>>40),(uint8_t)(device_status[i].mac_address>>32), \
				(uint8_t)(device_status[i].mac_address>>24),(uint8_t)(device_status[i].mac_address>>16), \
				(uint8_t)(device_status[i].mac_address>>8),(uint8_t)device_status[i].mac_address);
			zprintf(CRITICAL_IMPORTANCE, "   %02x",device_status[i].device_rssi);
			zprintf(CRITICAL_IMPORTANCE, "       %02x",device_status[i].hub_rssi);   
			zprintf(CRITICAL_IMPORTANCE, "       %08x",device_status[i].last_heard_from);
			zprintf(CRITICAL_IMPORTANCE, "       %dms\r\n",(get_microseconds_tick()-device_status[i].last_heard_from)/1000);


			
		}
	}
	
	zprintf(CRITICAL_IMPORTANCE,"END OF PAIRTABLE\r\n");
}

// adds an entry to the pairing table
// returns true if successful, or false if the table is full
// Scans the table looking for the first entry where valid==0, or
// where the mac address already has an entry
// Note does NOT store it to NVM. This is done later
bool add_mac_to_pairing_table(uint64_t mac)
{   
	uint8_t i = 0;
	do
	{
		if( device_status[i].mac_address == mac )
		{   // this mac is already here. Might need to think about nuances of an existing entry
			// that is valid vs one that is not valid.
			device_status[i].status.valid = 1;	// mark this entry as valid
			device_status[i].status.associated = 1;	// mark this entry as associated			   
			device_status[i].last_heard_from = get_microseconds_tick();  // microsecond counter
			surenet_update_device_table_line(&device_status[i], i, true, true);
			sn_GenerateSecurityKey(i);
			sn_CalculateSecretKey(i);
			return true;
		}
	} while( ++i < MAX_NUMBER_OF_DEVICES ); 
	i = 0;
	do  // if we get here, the mac is new to us
	{
		if( device_status[i].status.valid == 0 )
		{   // we have found a slot, so use it
			device_status[i].status.valid = 1;	// mark this entry as valid
			device_status[i].status.associated = 1;	// mark this entry as associated		  
			device_status[i].mac_address = mac;   // store the mac address
			device_status[i].last_heard_from = get_microseconds_tick();  // microsecond counter	
			sn_GenerateSecurityKey(i);
			sn_CalculateSecretKey(i);
			surenet_update_device_table_line(&device_status[i], i, false, true);
			return true;
		}
	} while( ++i < MAX_NUMBER_OF_DEVICES );
	zprintf(HIGH_IMPORTANCE,"Device associated, but pairing table full.\r\n");
	return false;
}

// remove a pairing table entry, using the mac as a lookup
// returns true if it found it and removed it (by setting valid to 0)
// returns false if it couldn't be found
bool remove_pairing_table_entry(uint64_t mac)
{
	uint8_t i		= 0;
	uint8_t found	= 0;
	bool 	retval	= false;
	
	if( mac == 0 )
	{	  
		zprintf(MEDIUM_IMPORTANCE, "Wiping all of the pairing table!\r\n");
		memset(&device_status[0], 0, sizeof (device_status)*MAX_NUMBER_OF_DEVICES);
		store_device_table();
		retval = true;
	} else
	{
		zprintf(MEDIUM_IMPORTANCE,"Removing: %08x %08x\r\n",(uint32_t)(mac >> 32), (uint32_t)(mac & 0xffffffff));
		do
		{	  
			if( device_status[i].mac_address == mac )
			{   // we have found mac, so mark this entry as invalid
				zprintf(MEDIUM_IMPORTANCE,"Found MAC at line %d\r\n", i);
				memset (&device_status[i], 0, sizeof (device_status[i]));
				store_device_table ();
				found++;
				retval = true;
			}   
		} while( ++i < MAX_NUMBER_OF_DEVICES );

		if( 0 == found )
		{
			zprintf(HIGH_IMPORTANCE,"Could not find MAC in Pair table\r\n");
			retval = false;
		}
	}   
	return retval;
}

/**************************************************************
 * Function Name   : is_device_already_known
 * Description	 : Check to see if this device is already in the pairing table
 * Inputs		 :
 * Outputs		 :
 * Returns		 :
 **************************************************************/
bool is_device_already_known(uint64_t addr, uint8_t dev_type)
{
	uint8_t index;
	
	if( convert_addr_to_index(addr,&index) == false )
	{
		return false;	// can't find MAC
	}
	
	if( device_status[index].status.device_type != dev_type )
	{
		return false;	// device type doesn't match (which would be odd as it
	}					// means the MAC has been re-used for a different device!
	
	return true;	// mac address and device types match
}

// adds dev_type and dev_rssi to the pairing table entry determined by addr
// Returns true if it could find a matching entry in the table.
// Returns false if it could not find a matching entry.
bool add_device_characteristics_to_pairing_table(uint64_t addr, uint8_t dev_type, uint8_t dev_rssi)
{
	uint8_t index;
	
	if( convert_addr_to_index(addr,&index) == false )
	{
		return false;
	}
	
	device_status[index].device_rssi = dev_rssi;
	device_status[index].status.device_type = dev_type;
	return true; 
}

/**************************************************************
 * Function Name   : trigger_key_send
 * Description	 : Ensures that the encryption key is sent to this device
 *				 : when the next conversation starts.
 * Inputs		 :
 * Outputs		 :
 * Returns		 :
 **************************************************************/
void trigger_key_send(uint64_t mac)
{
	uint8_t index;
	
	if( true == convert_addr_to_index(mac,&index) )
	{
		device_status_extra[index].SendSecurityKey = SECURITY_KEY_RENEW;
	}
}

/**************************************************************
 * Function Name   : convert_mac_to_index
 * Description	 : Convert a mac address to its index in the pairing table
 * Inputs		 :
 * Outputs		 :
 * Returns		 : Index in pairing table AND valid, or -1 if it's not present, or -1 if present but not valid
 **************************************************************/
int8_t convert_mac_to_index(uint64_t src_mac)
{
  
	uint8_t i = 0;
	if( 0ull == src_mac ){ return -1; } // Do not respond to non-Sure beacon requests.
	do
	{
		if( (device_status[i].mac_address == src_mac ) && ( 1 == device_status[i].status.valid ))
		{
			return i;
		}
	} while( ++i < MAX_NUMBER_OF_DEVICES );
	return -1;
}

/**************************************************************
 * Function Name   : are_we_paired_with_source
 * Description	 : Checks if we are paired with the source of this packet
 * Inputs		 :
 * Outputs		 :
 * Returns		 : True if we are paired, false if not
 **************************************************************/
bool are_we_paired_with_source(uint64_t src_mac)
{
	if( convert_mac_to_index(src_mac) == -1 )
	{
		return false;
	}
	return true;
}

/**************************************************************
 * Function Name   : store_device_table
 * Description	 : Store the device table in non-volatile storage.
 * Inputs		 :
 * Outputs		 :
 * Returns		 : true if successfully written, false if the write failed
 **************************************************************/
bool store_device_table(void)
{
	/*
	SEND_TO_FM_MSG	nvNvmMessage;	// Outgoing message	
	uint32_t		notifyValue;
	
	nvNvmMessage.ptrToBuf	= (uint8_t*)device_status;
	nvNvmMessage.dataLength = sizeof(device_status);
	nvNvmMessage.type		= FM_DEVICE_STATS;
	nvNvmMessage.action		= FM_PUT;
	nvNvmMessage.xClientTaskHandle = xTaskGetCurrentTaskHandle();
	
	xQueueSend(xNvStoreMailboxSend, &nvNvmMessage, 0);	// get device table into RAM
	
	if( xTaskNotifyWait(0, 0, &notifyValue, 100 ) == pdTRUE )	// nominal time to wait for notification, or use portMAX_DELAY if blocking
	{
		if( notifyValue == FM_ACK )
		{
			zprintf(LOW_IMPORTANCE,"Device Table written\r\n");
			return true;
		} else
		{
			zprintf(CRITICAL_IMPORTANCE,"Device Table write failed\r\n");
			return false;
		}
	} else
	{
		zprintf(HIGH_IMPORTANCE,"Notification of Device Table Write did not arrive\r\n");
		return false;
	}
	*/
	return true;
}

/**************************************************************
 * Function Name   : device_type_from_mac
 * Description	 : if the mac address is in the pairing table, 
 *				 : return the device type
 * Inputs		 : mac address
 * Outputs		 :
 * Returns		 : device type, or SURE_PRODUCT_UNKNOWN
 *				   : (we return zero to avoid using MQTT_Synapse.h 
 *				   : here) if mac not found in table
 **************************************************************/
uint8_t device_type_from_mac(uint64_t src_mac)
{
	int8_t index = convert_mac_to_index(src_mac);
	if( index != -1 )
	{
		return device_status[index].status.device_type;
	} else
	{
		return 0;	// SURE_PRODUCT_UNKNOWN
	}
}

/**************************************************************
 * Function Name   : device_type_from_index
 * Description	 : Find the DEVICE_TYPE from the index in the device table.
 * Inputs		 :
 * Outputs		 :
 * Returns		 :
 **************************************************************/
T_DEVICE_TYPE device_type_from_index(uint8_t index)
{
	return (T_DEVICE_TYPE)device_status[index].status.device_type;	
}

/**************************************************************
 * Function Name   : convert_index_to_mac
 * Description	 : return the mac address associated with index
 * Inputs		 : index
 * Outputs		 :
 * Returns		 : mac address
 **************************************************************/
uint64_t get_mac_from_index(uint8_t index)
{
	return device_status[index].mac_address;
}

/**************************************************************
 * Function Name   : last_heard_from	
 * Description	 : Calculates how recently we heard from any paired
 *				 : devices.
 * Inputs		 :
 * Outputs		 :
 * Returns		 : Time in us since most recent comms. 0 if nothing paired,
 *				 : and 0xffffffff if never heard from paired devices
 **************************************************************/
uint32_t last_heard_from(void)
{
	uint8_t i;
	uint8_t num_valid		= 0;
	uint32_t now 			= get_microseconds_tick();
	uint32_t most_recent 	= 0xffffffff;
	
	for( i = 0; i < MAX_NUMBER_OF_DEVICES; i++ )
	{
		if( 1 == device_status[i].status.valid )
		{
			num_valid++;
			if( (get_microseconds_tick() - device_status[i].last_heard_from) < most_recent )
			{
				most_recent = get_microseconds_tick() - device_status[i].last_heard_from;
			}
		}
	}
	
	if( 0 == num_valid )
	{
		return 0;	// no paired devices
	}
	
	return most_recent;	
}


