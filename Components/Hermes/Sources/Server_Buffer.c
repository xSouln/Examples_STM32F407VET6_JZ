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
* Filename: Server_Buffer.c
* Author:   Chris Cowdery 09/10/2019
* Purpose:  Manages a buffer for server-bound messages.
*
**************************************************************************/
//
// This set of functions provide a buffer for messages being sent to the Server
// Messages for the server are text strings of variable length, in two parts:
// - sub-topic (e.g. "messages/" for the Hub or "messages/MAC/" for a Device.
// - message body. This is a text string like "120 45 18 31"
// Note there is also a base topic, but this is per-hub, set at connection time, and only
// needs to be retained within the MQTT handler. We don't care about it.
// Our data is stored in one of two areas, depending on size:
// - Array of fixed size entries (for small messages)
// - Malloc'd area for large messages
// We have an array of pointers to each message, so the pointer can point to
// either an entry in the array, or a malloc'd area.
// Each entry in the buffer is a SERVER_BUFFER_ENTRY.
// If the buffer becomes full, then a message will not be queued, and the sending
// function has to deal with retries / NACKs.
// When a message is taken from the buffer for transmission, an 8 character 'seconds
// since Epoch' is prepended, and a 4 character message index / retry count.
// If MESSAGE_RETRY_INTERVAL seconds have elapsed since a message was last sent, and it hasn't been
// cleared, it is resent.
// If a message has been sent MAX_RETRIES times, it gets dropped anyway.
// Dropping a message is notified to the MQTT system as it may wish to take action, such as
// reconnect or resubscribe to that topic if it believes the Server is connected.
// Retrieving the next message logic:
// - Scan buffer looking for messages for which MESSAGE_RETRY_INTERVAL has elapsed.
// - If found, send it.
// - Otherwise find the oldest message that has not been sent yet and send that and
// - update the last_sent_timestamp and retries values.
// Dropping a message (either MAX_RETRIES being exceeded or an acknowledge from the server)
// - Message can be identified by it's index and content.
// - Buffer has to be marked free
// Adding a message to the buffer logic:
// - Locate a free slot. If there isn't one, fail
// - Add message to the end of the queue.
// - Update the buffer_entry_timestamp
// Reflected messages are used to 'free' a buffer entry.
// - Check the message index of the incoming message. Note that the retries count must be removed
// -  as the incoming message may be the reflection of a previous attempt to send.
// Also note that the message index should NOT simply be the position in the buffer. If it was, it
// means that a late reflection could erroneously 'free' a new entry if the buffer was full.

#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "hermes-time.h"
#include "SureNet-Interface.h"
#include "Server_Buffer.h"
#include "MQTT.h"   // to allow error notifications to be passed into MQTT stack in case it wants to take remedial action
#include "Backoff.h"

// Local #defines
#define SERVER_BUFFER_ENTRIES					32  // 128 messages is 4 for each of 32 devices
#define MAX_SERVER_MESSAGE_SIZE					MAX_MESSAGE_SIZE_SERVER_BUFFER // bit of a guess, but at least we can truncate at this length and dump an error message
#define SERVER_MESSAGE_ARRAY_STORAGE_THRESHOLD  192  // messages larger than this will be stored with pvPortMalloc() / pvPortFree()
                                                     //  Tom says that a typical Thalamus message is about 160 bytes
#define MESSAGE_STORED_VIA_MALLOC   0xffff
#define MESSAGE_STORED_STATICALLY	0xfffe
#define MAX_MESSAGE_RETRIES         5       // a message will be sent this many times at MESSAGE_RETRY_INTERVALS
#define MESSAGE_RETRY_INTERVAL      10     // interval between message retries
#define MAX_MESSAGE_ALARM           3       // Send a warning to MQTT to say we are concerned about this topic

#define BASE_MESSAGE_RETRY_MS		40000
#define MESSAGE_RETRY_MULTIPLIER	2
#define MESSAGE_RETRY_JITTER_MAX	5000

typedef struct
{
    bool free;
    uint8_t message[SERVER_MESSAGE_ARRAY_STORAGE_THRESHOLD];
} SERVER_BUFFER_MESSAGE;

typedef struct
{
    uint32_t	buffer_entry_timestamp;	// UTC when this message entered the buffer
    BACKOFF		backoff;				// Details required for retries and backing off publish attempts.
    // note index is a magic number - setting it to 0 marks this entry as free
    uint8_t 	index;      			// ID of this message, given when it entered the buffer. Used for removing messages when reflection is received from server
    uint8_t*	message;                // pointer to actual text message string within SERVER_BUFFER_MESSAGE or via pvPortMalloc
    uint16_t	message_storage;        // where the message is stored, array index (small messages) or
										// MESSAGE_STORED_VIA_MALLOC for pvPortMalloc (large messages)
    uint64_t	source_mac;				// most economical way of storing the topic. Will be NULL for Hub sourced messages
} SERVER_BUFFER_ENTRY;

extern bool global_message_trace_flag;

// Local variables
SERVER_BUFFER_ENTRY		server_buffer[SERVER_BUFFER_ENTRIES];       // make us a nice buffer to hold message entries
SERVER_BUFFER_MESSAGE	server_buffer_message[SERVER_BUFFER_ENTRIES];     // store for actual server messages ~ 24K
uint8_t 				next_message_index = 1;   // must never be zero
MQTT_MESSAGE			output_mqtt_message;
const BACKOFF_SPECS		publish_backoff_specs = {BASE_MESSAGE_RETRY_MS, MESSAGE_RETRY_MULTIPLIER, MESSAGE_RETRY_JITTER_MAX, MAX_MESSAGE_RETRIES};

// Local functions
void server_buffer_message_drop(uint32_t i);

/**************************************************************
 * Function Name   : server_buffer_init
 * Description     : Initialise our server buffer
 **************************************************************/
void server_buffer_init(void)
{
    uint32_t i;
	memset(server_buffer, 0, sizeof(server_buffer));	// explicitly zero these two here so they don't have to be in
	memset(server_buffer_message, 0, sizeof(server_buffer_message));	// the .bss section
    for( i = 0; i < SERVER_BUFFER_ENTRIES; i++ )
    {
        server_buffer_message[i].free = true; // overwrites the initialisation at startup time.
    }
}

/**************************************************************
 * Function Name   : server_buffer_add
 * Description     : Add a message to our buffer. MUST BE NULL TERMINATED
 *                 : Note it copies the message to an internal buffer, so the caller can
 *                 : free their memory on return from this function
 **************************************************************/
bool server_buffer_add(SERVER_MESSAGE* message)   // Add this message and topic
{
    uint32_t i, j;
    uint32_t oldest_time;
    uint32_t oldest_index;
    uint32_t len;
    // first find the next free space in our buffer
    i = 0;
    while( (i < SERVER_BUFFER_ENTRIES) && (server_buffer[i].index != 0) )
    {
        i++;
    }
	if( true == global_message_trace_flag )
	{
		zprintf(CRITICAL_IMPORTANCE,"Message from %08x%08x for server: %s\r\n",
			(uint32_t)(((message->source_mac)>>32) & 0xffffffff),
			(uint32_t)((message->source_mac) & 0xffffffff),
			message->message_ptr);
		DbgConsole_Flush();
	}
    if( i == SERVER_BUFFER_ENTRIES )
    {
        return false;	// bail out as we are full
    }
    server_buffer[i].buffer_entry_timestamp = get_UTC();
    server_buffer[i].backoff.specs = &publish_backoff_specs;
	Backoff_Reset(&server_buffer[i].backoff);
    server_buffer[i].index = next_message_index++;
    if( next_message_index == 0 ) next_message_index++;    // must never be zero
    server_buffer[i].source_mac = message->source_mac;  // most economical way of storing the topic. Will be NULL for Hub sourced messages

    // now deal with copying message to relevant place
    // first find the length of the string, up to MAX_SERVER_MESSAGE_SIZE
    len = strlen((char*)message->message_ptr) + 1;	// +1 to include terminating zero
    if( len < SERVER_MESSAGE_ARRAY_STORAGE_THRESHOLD )
    {
        // we are going to store the message text in server_buffer_message[]
        // Find a free slot
        j=0;
        while( (server_buffer_message[j].free == false) && (j < SERVER_BUFFER_ENTRIES) )
        {
            j++;
        }
        if( j == SERVER_BUFFER_ENTRIES )
        {
            sbuffer_printf("server_buffer_add() ERROR 1 - CANNOT ALLOCATE STORAGE\r\n");
			server_buffer[i].index = 0;	// mark this entry as free as we couldn't use it in the end
            return false;
        }
        // Now set up storage references in server_buffer
        server_buffer[i].message = server_buffer_message[j].message;    // set buffer to point to where message is stored
        server_buffer[i].message_storage = j;   // note index in server_buffer_message[] used to store message
        server_buffer_message[j].free = false;    // mark entry as used
        // and finally copy message
        strcpy((char*)server_buffer[i].message,(char*)message->message_ptr);
    } else
    { // message too large, so have to MALLOC some space to put it in.
        if( len < MAX_SERVER_MESSAGE_SIZE )
        {
            server_buffer[i].message = pvPortMalloc(len);
            if( server_buffer[i].message != NULL )
            {   // if we could allocate some storage, copy message into it
                strcpy((char*)server_buffer[i].message, (char*)message->message_ptr);
                server_buffer[i].message_storage = MESSAGE_STORED_VIA_MALLOC;   // Note that we have stored via MALLOC
            } else
            {
                sbuffer_printf("server_buffer_add() ERROR 2 - CANNOT ALLOCATE STORAGE\r\n");
				server_buffer[i].index = 0;	// mark this entry as free as we couldn't use it in the end
                return false;
            }
        } else
        {
			sbuffer_printf("server_buffer_add() ERROR - MESSAGE TOO LARGE - dropping message\r\n");
			server_buffer[i].index = 0;	// mark this entry as free as we couldn't use it in the end
            return true;	// we return true here otherwise we'll probably get
							// retries of the same message!
        }
    }
	return true;
}

bool server_buffer_construct_message(MQTT_MESSAGE* message, uint32_t index)
{	// Make header of form 0x0iir where i is index, r is retries.
	uint16_t header = (server_buffer[index].index << 4) + server_buffer[index].backoff.retries;
	// Message is of form: tttttttt hhhh m... where t is Timestamp, h is header, and m... is the actual message.
	uint32_t message_length = snprintf(message->message, sizeof(message->message), "%08x %04x %s\0",
									   server_buffer[index].buffer_entry_timestamp, header, server_buffer[index].message);

	if( message_length > sizeof(message->message) )
	{
		sbuffer_printf("server_buffer_construct_message() error: Buffered message too long to fit in MQTT message.\r\n");
		return false; // Message is too long! Someone fucked up.
	}

	if( server_buffer[index].source_mac != 0 ) // Only want to set the topic if we can successfully construct the message.
	{	// If source is a child device, MAC address is appended as sub-topic. Include '/' so we don' have to worry about it later.
		// Gotta swap MAC around!
		uint32_t	i, pos = 1;
		uint8_t*	data = (uint8_t*)&server_buffer[index].source_mac;
		message->subtopic[0] = '/';

		for( i = 0; i < sizeof(server_buffer[index].source_mac); i++ )
		{
			pos += sprintf(&message->subtopic[pos], "%02X", data[i]);
		}
	}
//	zprintf(LOW_IMPORTANCE,"sending to Server %s\r\n",message->message);
	return true;
}

/**************************************************************
 * Function Name   : server_buffer_get_next_message
 * Description     : Generates and returns a pointer to the next message.
 *                 : The message remains in the buffer, and it's retry counter
 *                 : is incremented.
 *                 : Also, if the maximum number of retries has been exceeded,
 *                 : then the message is dropped.
 * Returns         : NULL if there are no messages to retrieve
 **************************************************************/
MQTT_MESSAGE* server_buffer_get_next_message(void)
{
    uint32_t i;

	memset(output_mqtt_message.subtopic, '\0', sizeof(output_mqtt_message.subtopic)); // Null topic means Hub base topic "messages".

    // Need to find next message to send.
	// Backoff arranges sucessive transmissions to be spaced at increasing intervals,
	// to avoid network clashes. If too many retries are attempted, MQTT task
	// wants to take emergency action (QoS0 message). In the worst case, a message
	// is dropped; we may want to take more drastic action (such as reconnection).
	BACKOFF_RESULT	backoff_result;
    for( i = 0; i < SERVER_BUFFER_ENTRIES; i++ )
	{
		if( 0 != server_buffer[i].index )
		{
			backoff_result = Backoff_GetStatus(&server_buffer[i].backoff);
			switch( backoff_result )
			{
				case BACKOFF_FINAL_ATTEMPT:
					if( false == server_buffer_construct_message(&output_mqtt_message, i) )
					{	// Message could not be constructed, so drop it.
						server_buffer_message_drop(i);
						break;
					}
					MQTT_Alarm(MQTT_MESSAGE_FAILED_A_FEW_TIMES, &output_mqtt_message); // Notify MQTT that we might have a problem. MQTT will attempt a QoS0 transmission.
					Backoff_Progress(&server_buffer[i].backoff);
					break;

				case BACKOFF_READY:
					if( false == server_buffer_construct_message(&output_mqtt_message, i) )
					{	// Message could not be constructed, so drop it.
						server_buffer_message_drop(i);
						break;
					}
					Backoff_Progress(&server_buffer[i].backoff);
					return &output_mqtt_message; // Found a message reayd to go, and constructed properly, so return it.
				case BACKOFF_WAITING:	// Not resolved yet, but waiting longer to resend.
					break;

				case BACKOFF_FAILED:
					server_buffer_construct_message(&output_mqtt_message, i);
					MQTT_Alarm(MQTT_MESSAGE_FAILED_TOO_MANY_TIMES, &output_mqtt_message); // Notify MQTT that we have a problem, and are going to kill the message
					server_buffer_message_drop(i);
					break;
			}
		}
	}

	return NULL; // Didn't find any messages that can be sent.
}

/**************************************************************
 * Function Name   : server_buffer_message_drop
 * Description     : Kill a message in the buffer. Free any memory
 **************************************************************/
void server_buffer_message_drop(uint32_t i)
{
    if( server_buffer[i].message_storage == MESSAGE_STORED_VIA_MALLOC )
    {
        if( server_buffer[i].message != NULL )
        {
            vPortFree(server_buffer[i].message);   // Free the buffer
			server_buffer[i].message = NULL;	// mark the buffer as free to prevent it being freed again
        }
    } else if( server_buffer[i].message_storage < SERVER_BUFFER_ENTRIES )
	{	// mark the entry in the server_buffer_message buffer free
		server_buffer_message[server_buffer[i].message_storage].free = true;
	}
    server_buffer[i].index = 0;   // mark this entry as empty
}

/**************************************************************
 * Function Name   : server_buffer_process_reflected_message
 * Description     : Use a reflected message to remove an entry from the buffer
 *                 : This is the usual process in which buffer entries are removed.
 * Returns         : true if a reflected message has been handled
 **************************************************************/
bool server_buffer_process_reflected_message(char *message)
{   // messages are of the form utc hdr blah blah blah
    char*		pos;
    uint32_t	hdr;
    uint8_t		index;
    uint32_t	i;

    pos = strchr(message,' ');   // find next space,

    if( pos != NULL )
    {
        pos++;  // pos should now point to HDR which is a 4 digit hex number.
        sscanf(pos, "%x", &hdr); // extract HDR
        index = (hdr >> 4) & 0xff;    // extract index from hdr
        i = 0;
        while( ((server_buffer[i].index != index) || (server_buffer[i].index == 0)) \
				&& (i < SERVER_BUFFER_ENTRIES) )
        {   // search for the message with a matching index
            i++;
        }
        if( i < SERVER_BUFFER_ENTRIES )
        {   // found the message, so kill it
            server_buffer_message_drop(i);
            return true;
        }   // else it might have already been killed and this is an out of date reflection
    }
    return false;
}

/**************************************************************
 * Function Name   : server_buffer_dump
 * Description     : Dump out active buffer entries
 **************************************************************/
void server_buffer_dump()  // dump valid messages out of the UART
{
    uint32_t i;

    zprintf(HIGH_IMPORTANCE,"pos timestamp lastsent delay    retries index storage_type src_mac          message\r\n");
    for( i = 0; i < SERVER_BUFFER_ENTRIES; i++ )
    {
        if( server_buffer[i].index > 0 )
        {
            zprintf(HIGH_IMPORTANCE,"%03d %08X  %08X %08X %03d     %03d   %04x         %08X%08X ", i, server_buffer[i].buffer_entry_timestamp, \
                                                                 server_buffer[i].backoff.last_retry_tick, \
																 server_buffer[i].backoff.retry_delay_ms, \
                                                                 server_buffer[i].backoff.retries, \
                                                                 server_buffer[i].index, \
                                                                 server_buffer[i].message_storage, \
                                                                 (uint32_t)((server_buffer[i].source_mac&0xffffffff00000000)>>32), \
                                                                 (uint32_t)(server_buffer[i].source_mac&0xffffffff  ));
            zprintf(HIGH_IMPORTANCE,"%s\r\n",server_buffer[i].message);
			DbgConsole_Flush();
        }
    }
}
