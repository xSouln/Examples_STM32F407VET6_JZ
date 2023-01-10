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
* Filename: Hermes-app.c
* Author:   Chris Cowdery 22/07/2019
* Purpose:
*
* This is the main Hermes application. It exists as a FreeRTOS task
* and transfers data between the Wireless interface(s) and the Ethernet
* interface.
*
**************************************************************************/
#include "Components.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* FreeRTOS+IP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "queue.h"

/*Other includes*/
#include "hermes-app.h"
#include "hermes-time.h"

// SureNet includes
#include "Surenet-Interface.h"
#include "DeviceFirmwareUpdate.h"

// Application includes
#include "Device_Buffer.h"
#include "Server_Buffer.h"
#include "RegisterMap.h"
#include "SNTP.h"
#include "MQTT.h"
#include "message_parser.h"
#include "leds.h"
#include "BuildNumber.h"
#include "timer_interface.h"
#include "HTTP_Helper.h"

// externs
extern QueueHandle_t xNvStoreMailboxSend;

// global variables
uint8_t uptime_min_count = 0;
uint8_t bootRN = 0; // Random number assigned on boot. Supposed to be different every time to allow
					// the server to distinguish when the hub has restarted. when not 0, has been initialised
bool trigger_send_version_info = false;	// flag set when MSG_HUB_VERSION_INFO is received to trigger the sending of MSG_HUB_VERSION_INFO
uint32_t blocking_test_per = 0;		// %age packet loss
bool blocking_test_active = false;	// becomes true on reception of first PACKET_BLOCKING_TEST
									// and remains true for 5 mins after last packet.
									// Then reverts back to false so normal operation can resume
uint32_t blocking_test_failure_start;

bool trigger_delayed_pairing_mode_disabled_event = false;
uint32_t trigger_delayed_pairing_mode_disabled_event_timestamp;

// local functions
static void send_hub_report(void);
static void HermesApp_MaintainConnection(void);
static void button_handler(void);
void update_led_view(void);
void surenet_blocking_test_watchdog();
// Mailboxes for inter task communication
QueueHandle_t		xIncomingMQTTMessageMailbox;
QueueHandle_t		xOutgoingMQTTMessageMailbox;
QueueHandle_t		xBufferMessageMailbox;
QueueHandle_t		xSystemStatusMailbox;
EventGroupHandle_t	xConnectionStatus_EventGroup;

void hermes_app_init(void)
{
    // set up our mailboxes
    xIncomingMQTTMessageMailbox 	= xQueueCreate(INCOMING_MQTT_MESSAGE_QUEUE_DEPTH_SMALL, sizeof(MQTT_MESSAGE));
    xOutgoingMQTTMessageMailbox 	= xQueueCreate(1, sizeof(MQTT_MESSAGE));
	xBufferMessageMailbox			= xQueueCreate(1, sizeof(SERVER_MESSAGE));
	xSystemStatusMailbox			= xQueueCreate(5, sizeof(SYSTEM_STATUS_EVENTS));
	xConnectionStatus_EventGroup	= xEventGroupCreate();
}

// We put these here to ensure that we don't have to allocate enough space in the
// hermes_app_task() to provide for them
static MQTT_MESSAGE mqtt_message;
// This is the main Hermes Application task
void hermes_app_task(void *pvParameters)
{
    MQTT_MESSAGE* 	outgoing_message			= NULL;
	SERVER_MESSAGE	server_message;
	SYSTEM_STATUS_EVENTS 	system_event;
	uint32_t				last_heard;
	uint32_t				last_heard_check_time		= get_microseconds_tick();
	bool					rf_online					= false;
	bool					mqtt_message_pending		= false;
	bool 					blocking_test_ok 			= true;
	uint32_t				signature_change_timestamp;
	uint32_t random_value;


    device_buffer_init();  // initialise buffer for messages to go to Devices
    server_buffer_init();
    HubReg_Init();	// This sets up the Device Table part of the register map.
	DFU_init();

	do	// set BootRN to a non zero random value
	{
		HAL_RNG_GenerateRandomNumber(&hrng, &random_value);

		bootRN = (uint8_t)random_value;
						// Note this is not the same as Hub1. In Hub1, BootRN gets set to the low byte of
						// current time when MQTT subscription is successful.
	} while(0 == bootRN);

	process_system_event(STATUS_RF_OFFLINE);	// set up LED status to reflect initial value of rf_online variable

	signature_change_timestamp = get_UpTime();

    while(1)
    {
        Surenet_Interface_Handler();    // this polls the SureNet mailboxes and events, and calls back
                                        // in this context with any results.
                                        // The SureNet callbacks are all weakly linked stubs, and are
                                        // in Surenet-Interface.c
                                        // We need to call this as often as we can to minimise delays between
                                        // devices requesting data and us providing it. Note there are some
                                        // timing hard-stops in the devices as they will go to sleep if
                                        // they don't hear from the Hub.

                                        // Furthermore, the devices remain awake waiting for the reply, so
                                        // the faster we can reply, the longer their batteries will last.

		if( true == SNTP_IsTimeValid() ){ DFU_Handler(); }	// process any firmware update activity

		HubReg_Check_Full();	// Check for any registers whose values need to be sent to the Server

		HermesApp_MaintainConnection();	// Do connection maintenance like LED states, hub report, and SNTP.

		// check for messages arriving from MQTT interface

		if( (uxQueueMessagesWaiting(xIncomingMQTTMessageMailbox) > 0) &&
		    (pdPASS == xQueueReceive(xIncomingMQTTMessageMailbox,&mqtt_message,0)) )
        {
            process_MQTT_message_from_server(mqtt_message.message, mqtt_message.subtopic);
        }

		// if there is a message waiting from the MQTT Connect function, the grab it and mark it as pending
		if( (uxQueueMessagesWaiting(xBufferMessageMailbox) > 0) &&
		    (pdPASS == xQueueReceive(xBufferMessageMailbox, &server_message, 0)) )
		{
			mqtt_message_pending = true;
		}

		// If there is a pending message, then attempt to pass it into the server buffer.
		// If it doesn't go, don't clear the pending flag, and it will try again next time around.
		if( true == mqtt_message_pending )
		{
			if( true == server_buffer_add(&server_message) )
			{
				mqtt_message_pending = false;
			}
		}

		if( (uxQueueMessagesWaiting(xSystemStatusMailbox) > 0) &&
		    (pdPASS == xQueueReceive(xSystemStatusMailbox, &system_event, 0)) )
		{
			process_system_event(system_event);	// something has happened in the system, so
		}										// update LEDs

        if( uxQueueSpacesAvailable(xOutgoingMQTTMessageMailbox) != 0 ) // Space in the mailbox.
        {
			if( NULL == outgoing_message )
			{	// See if there's a message to send.
				outgoing_message = server_buffer_get_next_message();
			}

			if( NULL != outgoing_message )
			{
				xQueueSend(xOutgoingMQTTMessageMailbox, outgoing_message, 0); // Note this will copy the full size of the structure
																			  // which could be much more than the message.
				outgoing_message = NULL;
			}
        }

		button_handler();

		// Check status of RF comms every 5 seconds, and issue a system event if it's changed.
		if( (get_microseconds_tick() - last_heard_check_time) > 5*usTICK_SECONDS )
		{
			last_heard = surenet_get_last_heard_from();
			if (last_heard > (RF_COMMS_TIMEOUT * usTICK_SECONDS))
			{
				if( true == rf_online )
				{	// just gone offline
					rf_online = false;
					process_system_event(STATUS_RF_OFFLINE);
				}
			} else
			{
				if( false == rf_online )
				{	// just come online
					rf_online = true;
					process_system_event(STATUS_RF_ONLINE);
				}
			}
			last_heard_check_time = get_microseconds_tick();
		}

		if( true == blocking_test_active)
		{
			surenet_blocking_test_watchdog();
			if( blocking_test_per > 10)
			{	// instantaneous PER indicates a failure
				if( true ==  blocking_test_ok)
				{	// we were previously OK.
					process_system_event(STATUS_BLOCKING_TEST_BAD);	// Change LEDs to Red to indicate a problem
					blocking_test_failure_start = get_microseconds_tick();		// note when we turned the LEDs red.
					blocking_test_ok = false;	// flip state flag to indicate that we have an error condition
				}
			}
			else
			{	// instantaneous PER is OK.
				if( (false ==  blocking_test_ok) && ((get_microseconds_tick()-blocking_test_failure_start) > (2*usTICK_SECONDS)))
				{	// we were previously indicating an error
					process_system_event(STATUS_BLOCKING_TEST_GOOD);
					blocking_test_ok = true;
				}
			}

			//Print out the new PER value whenever it changes.
			static uint32_t blocking_test_per_prev = 0xFFFFFFFF;
			if (blocking_test_per != blocking_test_per_prev)
			{
				zprintf(CRITICAL_IMPORTANCE,"PER = %d%%\r\n",blocking_test_per);
				blocking_test_per_prev = blocking_test_per;
			}
		}
		// This is a fudge to hold off the pairing mode disabled event. The reason is that the Server
		// for unknown reasons sends a spurious STATUS_DISPLAY_SUCCESS event some seconds after pairing
		// has completed successfully and the LEDs have gone dim. This holdoff keeps the LEDs flashing
		// brightly, and somewhat masking the spurious LED flash. It mimics the behaviour of Hub1 which
		// does the same, although unintentionally in that case!
		if( (true == trigger_delayed_pairing_mode_disabled_event) &&
			((get_microseconds_tick()-trigger_delayed_pairing_mode_disabled_event_timestamp)>(7*usTICK_SECONDS)))
		{
			trigger_delayed_pairing_mode_disabled_event = false;
			process_system_event(STATUS_PAIRING_MODE_DISABLED);
		}

		if( (get_UpTime() - signature_change_timestamp) > SIGNATURE_UPDATE_PERIOD)
		{
			signature_change_timestamp = get_UpTime();
			zprintf(LOW_IMPORTANCE,"Sending new shared secret to the Server\r\n");
			send_shared_secret();
		}

		vTaskDelay(pdMS_TO_TICKS( 1 ));
    }
}

static void HermesApp_MaintainConnection(void)
{
	static Timer		sntp_update_timer	= EMPTY_TIMER;
	static EventBits_t	LastConnStatus		= 0;
	EventBits_t			ConnStatus			= xEventGroupGetBits(xConnectionStatus_EventGroup);

	if( ConnStatus != LastConnStatus )
	{	// Check for a change, and decide what to do about it.
		if( CONN_STATUS_FULL_CONNECTION == (ConnStatus & CONN_STATUS_FULL_CONNECTION) )
		{
			//process_system_event(STATUS_CONNECTED_TO_CLOUD); //Moved to MQTT.c
#ifdef UPLOAD_HUB_REGISTERS_ON_CLOUD_CONNECT
//			zprintf(CRITICAL_IMPORTANCE,"Uploading Hub register map...\r\n");
			HubReg_Refresh_All();
#endif
		} else if( CONN_STATUS_NETWORK_UP == (ConnStatus & CONN_STATUS_NETWORK_UP) )
		{
			//process_system_event(STATUS_CONNECTING_TO_CLOUD); //Moved to MQTT.c
		} else if( CONN_STATUS_STARTING == (ConnStatus & CONN_STATUS_STARTING) )
		{
			//process_system_event(STATUS_GETTING_CREDENTIALS); //Moved to MQTT.c
		} else
		{
			// ignore whatever the event was
		}

		LastConnStatus = ConnStatus;
	}

	if( CONN_STATUS_NETWORK_UP == (ConnStatus & CONN_STATUS_NETWORK_UP) )
	{
		if( (true == has_timer_expired(&sntp_update_timer)) ||
		    (true == SNTP_DidUpdateFail()) )
		{
			SNTP_AwaitUpdate(true, 0);	// Make the request, but don't wait around.
			countdown_ms(&sntp_update_timer, APP_SNTP_UPDATE_INTERVAL);
		}

		send_hub_report();	// Sends a once-per-hour update to the server
	}
}

/**************************************************************
 * Function Name   : trigger_send_hub_version_info
 * Description     : Trigger sending of hub version info next time send_hub_report() is called
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void trigger_send_hub_version_info(void)
{
	trigger_send_version_info = true;
}

/**************************************************************
 * Function Name   : send_hub_report
 * Description     : Sends a MSG_HUB_UPTIME message once per hour
 *                 : Also sends the Hub version number if global variable send_version_info == SEND_VERSION_NUMBER
 * Inputs          : and store in the fixed buffer register_set
 * Outputs         :
 * Returns         :
 **************************************************************/
extern uint8_t hub_debug_mode;	// If bit 1 is set, hub status updates sent every minute rather than every hour
static void send_hub_report(void)
{
	uint8_t msg[48] = {0};
	static uint32_t last_message_time = 0;
	static uint32_t mins = 0;
	HERMES_TIME_GMT gmt_time;  //hold hours mins and secs GMT
	SERVER_MESSAGE message;
	static bool			send_time_to_server = false;

	message.source_mac = 0;	// it's from the hub
	message.message_ptr = msg;

	get_gmt(get_UTC(), &gmt_time);	// convert UTC to time

	if(!last_message_time)
	{
	  	last_message_time = get_microseconds_tick();
	}

	if((trigger_send_version_info==true)&&(bootRN!=0))  //when not 0 has been initialised
	{
		uint8_t ver[4];
		ver[3] = SVN_REVISION & 0xff;
		ver[2] = (SVN_REVISION>>8) & 0xff;
		ver[1] = BUILD_MARK & 0xff;
		ver[0] = (BUILD_MARK>>8) & 0xff;
		sprintf((char *)&msg[0], "%d 0 5 %02x %02x %02x %02x %02x", MSG_HUB_VERSION_INFO, (unsigned int)bootRN, (unsigned int)ver[3], (unsigned int)ver[2], (unsigned int)ver[1], (unsigned int)ver[0]);

	   	if( true == server_buffer_add(&message))   // Add this message and topic
		{	// message successfully queued, so clear flag requesting it.
        trigger_send_version_info = false;
   }
   }

   if( (get_microseconds_tick() - last_message_time) >= usTICK_MINUTE )
   {
       last_message_time = get_microseconds_tick();
       mins++;
       uptime_min_count++;
       if( (hub_debug_mode & HUB_SEND_HUB_STATUS_UPDATES_EVERY_MINUTE) || (uptime_min_count >= 60) )
       {
			if( uptime_min_count >= 60 ) uptime_min_count = 0;
			send_time_to_server = true;
	   }
	}

	if( true == send_time_to_server)	// send the time message to the server. Note this might be a retry
	{
		sprintf((char *)&msg[0], "%d %08d %02d %02d %02d %02d %08x %x", MSG_HUB_UPTIME, \
				(uint32_t)mins, gmt_time.day, gmt_time.hour, gmt_time.minute, \
				gmt_time.second, get_UTC(), 0);	// last parameter was xively_restarts, is now 0
		if( true == server_buffer_add(&message))
		{
			send_time_to_server = false; // message successfully sent.
       }
   }
}

/**************************************************************
 * Function Name   : surenet_ping_response_cb
 * Description     : Called when a ping response is received from a device.
 *                 : Triggers a message to the Server
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_ping_response_cb(PING_STATS *ping_result)
{
	uint8_t msg[64] = {0};
	static uint16_t count = 0;
	SERVER_MESSAGE message;
   	uint32_t hub_rssi, device_rssi;

	message.source_mac = 0;	// it's from the hub
	message.message_ptr = msg;

	count++;
	hub_rssi = (uint32_t)((float)ping_result->hub_rssi_sum/(float)ping_result->num_good_pings);
	device_rssi = (uint32_t)((float)ping_result->device_rssi_sum/(float)ping_result->num_good_pings);
	sprintf((char *)&msg[0], "%d %02x %02x %02x %08x %08x %04x %04x %04x %04x %04x", MSG_HUB_PING, \
	   		ping_result->ping_res[0], ping_result->ping_res[1], ping_result->ping_res[2], \
	   		(ping_result->reply_timestamp)-(ping_result->transmission_timestamp), 0, \
			ping_result->num_good_pings, ping_result->num_bad_pings, \
			hub_rssi, device_rssi, ping_result->ping_value);
	if( false == server_buffer_add(&message))
	{
		zprintf(HIGH_IMPORTANCE,"Ping response not queued for Server - buffer full\r\n");
	}
}

/**************************************************************
 * Function Name   : surenet_pairing_mode_change_cb
 * Description     : Callback from RF stack when pairing mode has changed
 *				   : Updates hub registers so Server gets notified accordingly
 **************************************************************/
void surenet_pairing_mode_change_cb(PAIRING_REQUEST new_state)
{
	if( true != new_state.enable )
	{
		trigger_delayed_pairing_mode_disabled_event = true;
		trigger_delayed_pairing_mode_disabled_event_timestamp = get_microseconds_tick();
		HubReg_SetPairingMode(new_state);	// always tell server we have exited pairing mode
	} else
	{
		switch (new_state.source)
		{
			case PAIRING_REQUEST_SOURCE_SERVER:
			case PAIRING_REQUEST_SOURCE_BUTTON:
			case PAIRING_REQUEST_SOURCE_CLI:
				process_system_event(STATUS_PAIRING_MODE_ENABLED);
				HubReg_SetPairingMode(new_state);	// change register value and notify server
				break;
		}
	}
}

/**************************************************************
 * Function Name   : button_handler
 * Description     : Scans GPIO pin for button, and handles user intent.
 *                 : If the button is held for between 3 & 10 seconds before being released,
 *                 : then the Hub goes into pairing mode. If it's held for more than 10 seconds
 *                 : then the Hub clears it's pairing table/
 **************************************************************/
typedef enum
{
	BUTTON_HANDLER_IDLE,
	BUTTON_HANDLER_START_PRESSED_TIMER,
	BUTTON_HANDLER_PRESSED,
	BUTTON_HANDLER_WAIT_PAIRING,
	BUTTON_HANDLER_START_RELEASE_TIMER,
	BUTTON_HANDLER_WAIT_RELEASE,
} BUTTON_HANDLER_STATE;

void button_handler(void)
{
	static uint32_t 			button_timer 			= 0u;
	static BUTTON_HANDLER_STATE	button_handler_state 	= BUTTON_HANDLER_IDLE;
	static PAIRING_REQUEST 		request					= {0,true,PAIRING_REQUEST_SOURCE_BUTTON};

	switch(button_handler_state)
	{
    	case BUTTON_HANDLER_IDLE:	// wait for the button to be pressed
			if( READ_BUTTON() == BUTTON_PRESSED)
			{
				button_handler_state = BUTTON_HANDLER_START_PRESSED_TIMER;
			}
			break;

		case BUTTON_HANDLER_START_PRESSED_TIMER:	// start the pressed timer
			button_timer = get_microseconds_tick();
			button_handler_state = BUTTON_HANDLER_PRESSED;
			break;  // do nothing

		case BUTTON_HANDLER_PRESSED:	// wait for 125milliseconds
			if( READ_BUTTON() != BUTTON_PRESSED)
			{
				button_handler_state = BUTTON_HANDLER_IDLE;
			} else
			{
				if((get_microseconds_tick()-button_timer) >= (usTICK_MILLISECONDS*125))  //125ms
				{
					button_handler_state = BUTTON_HANDLER_WAIT_PAIRING;
				}
    		}
			break;

		case BUTTON_HANDLER_WAIT_PAIRING:	// set pairing mode and wait for either button release or 10 seconds
			if (READ_BUTTON() != BUTTON_PRESSED)
			{
        		surenet_set_hub_pairing_mode(request);
				button_handler_state = BUTTON_HANDLER_START_RELEASE_TIMER;
			} else
    		{
				if((get_microseconds_tick()-button_timer) >= (usTICK_SECONDS*10))
				{
					surenet_hub_clear_pairing_table();	// unpair all devices
					button_handler_state = BUTTON_HANDLER_START_RELEASE_TIMER;
				}
    		}
			break;

		case BUTTON_HANDLER_START_RELEASE_TIMER:	// start timer for button release
			button_timer = get_microseconds_tick();
			button_handler_state = BUTTON_HANDLER_WAIT_RELEASE;
			break;

		case BUTTON_HANDLER_WAIT_RELEASE:	// wait for button to be released
		default:
			if (READ_BUTTON() != BUTTON_PRESSED)  //ensure button is released for at least a second before doing anything else to avoid e.g. pairing mode inadvertently following clear pairing table
			{
				if((get_microseconds_tick()-button_timer) >= usTICK_SECONDS)
				{
					button_handler_state = BUTTON_HANDLER_IDLE;
				}
			} else
			{
				button_timer = get_microseconds_tick();
			}
			break;
	}
}

/////////////////////////// LED display decisions are made here /////////////////
/* There are four levels of priority:
 * Highest level = Success indications, double green or single orange flashes
 * Level 2       = Pairing status, flashing green when in pairing mode
 * Level 3       = Connection status, alternate red when connecting to SureFlap
 *                 and alternate green when connecting to the cloud
 * Lowest level  = Basic display when nothing is happening, solid green.
 * Each level may have an entry in it, or it may be clear. The prioritiser
 * looks at the levels from the top, and the highest priority level with an
 * entry is what gets displayed.
 *
 * Messages come in from the system and are sent here as a SYSTEM_STATUS_EVENTS.
 * these are used to look up the data to populate the appropriate priority level.
 *
 * Note that the Success Indications are handled slightly differently - they use
 * a facility with the LED driver to have a temporary pattern that overrides the
 * current one. So they fall outside the priority system, and are sent to the
 * LED driver immediately (with a timeout)
 */

typedef enum
{
	LEVEL_BASIC,
	LEVEL_RF_CONNECTION,
	LEVEL_NETWORK_CONNECTION,
	LEVEL_PAIRING,
	LEVEL_FORCED_FW_UPDATE,
	LEVEL_TEST,
	LEVEL_BACKSTOP,
} PRIORITY_LEVEL;

typedef struct
{
	bool				active;
	LED_COLOUR			colour;
	LED_PATTERN_TYPE	pattern;
	uint32_t			duration;
} LED_VIEW;

LED_VIEW led_view[LEVEL_BACKSTOP] =  {  { true, COLOUR_GREEN, LED_PATTERN_SOLID , 0}, // should always be true!
										{ false, COLOUR_RED, LED_PATTERN_SOLID , 0},
										{ false, COLOUR_RED, LED_PATTERN_SOLID , 0},
										{ false, COLOUR_RED, LED_PATTERN_SOLID , 0},
										{ false, COLOUR_RED, LED_PATTERN_SOLID , 0},
										{ false, COLOUR_RED, LED_PATTERN_SOLID , 0}};
LED_MODE brightness = LED_MODE_DIM; // This is the system brightness level

/**************************************************************
 * Function Name   : update_led_view
 * Description     : Examines the priority list of LED states and
 *                 : issues a command to the LED driver accordingly
 *                 : Note that all events are full brightness apart from
 *                 : the background green which is controlled by the system
 *                 : brightness setting.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void update_led_view(void)
{
	if( true == led_view[LEVEL_TEST].active)
	{
		LED_Request_Pattern(LED_MODE_NORMAL, led_view[LEVEL_TEST].colour, \
			led_view[LEVEL_TEST].pattern, led_view[LEVEL_TEST].duration);
	} else if( true == led_view[LEVEL_FORCED_FW_UPDATE].active)
	{
		LED_Request_Pattern(LED_MODE_NORMAL, led_view[LEVEL_FORCED_FW_UPDATE].colour, \
			led_view[LEVEL_FORCED_FW_UPDATE].pattern, led_view[LEVEL_FORCED_FW_UPDATE].duration);
	} else if( true == led_view[LEVEL_PAIRING].active)
	{
		LED_Request_Pattern(LED_MODE_NORMAL, led_view[LEVEL_PAIRING].colour, \
			led_view[LEVEL_PAIRING].pattern, led_view[LEVEL_PAIRING].duration);
	} else if( true == led_view[LEVEL_NETWORK_CONNECTION].active)
	{
		LED_Request_Pattern(LED_MODE_NORMAL, led_view[LEVEL_NETWORK_CONNECTION].colour, \
			led_view[LEVEL_NETWORK_CONNECTION].pattern, led_view[LEVEL_NETWORK_CONNECTION].duration);
	} else if( true == led_view[LEVEL_RF_CONNECTION].active)
	{
		LED_Request_Pattern(LED_MODE_NORMAL, led_view[LEVEL_RF_CONNECTION].colour, \
			led_view[LEVEL_RF_CONNECTION].pattern, led_view[LEVEL_RF_CONNECTION].duration);
	} else	// we assume that the basic level is 'live'
	LED_Request_Pattern(brightness, led_view[LEVEL_BASIC].colour, \
		led_view[LEVEL_BASIC].pattern, led_view[LEVEL_BASIC].duration);
}

/**************************************************************
 * Function Name   : process_system_event
 * Description     : Handles events coming in from the system and uses
 *                 : the information to set up the LEDs.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
const char *event_strings[] = { "STATUS_GETTING_CREDENTIALS",
								"STATUS_CONNECTING_TO_CLOUD",
								"STATUS_CONNECTED_TO_CLOUD",
								"STATUS_PAIRING_MODE_ENABLED",
								"STATUS_PAIRING_MODE_DISABLED",
								"STATUS_PAIRING_SUCCESS",
								"STATUS_DISCONNECT_ALL",
								"STATUS_RF_OFFLINE",
								"STATUS_RF_ONLINE",
								"STATUS_DISPLAY_SUCCESS",
								"STATUS_FIRMWARE_UPDATING",
								"STATUS_BLOCKING_TEST_GOOD",
								"STATUS_BLOCKING_TEST_BAD",
								"STATUS_BLOCKING_TEST_OFF",
								"STATUS_BACKSTOP"};

void process_system_event(SYSTEM_STATUS_EVENTS event)
{
	zprintf(LOW_IMPORTANCE,"process_system_event() received event %s\r\n",event_strings[event]);
	switch (event)
	{
		case STATUS_BLOCKING_TEST_GOOD:
			led_view[LEVEL_TEST].active 	= true;
			led_view[LEVEL_TEST].colour 	= COLOUR_GREEN;
			led_view[LEVEL_TEST].pattern 	= LED_PATTERN_VFAST_FLASH;
			led_view[LEVEL_TEST].duration 	= 0;
			break;
		case STATUS_BLOCKING_TEST_BAD:
			led_view[LEVEL_TEST].active 	= true;
			led_view[LEVEL_TEST].colour 	= COLOUR_RED;
			led_view[LEVEL_TEST].pattern 	= LED_PATTERN_VFAST_FLASH;
			led_view[LEVEL_TEST].duration 	= 0;
			break;
		case STATUS_BLOCKING_TEST_OFF:
			led_view[LEVEL_TEST].active 	= false;
			led_view[LEVEL_TEST].colour 	= COLOUR_RED;
			led_view[LEVEL_TEST].pattern 	= LED_PATTERN_ALTERNATE;
			led_view[LEVEL_TEST].duration 	= 0;
			break;
		case STATUS_GETTING_CREDENTIALS:	// Attempting to connect to hub.api.sureflap.io
			led_view[LEVEL_NETWORK_CONNECTION].active 	= true;
			led_view[LEVEL_NETWORK_CONNECTION].colour 	= COLOUR_RED;
			led_view[LEVEL_NETWORK_CONNECTION].pattern 	= LED_PATTERN_ALTERNATE;
			led_view[LEVEL_NETWORK_CONNECTION].duration 	= 0;
			break;
		case STATUS_CONNECTING_TO_CLOUD:	// Attempting to connect to AWS
			led_view[LEVEL_NETWORK_CONNECTION].active 	= true;
			led_view[LEVEL_NETWORK_CONNECTION].colour 	= COLOUR_GREEN;
			led_view[LEVEL_NETWORK_CONNECTION].pattern 	= LED_PATTERN_ALTERNATE;
			led_view[LEVEL_NETWORK_CONNECTION].duration 	= 0;
			break;
		case STATUS_CONNECTED_TO_CLOUD:		// Connected to AWS
			led_view[LEVEL_NETWORK_CONNECTION].active 	= false;
			led_view[LEVEL_NETWORK_CONNECTION].colour 	= COLOUR_RED;
			led_view[LEVEL_NETWORK_CONNECTION].pattern 	= LED_PATTERN_ALTERNATE;
			led_view[LEVEL_NETWORK_CONNECTION].duration 	= 0;
			LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_ORANGE, LED_PATTERN_FAST_FLASH, 500);
			break;
		case STATUS_PAIRING_MODE_ENABLED:	// Pairing mode entered either via register write or button
			led_view[LEVEL_PAIRING].active 	= true;
			led_view[LEVEL_PAIRING].colour 	= COLOUR_GREEN;
			led_view[LEVEL_PAIRING].pattern 	= LED_PATTERN_FLASH;
			led_view[LEVEL_PAIRING].duration 	= 0;
			break;
		case STATUS_PAIRING_MODE_DISABLED:	// Pairing mode exited
			led_view[LEVEL_PAIRING].active 	= false;
			led_view[LEVEL_PAIRING].colour 	= COLOUR_GREEN;
			led_view[LEVEL_PAIRING].pattern 	= LED_PATTERN_FLASH;
			led_view[LEVEL_PAIRING].duration 	= 0;
			break;
		case STATUS_PAIRING_SUCCESS:			// New device paired
			// This is a useful output when pairing without a live server connection.
			// If we could guarantee that the server was connected, we could ditch it
			// as we get a flash from the serve - STATUS_DISPLAY_SUCCESS
			LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_GREEN, LED_PATTERN_FAST_FLASH, 1600);
			break;
		case STATUS_DISCONNECT_ALL:			// 10sec push on button. All devices removed from pairing table
			LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_GREEN, LED_PATTERN_FAST_FLASH, 1600);
			break;
		case STATUS_RF_OFFLINE:				// No RF contact with devices for a short period
			led_view[LEVEL_RF_CONNECTION].active 	= true;
			led_view[LEVEL_RF_CONNECTION].colour 	= COLOUR_RED;
			led_view[LEVEL_RF_CONNECTION].pattern 	= LED_PATTERN_FLASH;
			led_view[LEVEL_RF_CONNECTION].duration 	= 0;
			break;
		case STATUS_RF_ONLINE:				// RF contact with devices
			led_view[LEVEL_RF_CONNECTION].active 	= false;
			led_view[LEVEL_RF_CONNECTION].colour 	= COLOUR_RED;
			led_view[LEVEL_RF_CONNECTION].pattern 	= LED_PATTERN_ALTERNATE;
			led_view[LEVEL_RF_CONNECTION].duration 	= 0;
			break;
		case STATUS_DISPLAY_SUCCESS:			// Command from Server to indicate animal activity
			LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_GREEN, LED_PATTERN_FAST_FLASH, 450);
			break;
		case STATUS_FIRMWARE_UPDATING:	// This is when the Server forces an immediate update. There is no return
			led_view[LEVEL_FORCED_FW_UPDATE].active 	= true;
			led_view[LEVEL_FORCED_FW_UPDATE].colour 	= COLOUR_RED;
			led_view[LEVEL_FORCED_FW_UPDATE].pattern 	= LED_PATTERN_MAX;
			led_view[LEVEL_FORCED_FW_UPDATE].duration 	= 0;
			break;
		default:
			zprintf(HIGH_IMPORTANCE,"process_system_event() received unknown event 0x%02x\r\n",event);
			break;
	}
	update_led_view();	// This will set the basic level, which we then temporarily override
}

/**************************************************************
 * Function Name   : set_led_brightness
 * Description     : Executes a server instruction to set the LED base brightness
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void set_led_brightness(LED_MODE value, bool store)
{
	brightness = value;
    if(true == store)
    store_led_brightness(value);                                                // put the brightness setting in persistent store
	update_led_view();	// assert any change in value
}

/**************************************************************
 * Function Name   : get_led_brightness
 * Description     : Gets the LED base brightness
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
LED_MODE get_led_brightness(void)
{
	/*
    PERSISTENT_DATA *pNvPersistent = (PERSISTENT_DATA*)FM_PERSISTENT_PAGE_ADDR; // Persistent data page
	return pNvPersistent->brightness;
	*/
}

/**************************************************************
 * Function Name   : surenet_device_pairing_success_cb()
 * Description     : Called when a pairing process has completed
 *                 : Pairing Mode is turned off automatically
 * Inputs          : Data about new device
 * Outputs         :
 * Returns         :
 **************************************************************/
void surenet_device_pairing_success_cb(ASSOCIATION_SUCCESS_INFORMATION *assoc_info)
{
    uint64_t mac_addr;
    mac_addr = assoc_info->association_addr;
    zprintf(LOW_IMPORTANCE,"surenet_device_pairing_success_cb() Remote Device :%08x", (uint32_t)((mac_addr&0xffffffff00000000)>>32));
    zprintf(LOW_IMPORTANCE,"%08x\r\n", (uint32_t)(mac_addr&0xffffffff));
//	zprintf(CRITICAL_IMPORTANCE,"surenet_device_pairing_success_cb() - source = %d\r\n",assoc_info->source);

	switch(assoc_info->source)
	{	// flash LEDs if pairing completed and was initiated by one of the following
		case PAIRING_REQUEST_SOURCE_SERVER:
		case PAIRING_REQUEST_SOURCE_BUTTON:
		case PAIRING_REQUEST_SOURCE_CLI:
		process_system_event(STATUS_PAIRING_SUCCESS);
			break;
	}
    return;
}

/**************************************************************
 * Function Name   : store_led_brightness
 * Description     : LED brightness is saved in Persistent store (page 2)
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void store_led_brightness(LED_MODE brightness_level)
{
	/*
	LED_MODE old_brightness_level;
	old_brightness_level = get_led_brightness();
	if( brightness_level != old_brightness_level )
	{
		FM_store_persistent_data(PERSISTENT_BRIGHTNESS, (uint32_t)brightness_level);
	}
	*/
}

/**************************************************************
 * Function Name   : readHwVersion
 * Description     : The H/W version is defined by binary weighted input pins
 * Inputs          :
 * Outputs         :
 * Returns         : Version number [0..15]
 **************************************************************/
/*
#define READ_RESISTOR_1()  (GPIO_PinRead(GPIO_PORT_VER_RES_1, GPIO_PIN_VER_RES_1))
#define READ_RESISTOR_2()  (GPIO_PinRead(GPIO_PORT_VER_RES_2, GPIO_PIN_VER_RES_2))
#define READ_RESISTOR_3()  (GPIO_PinRead(GPIO_PORT_VER_RES_3, GPIO_PIN_VER_RES_3))
#define READ_RESISTOR_4()  (GPIO_PinRead(GPIO_PORT_VER_RES_4, GPIO_PIN_VER_RES_4))
*/

uint32_t readHwVersion(void)
{
	/*
    uint8_t hw_version_num = 0;
    hw_version_num  =  READ_RESISTOR_1()       \
                    | (READ_RESISTOR_2() << 1) \
                    | (READ_RESISTOR_3() << 2) \
                    | (READ_RESISTOR_4() << 3);
                    */

    return 0;
}

/**************************************************************
 * Function Name   : surenet_blocking_test_cb
 * Description     : Called each time a blocking test message is received.
 *                 : Each value should be +1 from the previous value
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
#define BLOCKING_SAMPLE_SIZE	50
static uint32_t block_buf[BLOCKING_SAMPLE_SIZE];
static uint8_t block_inptr=0;
static uint32_t last_block_timestamp = 0;

void surenet_blocking_test_cb(uint32_t blocking_test_value)
{
	uint8_t	i;
	uint32_t oldest=0xffffffff;
	uint32_t newest=0;

	//If the next value in the sequence is skipped, print out what it should have been and what was actually received.
	static uint32_t blocking_test_value_prev = 0;
	if (blocking_test_value != blocking_test_value_prev + 1)
	{
		zprintf(CRITICAL_IMPORTANCE,"Skipped: 0x%08x   Actual: 0x%08x\r\n", blocking_test_value_prev + 1, blocking_test_value);
	}
	blocking_test_value_prev = blocking_test_value;

	blocking_test_active = true;
	last_block_timestamp = get_microseconds_tick();
//	zprintf(LOW_IMPORTANCE,"Blocking test sequence number = 0x%08x\r\n",blocking_test_value);
	block_buf[block_inptr++]=blocking_test_value;
	if( block_inptr>=BLOCKING_SAMPLE_SIZE) block_inptr = 0;

	for( i=0; i<BLOCKING_SAMPLE_SIZE; i++)
	{
		if( block_buf[i]<oldest) oldest = block_buf[i];
		if( block_buf[i]>newest) newest = block_buf[i];
	}

	blocking_test_per = ((newest-oldest)-(BLOCKING_SAMPLE_SIZE-1))*100/(BLOCKING_SAMPLE_SIZE);
}

void surenet_blocking_test_watchdog(void)
{
	if(( get_microseconds_tick()-last_block_timestamp)>(2*usTICK_SECONDS))		//No blocking test packets received in the last 2 seconds -> 100% PER.
	{
		blocking_test_per = 100;
	}
	if(( get_microseconds_tick()-last_block_timestamp)>(600*usTICK_SECONDS))	//No blocking test packets received in the last 10 mins -> turn off blocking test mode.
	{
		blocking_test_active = false;
		process_system_event(STATUS_BLOCKING_TEST_OFF);
	}
}

