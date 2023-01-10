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
* Filename: Surenet-Interface.h
* Author:   Chris Cowdery 23/07/2019
*
* This file handles the inter-task communication with SureNet. It contains weakly linked
* functions which can be overridden by the correct handlers in the application
* The weakly linked handlers exist in the context of the main applicaton.
*
*
**************************************************************************/

#ifndef SURENET_INTERFACE_H
#define	SURENET_INTERFACE_H
#include "devices.h"
// Public constants
#define MAX_PAYLOAD_SIZE  256

// public data structures
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

typedef struct
{
  HEADER header;
  uint8_t  payload[MAX_PAYLOAD_SIZE];
} PACKET;

typedef union
{
  uint8_t   buffer[MAX_PAYLOAD_SIZE+sizeof(HEADER)];
  PACKET  packet;
} RECEIVED_PACKET;

//General message struct
#define T_MESSAGE_PAYLOAD_SIZE 92    //88+4  88 should match MAX_RF_PAYLOAD in global.h, 4 is header size (address and count)
#define T_MESSAGE_HEADER_SIZE	4
#define T_MESSAGE_PARITY_SIZE	1

typedef struct
{
	int16_t command; // from DEVICE_COMMANDS in message_parser.h
	int16_t length;   // total length, i.e. sizeof(command)+sizeof(length)+payload_length
	uint8_t payload[T_MESSAGE_PAYLOAD_SIZE+T_MESSAGE_PARITY_SIZE];
}T_MESSAGE;

typedef struct
{
    T_MESSAGE *ptr;
    bool new_message;
    uint32_t handle;
} MESSAGE_PARAMS;

typedef enum	// used to indicate where a pairing request came from
{
	PAIRING_REQUEST_SOURCE_UNKNOWN,
	PAIRING_REQUEST_SOURCE_SERVER,
	PAIRING_REQUEST_SOURCE_BUTTON,
	PAIRING_REQUEST_SOURCE_CLI,
	PAIRING_REQUEST_SOURCE_TIMEOUT,
	PAIRING_REQUEST_SOURCE_BEACON_REQUEST,
} PAIRING_REQUEST_SOURCE;

typedef struct
{
	uint32_t timestamp;
	bool enable;
	PAIRING_REQUEST_SOURCE source;
} PAIRING_REQUEST;

typedef struct // used for the mailbox indicating a successful association
{
    uint64_t association_addr;
    uint8_t association_dev_type;
    uint8_t association_dev_rssi;
	PAIRING_REQUEST_SOURCE source;	// who put us in association mode in the first place
} ASSOCIATION_SUCCESS_INFORMATION;

typedef enum
{
    SN_ACCEPTED,
    SN_REJECTED,
    SN_CORRUPTED,
    SN_TIMEOUT,
} SN_DATA_RECEIVED_RESPONSE;

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

typedef struct
{
	DEVICE_STATUS	device_status;
	uint32_t		line;
	bool			limited;
} DEVICE_STATUS_REQUEST;

// These should come from the rest of the system, or be removed.
typedef uint16_t T_ERROR_CODE;
#define ERROR_NONE					0

// Surenet->Surenet-Interface EventGroup flags
#define SURENET_GET_PAIRMODE        (1<<0)
#define SURENET_GET_NUM_PAIRS       (1<<1)
#define SURENET_CLEAR_PAIRING_TABLE (1<<2)
#define SURENET_TRIGGER_CHANNEL_HOP (1<<3)
#define SURENET_GET_CHANNEL			(1<<4)
#define SURENET_GET_DEVICE_TABLE	(1<<5)

#define SURENET_UNASSIGNED_MAC	0x0ull

// The following are interface calls to / from SureNet
BaseType_t surenet_init(uint64_t *mac, uint16_t panid, uint8_t channel);  // initialises SureNet, SureNet Driver and the RF Stack.
void surenet_send_firmware_chunk(DEVICE_FIRMWARE_CHUNK *device_firmware_chunk);
void Surenet_Interface_Handler(); // must be called of the order of every 10ms at the slowest to ensure comms is quick with devices.
bool surenet_is_device_online(uint64_t mac_addr);
void surenet_unpair_device(uint64_t mac_addr);    // tell that device to clear off
void surenet_unpair_device_by_index(uint8_t index);    // tell that device to clear off
void surenet_set_hub_pairing_mode(PAIRING_REQUEST mode);
PAIRING_REQUEST surenet_get_hub_pairing_mode(void);
uint8_t surenet_how_many_pairs(void);
void surenet_hub_clear_pairing_table(void);
void surenet_trigger_channel_hop(void);
uint8_t surenet_get_channel(void);	// This will spin until it gets the answer
uint8_t surenet_set_channel(uint8_t channel);
void surenet_ping_device(uint64_t mac_address, uint8_t value);
uint32_t surenet_get_last_heard_from(void);
BaseType_t surenet_update_device_table_line(DEVICE_STATUS* status, uint32_t line, bool limited, bool wait);

// callbacks
void surenet_device_rcvd_segs_cb(DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX *params);
bool surenet_get_next_message_cb(uint64_t mac, T_MESSAGE **current_request_ptr, uint32_t *current_request_handle);
void surenet_clear_message_cb(uint32_t current_message_handle);
SN_DATA_RECEIVED_RESPONSE surenet_data_received_cb(RECEIVED_PACKET *rx_packet);
void surenet_pairing_mode_change_cb(PAIRING_REQUEST);
void surenet_device_pairing_success_cb(ASSOCIATION_SUCCESS_INFORMATION *assoc_info);    // call back to say association successful
void surenet_device_awake_notification_cb(DEVICE_AWAKE_MAILBOX *device_awake_mailbox);
void surenet_ping_response_cb(PING_STATS *ping_result);
void surenet_blocking_test_cb(uint32_t blocking_test_value);
#endif	/* SURENET_H */

