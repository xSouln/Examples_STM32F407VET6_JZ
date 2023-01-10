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
* Filename: DeviceFirmwareUpdate.c    
* Author:   Chris Cowdery 26/02/2020
* Purpose:  Handles Device Firmware Update
*
* This file handles the Application side of Device Firmware Update.
* It maintains a number of firmware caches for supporting simultaneous
* Devices each updating their firmware.
* These caches are updated via a Server request when the network
* requests data which is not in the cache.
* 
* The flow is as follows:
* Requests come into surenet_device_rcvd_segs_cb() and are stored in DFU_queue.
* The DFU_queue is organised as one entry per attached device, using the same
* index as that used by the Device Table. A new request overwrites an old one.
* DFU_Handler() is polled by hermes-app. It iterates through the queue looking for
* 'in use' entries. When it finds one, it runs a little state machine to check
* the cache to see if it has the data, if not it fetches it. Then it serves it back
* to SureNet via a mailbox to be sent to the device and frees the entry in the
* DFU_queue.
* The cache entry holds a 4K block of firmware and some metadata identifying the target
* device and where the firmware sits within the target memory. Cache entries are
* timestamped at time of last use, so old ones can be overwritten with newer ones.
* An empty cache entry is used (with a flag) to indicate that there is no more
* firmware available from the server.
*
* If a cache entry is older than MAXIMUM_CACHE_AGE, it is considered stale and will
* not be used. This is to cover the case where a device has a firmware update, then
* subsequently requests a second firmware update for a different revision of f/w.
* Because f/w revision is not known to the Hub, it could consider that it already
* has the required firmware page and start serving that instead of requesting it from
* the server.             
*
* To deal with the issue of too many Device Firmware Updates thrashing the
* cache and hence the Server with a 4K page request for every 128 bytes of f/w
* downloaded, a 'Gatekeeper' is used.
* The gatekeeper keeps a list of which devices are requesting firmware. The
* first DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES are recorded and timestamped.
* Requests from these devices are serviced normally. Any additional requests
* are ignored.
* When a firmware download had completed, the entry in the gatekeepers list is
* cleared so a waiting device will then get serviced.
* The list also contains timestamps, so if a device stops requesting firmware
* it's list entry will become stale and can be re-used.
**************************************************************************/
#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <compiler.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "hermes-time.h"

// Project includes
#include "SureNet-Interface.h"
#include "DeviceFirmwareUpdate.h"
#include "HTTP_Helper.h"
#include "devices.h"
#include "utilities.h"	// for crc16Calc()
#include "../MQTT/MQTT.h"	// for get_mqtt_connection_state() so we can hold off DFU unless we are connected.

#define MAX_PAGE_FETCH_ATTEMPTS			100		// If the Server hasn't given us the f/w page in this
												// many attempts, we give up and empty the queue entry
#define MAXIMUM_CACHE_AGE				(2*60*usTICK_SECONDS)	// a cache entry older than this is assumed to be stale.
																// This is somewhat empirically derived. A 'normal' update
																// takes about 17 minutes for a 80K image, so a 4K page
																// should have a life of very approximately 1 minute.

#define HTTP_HEADER_MAX_SIZE			300	// sort of empirically derived (measured number = 236)
#define CHUNKS_PER_PAGE					32
#define MAX_PAGE_SIZE					((CHUNK_SIZE)*CHUNKS_PER_PAGE)
// next group relate to the header in a received page from the Server
// Header = "CCCC 11111111 2222 33333333 4444 VV " - total 36 chars
// If the CRC (CCCC in above) and it's delimiter are removed, then the total length becomes 31
// If there is no subsequent payload, then there is no trailing space either, so the total length becomes 30.
#define PAGE_HEADER_SIZE				36	// this is for the 6 parameters
#define HEADER_ERASE_ADDRESS 			5	// offset of first byte to be CRC'd
#define HEADER_WITHOUT_CRC_LEN			(PAGE_HEADER_SIZE - HEADER_ERASE_ADDRESS)	// header without CRC or it's delimiter space
#define HEADER_WITHOUT_CRC_OR_PAYLOAD_LEN	(HEADER_WITHOUT_CRC_LEN - 1)	// header without CRC, it's delimiter, and the delimiter at the end of the header

#define DFU_GATEKEEPER_TIMEOUT	(usTICK_SECONDS * 60)
#define HTTP_REQUEST_TIMEOUT	(usTICK_SECONDS * 65)	// time to wait for an HTTP response
// local variables
uint8_t received_page[HTTP_HEADER_MAX_SIZE + PAGE_HEADER_SIZE + MAX_PAGE_SIZE];	// should be 4688 bytes
char *URL = "hub.api.surehub.io";
char *resource = "/api/firmware";
char contents[128];	// this is the request string for the HTTP Post Request - must be persistent as accessed from other task.

// For the DFU_queue, we use the same indexing system as the Device Table.
DEVICE_RCVD_SEGS_PARAMETERS_QUEUE DFU_queue[MAX_NUMBER_OF_DEVICES];

typedef struct
{	// each page has a max of 4K of actual program data. So 96K in a iDSCF is 24 pages
	uint64_t		device_mac;		// target device
	uint32_t		last_used;		// timestamp when this firmware cache entry was last read from
	bool 			in_use;
	bool 			last_page;		// not clear whether this means empty page, or final page with data in it...
	uint8_t 		number_of_chunks;	// max of 32
	uint8_t			page_number;	// maximum target code size is 1Mbyte (4K*256)
	uint8_t 		page[MAX_PAGE_SIZE];	// actual data
} DEVICE_FIRMWARE_PAGE;

typedef enum	// DFU state machine
{
	DFU_STATE_SEARCH,
	DFU_STATE_CACHE_CHECK,
	DFU_STATE_SEND_CHUNK,
	DFU_STATE_WAIT_FOR_HTTP_RESPONSE,	
}DFU_STATE;

DEVICE_FIRMWARE_PAGE	firmware_cache[DEVICE_FIRMWARE_CACHE_ENTRIES];

typedef struct
{
	uint64_t 	mac;
	uint32_t 	timestamp;
	bool		active;
} DFU_REQUEST_CONTROL;

DFU_REQUEST_CONTROL DFU_request_control[DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES];

extern PRODUCT_CONFIGURATION product_configuration;
extern QueueHandle_t xHTTPPostRequestMailbox;
// local functions
void DFU_queue_add(DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX *params);
bool DFU_cache_check(uint8_t device_index, uint16_t chunk_addr);
uint8_t DFU_cache_fetch(uint8_t device_index, uint16_t chunk_addr, bool send_secret_key);
void DFU_prepare_chunk(DEVICE_FIRMWARE_CHUNK *chunk,uint8_t device_index, uint16_t chunk_addr);
void DFU_FetchFirmwareFromServer(char *buf, uint64_t mac, char*serial, uint32_t page, T_DEVICE_TYPE type, bool send_secret_key);
void DFU_cache_flush(uint64_t device_mac);
bool DFU_parse_received_page(uint8_t device_index, uint8_t cache_entry, uint16_t chunk_address);
void DFU_gatekeeper_done(uint64_t mac);
BaseType_t DFU_Gatekeeper(uint64_t mac);

/**************************************************************
 * Function Name   : DFU_init
 * Description     : Initialises Device Firmware Update
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void DFU_init(void)
{
	uint8_t i;
	for (i=0; i<DEVICE_FIRMWARE_CACHE_ENTRIES; i++)
	{
		firmware_cache[i].in_use = false;
	}
	for( i=0; i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES; i++)
	{
		DFU_request_control[i].mac = 0;
		DFU_request_control[i].timestamp = 0;	
		DFU_request_control[i].active = false;		
	}
	memset(DFU_queue,0,sizeof(DFU_queue));
}

/**************************************************************
 * Function Name   : DFU_Handler
 * Description     : Processes any firmware update requests in the
 *                 : queue.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void DFU_Handler(void)
{
	static 		DFU_STATE DFU_state = DFU_STATE_SEARCH;
	static 		uint8_t device_no = 0;
	uint8_t 	device_initial;
	static 		DEVICE_FIRMWARE_CHUNK device_firmware_chunk;
	static 		uint8_t cache_entry;
	static 		uint32_t request_sent_timestamp;
	bool 		http_request_result;
	BaseType_t 	notify_response;
	bool		send_secret_next_time = false;
	
	switch(DFU_state)
	{
		case DFU_STATE_SEARCH:	// find next device with outstanding request
			device_no = (device_no+1) % MAX_NUMBER_OF_DEVICES;	// ensures we always advance
			device_initial = device_no;
			do
			{
				if (DFU_queue[device_no].in_use==true)	// found one
				{
					DFU_queue[device_no].attempts++;
					if( (DFU_queue[device_no].attempts<MAX_PAGE_FETCH_ATTEMPTS) && 
						(MQTT_STATE_CONNECTED == get_mqtt_connection_state() ))	// Only bother with f/w update if connected.
					{
						zprintf(LOW_IMPORTANCE,"DFU: Found queued request\r\n");
					DFU_state = DFU_STATE_CACHE_CHECK;
					break;
				}
					else
					{
						zprintf(LOW_IMPORTANCE,"DFU: Abandoning request - too many retries\r\n");
						DFU_queue[device_no].in_use=false;
					}
						
				}
				// else skip forwards
				device_no = (device_no+1) % MAX_NUMBER_OF_DEVICES;
			}while (device_no != device_initial);				
			// if we get here, we remain in this state as we never found any outstanding requests
			break;
		case DFU_STATE_CACHE_CHECK:	// see if the data we require has already been cached
			if (DFU_cache_check(device_no,DFU_queue[device_no].chunk_address) == false)
			{	// data being requested is not in the cache
				zprintf(LOW_IMPORTANCE,"DFU: Cache miss - requesting data from server\r\n");
				cache_entry = DFU_cache_fetch(device_no,DFU_queue[device_no].chunk_address, send_secret_next_time);
				request_sent_timestamp = get_microseconds_tick();
				DFU_state = DFU_STATE_WAIT_FOR_HTTP_RESPONSE;	// Wait for HTTP Post task to get the data and notify us
				break;
			}
			zprintf(LOW_IMPORTANCE,"DFU: Cache hit - found chunk in cache\r\n");			
			DFU_state = DFU_STATE_SEND_CHUNK;
			break;
		case DFU_STATE_WAIT_FOR_HTTP_RESPONSE:	// wait for HTTP Post Request task to notify us that it has a result
			notify_response = xTaskNotifyWait(0,0,(uint32_t *)&http_request_result,0);
			if ((notify_response == pdTRUE) || ((get_microseconds_tick()-request_sent_timestamp)>HTTP_REQUEST_TIMEOUT))
			{	// we got a response or a timeout		
				if ((notify_response == pdTRUE) && (http_request_result == true))
				{	// if the call was successful, populate the cache entry
					send_secret_next_time = false;
					zprintf(LOW_IMPORTANCE,"DFU: Received firmware page for device index %d, cache location=%d chunk address=0x%04x...\r\n",device_no,cache_entry,DFU_queue[device_no].chunk_address);
					if (DFU_parse_received_page(device_no, cache_entry, DFU_queue[device_no].chunk_address)==true)
					{
						DFU_state = DFU_STATE_SEND_CHUNK;	// got data in cache, so send it
					}
					else
					{
						zprintf(LOW_IMPORTANCE,"DFU: Data rejected\r\n");
						DFU_state = DFU_STATE_SEARCH;	// was a problem with the received data
					}
				}
				else
				{ // either a failure from the HTTP fetcher or a timeout
					send_secret_next_time = true;	// maybe keys are out of synch?
					zprintf(LOW_IMPORTANCE,"DFU: Did not receive page from server\r\n");
					DFU_state = DFU_STATE_SEARCH;	// couldn't get the data					
				}
			}
			else
			{	 // no notification yet, so wait here until either we get one, or we timeout.
				DFU_state = DFU_STATE_WAIT_FOR_HTTP_RESPONSE;
			}
			break;
		case DFU_STATE_SEND_CHUNK:
			zprintf(LOW_IMPORTANCE,"DFU: Sending chunk to device\r\n");
			DFU_prepare_chunk(&device_firmware_chunk, device_no, DFU_queue[device_no].chunk_address);
			surenet_send_firmware_chunk(&device_firmware_chunk);	// This makes the chunk available for the Hub Conversation state machine to send at the next opportunity
			DFU_queue[device_no].in_use = false;	// clear queue entry
			DFU_state = DFU_STATE_SEARCH;	// all done
			break;
		default:
			DFU_state = DFU_STATE_SEARCH;	// error, go back to start
			break;
	}
}

/**************************************************************
 * Function Name   : DFU_prepare_chunk
 * Description     : Extracts a chunk from the cache.
 * Inputs          :
 * Outputs         :
 * Returns         : true if f/w present, false if not.
 **************************************************************/
void DFU_prepare_chunk(DEVICE_FIRMWARE_CHUNK *chunk,uint8_t device_index, uint16_t chunk_address)
{
	uint32_t requested_page;
	uint32_t chunk_offset;	// chunk offset within page
	uint8_t i;
	uint64_t device_mac;
	
	requested_page = chunk_address / CHUNKS_PER_PAGE;	
	chunk_offset = chunk_address-(requested_page*CHUNKS_PER_PAGE);
	device_mac = DFU_queue[device_index].device_mac;
	
	for (i=0; i<DEVICE_FIRMWARE_CACHE_ENTRIES; i++)
	{
		if ((firmware_cache[i].in_use == true) && \
			(firmware_cache[i].device_mac == device_mac) && \
			(firmware_cache[i].page_number == requested_page))
		{
			if ((firmware_cache[i].last_page == true) || (chunk_offset>=firmware_cache[i].number_of_chunks))
			{	//a chunk has been requested which is not within this page. The only reason for this is if
				// the page is not 'full'. This means we have reached the end of the firmware image.
				// We send a chunk of length 0 in this case to indicate we have no data.
				firmware_cache[i].last_used = get_microseconds_tick();	// record when this cache entry was last used.
				chunk->device_index = device_index;
				chunk->chunk_address = chunk_address;
				chunk->len = 0;	// empty chunk
				DFU_gatekeeper_done(device_mac);	// empty entry in gatekeeper list
				zprintf(LOW_IMPORTANCE,"Indicating empty chunk\r\n");
				return;					
			}
			else
			{ // a chunk has been requested which is within the number of chunks in this page
				firmware_cache[i].last_used = get_microseconds_tick();	// record when this cache entry was last used.
				memcpy(chunk->chunk_data,&firmware_cache[i].page[chunk_offset*CHUNK_SIZE],CHUNK_SIZE);
				chunk->device_index = device_index;
				chunk->chunk_address = chunk_address;
				chunk->len = CHUNK_SIZE;	// always the same, a full chunk!
				return;
			}
		}
	}
	
	zprintf(CRITICAL_IMPORTANCE,"DFU_prepare_chunk() - FAILED TO FIND CACHE PAGE!\r\n");
	return;
}
		
/**************************************************************
 * Function Name   : DFU_cache_check
 * Description     : Checks the cache for the presence of specified f/w chunk
 * Inputs          :
 * Outputs         :
 * Returns         : true if f/w present, false if not.
 **************************************************************/
bool DFU_cache_check(uint8_t device_index, uint16_t chunk_address)
{
	uint32_t requested_page;
	uint8_t i;
	uint64_t device_mac;
	
	requested_page = chunk_address / CHUNKS_PER_PAGE;	
	device_mac = DFU_queue[device_index].device_mac;
	
	for (i=0; i<DEVICE_FIRMWARE_CACHE_ENTRIES; i++)
	{
		if ((firmware_cache[i].in_use == true) && \
			(firmware_cache[i].device_mac == device_mac) && \
			(firmware_cache[i].page_number == requested_page) && \
			((get_microseconds_tick() - firmware_cache[i].last_used) < MAXIMUM_CACHE_AGE))
		{	// Note we return a cache hit even if the chunk is past the end of valid
			// data because we detect this in DFU_Prepare_Chunk() and send a length of 0 (indicating we have reached the end)
			// Also return true if the cache is valid but empty (indicating we have reached the end as well)
			return true;	// found cache hit
		}
	}
	return false;	// cache miss
}

/**************************************************************
 * Function Name   : DFU_cache_fetch
 * Description     : Requests the appropriate page from the server
 *                 : Searches for an empty cache entry, if not, overwrites the oldest one
 * Inputs          :
 * Outputs         :
 * Returns         : 
 **************************************************************/
uint8_t DFU_cache_fetch(uint8_t device_index, uint16_t chunk_address, bool send_secret_key)
{
	uint8_t i;
	uint8_t cache_entry=0xff;
	uint32_t requested_page;
	uint32_t oldest;
	uint64_t device_mac;
	
	requested_page = chunk_address / CHUNKS_PER_PAGE;	
	device_mac = DFU_queue[device_index].device_mac;
	
	// look for an empty cache entry
	for (i=0; i<DEVICE_FIRMWARE_CACHE_ENTRIES; i++)
	{
		if (firmware_cache[i].in_use==false)
		{
			cache_entry=i;	// found one
			break;
		}
	}
	
	if (cache_entry == 0xff)
	{	// all cache entries used, so find the oldest one
		oldest = get_microseconds_tick() - firmware_cache[0].last_used;
		cache_entry = 0;
		for (i=1; i<DEVICE_FIRMWARE_CACHE_ENTRIES; i++)
		{
			if ((get_microseconds_tick()-firmware_cache[i].last_used)> oldest)
			{
				oldest = get_microseconds_tick()-firmware_cache[i].last_used;
				cache_entry = i;
			}
		}
	}
	
	firmware_cache[cache_entry].in_use = false;	//this will be changed to true when the page gets written to the cache
	// obtain the data from the server
	DFU_FetchFirmwareFromServer((char *)received_page, device_mac, \
							(char *)product_configuration.serial_number, requested_page, \
							DFU_queue[device_index].device_type, send_secret_key);	
		
	// this is sent off as a message to the HTTP Post Request task, so we don't block 
	// this task waiting for the reply.
	return cache_entry;	// so the main loop knows where to store this result.
}

/**************************************************************
 * Function Name   : parse_received_page
 * Description     : Process a received page from the Server, extracting
 *                 : the header fields, and populating the cache entry
 * Inputs          : The raw page from the server is in received_page[]
 * Outputs         :
 * Returns         : true if the page was OK, false if it failed CRC
 **************************************************************/
bool DFU_parse_received_page(uint8_t device_index, uint8_t cache_entry, uint16_t chunk_address)
{
	uint32_t crc_sent, erase_addr, erase_count, prog_addr, prog_count, ver;
	uint32_t crc_length;
	uint16_t crc;
	uint32_t data_size;
	uint32_t requested_page;
	
	requested_page = chunk_address / CHUNKS_PER_PAGE;
	// HTTP Header already trimmed off.
	uint8_t *data_start = received_page;
	
	firmware_cache[cache_entry].device_mac = DFU_queue[device_index].device_mac;	
	firmware_cache[cache_entry].last_used = get_microseconds_tick();
	firmware_cache[cache_entry].page_number = requested_page;
	firmware_cache[cache_entry].in_use = true;
	firmware_cache[cache_entry].number_of_chunks = 0;	
	firmware_cache[cache_entry].last_page = false;	// this page has data in it	
	
	if (NULL == data_start)
	{
		zprintf(HIGH_IMPORTANCE,"HTTP header with no firmware received - assuming because end of firmware image\r\n");
		firmware_cache[cache_entry].last_page = false;	// doesn't matter
		firmware_cache[cache_entry].in_use = false;		
		return false;
	}

	// parse the header
    sscanf((char *)data_start, "%x %x %x %x %x %x", (unsigned int *)&crc_sent, (unsigned int *)&erase_addr, (unsigned int *)&erase_count, (unsigned int *)&prog_addr, (unsigned int *)&prog_count, (unsigned int *)&ver);
	
	if ((ver & 0xff) != 0x02)
	{
		zprintf(LOW_IMPORTANCE,"Device Firmware page from Server is not for Thalamus target...r\r\n");
	}
	firmware_cache[cache_entry].number_of_chunks = prog_count;	
	if ((erase_addr==0) && (erase_count==0) && (prog_addr==0) && (prog_count==0))
	{		
		firmware_cache[cache_entry].last_page = true;	// no data
		return true;
	}

	// Handle the CRC. The first four bytes of the page are the CRC
	data_size = prog_count * 136;  //136 bytes per chunk
	crc_length = data_size ? (HEADER_WITHOUT_CRC_LEN + data_size) : HEADER_WITHOUT_CRC_OR_PAYLOAD_LEN;
	crc = CRC16(data_start+HEADER_ERASE_ADDRESS, crc_length, 0xcccc);
	
	if (crc!= (crc_sent&0xffff))
	{
		zprintf(HIGH_IMPORTANCE,"CRC fail on received page from server\r\n");
		firmware_cache[cache_entry].in_use = false;		
		return false;
	}

	memcpy(firmware_cache[cache_entry].page, data_start + PAGE_HEADER_SIZE, data_size);			

	zprintf(LOW_IMPORTANCE,"DFU: Processing page for dev=%d cache_entry=%d chunk=%d\r\n", \
		device_index, cache_entry, chunk_address);
	zprintf(LOW_IMPORTANCE,"DFU: Data received (first 256 bytes):");
	for( int i=0; i<256; i++)
	{
		zprintf(LOW_IMPORTANCE," %02x",firmware_cache[cache_entry].page[i]);
	}
	zprintf(LOW_IMPORTANCE,"\r\n");
	
	return true;
}

/**************************************************************
 * Function Name   : DFU_FetchFirmwareFromServer
 * Description     : Fetches a page of firmware from the server
 * Inputs          : *buf is pointer to buffer to put the result
 *                 : Also need to know what firmware to fetch!
 * Outputs         :
 * Returns         :
 **************************************************************/
void DFU_FetchFirmwareFromServer(char *buf, uint64_t mac, char*serial, uint32_t page, T_DEVICE_TYPE type, bool send_secret_key)
{
	char 						mac_addr_string[20];
	HTTP_POST_Request_params 	http_post_request_params;
	uint32_t 					notify_response;
	uint8_t 					*SharedSecret;
	uint64_t 					time_since_epoch_ms;
	int32_t						encrypted_data;
	mac = swap64(mac);
	snprintf(mac_addr_string, sizeof(mac_addr_string), "%08X%08X ", (uint32_t)(mac>>32), (uint32_t)mac);
	get_UTC_ms(&time_since_epoch_ms);
	
    memset(contents, 0, sizeof(contents));	// not sure this is necessary

	if( false == send_secret_key)
	{	// Send a 'normal' request
	    snprintf(contents, sizeof(contents), \
			"serial_number=%s&mac_address=%s&product_id=%d&page=%d&bootloader_version=&bv=&tv=%llu\0",
			serial, mac_addr_string, type, page, time_since_epoch_ms);		
	} else
	{ // Need to send the shared secret just in case we are out of synch.
		SharedSecret = GetSharedSecret();	// pointer to 16 byte array
	    snprintf(contents, sizeof(contents), \
			"serial_number=%s&mac_address=%s&product_id=%d&page=%d&bootloader_version=&bv=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x&tv=%llu\0",
			serial, mac_addr_string, type, page, \
			SharedSecret[0],SharedSecret[1],SharedSecret[2],SharedSecret[3], \
			SharedSecret[4],SharedSecret[5],SharedSecret[6],SharedSecret[7], \
			SharedSecret[8],SharedSecret[9],SharedSecret[10],SharedSecret[11], \
			SharedSecret[12],SharedSecret[13],SharedSecret[14],SharedSecret[15], \
			time_since_epoch_ms);
	}
	
	zprintf(LOW_IMPORTANCE,"DFU: Request : %s\r\n",contents);
	
	http_post_request_params.URL = URL;
	http_post_request_params.resource = resource;
	http_post_request_params.contents = contents;
	http_post_request_params.response_buffer = buf;
	http_post_request_params.response_size = sizeof(received_page);
	http_post_request_params.xClientTaskHandle = xTaskGetCurrentTaskHandle();
	http_post_request_params.tx_key_source = DERIVED_KEY_CURRENT;
	http_post_request_params.rx_key_source = DERIVED_KEY_CURRENT;	// assume for DFU that no key changes are going to occur
	http_post_request_params.encrypted_data = &encrypted_data;
	http_post_request_params.bytes_read = NULL;
	
	// ask HTTP_Post_Request task to get the data and notify us when it has the answer
	xTaskNotifyWait(0,0,&notify_response,0);	// clear any pending notifications
	
	xQueueSend(xHTTPPostRequestMailbox, &http_post_request_params, 0);	
	// If this fails, the DFU state machine will try again.
}

/**************************************************************
 * Function Name   : DFU_gatekeeper_done
 * Description     : Purge entry from gatekeeper list when f/w update is complete
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void DFU_gatekeeper_done(uint64_t mac)
{
	uint8_t i;
	for( i=0; i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES; i++)
	{
		if(mac == DFU_request_control[i].mac)
		{
			DFU_request_control[i].active = false;
			zprintf(LOW_IMPORTANCE,"DFU: Gatekeeper entry for %08x%08x closed\r\n", (uint32_t)((mac&0xffffffff00000000)>>32),(uint32_t)mac);
		}
	}	
}

/**************************************************************
 * Function Name   : DFU_Gatekeeper
 * Description     : Acts as a gatekeeper for requests from Devices
 *                 : requesting f/w. Will only allow 
 *                 : DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES simultaneous
 *                 : updates.
 * Inputs          :
 * Outputs         :
 * Returns         : pdPASS if the request is allowed, pdFAIL if rejected
 **************************************************************/
BaseType_t DFU_Gatekeeper(uint64_t mac)
{
	uint8_t i;
	
	// first look for an existing entry for this mac, and update it if found
	for( i=0; i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES; i++)
	{
		if( (true == DFU_request_control[i].active) && \
			(mac == DFU_request_control[i].mac))
		{
			DFU_request_control[i].timestamp = get_microseconds_tick();
			zprintf(LOW_IMPORTANCE,"DFU: Request allowed from in-progress Device\r\n");
			return pdPASS;	// Allow DFU to proceed
		}
	}
	
	// if we get here, then this is a new Device
	// First check for an empty or stale slot that can be utilised
	for( i=0; i<DEVICE_MAX_SIMULTANEOUS_FIRMWARE_UPDATES; i++)
	{
		if( (false == DFU_request_control[i].active) ||
			((get_microseconds_tick()-DFU_request_control[i].timestamp)> DFU_GATEKEEPER_TIMEOUT))
		{
			DFU_request_control[i].timestamp = get_microseconds_tick();
			DFU_request_control[i].active = true;
			DFU_request_control[i].mac = mac;
			zprintf(LOW_IMPORTANCE,"DFU: Request allowed from new Device\r\n");
			return pdPASS; // Allow DFU to proceed
		}
	}
	// If we get here, our device is new, and there is no available slot in 
	// the list. So we deny it.
	zprintf(LOW_IMPORTANCE,"DFU: Request blocked\r\n");
	return pdFAIL;
}

/**************************************************************
 * Function Name   : surenet_device_rcvd_segs_cb
 * Description     : Called when a DEVICE_RCVD_SEGS message is received from
 *                 : the device. Will stick the request in a queue to be
 *                 : serviced later.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_device_rcvd_segs_cb(DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX *params)
{
	uint32_t requested_chunk_address;
	uint64_t device_mac;
	
	requested_chunk_address = (params->rcvd_segs_params.fetch_chunk_upper<< 8) + \
								 params->rcvd_segs_params.fetch_chunk_lower;	// 16 bit chunk address
	device_mac = params->device_mac;
		
	if( pdPASS == DFU_Gatekeeper(params->device_mac))
	{
		zprintf(LOW_IMPORTANCE,"DFU: Device %08x%08x", (uint32_t)((device_mac&0xffffffff00000000)>>32),(uint32_t)device_mac);
		zprintf(LOW_IMPORTANCE," requested chunk for address 0x%04x\r\n", requested_chunk_address);		
		DFU_queue_add(params);
	}
	else
	{
		zprintf(LOW_IMPORTANCE,"DFU: Rejecting request from device %08x%08x\r\n", (uint32_t)((device_mac&0xffffffff00000000)>>32),(uint32_t)device_mac);
	}
}

/**************************************************************
 * Function Name   : DFU_queue_add
 * Description     : Adds a firmware request to a queue, overwriting any
 *                 : previous requests from the same device.
 *                 : Note that we drop most of the parameters from received_segments[] as they are not used.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void DFU_queue_add(DEVICE_RCVD_SEGS_PARAMETERS_MAILBOX *params)
{
	uint8_t device_index;
	if (convert_addr_to_index(params->device_mac, &device_index) == false)
	{
		return;	// don't queue as we don't know this source MAC
	}
	if (params->rcvd_segs_params.fetch_chunk_blocks != 0x01)
	{
		zprintf(CRITICAL_IMPORTANCE,"Firmware fetches of more than one Chunk not supported - request dropped\r\n");
		return;
	}
	DFU_queue[device_index].chunk_address = (params->rcvd_segs_params.fetch_chunk_upper<<8) + params->rcvd_segs_params.fetch_chunk_lower;
	DFU_queue[device_index].in_use = true;
	DFU_queue[device_index].attempts = 0;	
	DFU_queue[device_index].device_mac = params->device_mac;
	DFU_queue[device_index].device_type = (T_DEVICE_TYPE)device_type_from_mac(params->device_mac);		
}

/**************************************************************
 * Function Name   : DFU_cache_flush
 * Description     : Remove all entries for a specific device from the cache.
 *                 : Not used.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void DFU_cache_flush(uint64_t device_mac)
{
	uint8_t i;
	for (i=0; i<DEVICE_FIRMWARE_CACHE_ENTRIES; i++)
	{
		if (firmware_cache[i].device_mac == device_mac)
		{
			firmware_cache[i].in_use = false;
		}
	}
}
