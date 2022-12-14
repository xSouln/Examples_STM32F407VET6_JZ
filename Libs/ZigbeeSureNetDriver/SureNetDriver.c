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
* Filename: SureNetDriver.c
* Author:   Chris Cowdery 
* 
* SureNet Driver top level file - effectively wraps Atmel Stack.
* It provides the following facilities:
* 1. Association with Devices
* 2. Sending and receiving messages to / from devices.
* We access the SPI via a FreeRTOS API, but the nRST, INT and SP signals are controlled directly
*           
**************************************************************************/
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>      // isxdigit()

// SureNet
#include "mac_api.h"
#include "tal.h"
#include "bmm.h"
#include "app_config.h"

#include "SureNetDriver.h"
//==============================================================================
// Note that beacon_request_device_data.xx is not actually used, it is just stored. A hook could be added
// to pass it out higher up the stack if a need was found for the information.
BEACON_REQUEST_DEVICE_DATA beacon_request_device_data =
{
	.mac_addr=0,
	.device_channel=0,
	.device_platform=INVALID_DEVICE
};
//------------------------------------------------------------------------------


//==============================================================================
// Private functions
static bool assign_new_short_addr(uint64_t addr64, uint16_t *addr16);

// Private variables
static uint8_t current_channel;	// this is the master reference.
static uint8_t current_channel_page = 0;

static associated_device_t device_list[MAX_NUMBER_OF_DEVICES];
// Private defines
/** Defines the short address of the coordinator. */
#define COORD_SHORT_ADDR                MAC_NO_SHORT_ADDR_VALUE // forces usage of long MAC always

//local copy of pan_id
uint16_t pan_id;

PAIRING_MODE_TIMEOUT pairing_mode_timeout = { 0, false };

#define PAIRING_MODE_TIME					(90 * usTICK_SECONDS)
#define	PAIRING_MODE_TIME_BEACON_REQUEST 	(10 * usTICK_SECONDS)

// ------------------------------0-------------------10--------15--------20----------26
int8_t TX_Power_Per_Channel[] = {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,-8};

// This task is the SureNet Driver task. All activity relating to the Atmel AT86RF233 IC is handled in this task except
// that ISR's aren't, instead they are handled in rfisr_task and communicated with this one via Notifications
// Note that this function is called by sn_task() i.e. from the higher levels of the stack, and
// is also called during transmit_packet().



/* EOF */

