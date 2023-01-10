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
* Filename: SureNet.c
* Author:   Chris Cowdery
* 31/07/2013 - cjc - first revision
* 02/07/2019 - cjc - Massive overhaul for Hub2 project.
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
#include "Surenet-Interface.h"
#include "devices.h"
#include "SureNet.h"
#include "SureNetDriver.h"
#include "Block_XTEA.h"
#include "utilities.h"
#include "RegisterMap.h"
#include "DeviceFirmwareUpdate.h"	// for some useful parameters describing firmware chunks
#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/chacha.h"

#define PET_DOOR_DELAY		true	// This introduces a 50ms delay for Pet Doors between 
									// PACKET_DEVICE_AWAKE and the sending of PACKET_END_SESSION

#define PRINT_SURENET_ACK	false
#define PRINT_SURENET		false // gets SureNet to emit some useful debug messages.

#if SURENET_ACTIVITY_LOG
#define surenet_log_add(a, b) snla(a,b)
#else
#define surenet_log_add(a, b)
#endif

#if PRINT_SURENET
#define surenet_printf(...)			zprintf(LOW_IMPORTANCE, __VA_ARGS__)
#else
#define surenet_printf(...)
#endif

#if PRINT_SURENET_ACK
#define surenet_ack_printf(...)		zprintf(MEDIUM_IMPORTANCE, __VA_ARGS__)
#else
#define surenet_ack_printf(...)
#endif

#define	TIMEOUT_SECURITY_ACK		(usTICK_MILLISECONDS * 100)	// Although the Device should respond in 5.5ms, it actually takes it over 60ms...
#define	TIMEOUT_WAIT_FOR_DATA_ACK	(usTICK_MILLISECONDS * 25)
#define	TIMEOUT_DEVICE_STAYS_AWAKE	(usTICK_MILLISECONDS * 100)
// temporary logging stuff to see how the timing goes with lots of Devices attached.
#if SURENET_ACTIVITY_LOG
#define NUM_SURENET_LOG_ENTRIES	75
SURENET_LOG_ENTRY surenet_log_entry[NUM_SURENET_LOG_ENTRIES];
uint8_t	surenet_log_entry_in = 0;
void dump_surenet_log(void);
void snla(SURENET_LOG_ACTIVITY activity, uint8_t parameter);
#endif

// Private defines
#define START_CHANNEL 				11
#define NUMBER_OF_CHANNELS 			16
#define ACKNOWLEDGE_QUEUE_SIZE 		16
#define DATA_ACKNOWLEDGE_QUEUE_SIZE	16
#define ACKNOWLEDGE_LIFETIME  		25    // lifetime of acknowledges in 100ms increments (about!)
#define DATA_ACKNOWLEDGE_LIFETIME  	25    // lifetime of acknowledges in 100ms increments (about!)

#define SECURITY_KEY_SIZE	16
#define SECURITY_KEY_00 0x45  // E
#define SECURITY_KEY_01 0x75  // u
#define SECURITY_KEY_02 0x6c  // l
#define SECURITY_KEY_03 0x65  // e
#define SECURITY_KEY_04 0x72  // r
#define SECURITY_KEY_05 0x60  // '
#define SECURITY_KEY_06 0x73  // s
#define SECURITY_KEY_07 0x20  //
#define SECURITY_KEY_08 0x49  // I
#define SECURITY_KEY_09 0x64  // d
#define SECURITY_KEY_10 0x65  // e
#define SECURITY_KEY_11 0x6e  // n
#define SECURITY_KEY_12 0x74  // t
#define SECURITY_KEY_13 0x69  // i
#define SECURITY_KEY_14 0x74  // t
#define SECURITY_KEY_15 0x79  // y

#define MAX_SECURITY_KEY_USES	16

#define MAX_DETACH_RETRIES  5   // number of times the hub tries to send PACKET_DETACH to a device before giving up on it
                                 //need to be patient, RF module is asleep most of the time
                                 // Note this is multiplied by the HUB_TX_MAX_RETRIES constant as that is how many times
                                 // a packet is sent if the ACK does not arrive.
#define HUB_TX_MAX_RETRIES  5   // number of times the hub tries to send PACKET_DATA to a device before giving up on it
#define DETACH_ALL_MASK ((2^MAX_NUMBER_OF_DEVICES)-1);
#define MAX_BAD_KEY 8

// handle aging of received packets (stores their sequence number for a short period of time so we can check for duplicates)
#define RX_SEQ_BUFFER_SIZE 64

#define SEQUENCE_STORE_LIFETIME 20    // number of 100ms periods that a received packet sequence / mac is stored for
                                      // to enable repeat packets to be dropped.
#define SEQUENCE_STORE_LIFETIME_LONG 250    // longer wait for data messages

// Private enumerations
typedef enum
{
    PACKET_UNKNOWN,
    PACKET_DATA,
    PACKET_DATA_ACK,
    PACKET_ACK,
    PACKET_BEACON,
    PACKET_PAIRING_REQUEST,
    PACKET_PAIRING_CONFIRM,
    PACKET_CHANNEL_HOP,
    PACKET_DEVICE_AWAKE,
    PACKET_DEVICE_TX, // 9
    PACKET_END_SESSION,
    PACKET_DETACH,
    PACKET_DEVICE_SLEEP,
    PACKET_DEVICE_P2P,
    PACKET_ENCRYPTION_KEY,
    PACKET_REPEATER_PING,
    PACKET_PING,
    PACKET_PING_R,
    PACKET_REFUSE_AWAKE,
    PACKET_DEVICE_STATUS, // 0x13 / 19
    PACKET_DEVICE_CONFIRM,  //20
    PACKET_DATA_SEGMENT,
	PACKET_BLOCKING_TEST,
	PACKET_DATA_ALT_ENCRYPTED,		
} PACKET_TYPE;

typedef enum
{
    DETACH_IDLE,
    DETACH_FIND_NEXT_PAIR,
    DETACH_SEND_DETACH_COMMAND,
    DETACH_WAIT_FOR_ACK,
    DETACH_FINISH,
} DETACH_STATE;

typedef struct
{
    uint8_t seq_acknowledged;
    uint8_t ack_nack;
    uint8_t valid;
} ACKNOWLEDGE_QUEUE;

typedef enum
{
    ACK_NOT_ARRIVED,
    ACK_ARRIVED,
    NACK_ARRIVED
} ACK_STATUS;

typedef struct	// used to store information about a received message, and if it were a PACKET_DATA, the response sent
{
    uint8_t sequence_number;
    uint8_t valid;
    uint64_t src_mac;
	uint8_t ack_response;
} RECEIVED_SEQUENCE_NUMBER;

typedef struct
{
	uint32_t	timestamp;
	uint8_t 	data[CHUNK_SIZE];
	uint8_t len;	// 0 means no data.
	uint16_t chunk_address;
	bool has_data;
	uint8_t 	device_index;
} FIRMWARE_CHUNK;

typedef enum
{
	CAN_SLEEP 	= 0,	// Functions that return this value have their answers OR'd together
	BUSY 		= 1,	// to get an aggregate. Therefore if any are BUSY, the aggregate will also be BUSY.
} BUSY_STATE;

// Some debug data
char *packet_names[] = {"PACKET_UNKNOWN",
                        "PACKET_DATA",
                        "PACKET_DATA_ACK",
                        "PACKET_ACK",
                        "PACKET_BEACON",
                        "PACKET_PAIRING_REQUEST",
                        "PACKET_PAIRING_CONFIRM",
                        "PACKET_CHANNEL_HOP",
                        "PACKET_DEVICE_AWAKE",
                        "PACKET_DEVICE_TX",
                        "PACKET_END_SESSION",
                        "PACKET_DETACH",
                        "PACKET_DEVICE_SLEEP",
                        "PACKET_DEVICE_P2P",
                        "PACKET_ENCRYPTION_KEY",
                        "PACKET_REPEATER_PING",
                        "PACKET_PING",
                        "PACKET_PING_R",
                        "PACKET_REFUSE_AWAKE",
                        "PACKET_DEVICE_STATUS",
                        "PACKET_DEVICE_CONFIRM",
                        "PACKET_DATA_SEGMENT",
						"PACKET_BLOCKING_TEST",
						"PACKET_DATA_ALT_ENCRYPTED"};

// Check with Tom as to why this lot was required / and he has put the #defines in the chacha.h header, i.e. modifying WolfSSL?
#define CHACHA_CONST_WORDS	4
#define CHACHA_CONST_BYTES	(CHACHA_CONST_WORDS * sizeof(word32))
typedef union 
{
	uint32_t	words[CHACHA_CHUNK_WORDS];
	ChaCha		WolfStruct;
	struct 
	{
		uint8_t		constant[CHACHA_CONST_BYTES];
		uint8_t		key[CHACHA_MAX_KEY_SZ];
		uint32_t	position;
		uint8_t		IV[CHACHA_IV_WORDS];
	};
} CHACHA_CONTEXT;

// Private variables
uint64_t *rf_mac;   // range 0x08 0x00 to 0x08 0xff is for Hub2 development
static uint8_t SecurityKeys[MAX_NUMBER_OF_DEVICES][SECURITY_KEY_SIZE];
static uint8_t SecretKeys[MAX_NUMBER_OF_DEVICES][CHACHA_MAX_KEY_SZ];
TaskHandle_t surenet_task_handle = NULL;

static ACKNOWLEDGE_QUEUE acknowledge_queue[ACKNOWLEDGE_QUEUE_SIZE];    // 3x16=48bytes
static ACKNOWLEDGE_QUEUE data_acknowledge_queue[DATA_ACKNOWLEDGE_QUEUE_SIZE];  // 3x16=48bytes
static RECEIVED_SEQUENCE_NUMBER received_sequence_numbers[RX_SEQ_BUFFER_SIZE];
static uint32_t most_recent_rx_time;  // set when a PACKET_DATA is received by surenet_data_received()
static PAIRING_REQUEST pairing_mode;   // Note that this variable is set by calls to sn_set_hub_pairing_mode(), and is read by calls to sn_get_hub_pairing_mode()
static bool trigger_channel_hop=false;
static bool conv_end_session_flag=false;   // set when a PACKET_END_SESSION is received
static bool sn_transmission_complete=false;
static int current_conversee=0;
// Detach handler
static uint64_t detach_device_bits = 0;    // sizeof this is one determinant of the max number of pairs.
static DETACH_STATE detach_state=DETACH_IDLE;

// Ping related
PING_STATS ping_stats;
uint8_t ping_seq = 0;		// Ping sequence number.

// Device Firmware Update
FIRMWARE_CHUNK firmware_chunk[DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES];
uint8_t device_rcvd_segs[MAX_NUMBER_OF_DEVICES];	//0x00 is when both segments of a chunk are needed, 0x01 when only the 2nd segment.

// Externals
extern uint8_t uptime_min_count;	// Hub uptime - used for once-per-hour Pet Door messages
extern DEVICE_STATUS device_status[MAX_NUMBER_OF_DEVICES];
extern DEVICE_STATUS_EXTRA device_status_extra[MAX_NUMBER_OF_DEVICES];
extern bool remove_pairing_table_entry(uint64_t mac);
extern uint8_t hub_debug_mode;

// Mailboxes to communicate with SureNet-Interface
QueueHandle_t xAssociationSuccessfulMailbox;        // comes with data ASSOCIATION_SUCCESS_INFORMATION describing the new device
QueueHandle_t xRxMailbox;
QueueHandle_t xRxMailbox_resp;
QueueHandle_t xGetNumPairsMailbox_resp;
QueueHandle_t xSetPairingModeMailbox;
QueueHandle_t xUnpairDeviceMailbox;
QueueHandle_t xUnpairDeviceByIndexMailbox;
QueueHandle_t xGetPairmodeMailbox_resp;
QueueHandle_t xRemoveFromPairingTable;
QueueHandle_t xIsDeviceOnlineMailbox_resp;
QueueHandle_t xIsDeviceOnlineMailbox;
QueueHandle_t xGetNextMessageMailbox;
QueueHandle_t xGetNextMessageMailbox_resp;
QueueHandle_t xDeviceAwakeMessageMailbox;
QueueHandle_t xPairingModeHasChangedMailbox;
QueueHandle_t xGetChannelMailbox_resp;
QueueHandle_t xSetChannelMailbox;
QueueHandle_t xSendDeviceTableMailbox;
QueueHandle_t xPingDeviceMailbox;
QueueHandle_t xPingDeviceMailbox_resp;
QueueHandle_t xDeviceRcvdSegsParametersMailbox;
QueueHandle_t xDeviceFirmwareChunkMailbox;
QueueHandle_t xGetLastHeardFromMailbox_resp;
QueueHandle_t xBlockingTestMailbox;

// EventGroup to communicate with SureNet-Interface
EventGroupHandle_t xSurenet_EventGroup;

// Private Functions
void sn_task(void *pvParameters);
ACK_STATUS has_ack_arrived(int16_t);
ACK_STATUS has_data_ack_arrived(int16_t);
int16_t sn_transmit_packet(PACKET_TYPE type, uint64_t dest, uint8_t *payload_ptr, uint16_t length, int16_t seq, bool request_ack);
BUSY_STATE detach_handler(void);
BUSY_STATE hub_functions(void);
void age_acknowledge_queue(uint8_t ticks);
void age_data_acknowledge_queue(uint8_t ticks);
void age_sequence_numbers(uint8_t ticks);
void timeout_handler(void);
void rx_seq_init(void);
uint8_t send_data_segment(uint8_t *data, uint16_t start_block, uint8_t num_blocks);
bool is_packet_new(RECEIVED_PACKET *rx_packet, uint8_t *ack_payload);
bool isMacAddressForUs(uint64_t destination);
void sn_channel_control(uint8_t chn);
void store_ack_response(uint64_t mac, uint8_t seq_no, uint8_t ack_response);
void sn_ping_device(uint64_t device, uint8_t value);
void sn_send_chunk(uint8_t current_conversee,FIRMWARE_CHUNK *firmware_chunk);
void store_firmware_chunk(DEVICE_FIRMWARE_CHUNK *device_firmware_chunk);
int is_there_a_firmware_chunk(uint8_t conversee);

// Chacha / security stuff
void ChaCha_Encrypt(uint8_t* data, uint32_t length, uint32_t position, uint8_t pair_index);
PACKET_TYPE	sn_Encrypt(uint8_t* data, uint32_t length, uint32_t position, uint8_t dest_index);
void sn_Decrypt(uint8_t* data, uint32_t length, uint32_t pair_index, uint32_t position);

//////////////////////
// Interfacing and Task functions
//////////////////////
/**************************************************************
 * Function Name   : sn_init
 * Description     : Called before scheduler starts to initialise SureNet.
 *                 : Initialises some tasks, and also initialises the Surenet Driver.
 * Inputs          :
 * Outputs         :
 * Returns         : pdPASS if it was successful, pdFAIL if it could not complete (e.g. couldn't set up a task)
 **************************************************************/
BaseType_t sn_init(uint64_t *mac, uint16_t panid, uint8_t channel)
{
    int i;

    if (pdPASS != snd_init(mac, panid, channel)) // Create ISR task first
        return pdFAIL;
    rf_mac = mac;

    rx_seq_init();
    for (i=0; i<ACKNOWLEDGE_QUEUE_SIZE; i++)
        acknowledge_queue[i].valid=0;
    for (i=0; i<DATA_ACKNOWLEDGE_QUEUE_SIZE; i++)
        data_acknowledge_queue[i].valid=0;
    for (i=0; i<RX_SEQ_BUFFER_SIZE; i++)
        received_sequence_numbers[i].valid=0;

    for (i=0; i<MAX_NUMBER_OF_DEVICES; i++)
    {
        device_status_extra[i].send_detach = 0;
        device_status_extra[i].device_awake_status.awake=DEVICE_ASLEEP;
        device_status_extra[i].device_awake_status.data=DEVICE_HAS_NO_DATA;
        device_status_extra[i].device_web_connected = false;
        device_status_extra[i].SendSecurityKey=SECURITY_KEY_OK;
		device_status_extra[i].SecurityKeyUses=0;
		device_status_extra[i].encryption_type=SURENET_CRYPT_BLOCK_XTEA;
    }

	for( i=0; i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES; i++)
	{
		firmware_chunk[i].has_data = false;
		firmware_chunk[i].device_index = 0;
		firmware_chunk[i].timestamp = 0;
    }


    // Now set up some mailboxes and an Event Group to allow data to be sent between SureNet and the outside world
    // We will extern them from SureNet.
    xAssociationSuccessfulMailbox = xQueueCreate(1, sizeof(ASSOCIATION_SUCCESS_INFORMATION));
    xRxMailbox					= xQueueCreate(1, sizeof(RECEIVED_PACKET)); // This has actual received packets in there
    xRxMailbox_resp				= xQueueCreate(1, sizeof(SN_DATA_RECEIVED_RESPONSE));  // Flag to say if application accepted this packet.
    xGetNumPairsMailbox_resp	= xQueueCreate(1, sizeof(uint8_t));
    xSetPairingModeMailbox		= xQueueCreate(1, sizeof(PAIRING_REQUEST)); // queue with depth of one, and one parameter (bool)
    xUnpairDeviceMailbox		= xQueueCreate(1, sizeof(uint64_t));
	xUnpairDeviceByIndexMailbox = xQueueCreate(1, sizeof(uint8_t));
    xGetPairmodeMailbox_resp	= xQueueCreate(1, sizeof(PAIRING_REQUEST));
    xRemoveFromPairingTable		= xQueueCreate(1, sizeof(uint64_t));
    xIsDeviceOnlineMailbox		= xQueueCreate(1, sizeof(uint64_t)); // queue with depth of one, and one parameter (uint64_t)
    xIsDeviceOnlineMailbox_resp	= xQueueCreate(1, sizeof(bool)); // queue with depth of one, and one parameter (uint64_t)
    xGetNextMessageMailbox		= xQueueCreate(2, sizeof(uint64_t));  // Queue to send mac address up to application to see if it has messages for that destination
    xGetNextMessageMailbox_resp	= xQueueCreate(1, sizeof(MESSAGE_PARAMS));    // queue to send get_next_message info into SureNet
    xDeviceAwakeMessageMailbox	= xQueueCreate(1, sizeof(DEVICE_AWAKE_MAILBOX));  // queue for device awake message payload for app.
    xPairingModeHasChangedMailbox = xQueueCreate(1, sizeof(PAIRING_REQUEST));  // queue for pairing mode changed.
	xGetChannelMailbox_resp		= xQueueCreate(1, sizeof(uint8_t));
	xSetChannelMailbox			= xQueueCreate(1, sizeof(uint8_t));
	xSendDeviceTableMailbox		= xQueueCreate(1, sizeof(DEVICE_STATUS)*MAX_NUMBER_OF_DEVICES);
	xPingDeviceMailbox			= xQueueCreate(1, sizeof(PING_REQUEST_MAILBOX));
	xPingDeviceMailbox_resp		= xQueueCreate(1, sizeof(PING_STATS));
	xDeviceRcvdSegsParametersMailbox = xQueueCreate(1,sizeof(DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX));
	xDeviceFirmwareChunkMailbox = xQueueCreate(1,sizeof(DEVICE_FIRMWARE_CHUNK));
	xGetLastHeardFromMailbox_resp = xQueueCreate(1, sizeof(uint32_t));
	xBlockingTestMailbox		=xQueueCreate(3, sizeof(uint32_t));

	xSurenet_EventGroup = xEventGroupCreate();

	//xQueueSend(xSendDeviceTableMailbox, &device_status, 0);	// Send all device statuses to Register Map.
	// This was a waste of time as this mailbox is only checked after an SURENET_GET_DEVICE_TABLE is received.

	// Now set up Security Keys for each device

#if USE_RANDOM_KEY
	uint32_t j;
#else
	uint8_t key[SECURITY_KEY_SIZE] = {SECURITY_KEY_00,SECURITY_KEY_01,SECURITY_KEY_02,SECURITY_KEY_03, \
									  SECURITY_KEY_04,SECURITY_KEY_05,SECURITY_KEY_06,SECURITY_KEY_07, \
									  SECURITY_KEY_08,SECURITY_KEY_09,SECURITY_KEY_10,SECURITY_KEY_11, \
									  SECURITY_KEY_12,SECURITY_KEY_13,SECURITY_KEY_14,SECURITY_KEY_15};
#endif
	for (i=0; i<MAX_NUMBER_OF_DEVICES; i++)
	{
#if USE_RANDOM_KEY
		sn_GenerateSecurityKey(i);
#else
		memcpy(SecurityKeys[i],key,SECURITY_KEY_SIZE);
#endif
	}

        // now start the SureNet task
    if (xTaskCreate(sn_task, "SureNet", SURENET_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &surenet_task_handle) != pdPASS)
    {
        zprintf(CRITICAL_IMPORTANCE, "SureNet task creation failed!.\r\n");
        return pdFAIL;
    }

    return pdPASS;
}

/**************************************************************
 * Function Name   : sn_task
 * Description     : Main SureNet task. Runs as part of the RTOS.
 *                 : Also polls the various state machines.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_task(void *pvParameters)
{
    uint32_t diff;
    EventBits_t xEventBits;
    uint8_t u8result;
//    uint32_t minute_tick = get_microseconds_tick();
    uint32_t ageing_tick = get_microseconds_tick();
    uint64_t u64result;
	uint32_t u32result;
    bool bresult;
	PAIRING_REQUEST presult;
	PING_REQUEST_MAILBOX ping_request;
	DEVICE_FIRMWARE_CHUNK device_firmware_chunk;
	BUSY_STATE can_i_sleep;
	uint8_t	i;

	memset(&ping_stats,0,sizeof(ping_stats));

    snd_stack_init();    // start initialising the RF Stack. Note that multiple calls to snd_stack_task()
                        // are required before initialisation is complete.
    sn_devicetable_init();

	// Do this after initialising device_status[] as SecretKey requires MAC addresses
	for (i=0; i<MAX_NUMBER_OF_DEVICES; i++)	{ sn_CalculateSecretKey(i); }
	
    while(1)
    {
        snd_stack_task();  // run lower levels of stack. Note this is also called as part of sn_transmit_packet()

        // Check on any mailboxes for data to despatch into SureNet
        if( (uxQueueMessagesWaiting(xSetPairingModeMailbox) > 0) &&
		    (xQueueReceive( xSetPairingModeMailbox, &presult, 0 )==pdPASS) )
        {
            sn_set_hub_pairing_mode(presult);
        }
        if( (uxQueueMessagesWaiting(xUnpairDeviceMailbox) > 0) &&
		    (xQueueReceive( xUnpairDeviceMailbox, &u64result, 0 )==pdPASS) )
        {
            sn_unpair_device(u64result);
        }
        if( (uxQueueMessagesWaiting(xUnpairDeviceByIndexMailbox) > 0) &&
		    (xQueueReceive( xUnpairDeviceByIndexMailbox, &u8result, 0 )==pdPASS) )
        {
            sn_unpair_device(get_mac_from_index(u8result));
        }
        if( (uxQueueMessagesWaiting(xIsDeviceOnlineMailbox) > 0) &&
		    (xQueueReceive( xIsDeviceOnlineMailbox, &u64result, 0 )==pdPASS) )
        {
            bresult = sn_is_device_online(u64result);
            xQueueSend(xIsDeviceOnlineMailbox_resp,&bresult,0);   // put result in mailbox for Surenet-Interface
        }
        if( (uxQueueMessagesWaiting(xSetChannelMailbox) > 0) &&
		     (xQueueReceive( xSetChannelMailbox, &u8result, 0 )==pdPASS) )
        {
            snd_set_channel(u8result);
        }
        if( (uxQueueMessagesWaiting(xPingDeviceMailbox) > 0) &&
		    (xQueueReceive( xPingDeviceMailbox, &ping_request, 0 )==pdPASS) )
        {
            sn_ping_device(ping_request.mac_address,ping_request.value);
        }

        if( (uxQueueMessagesWaiting(xRemoveFromPairingTable) > 0) &&
		    (xQueueReceive(xRemoveFromPairingTable,&u64result,0)==pdPASS) )
        {	//Something in mailbox to unpair from pairing table
          remove_pairing_table_entry(u64result);
        }

       // Check on EventGroup messages to see if we have any info waiting for us
       // in SureNet Driver.

        xEventBits = xEventGroupWaitBits(xSurenet_EventGroup,
                                         SURENET_GET_PAIRMODE,
                                         pdTRUE,pdFALSE,0);
        if ((xEventBits & SURENET_GET_PAIRMODE)!=0)
        {   // Received request for current pairing mode
            PAIRING_REQUEST resp;
            resp = sn_get_hub_pairing_mode();   // this just retrieves a module level variable
            xQueueSend(xGetPairmodeMailbox_resp,&resp,0);   // put result in mailbox for Surenet-Interface
        }

        xEventBits = xEventGroupWaitBits(xSurenet_EventGroup,
                                         SURENET_GET_NUM_PAIRS,
                                         pdTRUE,pdFALSE,0);
        if ((xEventBits & SURENET_GET_NUM_PAIRS)!=0)
        {   // Received request for number of paired devices
            uint8_t num_pairs;
            num_pairs = sn_how_many_pairs();
            xQueueSend(xGetNumPairsMailbox_resp,&num_pairs,0);   // put result in mailbox for Surenet-Interface
        }

        xEventBits = xEventGroupWaitBits(xSurenet_EventGroup,
                                         SURENET_CLEAR_PAIRING_TABLE,
                                         pdTRUE,pdFALSE,0);
        if ((xEventBits & SURENET_CLEAR_PAIRING_TABLE)!=0)
        {   // Received request for number of paired devices
            sn_hub_clear_pairing_table();
        }

        xEventBits = xEventGroupWaitBits(xSurenet_EventGroup,
                                         SURENET_TRIGGER_CHANNEL_HOP,
                                         pdTRUE,pdFALSE,0);
        if ((xEventBits & SURENET_TRIGGER_CHANNEL_HOP)!=0)
        {   // Received request to trigger a channel hop
            sn_channel_control(0xff);
        }

        xEventBits = xEventGroupWaitBits(xSurenet_EventGroup,
                                         SURENET_GET_CHANNEL,
                                         pdTRUE,pdFALSE,0);
        if ((xEventBits & SURENET_GET_CHANNEL)!=0)
        {   // Received request to retrieve channel number
            u8result = snd_get_channel();
			xQueueSend(xGetChannelMailbox_resp,&u8result,0);   // put result in mailbox for Surenet-Interface
        }

		xEventBits = xEventGroupWaitBits(xSurenet_EventGroup,
                                         SURENET_GET_DEVICE_TABLE,
                                         pdTRUE,pdFALSE,0);
        if( (xEventBits & SURENET_GET_DEVICE_TABLE) != 0 )
        {   // Register Map wants to update its Device Table.
            xQueueSend(xSendDeviceTableMailbox, &device_status, 0);
        }

		xEventBits = xEventGroupWaitBits(xSurenet_EventGroup,
                                         SURENET_GET_LAST_HEARD_FROM,
                                         pdTRUE,pdFALSE,0);
        if( (xEventBits & SURENET_GET_LAST_HEARD_FROM) != 0 )
        {   // Main loop wants to know when we last heard from a device
			u32result = last_heard_from();
            xQueueSend(xGetLastHeardFromMailbox_resp, &u32result, 0);
        }

		// It is important that this next Mailbox handler comes between snd_stack_task()
		// and hub_functions(). This is because an incoming message requesting firmware will
		// be captured by snd_stack_task() and passed to the DFU handler for fulfillment.
		// The DFU handler will pass it's response into this mailbox, to be passed back
		// to the Device in hub_functions(). To allow the response to be passed back
		// to the device in the same Hub conversation as it was requested, it is necessary
		// for the handler ultimately called by snd_stack_task() to sleep this task for
		// a short period to allow DFU to complete it's processing before this task carries
		// on. To allow that to happen, the ordering of snd_stack_task(), this Mailbox, and
		// hub_functions() is crucial. The consequence of getting it wrong is that the
		// Device will request each firmware chunk twice, it will be sent it twice, and
		// because the final request triggers a Device Reboot, it will reboot twice which
		// looks the same as a failed firmware update!
		if (pdPASS == xQueueReceive(xDeviceFirmwareChunkMailbox,&device_firmware_chunk,0))
		{	// Queue data in a buffer to be sent at the next opportunity
			store_firmware_chunk(&device_firmware_chunk);
		}

        diff = get_microseconds_tick() - ageing_tick; // how long since the last time
        if( diff > usTICK_MILLISECONDS * 100 ) // will trigger at 100,000
        {
            diff = diff / (usTICK_MILLISECONDS * 100);
            if( diff > 255 ) diff = 255; // cap it because the age used in the queues is only 8 bits.
            age_acknowledge_queue((uint8_t)diff);
            age_data_acknowledge_queue((uint8_t)diff);
            age_sequence_numbers((uint8_t)diff);
            ageing_tick = get_microseconds_tick();
        }

		if (ping_stats.report_ping==true)
		{	// Need to send some ping results up to the application to be sent on to the Server
			xQueueSend(xPingDeviceMailbox_resp,&ping_stats,0);
			ping_stats.report_ping = false;	// just send the result the once.
		}
        can_i_sleep = hub_functions();    // run the state machine that oversees hub functions, e.g. send beacons, handle pairing requests etc. etc.
        can_i_sleep |= detach_handler();   // state machine to control detaching from all devices
        timeout_handler();  // check the time we last chatted to a device and mark it offline if it was a while ago
    	if( CAN_SLEEP == can_i_sleep)
		{	// there are no ongoing conversations with any devices, so we can afford a brief nap!
			vTaskDelay(pdMS_TO_TICKS( 1 ));
		}
	}
}

/**************************************************************
 * Function Name   : sn_device_pairing_success
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_device_pairing_success(ASSOCIATION_SUCCESS_INFORMATION *assoc_info)    // call back to say association successful
{
	bool known;
	known = is_device_already_known(assoc_info->association_addr, assoc_info->association_dev_type);
	// The logic is a bit counter-intuitive here - if the device is known already
	// then adding the mac will fail silently (no problem), and add_device_characteristics
	// will update the device RSSI (which we do want)
	add_mac_to_pairing_table(assoc_info->association_addr);   // store MAC in pairing table
	add_device_characteristics_to_pairing_table(assoc_info->association_addr,assoc_info->association_dev_type,assoc_info->association_dev_rssi);
	trigger_key_send(assoc_info->association_addr);   // need to send a new encryption key to the Device
	if (known==false)
	{
		store_device_table();	// write result to NVM
	}

    xQueueSend(xAssociationSuccessfulMailbox,assoc_info,0);   // put result in mailbox for Surenet-Interface
}

/**************************************************************
 * Function Name   : sn_transmit_packet()
 * Description     : Adds a packet into the TX buffer.
 *                 : Also note that this function will sleep until the RF Stack has dealt with the transmitted packet.
 * Inputs          : seq - if seq==-1, then the packet is sent with the next sequence number, if seq>=0, then seq is used as the seq number
 * Outputs         :
 * Returns         : -1 if the transmit failed, or the sequence number if the message was successfully queued.
 **************************************************************/
int16_t sn_transmit_packet(PACKET_TYPE type, uint64_t dest, uint8_t *payload_ptr, uint16_t length, int16_t seq, bool request_ack)
{
    TX_BUFFER pcTxBuffer;
    uint32_t timestamp;
    static uint8_t sequence_number=0;
    uint8_t PayLoadLen;
    int i;
    static uint8_t PayLoad[127];  //max 802.15.4 packet size
    // We add two bytes to the beginning of the payload, PACKET_TYPE and SEQUENCE_NUMBER.

    PayLoad[0] = type;	// PACKET_TYPE

    if (seq==-1)  // use next number in sequence
	{
      seq = sequence_number++;  
	}
	
    PayLoad[1] = seq;   // SEQUENCE_NUMBER

    if(length>sizeof(PayLoad)-2)
    {
      surenet_printf("payload too long\r\n");
      return -1;
    }

    for( i = 0; i < length; i++ ) PayLoad[i+2] = *payload_ptr++;  // copy payload over into our buffer

    if (PACKET_DATA == type)  //if PACKET_DATA, then encrypt the payload
    {
      	PayLoad[0] = sn_Encrypt(&PayLoad[2], length, PayLoad[1], current_conversee);
    }

    PayLoadLen = length + 2;

    pcTxBuffer.pucTxBuffer = PayLoad;
    pcTxBuffer.ucBufferLength = PayLoadLen;
    pcTxBuffer.uiDestAddr = dest;
    pcTxBuffer.xRequestAck = request_ack;

    sn_transmission_complete=false;
    timestamp = get_microseconds_tick();

    if (false==snd_transmit_packet(&pcTxBuffer))
    {
		surenet_printf("TX Fail #1\r\n");
        return -1;  // bail out early - couldn't queue the message for transmission
    }

    do
    { // This is slightly hokey but as we are in the same task, we can't spin until the transmission has completed.
        snd_stack_task(); // run the stack to facilitate the transmission
    } while( (sn_transmission_complete == false) && ((get_microseconds_tick() - timestamp) < (usTICK_MILLISECONDS * 100)) );

    if( sn_transmission_complete == false )
    {
		surenet_printf( "TX Fail #2\r\n");	// have had this one
        return -1;  // transmission failed
    }

        return seq;
}

/**************************************************************
 * Function Name   : sn_mark_transmission_complete
 * Description     : Callback from Surenet Driver to say that a transmission has
 *                 : successfully completed.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_mark_transmission_complete(void)
{
    sn_transmission_complete = true;
}

void sn_association_changed(PAIRING_REQUEST new_state)
{
    pairing_mode = new_state;   // ensure that the SureNet idea of what pairing mode we are in is accurate
                                // We need this because the pairing mode may change as part of Stack activity
                                // (i.e. Association Successful).
    xQueueSend(xPairingModeHasChangedMailbox,&new_state,0);   // put result in mailbox for Surenet-Interface
}

/**************************************************************
 * Function Name   : sn_ping_device
 * Description     : Set a flag requesting that the conversation state machine sends a ping to a device at the next opportunity
 * Inputs          : Either a mac address, or 0 for the first device in the pairing table
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_ping_device(uint64_t device, uint8_t value)
{
	int8_t index=0;
	if (device>0)
	{	// if a mac address is specified, look for it
		index = convert_mac_to_index(device);
		if (index<0)
		{	// return if mac address is not for a valid device
			return;
		}
	}
	if (device_status[index].status.valid!=0)
	{
		surenet_printf("Setting send_ping to true\r\n");
		ping_stats.mac_address = device_status[index].mac_address;	// in case the mac was specified as 0, i.e. use the first one in the pairing table
		ping_stats.found_ping = false;	// This flag is used to indicate that a ping has been responded to and the ping_stats are updated
		device_status_extra[index].send_ping=true;		// flag to trigger conversation state machine to send a ping
		device_status_extra[index].ping_value = value;	// value to be passed as part of the ping.
		ping_stats.hub_rssi_sum = 0;
		ping_stats.device_rssi_sum = 0;
	}
}
/////////////////////////////
// These functions are brought over from the previous Hub firmware
/////////////////////////////
/**************************************************************
 * Function Name   : sn_channel_control
 * Description     :
 * Inputs          : 0xff to instigate a channel hop, or the channel number up to NUMBER_OF_CHANNELS
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_channel_control(uint8_t chn)
{
  surenet_printf("SCb ");
  if (chn==0xff)
    trigger_channel_hop=true;
  else
    if ((chn>=START_CHANNEL)&&(chn<START_CHANNEL+NUMBER_OF_CHANNELS)) snd_set_channel(chn);

}

/**************************************************************
 * Function Name   : sn_is_device_online
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
bool sn_is_device_online(uint64_t mac_addr)
{
    uint8_t index;
    if (true == convert_addr_to_index(mac_addr,&index))
    {
        if (device_status[index].status.online==1)
        {
            return true;
        }
    }
    return false;
}
/**************************************************************
 * Function Name   : sn_how_many_pairs
 * Description     : Count number of valid devices we are paired with
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint8_t sn_how_many_pairs(void)
{
  int i;
  uint8_t total=0;
  for (i=0; i<MAX_NUMBER_OF_DEVICES; i++)
    if (device_status[i].status.valid==1) total++;
  return total;
}

/**************************************************************
 * Function Name   : sn_hub_clear_pairing_table()
 * Description     : Clear the hub pairing table. Send a message to each paired device
 *                 : telling them to detach
 **************************************************************/
void sn_hub_clear_pairing_table(void)
{
    surenet_printf("Called detach_trigger\r\n");
    detach_device_bits = DETACH_ALL_MASK;
}

/**************************************************************
 * Function Name   : sn_unpair_device(index)
 * Description     : Detach one paired device
 **************************************************************/
void sn_unpair_device(uint64_t mac_addr)
{
    uint8_t index;
	if( SURENET_UNASSIGNED_MAC == mac_addr )
	{
		return;
	}

	if( true == convert_addr_to_index(mac_addr, &index) )
	{
		detach_device_bits |= (1ull << index);
	}
}

/**************************************************************
 * Function Name   : detach_handler()
 * Description     : Handles the hub telling devices to detach. Runs
 *                 : through a cycle trying to communicate with each
 *                 : device in turn detaching them if it can.
 **************************************************************/
BUSY_STATE detach_handler(void)
{
    static uint32_t device_index;
    static uint32_t detach_timer;
    static uint8_t detach_retry_counter;

    switch(detach_state)
    {
        case DETACH_IDLE:
            if (detach_device_bits!=0)
            {
                device_index=0;
                surenet_printf("nulled device_index\r\n");
                detach_state=DETACH_FIND_NEXT_PAIR;
            }
            break;
        case DETACH_FIND_NEXT_PAIR:
            while(device_index<MAX_NUMBER_OF_DEVICES)
            {
                if((detach_device_bits)&(1ull<<device_index))
                {
                    detach_device_bits &= ~(1ull << device_index);
                    break;  // break out of this while() loop if this device needs clearing
                }
                device_index++;
            }
            if(device_index>=MAX_NUMBER_OF_DEVICES)
            {
                detach_state=DETACH_FINISH;
                break;
            }
            surenet_printf("Detaching index %x\r\n", device_index);
            if (0 == device_status[device_index].status.valid)
            { // if this entry is not valid, clear the entire entry in the pairing table.
                memset((uint8_t*)&device_status[device_index], 0, sizeof(DEVICE_STATUS));    //ensure connection table entry is fully reset
                memset((uint8_t*)&device_status_extra[device_index], 0, sizeof(DEVICE_STATUS_EXTRA));    //ensure connection table entry is fully reset
                surenet_update_device_table_line(&device_status[device_index], device_index, false, true);  //update hubRegisterBank
            }
            else
            {
              detach_timer=get_microseconds_tick();  // start the timeout
              detach_state=DETACH_SEND_DETACH_COMMAND;  // found a valid pair, so detach it
              detach_retry_counter=0;
            }
            break;
        case DETACH_SEND_DETACH_COMMAND:
			if( detach_retry_counter > MAX_DETACH_RETRIES )    // handle retrying sending detach commands
            {
                surenet_printf("Tried to detach device too many times, presuming it's off-line\r\n");
                surenet_update_device_table_line(&device_status[device_index], device_index, false,true);  //update hubRegisterBank
                device_status_extra[device_index].send_detach=0;
                detach_state=DETACH_WAIT_FOR_ACK;  //wont actually wait for ACK as we have already nulled send_detach
            } else if( (get_microseconds_tick() - detach_timer) > (usTICK_SECONDS * 10) )
            { // didn't get the ack within the timeout period
                surenet_printf("Can't send PACKET_DETACH, retrying\r\n");  //maybe device is offline so cant
                detach_retry_counter++;
                detach_state=DETACH_SEND_DETACH_COMMAND;  // so try again
            } else
            { //now send detach commands next time the device sends a device awake message
                device_status_extra[device_index].send_detach = 1;
                detach_timer=get_microseconds_tick();  // start the timeout
                detach_state=DETACH_WAIT_FOR_ACK;
            }
            break;
        case DETACH_WAIT_FOR_ACK: // an ack will have arrived in the acknowledge_queue, so we need to check it
            if( device_status_extra[device_index].send_detach == 0 )  // send_detach[] is cleared by the main state machine if the necessary ACK is received
            {
                surenet_printf("Device acknowledged PACKET_DETACH\r\n");
                device_status[device_index].status.associated=0;
                memset((uint8_t*)&device_status[device_index], 0, sizeof(DEVICE_STATUS));    //ensure connection table entry is fully reset
                memset((uint8_t*)&device_status_extra[device_index], 0, sizeof(DEVICE_STATUS_EXTRA));    //ensure connection table entry is fully reset
                surenet_update_device_table_line(&device_status[device_index], device_index, false,true);  //update hubRegisterBank
                device_index++;
                detach_state=DETACH_FIND_NEXT_PAIR;
            }  else if( (get_microseconds_tick() - detach_timer) > (usTICK_SECONDS * 10) )
            { // didn't get the ack within the timeout period
                  surenet_printf("Device didn't acknowledge PACKET_DETACH, retrying\r\n");
                  detach_retry_counter++;
                  detach_state=DETACH_SEND_DETACH_COMMAND;  // so try again
            }
            break;
        case DETACH_FINISH: // all done
			if( store_device_table() )
			{
				detach_state = DETACH_IDLE; // go back to start and wait for the next trigger
			}
			else
			{
				// stay where we are and have another go next time
			}
			break;
        default:
            detach_state = DETACH_IDLE;
            break;
  }

  if( DETACH_IDLE == detach_state )
  {
	  return CAN_SLEEP;
  }
  return BUSY;
}

/**************************************************************
 * Function Name   : sn_set_hub_pairing_mode
 * Description     : Put the hub in pairing mode
 * Inputs          :
 * Outputs         :
 * Returns         : Previous state of pairing mode
 **************************************************************/
void sn_set_hub_pairing_mode(PAIRING_REQUEST state)
{
	static uint8_t normal_channel = INITIAL_CHANNEL;  //override channel during pairing

	if( state.enable == pdTRUE )
	{ // want to set pairing mode
		normal_channel = snd_get_channel();
		snd_set_channel(INITIAL_CHANNEL);
	} else
	{	// want to clear pairing mode
		snd_set_channel(normal_channel);
	}
	snd_pairing_mode(state);  //call SureNetDriver to trigger the association process

}

/**************************************************************
 * Function Name   : sn_get_hub_pairing_mode
 * Description     : Find out if the hub is in pairing mode
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
PAIRING_REQUEST sn_get_hub_pairing_mode(void)
{
  return pairing_mode;
}

/**************************************************************
 * Function Name   : hub_functions
 * Description     : This function runs a state machine which controls the SureNet hub functionality.
 *                 : The sequence of operations is as follows:
 *                 : - Check for PACKET_DEVICE_AWAKE messages and hold a device conversation
 *                 : - Check RSSI for recent conversations and start a frequency hop if necessary
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
typedef enum
{
	HUB_STATE_INIT, // 0
	HUB_STATE_CONVERSATION_START, // 1
	HUB_STATE_CONVERSATION_PET_DOOR_PAUSE,
	HUB_STATE_CONVERSATION_SEND,  // 3
	HUB_STATE_CONVERSATION_WAIT_FOR_DATA_ACK, // 4
	HUB_STATE_CONVERSATION_RECEIVE, // 5
	HUB_STATE_HOP,  // 6
	HUB_STATE_REPEATER, // 7
	HUB_STATE_WAIT, // 8
	HUB_STATE_CONVERSATION_WAIT_FOR_DETACH_ACK, // 9
	HUB_STATE_CONVERSATION_WAIT_FOR_SECURITY_ACK,  // 10
	HUB_STATE_SEND_END_SESSION,
	HUB_STATE_SEND_PING,
	HUB_STATE_SEND_PING_WAIT,
	HUB_STATE_END_OR_RECEIVE,
} HUB_STATE_2;

HUB_STATE_2 hub_state=HUB_STATE_INIT;

BUSY_STATE hub_functions(void)
{
  static int16_t  hub_conv_seq;
  static int16_t ping_tx_result;
  static uint32_t  hub_conv_timestamp;
  static uint8_t tx_retries;
  ACK_STATUS ack_reply;
  uint16_t nblocks = 0;
  int i;
  static MESSAGE_PARAMS message_params;
  static BaseType_t result;
  static uint32_t	pet_door_timestamp;
	DEVICE_TX_PAYLOAD dtx_payload;

  switch (hub_state)
  {
    case HUB_STATE_INIT:
      current_conversee=0;
      hub_state=HUB_STATE_CONVERSATION_START;
      break;
    case HUB_STATE_CONVERSATION_START:  // Need to discover an awake device. Don't bother attempting
		// to talk to devices who's DEVICE_AWAKE message happened so long ago that they will
		// have gone back to sleep. Were we to attempt that, we'd just incur a pointless delay
      while( ((device_status_extra[current_conversee].device_awake_status.awake==DEVICE_ASLEEP) ||
			 ((get_microseconds_tick()-device_status[current_conversee].last_heard_from)>TIMEOUT_DEVICE_STAYS_AWAKE )) &&
			 (current_conversee<MAX_NUMBER_OF_DEVICES) )
        current_conversee++;
      if (current_conversee>=MAX_NUMBER_OF_DEVICES)
	  {
        hub_state=HUB_STATE_HOP;    // No devices are awake, or we have serviced them all, so skip the conversation phase
		for( i = 0; i < MAX_NUMBER_OF_DEVICES; i++ )
		{
			if( MAX_SECURITY_KEY_USES <= device_status_extra[i].SecurityKeyUses )
			{
				device_status_extra[i].SendSecurityKey=SECURITY_KEY_RENEW;
			}
		}
	  } else
      {
		surenet_log_add(SURENET_LOG_STATE_MACHINE_RESPONDED,current_conversee);
        hub_conv_timestamp = get_microseconds_tick();
        if((device_status_extra[current_conversee].SendSecurityKey!=SECURITY_KEY_OK)||
		   (device_status_extra[current_conversee].sec_key_invalid>=MAX_BAD_KEY))
        {
			// get new security key
#if USE_RANDOM_KEY
			sn_GenerateSecurityKey(current_conversee);
			surenet_printf("Generating new key for Device %0d\r\n",current_conversee);
#endif
// trivial obfuscation of security key top avoid sending in clear
            uint8_t SecurityKeyScramble[SECURITY_KEY_SIZE];
            uint8_t scramble[17] = "IrViNeCeNtReDrYd";
	            for(i=0;i<16;i++) SecurityKeyScramble[i] = 	SecurityKeys[current_conversee][i] ^ scramble[i];
            hub_conv_seq = sn_transmit_packet(PACKET_ENCRYPTION_KEY,device_status[current_conversee].mac_address,(uint8_t*)SecurityKeyScramble,SECURITY_KEY_SIZE,-1,false);  // send the encryption key
			surenet_log_add(SURENET_LOG_SENT_SECURITY_KEY,current_conversee);
//			zprintf(CRITICAL_IMPORTANCE,"[%02X] ",hub_conv_seq);
            hub_state = HUB_STATE_CONVERSATION_WAIT_FOR_SECURITY_ACK;
        } else
        {
			surenet_log_add(SURENET_LOG_STATE_MACHINE_SWITCHING_TO_SEND,current_conversee);
#if PET_DOOR_DELAY
			if( DEVICE_TYPE_CAT_FLAP == device_status[current_conversee].status.device_type)
			{
				pet_door_timestamp = get_microseconds_tick();
				hub_state=HUB_STATE_CONVERSATION_PET_DOOR_PAUSE;
			} else 
			{
          hub_state=HUB_STATE_CONVERSATION_SEND;
        }
#else
			hub_state=HUB_STATE_CONVERSATION_SEND;			
#endif
        }
      }
      break;

	  // This state introduces a pause into the conversation (i.e. slows it down)
	  // to match Hub1 timing. It only applies to Pet Doors as there is a concern
	  // that they occasionally crash with Hub2 timings.
	case HUB_STATE_CONVERSATION_PET_DOOR_PAUSE:
		if( (get_microseconds_tick() - pet_door_timestamp) > (50 * usTICK_MILLISECONDS))
		{
			hub_state=HUB_STATE_CONVERSATION_SEND;
      }
      break;

    case HUB_STATE_CONVERSATION_WAIT_FOR_SECURITY_ACK:// wait for the ack to arrive, or timeout and resend
      ack_reply = has_ack_arrived(hub_conv_seq);
		if( ack_reply == ACK_ARRIVED ) // we got an ack for the PACKET_ENCRYPTION_KEY we just sent
      {
        device_status_extra[current_conversee].SendSecurityKey = SECURITY_KEY_OK;
        device_status_extra[current_conversee].sec_key_invalid = 0;
        device_status_extra[current_conversee].SecurityKeyUses = 0;
        hub_conv_timestamp = get_microseconds_tick();
			surenet_log_add(SURENET_LOG_SECURITY_KEY_ACKNOWLEDGED,current_conversee);
        hub_state=HUB_STATE_CONVERSATION_SEND;  //now can send data if required
		}  else if( (ack_reply == NACK_ARRIVED) || ((get_microseconds_tick() - hub_conv_timestamp) > TIMEOUT_SECURITY_ACK) )
      { // didn't get the ack, so we will skip and move on to the next device.
			if( NACK_ARRIVED == ack_reply)
				surenet_log_add(SURENET_LOG_SECURITY_NACK,current_conversee);
			else
				surenet_log_add(SURENET_LOG_SECURITY_ACK_TIMEOUT,current_conversee);
        hub_state=HUB_STATE_CONVERSATION_START;  // loop around for next awake device
        device_status_extra[current_conversee].device_awake_status.awake=DEVICE_ASLEEP;
      }
      // else stay here
      break;

    case HUB_STATE_CONVERSATION_SEND: // Remember that the device could have gone back to sleep
      // now decide what to send the device. Options are:
      // PACKET_DATA if we have data to send
      // PACKET_DEVICE_TX if we have no data, but device_awake_status[].data=DEVICE_HAS_DATA
      // PACKET_END_SESSION if neither of us have any data
	 // OR, we might have a ping to send to the device, so we divert the state machine to send that before PACKET_END_SESSION.
      if( device_status_extra[current_conversee].send_detach == 1 )
      {
        tx_retries=0;
        hub_conv_timestamp = get_microseconds_tick();
        surenet_printf("Sending Detach\r\n");
        hub_conv_seq = sn_transmit_packet(PACKET_DETACH,device_status[current_conversee].mac_address,0,0,-1,false);  // send the detach command
        hub_state = HUB_STATE_CONVERSATION_WAIT_FOR_DETACH_ACK;  // wait for DETACH_ACK.
        break;
      }

		i = is_there_a_firmware_chunk(current_conversee);
		if ((i>=0) && (device_status_extra[current_conversee].device_awake_status.data==DEVICE_HAS_NO_DATA))
		{	// firmware to send, and device has nothing for us
			sn_send_chunk(current_conversee,&firmware_chunk[i]);
          }
	  	// this mailbox ping-pong usually takes about 70us.
      xQueueReceive(xGetNextMessageMailbox_resp,&message_params,0); // flush mailbox
      xQueueSend(xGetNextMessageMailbox,&device_status[current_conversee].mac_address,0); // request polling of application to see if there is a new message
      result = xQueueReceive(xGetNextMessageMailbox_resp,&message_params,pdMS_TO_TICKS(20)); // wait for other process to tell us if it has data to send
      // Note we can't wait too long as it will increase power consumption on the devices as it is awake whilst
      // waiting. Also the device will time out and go to sleep after 100ms approx anyway.
      if( pdPASS != result )
      {
            surenet_printf("%08d: Didn't get message from Application\r\n",get_microseconds_tick());
      }
      if( (pdPASS == result) && (message_params.new_message == true) )
      {   // we have a new message to send
		hub_conv_timestamp = get_microseconds_tick();
		tx_retries = 0;
		hub_conv_seq = sn_transmit_packet(PACKET_DATA, device_status[current_conversee].mac_address, (uint8_t*)message_params.ptr, message_params.ptr->length, -1, false);
		surenet_log_add(SURENET_LOG_DATA_SEND,current_conversee);
		hub_state = HUB_STATE_CONVERSATION_WAIT_FOR_DATA_ACK;  // wait for DATA_ACK.
      }
      else
	  {	// we've nothing more to send, so have three possible next steps.
		// Send a ping
		// Send an END_SESSION
		// switch to receive mode
		if (device_status_extra[current_conversee].send_ping == true)
		{
			ping_stats.ping_attempts=0;
			hub_state = HUB_STATE_SEND_PING;
		}
		else
			hub_state = HUB_STATE_END_OR_RECEIVE;	// decide next step
	  }
	  break;

	case HUB_STATE_SEND_END_SESSION:
        surenet_printf("Sending PACKET_END_SESSION\r\n");
		surenet_log_add(SURENET_LOG_END_OF_CONVERSATION,current_conversee);
		device_status_extra[current_conversee].tx_end_cnt++;
		sn_transmit_packet(PACKET_END_SESSION, device_status[current_conversee].mac_address, &device_status_extra[current_conversee].tx_end_cnt, 1,-1,false);
		device_status_extra[current_conversee].device_awake_status.awake=DEVICE_ASLEEP;
		hub_state=HUB_STATE_CONVERSATION_START; // loop around for the next awake device
	  	break;

	case HUB_STATE_SEND_PING:
		// send a ping to the device
		ping_stats.ping_attempts++;
		ping_tx_result = sn_transmit_packet(PACKET_PING, device_status[current_conversee].mac_address, &device_status_extra[current_conversee].ping_value, 1,-1,false);
		if(ping_tx_result > -1)
		{	// PING was successfully transmitted
			ping_stats.transmission_timestamp = get_microseconds_tick();
			ping_seq = (uint8_t)ping_tx_result;
			surenet_printf( "Sent Ping with seq=%x value=%x\r\n", ping_seq, device_status_extra[current_conversee].ping_value);
			device_status_extra[current_conversee].send_ping = false;
			hub_state = HUB_STATE_SEND_PING_WAIT;	// now wait for the ping to come back
			hub_conv_timestamp = get_microseconds_tick();
		}
		else //else just try again - it's unlikely to fail to transmit so we shouldn't get stuck here for long.
		{
			surenet_printf( "!! Ping failed to send: %d\r\n", ping_tx_result);
		}
		break;

	case HUB_STATE_SEND_PING_WAIT:	// wait for either the ping reply, or a short timeout to expire
	  	if( (get_microseconds_tick() - hub_conv_timestamp) > usTICK_MILLISECONDS * 2000 )
		{	// never got the reply, so try again
			if( ping_stats.ping_attempts < 5 )
			{
				ping_stats.num_bad_pings++;
				device_status_extra[current_conversee].send_ping = true;
				hub_state = HUB_STATE_SEND_PING;	// try again
			} else
			{ // send the ping too many times, so move on
				hub_state = HUB_STATE_END_OR_RECEIVE;
			}
		} else if( ping_stats.found_ping == true )
		{
			ping_stats.num_good_pings++;
			hub_state = HUB_STATE_END_OR_RECEIVE;
		}
		break;

	case HUB_STATE_END_OR_RECEIVE:
		if (device_status_extra[current_conversee].device_awake_status.data==DEVICE_HAS_NO_DATA)
		{
			hub_state = HUB_STATE_SEND_END_SESSION;
		}
		else if (device_status_extra[current_conversee].device_awake_status.data==DEVICE_HAS_DATA)
		{	// Note that in Hub1 code, the 'if' part of the above else if is not present, so this
			// block catches everything other than NO_DATA, i.e. DEVICE_HAS_DATA,DEVICE_DETACH,DEVICE_SEND_KEY,
			// DEVICE_DONT_SEND_KEY,DEVICE_RCVD_SEGS,DEVICE_SEGS_COMPLETE
			dtx_payload.transmit_end_count = device_status_extra[current_conversee].tx_end_cnt++;
			dtx_payload.encryption_type = device_status_extra[current_conversee].encryption_type;
//			zprintf(LOW_IMPORTANCE,"PDTX\r\n");
			sn_transmit_packet(PACKET_DEVICE_TX, device_status[current_conversee].mac_address, 
							   (uint8_t *)&dtx_payload, sizeof(DEVICE_TX_PAYLOAD),-1,false);
			most_recent_rx_time = get_microseconds_tick(); // this will get updated when a PACKET_DATA is received
			conv_end_session_flag = false;        // this will get set to true when a PACKET_END_SESSION is received
			hub_state=HUB_STATE_CONVERSATION_RECEIVE;
		}
		else
		{
            surenet_printf("UNHANDLED REQUEST FROM DEVICE - aborting conversation\r\n");
            surenet_printf("device_awake_status.data=%d\r\n", device_status_extra[current_conversee].device_awake_status.data);
			hub_state = HUB_STATE_SEND_END_SESSION;	// end session normally
		}
		break;

    case HUB_STATE_CONVERSATION_WAIT_FOR_DETACH_ACK:// wait for the Detach ack to arrive, or timeout and resend the detach command
      ack_reply = has_ack_arrived(hub_conv_seq);
      if (ack_reply==ACK_ARRIVED) // we got an ack for the PACKET_DATA we just sent
      {
         surenet_printf("Got a detach ack %x\r\n", hub_conv_seq);
         hub_state = HUB_STATE_CONVERSATION_START;  // loop around for next awake device
         device_status_extra[current_conversee].send_detach = 0;
      }
      else if( (get_microseconds_tick() - hub_conv_timestamp) > usTICK_SECONDS * 2 )
      {
         tx_retries++;
         if (tx_retries<HUB_TX_MAX_RETRIES)
         { // retry by sending the exact same packet again
            hub_conv_timestamp = get_microseconds_tick();
            surenet_printf("Detach retry ");
			zprintf(LOW_IMPORTANCE,"PD: Sending PACKET_DETACH #2\r\n");
            hub_conv_seq = sn_transmit_packet(PACKET_DETACH,device_status[current_conversee].mac_address,0,0,hub_conv_seq,false);  // send the detach command
         } else
         {
            surenet_printf("Failed to get ACK for detach command but delete device anyway\r\n");
            hub_state=HUB_STATE_CONVERSATION_START;  // loop around for next awake device
            device_status_extra[current_conversee].send_detach = 0;
         }
      }
      break;
    case HUB_STATE_CONVERSATION_WAIT_FOR_DATA_ACK:// wait for the ack to arrive, or timeout and resend
      ack_reply = has_data_ack_arrived(hub_conv_seq);
      if (ack_reply==ACK_ARRIVED) // we got an ack for the PACKET_DATA we just sent
	  {
        surenet_clear_message_cb(message_params.handle);  // tell the main code that this request has gone
		surenet_log_add(SURENET_LOG_DATA_ACK,current_conversee);
        hub_conv_timestamp = get_microseconds_tick();
        hub_state=HUB_STATE_CONVERSATION_SEND;  // see if more data has arrived.
      }
      else if (ack_reply==NACK_ARRIVED) // we got a nack for the PACKET_DATA we just sent
      { // so we need to stop transmitting and switch to receive mode
        surenet_printf("Got an NACK - device full - switching to receive %x\r\n", hub_conv_seq);
		dtx_payload.transmit_end_count = device_status_extra[current_conversee].tx_end_cnt++;
		dtx_payload.encryption_type = device_status_extra[current_conversee].encryption_type;
		sn_transmit_packet(PACKET_DEVICE_TX, device_status[current_conversee].mac_address, 
						   (uint8_t *)&dtx_payload, sizeof(DEVICE_TX_PAYLOAD),-1,false);
 		surenet_log_add(SURENET_LOG_DATA_NACK,current_conversee);
        most_recent_rx_time = get_microseconds_tick(); // this will get updated when a PACKET_DATA is received
        conv_end_session_flag = false;        // this will get set to true when a PACKET_END_SESSION is received
        hub_state=HUB_STATE_CONVERSATION_RECEIVE;
      }
      else if( (get_microseconds_tick() - hub_conv_timestamp) > TIMEOUT_WAIT_FOR_DATA_ACK )
      { // didn't get the ack, so we need to resend
		  if( ((tx_retries++) < HUB_TX_MAX_RETRIES) && 
			  ((get_microseconds_tick() - device_status[current_conversee].last_heard_from) < (usTICK_MILLISECONDS * 100)) )
		  {	// retry
				surenet_printf("Retrying send\r\n");
				sn_transmit_packet(PACKET_DATA,device_status[current_conversee].mac_address, (uint8_t*)message_params.ptr,message_params.ptr->length,hub_conv_seq,false);
				surenet_log_add(SURENET_LOG_DATA_RETRY_ATTEMPT,current_conversee);
				hub_conv_timestamp = get_microseconds_tick();	
				// remain in this state
		  } else
		  { // give up
          surenet_printf("Retried DATA too many times, giving up %x, sending PACKET_END_SESSION\r\n", hub_conv_seq);
				sn_transmit_packet(PACKET_END_SESSION, device_status[current_conversee].mac_address, 0, 0,-1,false); // this will probably fail
                                                                                        //if we haven't managed to send the data
          device_status_extra[current_conversee].device_awake_status.awake=DEVICE_ASLEEP;
			surenet_log_add(SURENET_LOG_DATA_RETRY_FAIL,current_conversee);
          hub_state=HUB_STATE_CONVERSATION_START;  // loop around for next awake device
            	}
      } // else stay here
      break;

    case HUB_STATE_CONVERSATION_RECEIVE:  // receive and acknowledge data from the device, until timeout or PACKET_END_SESSION
		if( (conv_end_session_flag == true) || ((get_microseconds_tick() - most_recent_rx_time) > usTICK_SECONDS) )
		{ // timeout if we've not received a new message in 1 sec.
			if( true == conv_end_session_flag)
				surenet_log_add(SURENET_LOG_END_OF_SESSION,current_conversee);
			else
				surenet_log_add(SURENET_LOG_TIMEOUT,current_conversee);
			hub_state = HUB_STATE_CONVERSATION_START; // conversation over
			device_status_extra[current_conversee].device_awake_status.awake = DEVICE_ASLEEP;
		}
      break;
    case HUB_STATE_HOP:   // conversation over - look at whether we need a frequency hop......
		if( trigger_channel_hop == true ){ surenet_printf("Channel hopping by hub not implemented\r\n"); }
		current_conversee = 0;
		hub_state = HUB_STATE_CONVERSATION_START;
		break;
    default:
		hub_state = HUB_STATE_CONVERSATION_START;
		break;
  }

  // decide if we are busy (i.e. mid activity), or about to start again.
  if( HUB_STATE_CONVERSATION_START == hub_state)
  {
	  return CAN_SLEEP;
  }
  return BUSY;
}

/**************************************************************
 * Function Name   : sn_send_chunk
 * Description     : Send a chunk (in two segments) to the Device
 * Inputs          : uint8_t device_rcvd_segs[MAX_NUMBER_OF_DEVICES][2];
 *                 : device_rcvd_segs[current_conversee]=0x00 is when both segments of a chunk are needed,
 *                 : and =0x01 when only the 2nd segment is needed.
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_send_chunk(uint8_t current_conversee,FIRMWARE_CHUNK *firmware_chunk)
{
#define PACKET_DATA_SEGMENT_HEADER_SIZE			5
#define DATA_SEGMENT_STATUS_LAST_CHUNK			(1<<2)	// last chunk in firmware update
#define DATA_SEGMENT_STATUS_LAST_CHUNK_IN_CACHE	(1<<1)	// not used
#define DATA_SEGMENT_STATUS_LAST_SEG			(1<<0)	// last segment in chunk
	uint64_t dest_mac;
	uint8_t payload[FIRST_SEGMENT_SIZE+PACKET_DATA_SEGMENT_HEADER_SIZE];	// 5 byte header + 72 byte payload
	uint8_t seg1_length,seg2_length;

	dest_mac = device_status[current_conversee].mac_address;
	payload[1] = firmware_chunk->chunk_address>>8;	// chunk address
	payload[2] = firmware_chunk->chunk_address;

	// work out the length of the two segments, limiting them to their respective max sizes.
	seg1_length = ((firmware_chunk->len)>FIRST_SEGMENT_SIZE)?FIRST_SEGMENT_SIZE:firmware_chunk->len;
	seg2_length = ((firmware_chunk->len)>FIRST_SEGMENT_SIZE)?firmware_chunk->len-FIRST_SEGMENT_SIZE:0;	// any left over from first segment

	if (device_rcvd_segs[current_conversee]==0x00)
	{ // first segment - only send it if asked for
	payload[0] = 0;	// first segment
		payload[3] = seg1_length;	// length (trimmed to 72)
		payload[4] = (seg2_length==0)?DATA_SEGMENT_STATUS_LAST_CHUNK:0;	// set status bit if this is the last segment
		if (seg1_length>0)
			memcpy(&payload[PACKET_DATA_SEGMENT_HEADER_SIZE],firmware_chunk->data,seg1_length);
		sn_transmit_packet(PACKET_DATA_SEGMENT, dest_mac, payload, seg1_length+PACKET_DATA_SEGMENT_HEADER_SIZE, -1,false);
	}

	// Note we do handle having the second segment only partly filled and using this to indicate it is the
	// final segment. But this is unlikely to ever be used.
	// Also note that if the second segment is full, we assume it's not the last one.
	if (seg2_length>0) 	// Only send the 2nd segment if there's anything in it
	{
	payload[0] = 1;	// second segment
		payload[3] = seg2_length;	// length (trimmed to 64)
		payload[4] = (seg2_length<SECOND_SEGMENT_SIZE)?DATA_SEGMENT_STATUS_LAST_CHUNK:DATA_SEGMENT_STATUS_LAST_SEG;	// status is 4 for last one, 1 for others.
		if (seg2_length>0)
			memcpy(&payload[PACKET_DATA_SEGMENT_HEADER_SIZE],(firmware_chunk->data)+FIRST_SEGMENT_SIZE,seg2_length);
		sn_transmit_packet(PACKET_DATA_SEGMENT, dest_mac, payload, seg2_length+PACKET_DATA_SEGMENT_HEADER_SIZE, -1,false);
	}
	zprintf(LOW_IMPORTANCE,"DFU: Sending ");
	for( int i=0; i<16; i++)
	{
		zprintf(LOW_IMPORTANCE," %02X",firmware_chunk->data[i]);
	}
	zprintf(LOW_IMPORTANCE,"\r\n");
	firmware_chunk->has_data = false;	// free this chunk so it won't get sent again.
}

/**************************************************************
 * Function Name   : has_data_ack_arrived()
 * Description     : Check the acknowledge queue to see if an ack
 *                 : with that sequence number has arrived recently
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
ACK_STATUS has_data_ack_arrived(int16_t seq)
{
  int i;
  for (i=0; i<DATA_ACKNOWLEDGE_QUEUE_SIZE; i++)
  {
    if ((data_acknowledge_queue[i].valid!=0) && (data_acknowledge_queue[i].seq_acknowledged==seq))
    {
      data_acknowledge_queue[i].valid=0;
      if (data_acknowledge_queue[i].ack_nack==0)
        return ACK_ARRIVED;
      else
        return NACK_ARRIVED;
    }
  }
  return ACK_NOT_ARRIVED;
}

/**************************************************************
 * Function Name   : has_ack_arrived()
 * Description     : Check the acknowledge queue to see if an ack
 *                 : with that sequence number has arrived recently
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
ACK_STATUS has_ack_arrived(int16_t seq)
{
  int i;
//  zprintf(CRITICAL_IMPORTANCE,"{%02x} ",seq);
  for (i=0; i<ACKNOWLEDGE_QUEUE_SIZE; i++)
  {
    if ((acknowledge_queue[i].valid!=0) && (acknowledge_queue[i].seq_acknowledged==seq))
    {
      acknowledge_queue[i].valid=0;
      if (acknowledge_queue[i].ack_nack==0)
        return ACK_ARRIVED;
      else
        return NACK_ARRIVED;
    }
  }
  return ACK_NOT_ARRIVED;
}

/**************************************************************
 * Function Name   : age_data_acknowledge_queue
 * Description     : Age the data acknowledge queue, so items within it eventually time out
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void age_data_acknowledge_queue(uint8_t ticks)
{
  int i;

  for (i=0; i<DATA_ACKNOWLEDGE_QUEUE_SIZE; i++)
  {
    if (data_acknowledge_queue[i].valid!=0)
    {
      if (data_acknowledge_queue[i].valid>=ticks)
        data_acknowledge_queue[i].valid-=ticks;
      else
        data_acknowledge_queue[i].valid=0;
  }
}
}
/**************************************************************
 * Function Name   : age_acknowledge_queue
 * Description     : Age the acknowledge queue, so items within it eventually time out
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void age_acknowledge_queue(uint8_t ticks)
{
    int i;

    for (i=0; i<ACKNOWLEDGE_QUEUE_SIZE; i++)
    {
        if (acknowledge_queue[i].valid!=0)
        {
          if (acknowledge_queue[i].valid>=ticks)
            acknowledge_queue[i].valid-=ticks;
          else
            acknowledge_queue[i].valid=0;
        }
    }
}

/**************************************************************
 * Function Name   : isMacAddressForUs(uint8_t* address)
 * Description     : Compares the mac address in desination with our mac address.
 * Inputs          :
 * Outputs         :
 * Returns         : True if mac address matches ours, or it's a broadcast address. False if not.
 **************************************************************/
bool isMacAddressForUs(uint64_t destination)
{
    if( (destination==0) || (destination==0xffffffffffffffff) ) return true;  // packet is for us if destination==0 | destination==0xffffffffffffffff
    if( destination != *rf_mac ) return false;
    return true;
}

/**************************************************************
 * Function Name   : surenet_data_received
 * Description     : This function passes a received message out
 *                 : via a mailbox, and waits for a response indicating
 *                 : whether the message was accepted.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
SN_DATA_RECEIVED_RESPONSE sn_data_received(RECEIVED_PACKET *rx_packet)
{
    SN_DATA_RECEIVED_RESPONSE result;
    // stick the message in a mailbox

    xQueueSendToFront(xRxMailbox,rx_packet,0); // Notify SureNet-Interface

    // wait for a mailbox message indicating true or false
    if (xQueueReceive( xRxMailbox_resp, &result, usTICK_MILLISECONDS * 100 )==pdPASS)
    {
        return result;
    }
    return SN_TIMEOUT;   // never got a result from SureNet-Interface
}
/**************************************************************
 * Function Name   : convert_IEEE_to_sureflap
 * Description     : Builds a SureFlap format frame for subsequent processing
 *                 : Source information is in rx_buffer which is a module level variable
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void convert_IEEE_to_sureflap(RECEIVED_PACKET *rx_packet, RX_BUFFER *rx_buffer)
{
    // received packet has two SureNet 'header bytes' at the start of the payload, which need to be
    // moved into the header. The first byte of the payload is the PACKET_TYPE and the 2nd is the sequence number.
    // Everything else for the header is included in other structure members of rx_buffer.
	uint16_t calculated_crc;

    rx_packet->packet.header.packet_length = rx_buffer->ucBufferLength+sizeof(HEADER)-2;  // the -2 is because the first two bytes of
                                              // the payload will be moved to the header
    rx_packet->packet.header.sequence_number = rx_buffer->ucRxBuffer[1]; // 2nd byte of payload is sequence number
    rx_packet->packet.header.packet_type = rx_buffer->ucRxBuffer[0]; // 1st byte of payload is packet type
	rx_packet->packet.header.source_address = rx_buffer->uiSrcAddr;
    rx_packet->packet.header.destination_address = rx_buffer->uiDstAddr;
    rx_packet->packet.header.crc = 0; // will get overwritten
    rx_packet->packet.header.rss = 0; // will get overwritten
    rx_packet->packet.header.spare = 0;
	if (rx_buffer->ucBufferLength<2)
	{
		zprintf(CRITICAL_IMPORTANCE,"CORRUPTED RX BUFFER - HALTED RF STACK - NEED TO INVESTIGATE WHY\r\n");
		DbgConsole_Flush();
		// if ucBufferLength is less than 2, then the memcpy() line below will try to copy
		// a negative number of bytes, which will translate to a huge positive number and so will
		// run out of memory!!
		while(1);
	}
    memcpy(&rx_packet->packet.payload,&rx_buffer->ucRxBuffer[2],rx_buffer->ucBufferLength-2); // copy payload
    calculated_crc = CRC16((uint8_t*)&rx_packet->buffer, rx_packet->packet.header.packet_length, 0xcccc);  // calculate CRC on header & payload, probably not needed by SureNet
    rx_packet->packet.header.crc = calculated_crc;
    rx_packet->packet.header.rss = rx_buffer->ucRSSI;

}

/**************************************************************
 * Function Name   : sn_process_received_packet() - callback from the SureNetDriver
 * Description     : Process the packet in struct rx_buffer, firstly converting it's format via convert_IEEE_to_sureflap();
 *                 : This function also dewhitens / decrypts the payload
 *                 : Note that by this point, the CRC is set to zero, so it's invalid.
 *                 : The packet is not dequeued until this function finishes, i.e. it is dequeued by the caller of this fn.
 *                 : This packet could be a repeat, but bool repeated_packet advises us of this
 * Inputs          :
 * Outputs         :
 * Returns         : true if the packet was OK, false if not
 **************************************************************/
RECEIVED_PACKET  rx_packet; // putting this variable outside the function means the debugger can see it out of context.
DEVICE_FIRMWARE_CHUNK dummy_chunk;	// allocated at compile time - this is quite large for the stack
bool sn_process_received_packet(RX_BUFFER *rx_buffer)
{
    int i;
    SN_DATA_RECEIVED_RESPONSE sn_resp;
    static uint8_t ack_payload[2];
    int8_t device_index;
    uint16_t packet_length;
    PACKET_DEVICE_AWAKE_PAYLOAD *pda_payload;   // used to extract data from a PACKET_DEVICE_AWAKE
    DEVICE_AWAKE_MAILBOX device_awake_mailbox;  // we make a copy of the packet_device_awake data and send it to the application.
	DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX device_rcvd_segs_parameters;
	uint32_t blocking_sequence_no;

    convert_IEEE_to_sureflap(&rx_packet, rx_buffer);

    packet_length = rx_packet.packet.header.packet_length;

    // firstly we need to work out if the packet is for us:
    // This means either the destination address matches our address or the destination is either all 0's or all '1's
    if (isMacAddressForUs(rx_packet.packet.header.destination_address)==false)
    {
        surenet_printf("Packet not for us\r\n");    // Should never happen as Driver strips out packets not for us?
        return true;  // perfectly valid packet, just not for us!)
    }
    else
    { // packet is for us, so we need to process it. It could be a repeated packet.
        device_index = convert_mac_to_index(rx_packet.packet.header.source_address); // Note this only returns the index if the device is 'valid' as well as having a matching MAC address.
        if( (device_index < 0) || (device_status[device_index].status.associated == 0) )
        {
			if( rx_packet.packet.header.packet_type != PACKET_BLOCKING_TEST)
			{
            return false;  //not yet associated with this device (even if device is stored in NVM)
			}               //e.g. if hub rebooted but device hasn't detected connection drop
        }
        if( (device_index >= 0) && (device_index < MAX_NUMBER_OF_DEVICES) )
        {
            device_status[device_index].last_heard_from = get_microseconds_tick();
            if( device_status[device_index].status.online != 1 )
            {
                device_status[device_index].status.online = 1;  //update 'online' flag NOT SURE IF IT IS ACTUALLY USED!!
                surenet_update_device_table_line(&device_status[device_index], device_index, true,true);  //call limted version so no time update flags set to limit message rate to Xively
            }
        }

        if (is_packet_new(&rx_packet,&ack_payload[0])==false)  // some repeated packets do have to be processed, (acknowledged)
        {
            switch (rx_packet.packet.header.packet_type)
            {
                case PACKET_DATA:
				case PACKET_DATA_ALT_ENCRYPTED:
          // For first time data packets, we need to process and acknowledge. For repeated ones, we still need to
          // acknowledge because a repeated data packet means that the acknowledge was lost last time around
                    sn_transmit_packet(PACKET_DATA_ACK,rx_packet.packet.header.source_address,ack_payload,2,-1,false);
                    break;
                default:
                    break;
            }
        }
        else  // this is not a repeated packet, so deal with it 'normally'
        {
            surenet_printf("%08x: Processing packet of type %s\r\n",get_microseconds_tick(),packet_names[rx_packet.packet.header.packet_type]);

            switch (rx_packet.packet.header.packet_type)
            {
				case PACKET_DATA_ALT_ENCRYPTED:
					device_status_extra[device_index].encryption_type = SURENET_CRYPT_CHACHA;
					// fall through to PACKET_DATA processing
                case PACKET_DATA:
                    if (rx_packet.packet.header.spare==0)  //exclude test packets
					{
						sn_Decrypt(rx_packet.packet.payload, packet_length-sizeof(HEADER), 
								   device_index,  rx_packet.packet.header.sequence_number); // decrypt payload
						device_status_extra[device_index].SecurityKeyUses++;
					}
					if( rx_packet.packet.payload[2]-1>sizeof(RECEIVED_PACKET))
					{	// size byte is wrong
                        if(device_status_extra[device_index].sec_key_invalid<MAX_BAD_KEY) device_status_extra[device_index].sec_key_invalid++;
                        return false;
                    }
					uint8_t pM;
                    pM = (uint8_t)GetParity((int8_t*)(&rx_packet.packet.payload[0]), rx_packet.packet.payload[2]-1); //calculate parity and drop it in at the end of the array
                    if(pM!=rx_packet.packet.payload[rx_packet.packet.payload[2]-1])
                    { // if parity fails, we assume that is due to a bad security key.
                        if(device_status_extra[device_index].sec_key_invalid<MAX_BAD_KEY) device_status_extra[device_index].sec_key_invalid++;
                        return false;
                    }
					rx_packet.packet.header.packet_length--;	//this removes the parity byte as we no longer need it
                    device_status_extra[device_index].sec_key_invalid = 0;

                    most_recent_rx_time = get_microseconds_tick(); // make a note of the current time to permit receive data timeouts
                    ack_payload[0]=rx_packet.packet.header.sequence_number;
                    //  need to check if we are paired before accepting the packet
                    if (are_we_paired_with_source(rx_packet.packet.header.source_address)==true)
                    {
                        sn_resp = sn_data_received(&rx_packet); // pass packet up the stack for consumption or rejection
                        switch(sn_resp)
                        {
                            case SN_ACCEPTED:
                                ack_payload[1] = 0;  // message accepted, so send an ACK
                                break;
                            case SN_CORRUPTED:
                                if(device_status_extra[device_index].sec_key_invalid<MAX_BAD_KEY)
                                    device_status_extra[device_index].sec_key_invalid++; // could be corrupted because of a bad security key
                                ack_payload[1] = 0xff; // message rejected (corrupted), so send a NACK
                                break;
                            case SN_REJECTED:   // stack rejected message for unknown reasons
                            case SN_TIMEOUT:    // stack failed to respond in a timely manner
                            default:
                                ack_payload[1] = 0xff; // message rejected, so send a NACK
                                break;
                        }
                    }
                    else
					{
						ack_payload[1] = 0xff;     // send a NACK back   - not paired
					}
                    sn_transmit_packet(PACKET_DATA_ACK,rx_packet.packet.header.source_address,ack_payload,2,-1,false);
					store_ack_response(rx_packet.packet.header.source_address, rx_packet.packet.header.sequence_number, ack_payload[1]);	// record ack response in case we have a repeated packet and need to repeat the ack
					surenet_ack_printf("(1424) Packet data: %x %x\r\n", ack_payload[0], ack_payload[1]);
					break;
                case PACKET_DATA_ACK: // call the host to indicate the acknowledgement
                    i=0;
                    while ((i<DATA_ACKNOWLEDGE_QUEUE_SIZE) && (data_acknowledge_queue[i].valid!=0))
                        i++;  // find the first free space in the queue
                    if (i<DATA_ACKNOWLEDGE_QUEUE_SIZE)
                    {
                        data_acknowledge_queue[i].valid=DATA_ACKNOWLEDGE_LIFETIME;
                        data_acknowledge_queue[i].seq_acknowledged=rx_packet.packet.payload[0];
                        data_acknowledge_queue[i].ack_nack=rx_packet.packet.payload[1];
					}
                    else
                        surenet_printf("Data ack queue full, dropping data ack\r\n");
                    break;
                case PACKET_ACK:  // here we need to store the information in the ack payload in a buffer
                    i=0;
                    while ((i<ACKNOWLEDGE_QUEUE_SIZE) && (acknowledge_queue[i].valid!=0))
                        i++;  // find the first free space in the queue
                    if (i<ACKNOWLEDGE_QUEUE_SIZE)
                    {
//						zprintf(CRITICAL_IMPORTANCE,"(%02X %d) ",rx_packet.packet.payload[0],i);
                        acknowledge_queue[i].valid=ACKNOWLEDGE_LIFETIME;
                        acknowledge_queue[i].seq_acknowledged=rx_packet.packet.payload[0];
                        acknowledge_queue[i].ack_nack=rx_packet.packet.payload[1];
						surenet_ack_printf("(1449) Packet ack %x %x\r\n", ack_payload[0], ack_payload[1]);
					}
                    else
					{
						zprintf(HIGH_IMPORTANCE,"*");
                        surenet_printf("Ack queue full, dropping ack\r\n");
					}
                    break;
                case PACKET_BEACON:
                    surenet_printf("Beacon received SHOULD NEVER SEE THIS\r\n");
                    break;
                case PACKET_PAIRING_REQUEST:
                    break;
                case PACKET_DEVICE_STATUS:  //Allows any relevant initialisation details to be supplied by device
                                            //and security key to be setup
                    ack_payload[0]=rx_packet.packet.header.sequence_number;
                    ack_payload[1] = 0;         // send an ACK back
                    if(rx_packet.packet.payload[0]!=device_status[device_index].status.device_type)
                    {
                        surenet_printf("Error, mismatched device type\r\n");
                        ack_payload[1] = 0xFF;         // send a NACK back
                    }
                    else if(rx_packet.packet.payload[1]==DEVICE_SEND_KEY)  //device wants security key to be sent
                    {
                        device_status_extra[device_index].SendSecurityKey = SECURITY_KEY_RENEW;
                    }
                        //currently just send an ACK/NACK back but could add other initialisation info if required
                        //although not secure at this stage so better to send as a PACKET_DATA later
                    surenet_printf("Sending PACKET_DEVICE_CONFIRM\r\n");
                    sn_transmit_packet(PACKET_DEVICE_CONFIRM,rx_packet.packet.header.source_address,ack_payload,2,-1,false);
                    break;
                case PACKET_PAIRING_CONFIRM:
                    break;
                case PACKET_CHANNEL_HOP:
                    break;
                case PACKET_DEVICE_AWAKE: // message received from device indicating that it is awake and wants a chat.
                    if (are_we_paired_with_source(rx_packet.packet.header.source_address)==true)
                    { // only accept DEVICE_AWAKE messages if we are paired
						surenet_log_add(SURENET_LOG_DEVICE_AWAKE_RECIVED,convert_mac_to_index(rx_packet.packet.header.source_address));
                        if ((device_index<MAX_NUMBER_OF_DEVICES) && (device_index>=0))
                        {
                            pda_payload = (PACKET_DEVICE_AWAKE_PAYLOAD*)(&rx_packet.packet.payload[0]);
                            memcpy(&device_awake_mailbox.payload,pda_payload,sizeof(PACKET_DEVICE_AWAKE_PAYLOAD));
                            device_awake_mailbox.src_addr = rx_packet.packet.header.source_address;
                            xQueueSend(xDeviceAwakeMessageMailbox,&device_awake_mailbox,0);   // put result in mailbox for Surenet-Interface
                            device_status_extra[device_index].device_awake_status.awake=DEVICE_AWAKE;
                            if(pda_payload->device_data_status==DEVICE_SEND_KEY)
                            {
                                device_status_extra[device_index].SendSecurityKey = SECURITY_KEY_RENEW;
                                pda_payload->device_data_status = DEVICE_HAS_NO_DATA;
                            }
                            if(pda_payload->device_data_status==DEVICE_DETACH)
                            {
                                pda_payload->device_data_status = DEVICE_HAS_NO_DATA;
                                device_status[device_index].status.valid=0;
                                device_status[device_index].status.associated=0;
                                surenet_update_device_table_line(&device_status[device_index], device_index, true, true);  //call limted version so no time update flags set to limit message rate to Xively
                                sn_unpair_device(device_status[device_index].mac_address);       // remove from NVM Device Stats
                                surenet_printf( " Detached device \r\n");
                            }
                            if( pda_payload->device_data_status == DEVICE_RCVD_SEGS )
                            {	// copy device_rcvd_segs parameters into a mailbox and send to application
								// First update encryption flag for this device
								if( packet_length - sizeof(HEADER) > offsetof(PACKET_DEVICE_AWAKE_PAYLOAD, encryption_type_extended) )
								{	// Last byte is optional, so check it's actually there.
									device_status_extra[device_index].encryption_type = pda_payload->encryption_type_extended;
								} else
								{	// Not specified, so default to XTEA. Must be an older firmware, or a Pet Door Connect.
									device_status_extra[device_index].encryption_type = SURENET_CRYPT_BLOCK_XTEA;
								}								
                                pda_payload->device_data_status = DEVICE_HAS_NO_DATA;
								device_rcvd_segs[device_index] = pda_payload->rcvd_segs_params.received_segments[0];	//keep this information at SureNet level as it is used when deciding which segments of a chunk to send.
                                memcpy(&device_rcvd_segs_parameters, &pda_payload->rcvd_segs_params, sizeof(device_rcvd_segs_parameters));
								device_rcvd_segs_parameters.device_mac = rx_packet.packet.header.source_address;	// record which device made this request
                                xQueueSend(xDeviceRcvdSegsParametersMailbox,&device_rcvd_segs_parameters,0);   // put result in mailbox for Surenet-Interface
								// Now we are going to cheekily pause this task to give the DFU task
								// an opportunity to find the required f/w page and load it into
								// the mailbox for this task to consume. By doing this, it means that
								// the f/w chunk should be sent on the same Conversation that
								// requested it rather than the one after.
								xQueuePeek(xDeviceFirmwareChunkMailbox, &dummy_chunk, pdMS_TO_TICKS(10));
                            }
                            if(pda_payload->device_data_status==DEVICE_SEGS_COMPLETE)
                            {	// This indicates the successful reception of a chunk. Not sure we care
                                pda_payload->device_data_status = DEVICE_HAS_NO_DATA;
                            }
							
							if( DEVICE_HAS_DATA == pda_payload->device_data_status)
							{	// there is data for us, so we need to establish what decryption to use
								if( packet_length - sizeof(HEADER) > offsetof(PACKET_DEVICE_AWAKE_PAYLOAD, encryption_type) )
								{	// Last byte is optional, so check it's actually there.
									device_status_extra[device_index].encryption_type = pda_payload->encryption_type;
								} else
								{	// Not specified, so default to XTEA. Must be an older firmware, or a Pet Door Connect.
									device_status_extra[device_index].encryption_type = SURENET_CRYPT_BLOCK_XTEA;
								}			
							}
							
                            device_status_extra[device_index].device_awake_status.data=(DEVICE_DATA_STATUS)pda_payload->device_data_status;
                            device_status[device_index].device_rssi = pda_payload->device_rssi;
                            device_status[device_index].hub_rssi = rx_packet.packet.header.rss;
                            if(device_status[device_index].lock_status!=pda_payload->lock_status||(device_status[device_index].status.valid==0))  //has lock_status changed
                            {
                                device_status[device_index].lock_status = pda_payload->lock_status;
                                surenet_update_device_table_line(&device_status[device_index], device_index, true, true);  //call limted version so no time update flags set to limit message rate to Xively
                            }

                            if(device_status_extra[device_index].deviceMinutes!=pda_payload->device_minutes) //have minutes changed
                            { // minutes changed
								device_status_extra[device_index].deviceMinutes=pda_payload->device_minutes;
                                if( (hub_debug_mode & HUB_SEND_TIME_UPDATES_FROM_DEVICES_EVERY_MINUTE) || \
									(uptime_min_count == (device_index+1) ))  //Cat Flap sends hourly messages itself
								{ // minutes changed, and either hub_debug_mode==1 or hour changed
                                    surenet_update_device_table_line(&device_status[device_index], device_index, false, true);  //call limted version so no time update flags set to limit message rate to Xively
									// Now we have to synthesise a message to the Server, setting registers 33,34,35 which are
									// Battery voltage, hours and minutes. So we need to use COMMAND_SET_REG, length, reg addr,
									// number of registers, reg values, parity.
									// The fake message will end up in surenet_data_received_cb() so we have to ensure we
									// include all the sanity checks it applies
									T_MESSAGE *rx_message;
									rx_message=(T_MESSAGE *)((uint8_t *)&rx_packet+sizeof(HEADER)); //skip past SureNet Header
									rx_message->payload[4] = pda_payload->battery_voltage;	// we do the data copying first to ensure we don't overwrite it
									rx_message->payload[5] = pda_payload->device_hours;		// with any of the fixed values we write below.
									rx_message->payload[6] = pda_payload->device_minutes;
									rx_packet.packet.header.packet_type = PACKET_DATA;
									rx_packet.packet.header.packet_length = 12+23;	// rx_message->length (below) + sizeof(header) -1. Not sure why -1 is needed.
									rx_message->command = 2;	// COMMAND_SET_REG - This is not available as a #define in this part of the code
									rx_message->length = 12; // 2 for COMMAND_, 2 for length, 2 for reg address, 2 for num of reg, 3 for values, and 1 for parity
									rx_message->payload[0]=0;	// register number high byte
									rx_message->payload[1]=33;	// register number low byte
									rx_message->payload[2]=0;	// num of reg high byte
									rx_message->payload[3]=3;	// num of reg low byte
									// We don't need to bother with the parity calculation as it isn't checked
									sn_data_received(&rx_packet);
                                }
                            }
                        }
                    } else
                    {
                        surenet_printf( "Send refuse awake\r\n");
                        sn_transmit_packet(PACKET_REFUSE_AWAKE,rx_packet.packet.header.source_address,0,0,-1,false);  //tell device that we are not paired
                    }
                    break; // case PACKET_DEVICE_AWAKE:
                case PACKET_DEVICE_TX:
                    break;
                case PACKET_END_SESSION:
                    conv_end_session_flag=true;
                    break;
                case PACKET_DETACH: // not sure as a Hub we should ever get this
                    surenet_printf( "Received PACKET_DETACH! - should never happen\r\n");
                    ack_payload[0]=rx_packet.packet.header.sequence_number;
                    ack_payload[1] = 0;         // send an ACK back
                    sn_transmit_packet(PACKET_ACK,rx_packet.packet.header.source_address,ack_payload,2,-1,false);
                    break;
                case PACKET_DEVICE_SLEEP:
                    break;
                case PACKET_DEVICE_P2P:
                    break;
                case PACKET_ENCRYPTION_KEY:
                    break;
                case PACKET_REPEATER_PING:
                    break;
                case PACKET_PING_R:
                {   // curly brace to overcome weird 'C' limitation of not being able to declare variables inside a case statement
                    uint8_t tmp = ~ping_seq;  //cant use ~ping_seq directly, treated as uint32_t!!!
                    static uint8_t last_ping_val;
                    ping_stats.ping_value = last_ping_val+1;  //force failure if sequence number doesn't match so increments nbad;

                    if(rx_packet.packet.payload[0]==tmp)  //Device inverts sequence number as a check
                    {
                        ping_stats.reply_timestamp = get_microseconds_tick();  //May want to use this for timing ping response time
                        ping_stats.found_ping = true;						// stop repeated pings
						ping_stats.report_ping = true;						// and report it's happened
						ping_stats.num_good_pings++;
                        ping_stats.ping_rss = rx_packet.packet.payload[2];
                        ping_stats.ping_value = ~rx_packet.packet.payload[0];
                        memcpy(ping_stats.ping_res, rx_packet.packet.payload, 3);
                        ping_stats.ping_res[3] = rx_packet.packet.header.rss;
						ping_stats.hub_rssi_sum += ping_stats.ping_res[3];
						ping_stats.device_rssi_sum += ping_stats.ping_rss;
                    }

                    surenet_printf( "New ping counts %x %x\r\n", (unsigned int)ping_stats.num_good_pings, (unsigned int)ping_stats.num_bad_pings);
                    last_ping_val = ping_stats.ping_value+1;
                    break;
                }
				case PACKET_BLOCKING_TEST:
					blocking_sequence_no = 	 rx_packet.packet.payload[0] + (rx_packet.packet.payload[1]<<8) + \
											(rx_packet.packet.payload[2]<<16) + (rx_packet.packet.payload[3]<<24);
            		xQueueSend(xBlockingTestMailbox,&blocking_sequence_no,0);   // put result in mailbox for Surenet-Interface
					break;
                default:
                    return false;       // bad packet
                    break;

            } // switch (rx_packet.packet.header.packet_type)
        } // repeated_packet==false
    } // packet is for us
    return true;
}

/**************************************************************
 * Function Name   : rx_seq_init()
 * Description     : Clear valid flag in list of received sequence numbers
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void rx_seq_init(void)
{
  int i;
  for (i=0; i<RX_SEQ_BUFFER_SIZE; i++)
  {
    received_sequence_numbers[i].valid=0;
  }
}

/**************************************************************
 * Function Name   : store_ack_response
 * Description     : Inspect list of received_sequence_numbers[], looking for a
 *                 : mac and seq_no match. If found, store ack_response there.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void store_ack_response(uint64_t mac, uint8_t seq_no, uint8_t ack_response)
{
	uint32_t i = 0;

	while( ((received_sequence_numbers[i].src_mac != mac) || (received_sequence_numbers[i].sequence_number != seq_no)) && (i<256) )
	{
		i++;
	}
	if( i < 256 )
	{
		received_sequence_numbers[i].ack_response = ack_response;
	} else
	{	// should never happen because the initial call to is_packet_new() should have made this entry
		surenet_printf( "Failed to match received_sequence_number[] entry\r\n");
	}
}

/**************************************************************
 * Function Name   : is_packet_new()
 * Description     : Scan list of sequence numbers of recently received packets, and see if
 *                 : this sequence number rx_packet.x is already there. Also use the src mac as multiple
 *                 : sources could be using the same sequence number.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
bool is_packet_new(RECEIVED_PACKET *rx_packet, uint8_t *ack_payload)
{
  int i;
  uint8_t seq_num = rx_packet->packet.header.sequence_number;
  uint8_t type = rx_packet->packet.header.packet_type;
  uint8_t lifetime = SEQUENCE_STORE_LIFETIME;
  if(type==PACKET_DATA)
  {
      lifetime = SEQUENCE_STORE_LIFETIME_LONG;
  }
  for (i=0; i<RX_SEQ_BUFFER_SIZE; i++)
  {
    if ((received_sequence_numbers[i].valid!=0) && (received_sequence_numbers[i].sequence_number==seq_num))
    { // we have found the sequence number, but just do a sanity check on the MAC before confirming it's a duplicate
      if (received_sequence_numbers[i].src_mac==rx_packet->packet.header.source_address)
      {
        received_sequence_numbers[i].valid = lifetime;  //reset counter to starting value
		ack_payload[0]=received_sequence_numbers[i].sequence_number;
		ack_payload[1]=received_sequence_numbers[i].ack_response;	// previous value sent
        return false; // MAC addresses match, as does sequence number, so it's not a new packet
      }
    }
  }

  // must be a new packet as we've not found any matching numbers
  // We add it to the sequence number list now
#if (true == SURENET_SHOW_ACK_STUFF)
  surenet_printf( "New packet\r\n");
#endif
  i=0;
  while(i<RX_SEQ_BUFFER_SIZE)
  {
    if (received_sequence_numbers[i].valid==0)
    {
      received_sequence_numbers[i].valid=lifetime;
      received_sequence_numbers[i].sequence_number=seq_num;
      received_sequence_numbers[i].src_mac=rx_packet->packet.header.source_address;
      return true;
    }
    i++;
  }
  // If we get here, it means we couldn't store the sequence number, so a repeated packet may occur
  surenet_printf( "Sequence queue full - repeats of this packet won't be dropped\r\n");
  return true;
}

/**************************************************************
 * Function Name   : age_sequence_numbers
 * Description     : Makes previously seen packets 'age'. Called about every 100ms
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void age_sequence_numbers(uint8_t ticks)
{
  int i;

  for (i=0; i<RX_SEQ_BUFFER_SIZE; i++)
  {
    if (received_sequence_numbers[i].valid!=0)
    {
      if (received_sequence_numbers[i].valid>=ticks)
        received_sequence_numbers[i].valid-=ticks;
      else
       received_sequence_numbers[i].valid=0;
    }
  }
}

/**************************************************************
 * Function Name   :timeout_handler()
 * Description     : Check all the devices to see if they have gone offline
 *                 : Update the pairing table accordingly
 *                 : Note this function does not block as doing so may deadlock against Hermes-App
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void timeout_handler(void) // Deleted old comment to stop Tom chirruping.
{
    static int i=0;
	BaseType_t retval;
    uint32_t now;

    now = get_microseconds_tick();
	retval = pdPASS;

	// A better implementation would be to update as many device table lines as possible
	// until surenet_update_device_table_line does not return pdPASS, then exit this fn
	// and try again on the next iteration.

        if (((now-device_status[i].last_heard_from)>OFFLINE_TIMEOUT) && \
            (device_status[i].status.online==1))
        {
            surenet_printf("Device %d has gone offline\r\n",i);
            device_status[i].status.online=0; // set device as offline (but leave it still valid)
		retval = surenet_update_device_table_line(&device_status[i], i, false, false);
        }

	if( pdPASS == retval)
	{
		if( i++>= MAX_NUMBER_OF_DEVICES) i = 0;
    }

  // Note that in Hub1, there is additional code in this function to trigger
  // a channel hop if no devices have been heard of from a long time.
  // This has been omitted because the handling code is not operational.
}

/**************************************************************
 * Function Name   : store_firmware_chunk
 * Description     : Store a firmware chunk in the array firmware_chunk[]
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void store_firmware_chunk(DEVICE_FIRMWARE_CHUNK *device_firmware_chunk)
{
	uint8_t i=0;
	// first strategy to find an empty slot is to look for one where has_data == false
	while( (firmware_chunk[i].has_data == true) && (i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES))
	{
		i++;
	}
	if( DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES == i)
	{	// did not find a slot without data, so we will look for the oldest one
		uint8_t oldest_index = 0;
		uint32_t oldest_age = get_microseconds_tick() - firmware_chunk[0].timestamp;
		uint32_t current_age;
		for( i=1; i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES; i++)
		{
			current_age = get_microseconds_tick() - firmware_chunk[i].timestamp;
			if( current_age>oldest_age)
			{
				oldest_index = i;
				oldest_age = current_age;
			}
		}
		i = oldest_index;
	}

	// Now i is the index of either a free entry in firmware_chunk[] or the oldest

	memcpy(firmware_chunk[i].data,device_firmware_chunk->chunk_data,CHUNK_SIZE);
	firmware_chunk[i].has_data = true;
	firmware_chunk[i].timestamp = get_microseconds_tick();
	firmware_chunk[i].device_index = device_firmware_chunk->device_index;
	firmware_chunk[i].len = device_firmware_chunk->len;
	firmware_chunk[i].chunk_address = device_firmware_chunk->chunk_address;
}

/**************************************************************
 * Function Name   : is_there_a_firmware_chunk
 * Description     : Check to see if there is a valid firmware chunk for this conversee
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
int is_there_a_firmware_chunk(uint8_t conversee)
{
	int i;

	for( i=0; i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES; i++)
	{
		if( (firmware_chunk[i].device_index == conversee) &&
			(firmware_chunk[i].has_data == true))
		{
			break;
		}
	}
	if( DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES == i)
	{
		i = -1;	// flag that no chunk was found
	}

	return i;
}

/**************************************************************
 * Function Name   : sn_GenerateSecurityKey
 * Description     : Generates key for use with XTEA
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_GenerateSecurityKey(uint8_t device_index)
{
	uint8_t i;
	for (i=0; i<SECURITY_KEY_SIZE; i=i+4)
	{
		*(uint32_t *)&SecurityKeys[device_index][i] = hermes_rand();
	}
}

/**************************************************************
 * Function Name   : sn_CalculateSecretKey
 * Description     : Generates key for use with ChaCha
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void sn_CalculateSecretKey(uint8_t device_index)
{
	uint8_t			ChaCha_SureKey_Constant[CHACHA_MAX_KEY_SZ]; // = "Sing, Goddess, of Achilles' rage";
	const uint8_t 	obfuscated_constant[]={0x29,0x23,0xbe,0x84,0xe1,0x6c,0xd6,0xae,0x52,0x90,0x49,0xf1,0xf1,0xbb,0xe9,0xeb,0xb3,0xa6,0xdb,0x3c,0x87,0x0c,0x3e,0x99,0x24,0x5e,0x0d,0x1c,0x06,0xb7,0x47,0xde,0x7a,0x4a,0xd0,0xe3,0xcd,0x4c,0x91,0xc1,0x36,0xf4,0x2c,0x82,0x82,0x97,0xc9,0x84,0xd5,0x86,0x9a,0x5f,0xef,0x65,0x52,0xf5,0x41,0x2d,0x2a,0x3c,0x74,0xd6,0x20,0xbb};
	wc_Sha256		context;
	int				i;
	
    for( i=0; i<32; i++)
    {
        ChaCha_SureKey_Constant[i]=obfuscated_constant[i+32] ^ obfuscated_constant[i];
    }
	
	wc_InitSha256(&context);
	wc_Sha256Update(&context, (const byte*)ChaCha_SureKey_Constant, CHACHA_MAX_KEY_SZ);
	wc_Sha256Update(&context, (const byte*)&device_status[device_index].mac_address, sizeof(device_status[device_index].mac_address));
	wc_Sha256Final(&context, (byte*)SecretKeys[device_index]);
}

void ChaCha_Encrypt(uint8_t* data, uint32_t length, uint32_t position, uint8_t pair_index)
{
	static CHACHA_CONTEXT	Context;
	
	wc_Chacha_SetKey(&Context.WolfStruct, SecretKeys[pair_index], CHACHA_MAX_KEY_SZ);
	wc_Chacha_SetIV(&Context.WolfStruct, SecurityKeys[pair_index], position);
	wc_Chacha_Process(&Context.WolfStruct, data, data, length);
}

PACKET_TYPE sn_Encrypt(uint8_t* data, uint32_t length, uint32_t position, uint8_t dest_index)
{
//	uint32_t	i;
	device_status_extra[dest_index].SecurityKeyUses++;
	
//	crypt_printf("\r\n\t# Encrypting:\t");
//	for( i = 0; i < length; i++ ){ crypt_printf(" %02x", data[i]); }
//	crypt_printf("\r\n\t# Result:\t");
//	
	switch( device_status_extra[dest_index].encryption_type )
	{
		case SURENET_CRYPT_BLOCK_XTEA:
		    XTEA_64_Encrypt(data, length, SecurityKeys[dest_index]);
//			for( i = 0; i < length; i++ ){ crypt_printf(" %02x", data[i]); }
			return PACKET_DATA;
		case SURENET_CRYPT_CHACHA:
			ChaCha_Encrypt(data, length, position, dest_index);
//			for( i = 0; i < length; i++ ){ crypt_printf(" %02x", data[i]); }
			return PACKET_DATA_ALT_ENCRYPTED;
		default:
//			crypt_printf("No Encryption.");
			return PACKET_DATA; // Just leave it be.
	}
}

void sn_Decrypt(uint8_t* data, uint32_t length, uint32_t pair_index, uint32_t position)
{
//	uint32_t i;
	if( pair_index > MAX_NUMBER_OF_DEVICES ){ return; } 
	device_status_extra[pair_index].SecurityKeyUses++;
	
//	crypt_printf("\r\n\t# Decrypting:\t");
//	for( i = 0; i < length; i++ ){ crypt_printf(" %02x", data[i]); }
//	crypt_printf("\r\n\t# Result:\t");
//	
	switch( device_status_extra[pair_index].encryption_type )
	{
		case SURENET_CRYPT_BLOCK_XTEA:
			XTEA_64_Decrypt(data,length, SecurityKeys[pair_index]); // decrypt payload
//			for( i = 0; i < length; i++ ){ crypt_printf(" %02x", data[i]); }
			break;
		case SURENET_CRYPT_CHACHA:
			ChaCha_Encrypt(data, length, position, pair_index);
//			for( i = 0; i < length; i++ ){ crypt_printf(" %02x", data[i]); }
			break;
		default:
			// Just leave it be.
			break;
	}
}

#if SURENET_ACTIVITY_LOG
const char*	log_names[] 	= { "SURENET_LOG_NOTHING                 ",
								"SURENET_LOG_DEVICE_AWAKE_RECEIVED   ",
								"SURENET_LOG_STATE_MACHINE_RESPONDED ",
								"SURENET_LOG_END_OF_CONVERSATION     ",
								"SURENET_LOG_TIMEOUT                 ",
								"SURENET_LOG_SECURITY_NACK           ",
								"SURENET_LOG_SECURITY_ACK_TIMEOUT    ",
								"SURENET_LOG_DATA_RETRY_FAIL         ",
								"SURENET_LOG_STATE_MACHINE_SW_TO_SEND",
								"SURENET_LOG_SENT_SECURITY_KEY       ",
								"SURENET_LOG_SECURITY_KEY_ACK        ",
								"SURENET_LOG_DATA_SEND               ",
								"SURENET_LOG_DATA_ACK                ",
								"SURENET_LOG_DATA_NACK               ",
								"SURENET_LOG_DATA_RETRY_ATTEMPT      ",
								"SURENET_LOG_END_OF_SESSION          "};

void dump_surenet_log(void)
{
	uint8_t i;
	uint32_t delta;
	zprintf(CRITICAL_IMPORTANCE,"idx  timestamp delta     SURENET_LOG_ACTIVITY                 Device index\r\n");
	for (i=0; i<NUM_SURENET_LOG_ENTRIES; i++)
	{
		if( i>0)
			delta = surenet_log_entry[i].timestamp - surenet_log_entry[i-1].timestamp;
		else
			delta = 0;
		zprintf(CRITICAL_IMPORTANCE,"[%02d] %09d %9d %s %03d\r\n",i,surenet_log_entry[i].timestamp,
				delta, log_names[surenet_log_entry[i].activity], surenet_log_entry[i].parameter);
		DbgConsole_Flush();
	}
}

uint64_t get_microseconds_tick_64(void)
{
	static uint32_t last_timestamp;
	static uint64_t adder=0;
	uint64_t timestamp;

	timestamp = get_microseconds_tick();

	if( ((last_timestamp & 0x80000000)!=0) && ((timestamp & 0x80000000)==0))
	{ // just rolled over
		adder = adder + 0x100000000;
	}
	last_timestamp = timestamp;

	return (uint64_t)timestamp + adder;
}

const uint64_t dec_vals[]={1000000000000000000,
							100000000000000000,
							 10000000000000000,
							  1000000000000000,
							   100000000000000,
							    10000000000000,
								 1000000000000,
								  100000000000,
								   10000000000,
								    1000000000,
									 100000000,
									  10000000,
									   1000000,
									    100000,
										 10000,
										  1000,
										   100,
										    10,
											 1};
void longprint(uint64_t value)
{
	uint8_t dec_val_index = 0;
	uint8_t this_digit;
	bool started = false;
	do
	{
		this_digit=0;
		while( value>=dec_vals[dec_val_index])
		{
			value = value - dec_vals[dec_val_index];
			this_digit++;
		}
		if( this_digit>0)
		{	// if it's a non zero digit, print it
			zprintf(CRITICAL_IMPORTANCE,"%x",this_digit);
			started = true; // start printing digits now as we've hit a non zero digit
		}
		else
		{	// this one is a zero, so only print it if we have had non zero digits before this one
			if( true == started)
			{
				zprintf(CRITICAL_IMPORTANCE,"0");
			}
		}
		dec_val_index++;
	}while(dec_val_index<0x13);
}

void snla(SURENET_LOG_ACTIVITY activity, uint8_t parameter)
{
	//#define SNLOG_REALTIME_DUMP
#ifdef SNLOG_REALTIME_DUMP
	uint64_t timestamp;
	timestamp = get_microseconds_tick_64();
#endif

	surenet_log_entry[surenet_log_entry_in].timestamp = get_microseconds_tick();
	surenet_log_entry[surenet_log_entry_in].activity = activity;
	surenet_log_entry[surenet_log_entry_in].parameter = parameter;

#ifdef SNLOG_REALTIME_DUMP
	portENTER_CRITICAL();
	longprint(timestamp);
	zprintf(CRITICAL_IMPORTANCE,",%d,%d\r\n",activity,parameter);
	portEXIT_CRITICAL();
#endif
	surenet_log_entry_in++;
	if( surenet_log_entry_in >= NUM_SURENET_LOG_ENTRIES)
		surenet_log_entry_in = 0;
}

#endif
