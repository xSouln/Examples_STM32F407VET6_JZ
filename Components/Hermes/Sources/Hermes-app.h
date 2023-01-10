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
* Filename: Hermes-app.h
* Author:   Chris Cowdery 22/07/2019
* Purpose:
*
* This is the main Hermes application. It exists as a FreeRTOS task
* and transfers data between the Wireless interface(s) and the Ethernet
* interface.
*
**************************************************************************/

#ifndef __HERMES_APP_H__
#define __HERMES_APP_H__

#include "leds.h"

#define UPLOAD_HUB_REGISTERS_ON_CLOUD_CONNECT	// Ash said this was a good idea at 09:31 on 26th August 2020.
// [09:31] Ashleigh Hopkins
//    I think it's good if it does
// [09:32] Ashleigh Hopkins
//    There are some circumstances where we don't request it for whatever reason

#define SIGNATURE_UPDATE_PERIOD		(3600*6)	// 6 hours
#define RF_COMMS_TIMEOUT	60	// seconds

#define APP_SNTP_UPDATE_INTERVAL	(15 * 60 * 1000)	// In milliseconds. How often to refresh SNTP.
#define APP_SNTP_RETRY_INTERVAL		(1000)				// In milliseconds. How often to retry SNTP request if it fails.

#define BUTTON_PRESSED	0
#define READ_BUTTON() 0

typedef enum
{
	CONN_STATUS_STARTING		= (1<<0),
	CONN_STATUS_NETWORK_UP		= (1<<1),
	CONN_STATUS_MQTT_UP			= (1<<2),
	CONN_STATUS_FULL_CONNECTION	= CONN_STATUS_NETWORK_UP | CONN_STATUS_MQTT_UP,
} CONNECTION_STATUS_EVENT_BITS;

typedef enum	// These are events sent from various parts of the system to the App
{				// so it can update the LEDs accordingly
	STATUS_GETTING_CREDENTIALS,		// Attempting to connect to hub.api.sureflap.io
	STATUS_CONNECTING_TO_CLOUD,		// Attempting to connect to AWS
	STATUS_CONNECTED_TO_CLOUD,		// Connected to AWS
	STATUS_PAIRING_MODE_ENABLED,	// Pairing mode entered either via register write or button
	STATUS_PAIRING_MODE_DISABLED,	// Pairing mode exited
	STATUS_PAIRING_SUCCESS,			// New device paired
	STATUS_DISCONNECT_ALL,			// 10sec push on button. All devices removed from pairing table
	STATUS_RF_OFFLINE,				// No RF contact with devices for a short period
	STATUS_RF_ONLINE,				// RF contact with devices
	STATUS_DISPLAY_SUCCESS,			// Command from Server to indicate animal activity
	STATUS_FIRMWARE_UPDATING,		// Used when the Server forces an immediate f/w update at boot time
	STATUS_BLOCKING_TEST_GOOD,
	STATUS_BLOCKING_TEST_BAD,
	STATUS_BLOCKING_TEST_OFF,
	STATUS_BACKSTOP,
} SYSTEM_STATUS_EVENTS;

void hermes_app_init(void);
void hermes_app_task(void *pvParameters);
void trigger_send_hub_version_info(void);
void set_led_brightness(LED_MODE value, bool store);
void process_system_event(SYSTEM_STATUS_EVENTS event);
void store_led_brightness(LED_MODE bright_level);
uint32_t readHwVersion(void);
void update_led_view(void);
LED_MODE get_led_brightness(void);

#endif


