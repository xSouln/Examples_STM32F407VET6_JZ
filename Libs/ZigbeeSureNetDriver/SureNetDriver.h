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
* Filename: SureNetDriver.h
* Author:   Chris Cowdery 
* 
* SureNet Driver top level file - effectively wraps Atmel Stack.
* It provides the following facilities:
* 1. Association with Devices
* 2. Sending and receiving messages to / from devices.
* We access the SPI via a FreeRTOS API, but the nRST, INT and SP signals are controlled directly
*           
**************************************************************************/
#ifndef __SURENETDRIVER_H__
#define __SURENETDRIVER_H__
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//types:

//------------------------------------------------------------------------------

typedef enum
{
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

} T_DEVICE_TYPE;
//------------------------------------------------------------------------------

#define OFFLINE_TIMEOUT (usTICK_SECONDS * 180)

// THIS LOT IS A BIT OF HOKEY MECHANISM TO SANITY CHECK THE MAC ADDRESS OF CONNECTED DEVICES
#define MAC_BYTE_3  0xff
#define MAC_BYTE_4  0xfe
#define UID_1 0x70
#define UID_2 0xB3
#define UID_3 0xD5
#define UID_4 0xF9
#define UID_5 0xC0  //NB only top nibble is valid
//------------------------------------------------------------------------------

typedef enum
{
	BEACON_PAYLOAD_SUREFLAP_PROTOCOL_ID,
	BEACON_PAYLOAD_SUREFLAP_VERSION,
	BEACON_PAYLOAD_SOLE_PAN_COORDINATOR,

} BEACON_PAYLOAD_INDEX;
//------------------------------------------------------------------------------
// This is used to store the type of remote device requesting a Beacon
typedef enum
{
    INVALID_DEVICE,
    NON_THALAMUS_BASED_DEVICE,
    THALAMUS_BASED_DEVICE,

} DEVICE_PLATFORM;
//------------------------------------------------------------------------------
// This is used to store a couple of attributes of a remote device requesting a Beacon
typedef struct
{
    uint64_t    mac_addr;
    DEVICE_PLATFORM device_platform;    // Thalamus or not
    uint8_t device_channel;             // RF Channel

} BEACON_REQUEST_DEVICE_DATA;
//------------------------------------------------------------------------------
// This type definition of a structure can store the short address and the extended address of a device.
typedef struct associated_device_tag
{
	uint16_t short_addr;
	uint64_t ieee_addr;

} associated_device_t;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t 	timestamp;
	uint32_t	timeout;
	bool 		active;

} PAIRING_MODE_TIMEOUT;
//------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------
typedef enum
{
	SURENET_CRYPT_BLOCK_XTEA,
	SURENET_CRYPT_CHACHA,
	SURENET_CRYPT_AES128,

} SURENET_ENCRYPTION_TYPE;
//------------------------------------------------------------------------------

typedef enum
{
    DEVICE_ASLEEP,
    DEVICE_AWAKE,

} DEVICE_SLEEP_STATUS;
//------------------------------------------------------------------------------

typedef struct
{
    DEVICE_SLEEP_STATUS awake;
    DEVICE_DATA_STATUS data;

} DEVICE_AWAKE_STATUS;
//------------------------------------------------------------------------------

typedef enum
{
	SECURITY_KEY_OK,
	SECURITY_KEY_RENEW,

} SECURITY_KEY_ACTION;
//------------------------------------------------------------------------------

typedef struct
{
    uint8_t valid :1;
    uint8_t online :1;
    uint8_t device_type :5;  //one of T_DEVICE_TYPE
    uint8_t associated :1;

} DEVICE_STATUS_BITS;
//------------------------------------------------------------------------------

typedef struct
{
    uint64_t mac_address;
    DEVICE_STATUS_BITS status; // This ought to be BOOL, but BOOL takes 32 bits or 4 bytes on PIC32, which is a bloody waste for a single bit!
    uint8_t lock_status; //current status of door locks, sent with every DEVICE_AWAKE
    uint8_t device_rssi; //signal strength seen from device
    uint8_t hub_rssi; //signal strength seen from hub
    uint32_t last_heard_from; // This is a problem because we store the array of DEVICE_STATUS's in NVM, which is very small.
								//actually because of alignment we use 32 bits anyway
								//now store some other per device useful information in gap
} DEVICE_STATUS;

#define DEVICE_STATUS_LAST_HEARD_POS	12
//------------------------------------------------------------------------------

typedef struct
{
    uint8_t sec_key_invalid;
    SECURITY_KEY_ACTION SendSecurityKey; //used to track sending of security key to devices
    DEVICE_AWAKE_STATUS device_awake_status;
    uint8_t deviceMinutes; // used to remember the minutes timestamp of last time the device communicated.
    bool device_web_connected;
    uint8_t tx_end_cnt;
    uint8_t send_detach;
	bool send_ping;
	uint8_t ping_value;
	uint8_t SecurityKeyUses;
	SURENET_ENCRYPTION_TYPE	encryption_type;

} DEVICE_STATUS_EXTRA;
//------------------------------------------------------------------------------

typedef enum
{	// Not used in Hub2, but sent by Devices for debugging
    TX_STAT_SUCCESSES=0,
    TX_STAT_FAILED_ACK_SENDS,
    TX_STAT_GOOD_TRANSMISSIONS,
    TX_STAT_BAD_TRANSMISSIONS,
    TX_STAT_BACKSTOP

} TX_STAT_INDICES;
//------------------------------------------------------------------------------

typedef struct	// parameters used as part of DEVICE_RCVD_SEGS message
{
    uint8_t fetch_chunk_upper;
    uint8_t fetch_chunk_lower;
    uint8_t fetch_chunk_blocks; // 14
    uint8_t received_segments[9];

} DEVICE_RCVD_SEGS_PARAMETERS;
//------------------------------------------------------------------------------
// This would be more logically located in SureNet.h, however, it needs DEVICE_DATA_STATUS
// which is used in two orthogonal ways, as a parameter in PACKET_DEVICE_AWAKE, and
// to manage the HUB_CONVERSATION. Perhaps they should be separated.
typedef struct
{
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
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t					transmit_end_count;
	SURENET_ENCRYPTION_TYPE	encryption_type;

} DEVICE_TX_PAYLOAD;
//------------------------------------------------------------------------------
// used to send rcvd segs params into the application
typedef struct
{
	DEVICE_RCVD_SEGS_PARAMETERS rcvd_segs_params;
	uint64_t	device_mac;

} DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX;
//------------------------------------------------------------------------------

typedef struct
{
    uint64_t src_addr;
    PACKET_DEVICE_AWAKE_PAYLOAD payload;

} DEVICE_AWAKE_MAILBOX;
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t device_index; // device for which this is intended
	uint8_t len;
	uint16_t chunk_address;
	uint8_t chunk_data[136]; // 136 bytes - 8 byte header (which is 4 byte destination, 4 byte checksum), and 128 bytes of data.

} DEVICE_FIRMWARE_CHUNK;
//------------------------------------------------------------------------------

typedef struct
{
	uint64_t mac_address;
	uint8_t value;

} PING_REQUEST_MAILBOX;
//------------------------------------------------------------------------------
// used to pass a buffer to be transmitted
typedef struct
{
    uint8_t *pucTxBuffer;   // transmit buffer pointer
    uint64_t uiDestAddr;    // destination address
    uint8_t ucBufferLength;
    bool xRequestAck;   // set bit 5 of FCF to 1 if we want an ACK from the other end.

} TX_BUFFER;
//------------------------------------------------------------------------------

typedef struct
{
  uint16_t  packet_length;  // including header and parity after payload
  uint8_t   sequence_number;
  uint8_t   packet_type;	// should be PACKET_TYPE?
  uint64_t  source_address;
  uint64_t  destination_address;
  uint16_t  crc;
  uint8_t   rss;    // received signal strength.
  uint8_t   spare;	// sizeof = 24

} HEADER;
//------------------------------------------------------------------------------

#define MAX_PAYLOAD_SIZE  256

typedef struct
{
  HEADER header;
  uint8_t  payload[MAX_PAYLOAD_SIZE];

} PACKET;
//------------------------------------------------------------------------------

#define T_MESSAGE_PAYLOAD_SIZE 92    //88+4  88 should match MAX_RF_PAYLOAD in global.h, 4 is header size (address and count)
#define T_MESSAGE_HEADER_SIZE	4
#define T_MESSAGE_PARITY_SIZE	1

typedef struct
{
	int16_t command; // from DEVICE_COMMANDS in message_parser.h
	int16_t length;   // total length, i.e. sizeof(command)+sizeof(length)+payload_length
	uint8_t payload[T_MESSAGE_PAYLOAD_SIZE + T_MESSAGE_PARITY_SIZE];

} T_MESSAGE;
//------------------------------------------------------------------------------

typedef struct
{
    T_MESSAGE *ptr;
    bool new_message;
    uint32_t handle;

} MESSAGE_PARAMS;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

typedef union
{
  uint8_t   buffer[MAX_PAYLOAD_SIZE + sizeof(HEADER)];
  PACKET  packet;

} RECEIVED_PACKET;
//------------------------------------------------------------------------------
// used for the mailbox indicating a successful association

//------------------------------------------------------------------------------

typedef enum
{
    SN_ACCEPTED,
    SN_REJECTED,
    SN_CORRUPTED,
    SN_TIMEOUT,

} SN_DATA_RECEIVED_RESPONSE;
//------------------------------------------------------------------------------

typedef struct
{
	uint64_t mac_address;
	bool found_ping;					// set when a ping has been received
	bool report_ping;					// set when a ping should be reported
	uint32_t transmission_timestamp;
	uint32_t reply_timestamp;
	uint32_t ping_attempts;
	uint32_t num_bad_pings;
	uint32_t num_good_pings;
	uint8_t ping_rss;
	uint8_t ping_res[4];
	uint8_t ping_value;
	uint32_t hub_rssi_sum;
	uint32_t device_rssi_sum;

} PING_STATS;
//------------------------------------------------------------------------------

typedef struct
{
	DEVICE_STATUS	device_status;
	uint32_t		line;
	bool			limited;

} DEVICE_STATUS_REQUEST;
//------------------------------------------------------------------------------
// These should come from the rest of the system, or be removed.
typedef uint16_t T_ERROR_CODE;
//==============================================================================
//defines:

#define usTICK_SECONDS 		1000000
#define usTICK_MILLISECONDS	1000
#define usTICK_MINUTE		(usTICK_SECONDS * 60)
#define usTICK_HOUR			(usTICK_MINUTE * 60)

#define xstrfy(x)	strfy(x)
#define strfy(x)	#x
#define MAX_NUMBER_OF_DEVICES   60  // maximum number of devices that can pair with the hub. Limited by sizeof(DEVICE_STATUS) and size of storage area in NVM for table
                                    // Also by width of DETACH_ALL_MASK which is currently a uint64_t

#define ERROR_NONE					0

// Surenet->Surenet-Interface EventGroup flags
#define SURENET_GET_PAIRMODE        (1<<0)
#define SURENET_GET_NUM_PAIRS       (1<<1)
#define SURENET_CLEAR_PAIRING_TABLE (1<<2)
#define SURENET_TRIGGER_CHANNEL_HOP (1<<3)
#define SURENET_GET_CHANNEL			(1<<4)
#define SURENET_GET_DEVICE_TABLE	(1<<5)

#define SURENET_UNASSIGNED_MAC	0x0ull

// These #defines are used to create the Beacon Payload
#define SUREFLAP_HUB                    0x7e
#define HUB_SUPPORTS_THALAMUS           0x02
#define HUB_DOES_NOT_SUPPORT_THALAMUS   0x01
#define HUB_IS_SOLE_PAN_COORDINATOR     0x00
//==============================================================================
//functions:

int snd_init(uint64_t *mac_addr, uint16_t panid, uint8_t channel);
bool set_beacon_request_data(uint64_t mac_address, uint8_t src_address_mode, uint8_t data); // called by MAC with payload from Beacon Request message from device
uint16_t fcs_calculate(uint8_t *data, uint8_t len);
void snd_stack_init(void);
void snd_stack_task(void);

void snd_set_channel(uint8_t ucChannel);
uint8_t snd_get_channel(void);
bool snd_transmit_packet(TX_BUFFER *pcTxBuffer);
bool snd_have_we_seen_beacon(uint64_t mac_address);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
