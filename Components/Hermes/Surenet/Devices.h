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
* Filename: Devices.h
* Author:   Chris Cowdery 11/06/2019
* Purpose:  
* 
* This file handles the devices table (which is a table of
* previously associated devices). The table is persistent.
* The devices table is directly accessed from SureNet.c via an extern.
* This is because there is a lot of interaction, and handler functions would be cumbersome.
*           
**************************************************************************/

#ifndef __DEVICES_H__
#define __DEVICES_H__
//#include "SureNetDriver-Public.h"

#include <stdint.h>
#include <stdbool.h>

#define xstrfy(x)	strfy(x)
#define strfy(x)	#x
#define MAX_NUMBER_OF_DEVICES   60  // maximum number of devices that can pair with the hub. Limited by sizeof(DEVICE_STATUS) and size of storage area in NVM for table
                                    // Also by width of DETACH_ALL_MASK which is currently a uint64_t


typedef enum {
	DEVICE_TYPE_UNKNOWN = 0,
	DEVICE_TYPE_HUB,			// iHB - Hub
	DEVICE_TYPE_REPEATER,		// Never implemented
	DEVICE_TYPE_CAT_FLAP,		// iMPD - Pet Door Connect
	DEVICE_TYPE_FEEDER,			// iMPF - Microchip Pet Feeder Connect
	DEVICE_TYPE_PROGRAMMER,		// Production Programmer
	DEVICE_TYPE_DUALSCAN,		// iDSCF - Cat Flap Connect
    DEVICE_TYPE_FEEDER_LITE,	// MPF2 - not connect
    DEVICE_TYPE_POSEIDON,		// iCWS - Felaqua
	DEVICE_TYPE_BACKSTOP		// Should always be last.
}T_DEVICE_TYPE;	

#define OFFLINE_TIMEOUT (usTICK_SECONDS * 180)

// THIS LOT IS A BIT OF HOKEY MECHANISM TO SANITY CHECK THE MAC ADDRESS OF CONNECTED DEVICES
#define MAC_BYTE_3  0xff
#define MAC_BYTE_4  0xfe
#define UID_1 0x70
#define UID_2 0xB3
#define UID_3 0xD5
#define UID_4 0xF9
#define UID_5 0xC0  //NB only top nibble is valid

typedef enum
{
    DEVICE_HAS_NO_DATA,
    DEVICE_HAS_DATA,
    DEVICE_DETACH,
    DEVICE_SEND_KEY,
    DEVICE_DONT_SEND_KEY,
    DEVICE_RCVD_SEGS,
    DEVICE_SEGS_COMPLETE,
} DEVICE_DATA_STATUS;

typedef enum
{
	SURENET_CRYPT_BLOCK_XTEA,
	SURENET_CRYPT_CHACHA,
	SURENET_CRYPT_AES128,
} SURENET_ENCRYPTION_TYPE;

typedef enum
{
    DEVICE_ASLEEP,
    DEVICE_AWAKE,
} DEVICE_SLEEP_STATUS;

typedef struct
{
    DEVICE_SLEEP_STATUS awake;
    DEVICE_DATA_STATUS data;
} DEVICE_AWAKE_STATUS;

typedef enum
{
	SECURITY_KEY_OK,
	SECURITY_KEY_RENEW,
} SECURITY_KEY_ACTION;

typedef struct
{
    uint8_t valid :1;
    uint8_t online :1;
    uint8_t device_type :5;  //one of T_DEVICE_TYPE
    uint8_t associated :1;
} DEVICE_STATUS_BITS;

typedef struct
{
    uint64_t mac_address;
    DEVICE_STATUS_BITS status;            // This ought to be BOOL, but BOOL takes 32 bits or 4 bytes on PIC32, which is a bloody waste for a single bit!
    uint8_t lock_status;                  //current status of door locks, sent with every DEVICE_AWAKE
    uint8_t device_rssi;                  //signal strength seen from device
    uint8_t hub_rssi;                     //signal strength seen from hub
    uint32_t last_heard_from;           // This is a problem because we store the array of DEVICE_STATUS's in NVM, which is very small.
                                        //actually because of alignment we use 32 bits anyway
                                        //now store some other per device useful information in gap
} DEVICE_STATUS; 
#define DEVICE_STATUS_LAST_HEARD_POS	12

typedef struct
{
    uint8_t sec_key_invalid;
    SECURITY_KEY_ACTION SendSecurityKey;            //used to track sending of security key to devices
    DEVICE_AWAKE_STATUS device_awake_status;
    uint8_t deviceMinutes;              // used to remember the minutes timestamp of last time the device communicated.
    bool device_web_connected;  
    uint8_t tx_end_cnt; 
    uint8_t send_detach;
	bool send_ping;
	uint8_t ping_value;
	uint8_t SecurityKeyUses;
	SURENET_ENCRYPTION_TYPE	encryption_type;
} DEVICE_STATUS_EXTRA; 

typedef enum
{	// Not used in Hub2, but sent by Devices for debugging
    TX_STAT_SUCCESSES=0,
    TX_STAT_FAILED_ACK_SENDS,
    TX_STAT_GOOD_TRANSMISSIONS,
    TX_STAT_BAD_TRANSMISSIONS,
    TX_STAT_BACKSTOP
} TX_STAT_INDICES;


typedef struct	// parameters used as part of DEVICE_RCVD_SEGS message
{
    uint8_t fetch_chunk_upper;
    uint8_t fetch_chunk_lower;
    uint8_t fetch_chunk_blocks; // 14
    uint8_t received_segments[9];
} DEVICE_RCVD_SEGS_PARAMETERS;

typedef struct     // This would be more logically located in SureNet.h, however, it needs DEVICE_DATA_STATUS
{                           // which is used in two orthogonal ways, as a parameter in PACKET_DEVICE_AWAKE, and 
                            // to manage the HUB_CONVERSATION. Perhaps they should be separated.
    DEVICE_DATA_STATUS device_data_status;
    uint8_t battery_voltage;    // multiply by 32 to get device battery voltage in millivolts
    uint8_t device_hours;
    uint8_t device_minutes;
    uint8_t lock_status;    // 4 - only used on Pet Door
    uint8_t device_rssi;    // 5
    uint8_t awake_count;    // 6
    uint8_t sum;            // 7 - no idea what this is
    uint8_t tx_stats[TX_STAT_BACKSTOP];     // 8-11 TBC
// used for firmware download, if device_data_status == DEVICE_RCVD_SEGS
	union
	{
		SURENET_ENCRYPTION_TYPE encryption_type;
    DEVICE_RCVD_SEGS_PARAMETERS rcvd_segs_params;
	};
	SURENET_ENCRYPTION_TYPE encryption_type_extended;
} PACKET_DEVICE_AWAKE_PAYLOAD;

typedef struct
{
	uint8_t					transmit_end_count;
	SURENET_ENCRYPTION_TYPE	encryption_type;
} DEVICE_TX_PAYLOAD;

typedef struct	// used to send rcvd segs params into the application
{
	DEVICE_RCVD_SEGS_PARAMETERS rcvd_segs_params;
	uint64_t	device_mac;
} DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX;

typedef struct
{
    uint64_t src_addr;
    PACKET_DEVICE_AWAKE_PAYLOAD payload;
} DEVICE_AWAKE_MAILBOX;

typedef struct
{
	uint8_t device_index;	// device for which this is intended
	uint8_t len;
	uint16_t chunk_address;
	uint8_t chunk_data[136];	// 136 bytes - 8 byte header (which is 4 byte destination, 4 byte checksum), and 128 bytes of data. 
} DEVICE_FIRMWARE_CHUNK;

typedef struct
{
	uint64_t mac_address;
	uint8_t value;
} PING_REQUEST_MAILBOX;

void sn_devicetable_init(void);
void device_table_dump(void);
bool add_mac_to_pairing_table(uint64_t mac);
void trigger_key_send(uint64_t mac);
int8_t convert_mac_to_index(uint64_t src_mac);
bool is_device_already_known(uint64_t addr,uint8_t dev_type);
bool add_device_characteristics_to_pairing_table(uint64_t addr,uint8_t dev_type,uint8_t dev_rssi);
void device_status_sanitise(void);
bool are_we_paired_with_source(uint64_t);
bool convert_addr_to_index(uint64_t addr, uint8_t *index);
uint8_t device_type_from_mac (uint64_t src_mac);
T_DEVICE_TYPE device_type_from_index (uint8_t index);
uint64_t get_mac_from_index (uint8_t index);
bool store_device_table(void);
uint32_t last_heard_from(void);

#endif //__PAIRING_H__
