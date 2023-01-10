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
* Filename: HubFirmwareUpdate.c   
* Author:   Chris Cowdery 01/09/2020
* Purpose:   
*   
* This file handles updating the Hub firmware with the firmware
* coming down from the Server.
*            
**************************************************************************/

#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <compiler.h>
#include <limits.h> // for ULONG_MAX
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "hermes-time.h"

// Project includes
#include "SureNet-Interface.h"
#include "HubFirmwareUpdate.h"
#include "HTTP_Helper.h"
#include "utilities.h"	// for crc16Calc
#include "Buildnumber.h"
#include "BankManager.h"
#include "NetworkInterface.h"	// for NetworkInterface_IsActive()
#include "signing.h"
#include "wolfssl/wolfcrypt/wc_encrypt.h"

typedef enum
{
	HFU_STATE_IDLE,
	HFU_STATE_START_UPDATE,			// initialise variables etc
	HFU_STATE_WAIT_NETWORK,			// Wait for the network to come up
	HFU_STATE_FETCH_PAGE,			// Fetch the next page from the server
	HFU_STATE_WAIT_PAGE_RECEIVED,	// Wait for the page to be received
	HFU_STATE_PROCESS_PAGE,			// Validate and decrypt page
	HFU_STATE_PROGRAM_PAGE,			// Write the page to Flash
	HFU_STATE_FINISH,				// Flip bank switch and reset
	HFU_STATE_PAUSE,				// Pause before retrying to give the network time to come up again
} HFU_STATE;

HFU_STATE HFU_State;

typedef enum
{
	HFU_PAGE_OK,
	HFU_PAGE_RECEIVED_PARAMETER_BLOCK,
	HFU_PAGE_CRC_FAIL,	// probably page too short
	HFU_PAGE_PARAM_ERROR,
	HFU_PAGE_PROGRAM_FAILURE,
	HFU_PAGE_EMPTY,
	HFU_PAGE_FINAL,
} HFU_PROCESS_PAGE_RESULT;

EventGroupHandle_t xHFU_EventGroup;
#define HFU_TRIGGER_UPDATE			(1<<0)

#define	MAX_PAGE_FETCH_RETRYS		10	// number of times a page fetch from the server will
										// be attempted before giving up on the f/w update.
#define PAGE_FETCH_RETRIES_BEFORE_NEW_SECRET	3	// after this many retries, we send a new
													// shared secret to the Server in case it has 
													// got out of step.

#define HFU_HTTP_HEADER_MAX_SIZE	300	// sort of empirically derived (measured number = 236)
#define HFU_PAGE_IV_SIZE			16	// Size of optional IV prepended to firmware page
#define HFU_PAGE_SIZE				4096
// next group relate to the header in a received page from the Server
// Header = "CCCC 11111111 2222 33333333 4444 VV " - total 36 chars
// If the CRC (CCCC in above) and it's delimiter are removed, then the total length becomes 31
// If there is no subsequent payload, then there is no trailing space either, so the total length becomes 30.
#define HFU_PAGE_HEADER_SIZE					36	// this is for the 6 parameters
#define HFU_HEADER_ERASE_ADDRESS 				5	// offset of first byte to be CRC'd
#define HFU_HEADER_WITHOUT_CRC_LEN				(HFU_PAGE_HEADER_SIZE - HFU_HEADER_ERASE_ADDRESS)	// header without CRC or it's delimiter space
#define HFU_HEADER_WITHOUT_CRC_OR_PAYLOAD_LEN	(HFU_HEADER_WITHOUT_CRC_LEN - 1)	// header without CRC, it's delimiter, and the delimiter at the end of the header

#define HFU_HTTP_REQUEST_TIMEOUT	(usTICK_SECONDS * 70)	// time to wait for an HTTP response - server gateway has a timeout of 60 sec
#define HFU_RETRY_DELAY				(usTICK_SECONDS * 10)	// time to wait between retries to give the network an opportunity to recover

#define KEY_LENGTH 16
uint8_t key[KEY_LENGTH] = {0xa7, 0x1e, 0x56, 0x9f, 0x3e, 0xd4, 0x2a, 0x73, 0xcc, 0x41, 0x70, 0xbb, 0xf3, 0xd3, 0x4e, 0x69};
uint8_t key2[KEY_LENGTH];
uint8_t key2_order[KEY_LENGTH] = {4, 9, 10, 14, 12, 5, 13, 1, 2, 15, 0, 3, 6, 7, 11, 8};

#define	ERASE_PRESERVE_MASK	0x8000	// bit 15 set means erase only overwrites rather than erases

#define	PARAMETER_REGION	0xF0000000	// firmware that is targeted at this memory area is
										// actually parameters for the f/w image.
#define PARAMETER_REGION_SIZE	0x10000	// size of special region

extern QueueHandle_t xHTTPPostRequestMailbox;

extern	uint32_t		m_bank_a_start;
extern	uint32_t		m_bank_a_end;
extern	uint32_t		m_bank_b_start;

// module level variables
uint8_t HFU_received_page[HFU_HTTP_HEADER_MAX_SIZE + HFU_PAGE_IV_SIZE + HFU_PAGE_HEADER_SIZE + HFU_PAGE_SIZE];
char *HFU_URL = "hub.api.surehub.io";
char *HFU_resource = "/api/firmware";
char HFU_contents[128];	// this is the request string for the HTTP Post Request - must be persistent as accessed from other task.
extern PRODUCT_CONFIGURATION product_configuration;	// This is a RAM copy of the product info from Flash.
static BM_BANK current_bank;
static uint32_t program_address_offset;	// this is the address offset we add to the page address
										// to place the data in the correct bank
static bool retry_if_fail;	// controls behaviour if f/w update fails. Do we give up or try again?

#define MAX_NUM_FIRMWARE_BLOCKS	248	// this is 248 pages of 4K which is 0x8000 less than 1Mbyte.
uint32_t CRC_list[MAX_NUM_FIRMWARE_BLOCKS+1];	// list of CRCs for each f/w block
	
bool CRC_list_populated = false;
	
// private functions
void hfu_fetch_page(uint32_t page, int32_t *encrypted_data, uint32_t *bytes_read);
HFU_PROCESS_PAGE_RESULT hfu_process_page(uint32_t page, int32_t encrypted_data, uint32_t bytes_read);
BaseType_t hfu_program_page(uint32_t page);
void dump(uint8_t *addr, uint8_t lines);

/**************************************************************
 * Function Name   : HFU_Init
 * Description     : Initialise Hub Firmware Update
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void HFU_init(void)
{
	uint8_t i;
	uint8_t secret_serial_byte;
	uint8_t chars[3];
	uint32_t result;
	
	retry_if_fail = false;
	HFU_State = HFU_STATE_IDLE;
	xHFU_EventGroup = xEventGroupCreate();	
//	zprintf(CRITICAL_IMPORTANCE,"Key2: ");	
	// Generate key2 from re-ordered Secret Serial
	for (i=0; i<KEY_LENGTH; i++)
	{	// secret serial is stored in ASCII hex which complicates matters somewhat
		chars[0] = product_configuration.secret_serial[key2_order[i]*2];	// high byte
		chars[1] = product_configuration.secret_serial[(key2_order[i]*2)+1]; // low byte
		chars[2] = '\0';
		sscanf((char *)chars,"%02x",&result);
		key2[i] = (uint8_t)result;
//		zprintf(CRITICAL_IMPORTANCE,"%02X,",key2[i]);
	}
//	zprintf(CRITICAL_IMPORTANCE,"\r\n");	
}

/**************************************************************
 * Function Name   : HFU_trigger
 * Description     : Starts the Hub Firmware Update process
 *                 : by sending an event to the HFU task
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void HFU_trigger(bool retry)
{
//	zprintf(CRITICAL_IMPORTANCE,"Hub Firmware Update triggered\r\n");
	retry_if_fail = retry;
	xEventGroupSetBits(xHFU_EventGroup,HFU_TRIGGER_UPDATE);	
}

/**************************************************************
 * Function Name   : HFU_task
 * Description     : This is the task within with Hub Firmware Update
 *                 : operates.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void HFU_task(void *pvParameters)
{
    EventBits_t 				xEventBits;
	uint32_t					page_to_fetch;
	uint32_t					firmware_block;
	HFU_PROCESS_PAGE_RESULT		result;
	bool 						http_request_result;
	uint32_t					retry_count;
	uint32_t 					page_fetch_retry_count;
	uint32_t					pause_timestamp;
	int32_t						encrypted_data;	// value of x-enc HTTP header field on a firmware page
	uint32_t					bytes_read;		// how many bytes of firmware page were actually read
	
	while(1)
	{
		xEventBits = xEventGroupWaitBits(xHFU_EventGroup,
                                         HFU_TRIGGER_UPDATE,
                                         pdTRUE,pdFALSE,0);
        if( (xEventBits & HFU_TRIGGER_UPDATE) != 0 )
        {   // Event received requesting the start of a Hub Firmware Update
            if( HFU_STATE_IDLE == HFU_State )
			{	// state is currently idle, so we will advance the state machine
				HFU_State = HFU_STATE_START_UPDATE;
			}
        }
		
		switch( HFU_State )
		{
			case HFU_STATE_IDLE:	// do nothing. We get moved on from here by other means			
				break;
			case HFU_STATE_START_UPDATE:	// initialise variables etc
				page_to_fetch = 0;
				firmware_block = 0;
				page_fetch_retry_count = 0;
				retry_count = 0;	 // this is used to abort an update if the server stops responding
				CRC_list_populated = false;
				current_bank = BM_GetCurrentBank();
				if( BM_BANK_UNKONWN != current_bank )
				{
					BM_SetBankMark(BANK_MARK_UPDATING);	// prevents the bank accidentally being booted
					if( BM_BANK_A == current_bank )
					{	// we are in bank A, so data needs to be offset when written to 
						// flash to place it in bank B
						program_address_offset = (uint32_t)&m_bank_b_start - (uint32_t)&m_bank_a_start;
					} else
					{	// we are in bank B, so the data can be written at its native address
						program_address_offset = 0;
					}
					HFU_State = HFU_STATE_WAIT_NETWORK;
				}
				else
				{
					HFU_State = HFU_STATE_IDLE;					
					zprintf(CRITICAL_IMPORTANCE,"Cannot do firmware update as I don't know which bank to program\r\n");
				}
				break;
			case HFU_STATE_WAIT_NETWORK:
				if( true == NetworkInterface_IsActive() )
				{	// If Ethernet is alive, start to fetch pages
					zprintf(LOW_IMPORTANCE,"Network up, firmware fetch starting\r\n");
					HFU_State = HFU_STATE_FETCH_PAGE;
				}
				break;
			case HFU_STATE_FETCH_PAGE:		// Fetch the next page from the server
				hfu_fetch_page(page_to_fetch, &encrypted_data, &bytes_read);
				HFU_State = HFU_STATE_WAIT_PAGE_RECEIVED;
				break;
			case HFU_STATE_WAIT_PAGE_RECEIVED:	// The page has already been received actually.
				xTaskNotifyWait(0,ULONG_MAX,(uint32_t *)&http_request_result,0); // pick up the result
				if (http_request_result == true) 
					{	// if the call was successful, process the received page
//						zprintf(LOW_IMPORTANCE,"Received Hub firmware page\r\n");
						HFU_State = HFU_STATE_PROCESS_PAGE;
						retry_count = 0; // we got a page, so reset retry counter.
					} else // either no response, or a timeout, or a response that was false
					{
						zprintf(LOW_IMPORTANCE,"Failed to receive Hub firmware page\r\n");
						retry_count++;
						if( MAX_PAGE_FETCH_RETRYS <= retry_count )
						{	// give up
							zprintf(HIGH_IMPORTANCE,"Server not responding any more - aborting firmware update\r\n");
							if( false == retry_if_fail )
							{
								HFU_State = HFU_STATE_IDLE;
							} else
							{
								HFU_State = HFU_STATE_START_UPDATE;
							}
						} else
						{
							pause_timestamp = get_microseconds_tick();
							HFU_State = HFU_STATE_PAUSE;	// try again
						}
					}
				// else wait in this state
				break;
			case HFU_STATE_PAUSE:
				if( (get_microseconds_tick()-pause_timestamp) > HFU_RETRY_DELAY)
				{
					HFU_State = HFU_STATE_FETCH_PAGE;	// try again
				}
				break;
			case HFU_STATE_PROCESS_PAGE:		// Write the page to Flash
				result = hfu_process_page(firmware_block, encrypted_data, bytes_read);	// check and decrypt page
				switch( result )
				{
					case HFU_PAGE_OK:
						firmware_block++; // fall through, no break.
					case HFU_PAGE_RECEIVED_PARAMETER_BLOCK:					
//					zprintf(LOW_IMPORTANCE,"Page processed and programmed OK\r\n");
						page_to_fetch++;
						page_fetch_retry_count = 0;
						HFU_State = HFU_STATE_FETCH_PAGE;	// move on to next page					
						break;
					case HFU_PAGE_CRC_FAIL:	// probably page too short
					case HFU_PAGE_PROGRAM_FAILURE:	// programming error (verify fail?)
					case HFU_PAGE_EMPTY:
						page_fetch_retry_count++;
						if( MAX_PAGE_FETCH_RETRYS > page_fetch_retry_count)
						{
							zprintf(LOW_IMPORTANCE,"Retrying page fetch\r\n");
							HFU_State = HFU_STATE_FETCH_PAGE;	// try again
						} else	// retried too many times and received a duff page each time
						{
							if( false == retry_if_fail )
							{	// totally give up
								HFU_State = HFU_STATE_IDLE;
							} else
							{	// or restart the whole process again
								HFU_State = HFU_STATE_START_UPDATE;
							}
						}					
						break;
					case HFU_PAGE_FINAL:	
						//	zprintf(LOW_IMPORTANCE,"Final page received\r\n");
						HFU_State = HFU_STATE_FINISH;	// this is the last page, with no data
						break;
					case HFU_PAGE_PARAM_ERROR:	// any other error, e.g. params out of range						
					default:
						zprintf(LOW_IMPORTANCE,"Failed to process page - aborting\r\n");
						if( false == retry_if_fail )
						{
							HFU_State = HFU_STATE_IDLE;
						} else
						{
							HFU_State = HFU_STATE_START_UPDATE;
						}
						break;					
				}
				break;
			case HFU_STATE_FINISH:				// Flip bank switch and reset
				BM_ConfirmBank(true); // Also marks the bank as valid (so won't boot without this call)
				zprintf(LOW_IMPORTANCE,"Firmware update complete - restarting...\r\n");
				DbgConsole_Flush();  // Wait to allow uart output buffer to empty
				portENTER_CRITICAL();
				NVIC_SystemReset();			// reset and hopefully come up in the new bank
				portEXIT_CRITICAL();		// should never get here...
				HFU_State = HFU_STATE_IDLE;	// or here
				break;
			default:
				HFU_State = HFU_STATE_IDLE;	// should never get here
				break;				
		}	
		
		if( HFU_STATE_IDLE == HFU_State )
		{ // we can sleep here as we aren't doing anything
			vTaskDelay(pdMS_TO_TICKS( 100 ));   // give up CPU for 100ms
		}		
	}
}

/**************************************************************
 * Function Name   : hfu_fetch_page
 * Description     : Fetch a page of firmware from the server
 *                 : using HTTPS
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void hfu_fetch_page(uint32_t page, int32_t *encrypted_data, uint32_t *bytes_read)
{
	HTTP_POST_Request_params 	http_post_request_params;
	bool	 					notify_response;
	uint8_t 					*SharedSecret;
	uint64_t 					time_since_epoch_ms;
	DERIVED_KEY_SOURCE 			key;
	static	uint8_t				key_failures = 0;
	
    memset(HFU_contents, 0, sizeof(HFU_contents));	// not sure this is necessary
	
	get_UTC_ms(&time_since_epoch_ms);
		
	if( key_failures<=PAGE_FETCH_RETRIES_BEFORE_NEW_SECRET)
	{	// Send a 'normal' request
		zprintf(LOW_IMPORTANCE,"HFU: Using Derived Key from Flash\r\n");
		key = DERIVED_KEY_FLASH;	// use the Derived Key from before the power cycle / restart
	    snprintf(HFU_contents, sizeof(HFU_contents), \
			"serial_number=%s&mac_address=&product_id=%d&page=%d&bootloader_version=&bv=&tv=%llu"FIRMWARE_ENCRYPTION"\0",
			product_configuration.serial_number, DEVICE_TYPE_HUB, page, time_since_epoch_ms);		
	} else
	{ // Need to send the shared secret just in case we are out of synch.
		SharedSecret = GetSharedSecret();	// pointer to 16 byte array
		zprintf(LOW_IMPORTANCE,"HFU: Using Derived Key from RAM\r\n");
		key = DERIVED_KEY_CURRENT;	// there is no old key, so use the current key	
	    snprintf(HFU_contents, sizeof(HFU_contents), \
			"serial_number=%s&mac_address=&product_id=%d&page=%d&bootloader_version=&bv=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x&tv=%llu"FIRMWARE_ENCRYPTION"\0",
			product_configuration.serial_number, DEVICE_TYPE_HUB, page, \
			SharedSecret[0],SharedSecret[1],SharedSecret[2],SharedSecret[3], \
			SharedSecret[4],SharedSecret[5],SharedSecret[6],SharedSecret[7], \
			SharedSecret[8],SharedSecret[9],SharedSecret[10],SharedSecret[11], \
			SharedSecret[12],SharedSecret[13],SharedSecret[14],SharedSecret[15], \
			time_since_epoch_ms);			
	}
	
//	zprintf(CRITICAL_IMPORTANCE,"HFU request : %s\r\n",HFU_contents);
	
	zprintf(LOW_IMPORTANCE,"Requesting page %d from server\r\n",page);
	
	http_post_request_params.URL = HFU_URL;
	http_post_request_params.resource = HFU_resource;
	http_post_request_params.contents = HFU_contents;
	http_post_request_params.response_buffer = (char*)HFU_received_page;
	http_post_request_params.response_size = sizeof(HFU_received_page);
	http_post_request_params.encrypted_data = encrypted_data;
	http_post_request_params.xClientTaskHandle = xTaskGetCurrentTaskHandle();
	http_post_request_params.tx_key_source = key;
	http_post_request_params.rx_key_source = key;	
	http_post_request_params.bytes_read = bytes_read;	
	// ask HTTP_Post_Request task to get the data and notify us when it has the answer
//	xTaskNotifyWait(0,0,(uint32_t *)&notify_response,0);	// clear any pending notifications
	
	xQueueSend(xHTTPPostRequestMailbox, &http_post_request_params, 0);	
	// If this fails, the HFU state machine will try again.	
	
	xTaskNotifyWait(ULONG_MAX,0,(uint32_t *)&notify_response,portMAX_DELAY);	// wait for the response
	// Note that the response remains in the notification as we read it out in HFU_Task() 
	// in the next state as well.
	
	// If the HTTP post succeeded (we got a correctly signed response), we can write the RAM key to Flash as we just sent
	// it to the Server. Note that the response may not contain any firmware, or duff firmware which will be detected later.
	if( true == notify_response)
	{	// signatures checked out
		key_failures = 0;	// so no need to send a new key
		if( DERIVED_KEY_CURRENT == key)	
		{ // if we just sent a new key, store it.
			StoreDerivedKey();
		}
	}
	else
	{ // something went wrong with the http response, probably bad keys. But could be a gateway timeout or no signatures at all.
		key_failures++;
	}
	
}

/**************************************************************
 * Function Name   : hfu_process_page
 * Description     : Take a received page from the Server, decode and
 *                 : decrypt it and validate it.
 * Inputs          :
 * Outputs         : 
 * Returns         : member of of HFU_PROCESS_PAGE_RESULT
 **************************************************************/
HFU_PROCESS_PAGE_RESULT hfu_process_page(uint32_t firmware_block, int32_t encrypted_data, uint32_t bytes_read)
{
	uint32_t 	crc_sent, erase_addr, erase_count, prog_addr, prog_count, ver;
	uint32_t 	crc_length;
	uint16_t 	crc;
	uint32_t 	data_size;
	uint8_t		key_pos;
	uint32_t	data_pos;
	uint32_t	calc_crc;
	bool 		res;
	int i;
	
	// HTTP Header already trimmed off.
	uint8_t *data_start = HFU_received_page;
	
	if( NULL == data_start )
	{
		zprintf(LOW_IMPORTANCE,"HTTP header with no firmware received\r\n");
		return HFU_PAGE_EMPTY;		
	}

	// Now decrypt the page if required
	switch( encrypted_data)
	{
		case REPROG_CRYPT_XOR_AES:
			zprintf(LOW_IMPORTANCE,"Hub firmware page encrypted with type 1\r\n");
			// Decrypt the firmware page (including it's header) using AES
			wc_AesCbcDecryptWithKey(data_start + HFU_PAGE_IV_SIZE, data_start + HFU_PAGE_IV_SIZE, \
									bytes_read - HFU_PAGE_IV_SIZE, GetDerivedKey(), \
									DERIVED_KEY_LENGTH, data_start);
			data_start += HFU_PAGE_IV_SIZE;
			break;
		case REPROG_CRYPT_NOT_SPECIFIED:
			zprintf(LOW_IMPORTANCE,"Hub firmware page encryption not specified\r\n");
			break;			
		case REPROG_CRYPT_XOR:
			zprintf(LOW_IMPORTANCE,"Hub firmware page encrypted with type 0\r\n");
			break;
		default:
			zprintf(LOW_IMPORTANCE,"Hub firmware page encrypted with unsupported encryption\r\n");
			return HFU_PAGE_PARAM_ERROR;	// might as well abort as the Server is sending us data we cannot process
			break;
	}
		
	// parse the header
    sscanf((char *)data_start, "%x %x %x %x %x %x", (uint32_t *)&crc_sent, (uint32_t *)&erase_addr, \
													(uint32_t *)&erase_count, (uint32_t *)&prog_addr, \
													(uint32_t *)&prog_count, (uint32_t *)&ver);
	
	zprintf(LOW_IMPORTANCE,"Received page, crc=%04x erase_addr=%08x erase_count=%08x prog_addr=%08x prog_count=%04x ver=%02x\r\n",
								crc_sent,erase_addr,erase_count,prog_addr,prog_count,ver);
	
	
	if( (ver & 0xff) != 0x01 )
	{
		zprintf(LOW_IMPORTANCE,"Device Firmware page from Server is not for Hub target...r\r\n");
	}

	if( (erase_addr==0) && (erase_count==0) && (prog_addr==0) && (prog_count==0) )
	{		
		return HFU_PAGE_FINAL;	
	}

	if( 0x00000000 != erase_addr )	// erase_addr == 0 means do not erase anything
	{
		if( (erase_addr < (uint32_t)&m_bank_a_start) || 
			((erase_addr + (erase_count * HFU_PAGE_SIZE)-1) > (uint32_t)&m_bank_a_end) )
		{
			zprintf(HIGH_IMPORTANCE,"Attempt to erase from out-of-bank address 0x%08X\r\n",erase_addr);
			return HFU_PAGE_PARAM_ERROR;
		}
	}

	//Now to decrypt the data part. Two stage process.
	if( 0 != prog_count )
	{	// only decrypt if there is data to decrypt!
		key_pos = 0;
		for( data_pos=0; data_pos<prog_count; data_pos++ )
		{
			data_start[HFU_PAGE_HEADER_SIZE+data_pos] ^= key2[key_pos];
			data_start[HFU_PAGE_HEADER_SIZE+data_pos] ^= key[key_pos];
			key_pos = (KEY_LENGTH - 1 > key_pos) ? key_pos + 1 : 0;	// increment and wrap key_pos
		}
	}

	// Handle the CRC. The first four bytes of the page are the CRC
	data_size = prog_count;
	crc_length = data_size ? (HFU_HEADER_WITHOUT_CRC_LEN + data_size) : HFU_HEADER_WITHOUT_CRC_OR_PAYLOAD_LEN;
//	zprintf(LOW_IMPORTANCE,"Calculating CRC, start=%08x len=%08x\r\n",data_start+HFU_HEADER_ERASE_ADDRESS, crc_length);
	crc = CRC16(data_start+HFU_HEADER_ERASE_ADDRESS, crc_length, 0xcccc);
	
	if( crc != (crc_sent&0xffff) )
	{
		zprintf(LOW_IMPORTANCE,"CRC fail on received page from server\r\n");
		return HFU_PAGE_CRC_FAIL;
	}

	if( PARAMETER_REGION == prog_addr )
	{	// This is parameter data that should not be programmed
		// Ought to have a CRC in the data so it can be verified
		memcpy(CRC_list,data_start+HFU_PAGE_HEADER_SIZE,sizeof(CRC_list));
		CRC_list_populated = true;
	} else if( (prog_addr < (uint32_t)&m_bank_a_start) || 
	    ((prog_addr + (prog_count-1)) > (uint32_t)&m_bank_a_end) )
	{
		zprintf(HIGH_IMPORTANCE,"Attempt to program to out-of-bank address 0x%08X\r\n",prog_addr);
		return HFU_PAGE_PARAM_ERROR;
	}	
	
	if( 0x00000000 != erase_addr )
	{	// now perform erase functionality
		if( (erase_count & ERASE_PRESERVE_MASK) != 0 )
		{ 
			zprintf(HIGH_IMPORTANCE,"Overwrite is not supported\r\n");
		}
		
		erase_count &= ~ERASE_PRESERVE_MASK;	// clear PRESERVE bit
		erase_count *= HFU_PAGE_SIZE;	// convert number of pages to number of bytes
		erase_addr += program_address_offset;	// ensure we erase from the 'other' bank
		
//		zprintf(LOW_IMPORTANCE,"Erasing 0x%04x bytes from address 0x%08X\r\n",erase_count,erase_addr);
		res = hermesFlashRequestErase((uint8_t *)erase_addr, erase_count, true);	// wait for completion
		if( false == res )
			zprintf(HIGH_IMPORTANCE,"Erase fail\r\n");

        DCACHE_InvalidateByRange(erase_addr, HFU_PAGE_SIZE);		
	}
	
	if( PARAMETER_REGION == prog_addr )
	{	// we have done any erasing that was requested as part of the same block, so finish here.
		return HFU_PAGE_RECEIVED_PARAMETER_BLOCK;
	}
		
	zprintf(LOW_IMPORTANCE,"Programming 0x%04x bytes to address 0x%08X\r\n",prog_count,prog_addr);

	// Note we do not need to adjust prog_addr to match the destination bank because BM_EnscribeData() does it for us.
	res = BM_EnscribeData(data_start+HFU_PAGE_HEADER_SIZE, prog_addr, prog_count);
    DCACHE_InvalidateByRange(prog_addr, HFU_PAGE_SIZE);
	
	if( (true == res) && (true == CRC_list_populated) )
	{	// calculate the cumulative CRC over this block and verify it against the list
		// We would prefer to use the DCP as it has a CRC32 function. However, it has a fixed initial value of
		// 0xffffffff, whereas we want an arbitrary value
		calc_crc = CRC32(data_start+HFU_PAGE_HEADER_SIZE, prog_count, CRC_list[firmware_block]);
//		zprintf(CRITICAL_IMPORTANCE,"Page %d Expected CRC = 0x%08x, Calculated CRC = 0x%08x\r\n",firmware_block,CRC_list[firmware_block+1],calc_crc);
		if( calc_crc != CRC_list[firmware_block+1])
		{
			zprintf(LOW_IMPORTANCE,"CRC fail for page %d\r\n",firmware_block);
			res = false;
		}		
	}
	
	if( false == res)
	{
		zprintf(HIGH_IMPORTANCE,"Flash write or verify failure\r\n");
		return HFU_PAGE_PROGRAM_FAILURE;
	} else
	{
		return HFU_PAGE_OK;
	}
}

void dump(uint8_t *addr, uint8_t lines)
{
#define LINELEN 16
  uint8_t i,j,c;
  zprintf(CRITICAL_IMPORTANCE,"\r\n");
  for (i=0; i<lines; i++) // should be i<16 for entire eeprom contents
  {
	zprintf(CRITICAL_IMPORTANCE,"%08X: ",addr+(i*LINELEN));
    for (j=0; j<LINELEN; j++)
    {
  		zprintf(CRITICAL_IMPORTANCE,"%02X ",*(addr+(i*LINELEN)+j));
    }
    for (j=0; j<LINELEN; j++)
    {
        c = *(addr+(i*LINELEN)+j);
        if ((c>31) && (c<127))
  			zprintf(CRITICAL_IMPORTANCE,"%c",c);
        else
 			zprintf(CRITICAL_IMPORTANCE,".");
    }
  	zprintf(CRITICAL_IMPORTANCE,"\r\n");
	DbgConsole_Flush();
  }
}
