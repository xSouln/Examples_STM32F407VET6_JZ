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
* Filename: Device_Buffer.c    
* Author:   Chris Cowdery 9/10/2019
* Purpose:  Handles messages for Devices
*             
*             
*
**************************************************************************/
//
// This set of functions provide a buffer for messages being sent to Devices
// Messages come in as a T_MESSAGE with a destination MAC address.
// They are stored, with a timestamp.
// The next message for a device can be retrieved via it's MAC address.
// It is always possible to store messages.
// There is a maximum number of messages that will be stored for a single Device,
// attempting to exceed this will overwrite the oldest.
// If the buffer becomes full, the first
// strategy is to search for the oldest message for that MAC address, which will
// be overwritten. If there are no messages for the MAC address, then the oldest
// message will be overwritten.
// The buffer is an array of entries, each one large enough for a full size T_MESSAGE.
// This is a bit wasteful, however it allows easy overwriting of old entries.
// We deem an entry to be empty if the MAC address and timestamp are both zero.

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
#include "Device_Buffer.h"

// Local #defines
#define DEVICE_BUFFER_SIZE  64          // Maximum number of messages in the buffer.
#define MAX_MESSAGES_FOR_SAME_DEVICE  8 // maximum messages allowed into the buffer for the same Device

typedef struct
{
    uint32_t timestamp;
    uint64_t mac_address;
    T_MESSAGE message;
} DEVICE_BUFFER;

// Local variables

DEVICE_BUFFER device_buffer[DEVICE_BUFFER_SIZE];    // make us a nice buffer to hold messages

/**************************************************************
 * Function Name   : device_buffer_init
 * Description     : Initialise our device buffer
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void device_buffer_init(void)
{
    memset(device_buffer,0,sizeof(device_buffer));    // clear our buffer;
}

/**************************************************************
 * Function Name   : device_message_is_new()
 * Description     : Scans the Device Buffer looking for a matched message
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
bool device_message_is_new(uint64_t mac, T_MESSAGE *message)
{
    uint32_t i;
    uint32_t j;
    uint8_t *pMessage1,*pMessage2;

    pMessage1 = (uint8_t *)message;

    for (i=0; i<DEVICE_BUFFER_SIZE; i++)
    {
        if (device_buffer[i].mac_address == mac)
        {   // device matches, so now compare message
            pMessage2 = (uint8_t *)&device_buffer[i].message;

            for (j=0; j<message->length; j++)
            {
                if (pMessage1[j]!=pMessage2[j])
                    break;  // if we have a mismatch, break out of this for() loop.
            }
            if (j==message->length)
                return false;    // if we got to the end of the for loop, all characters must match, so it's a duplicate
        }
    }
    return true;
}

/**************************************************************
 * Function Name   : device_buffer_add()
 * Description     : Store a message in the device buffer.
 *                 : Deals with a full buffer by overwriting oldest entry
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void device_buffer_add(uint64_t mac, T_MESSAGE *message)
{
    uint32_t i;
    uint32_t oldest_index;
    uint32_t age, oldest_age;       // difference between now and timestamp
    uint32_t same_destination;      // count of messages in the buffer that are already going to the same destination

	same_destination=0;
    for (i=0; i<DEVICE_BUFFER_SIZE; i++)
    {   // count how many messages we have that are already intended for this destination
        if (device_buffer[i].mac_address == mac)
            same_destination++;
    }

    // Need to find a place to store the message. First strategy is to just look for a free slot
    i=0;
    while((i<DEVICE_BUFFER_SIZE) && ((device_buffer[i].timestamp!=0) || (device_buffer[i].mac_address!=0)))
    {
        i++;
    }   //

    if ((i<DEVICE_BUFFER_SIZE) && (same_destination<MAX_MESSAGES_FOR_SAME_DEVICE))
    {   // found a slot
        device_buffer[i].mac_address = mac;
        device_buffer[i].timestamp = get_microseconds_tick();
        memcpy(&device_buffer[i].message,message,sizeof(T_MESSAGE));
    }
    else
    {   // couldn't find a slot, so need a new strategy.
        // First attempt is to find the oldest entry using the current mac address and overwrite that one.
        oldest_index=0xffffffff;    // invalid
        oldest_age=0;
        for (i=0; i<DEVICE_BUFFER_SIZE; i++)
        {
            if (device_buffer[i].mac_address == mac)
            {   // message is for same destination
                age = get_microseconds_tick()-device_buffer[i].timestamp;
                if (age>oldest_age) // this one is the oldest so far
                {
                    oldest_index=i; // so store the details
                    oldest_age = age;
                }
            }
        }
        if (oldest_index<0xffffffff)
        {   // we found one, so overwrite it
            device_buffer[oldest_index].mac_address = mac;
            device_buffer[oldest_index].timestamp = get_microseconds_tick();
            memcpy(&device_buffer[oldest_index].message,message,sizeof(T_MESSAGE));
        }
        else
        {   // we didn't find one with a matching mac, so now we are on our final
            // strategy which is just to overwrite the oldest one
            oldest_index=0xffffffff;    // invalid
            oldest_age=0;
            for (i=0; i<DEVICE_BUFFER_SIZE; i++)
            {
                age = get_microseconds_tick()-device_buffer[i].timestamp;
                if (age>oldest_age) // this one is the oldest so far
                {
                    oldest_index=i; // so store the details
                    oldest_age = age;
                }
            }
            if (oldest_index<0xffffffff)
            {   // we found one, so overwrite it
                device_buffer[oldest_index].mac_address = mac;
                device_buffer[oldest_index].timestamp = get_microseconds_tick();
                memcpy(&device_buffer[oldest_index].message,message,sizeof(T_MESSAGE));
            }
            else
            {
                zprintf(HIGH_IMPORTANCE, "INTERNAL INABILITY TO STORE MESSAGE FOR DEVICE!!!! - SHOULD NEVER HAPPEN!!!\r\n");
            }
        }
    }
    return;
}

/**************************************************************
 * Function Name   : device_buffer_get_next()
 * Description     : Retrieves a pointer to the next message for a device
 *                 : Does not clear the buffer
 * Inputs          :
 * Outputs         :
 * Returns         : NULL if there is no message
 **************************************************************/
T_MESSAGE *device_buffer_get_next(uint64_t mac_address, uint32_t *index)
{
    uint32_t i;
    uint32_t oldest_age;
    uint32_t oldest_index;
    uint32_t age;
    T_MESSAGE *retval = NULL;

    oldest_index=0xffffffff;    // invalid
    oldest_age=0;
    for (i=0; i<DEVICE_BUFFER_SIZE; i++)
    {
        if (device_buffer[i].mac_address == mac_address)
        {   // message is for same destination
            age = get_microseconds_tick()-device_buffer[i].timestamp;
            if (age>oldest_age) // this one is the oldest so far
            {
                oldest_index=i; // so store the details
                oldest_age = age;
            }
        }
    }
    if (oldest_index!=0xffffffff)
    { // found an entry
        retval = &device_buffer[oldest_index].message;
    }

    *index = oldest_index;  // also return which item this is in the buffer so it can be cleared later on.
    return retval;
}

/**************************************************************
 * Function Name   : device_buffer_clear_entry()
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void device_buffer_clear_entry(uint32_t index)
{
    if (index<DEVICE_BUFFER_SIZE)   // test to ensure we don't splat memory or cause a fault if index is wrong!
    {
        memset(&device_buffer[index],0,sizeof(DEVICE_BUFFER));
    }
}

/**************************************************************
 * Function Name   : device_buffer_dump
 * Description     : Outputs the device buffer to the terminal
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void device_buffer_dump(void)
{
    uint32_t 	i, j;
    char*		message_body;
    zprintf(CRITICAL_IMPORTANCE, "idx device_mac    timestamp     message\r\n");
    for( i = 0; i < DEVICE_BUFFER_SIZE; i++ )
    {
 		if ((device_buffer[i].timestamp!=0) && (device_buffer[i].mac_address!=0))
		{
        message_body = (char*)&device_buffer[i].message;
        zprintf(CRITICAL_IMPORTANCE, "%02d ", i);
        zprintf(CRITICAL_IMPORTANCE, "%08x%08x ", (uint32_t)((device_buffer[i].mac_address&0xffffffff00000000)>>32), (uint32_t)(device_buffer[i].mac_address&0xffffffff));
			zprintf(CRITICAL_IMPORTANCE, "%04x ",device_buffer[i].timestamp);
        for( j = 0; j < device_buffer[i].message.length; j++ )
        {
            zprintf(CRITICAL_IMPORTANCE, "%02x ", message_body[j]);
        }
        zprintf(CRITICAL_IMPORTANCE, "\r\n");
			DbgConsole_Flush();
		}
    }
}


// Here are the hooks into SureNet to pass messages into it for transmission to Devices
/**************************************************************
* Function Name   : surenet_get_next_message_cb
* Description     : Called by SureNet when it wants the next message to send to mac
*                 : mac is the mac address of the destination
*                 : current_request_ptr is a pointer to a T_MESSAGE to send
*                 : current_request_handle is a pointer to a message handle, which is returned in surenet_clear_message_cb
*                 : so we know which message has been successfully sent.
*                 : of the message to be used as a parameter to surenet_clear_request_cb()
*                 : to tell the host code that SureNet has processed that message.
* Inputs          :
* Outputs         :
* Returns         : true if there is a message to go (and parameters have been set), false if no message
 **************************************************************/
bool surenet_get_next_message_cb(uint64_t mac, T_MESSAGE **current_request_ptr, uint32_t *current_request_handle)
{
	T_MESSAGE *message;
	uint32_t handle;
	message = device_buffer_get_next(mac, &handle);    // get a pointer to the oldest message for this device

	if (message!=NULL)
	{
		*current_request_ptr = message;
		*current_request_handle = handle;
		return true;
	}
	return false;   // no new message at the moment.
}

/**************************************************************
 * Function Name   : surenet_clear_message_cb
 * Description     : Marks a message_for_device as invalid, i.e. consumed by SureNet
 * Inputs          : index of message, as set when calling surenet_get_next_message_cb()
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_clear_message_cb(uint32_t current_request_handle)
{
	device_buffer_clear_entry(current_request_handle);
    return;
}


