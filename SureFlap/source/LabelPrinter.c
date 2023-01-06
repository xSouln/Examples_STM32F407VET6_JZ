/*****************************************************************************
*
* SUREFLAP CONFIDENTIALITY & COPYRIGHT NOTICE
*
* Copyright © 2013-2021 Sureflap Limited.
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
* Filename: LabelPrinter.c   
* Author:   Chris Cowdery 15/06/2020
* Purpose:  Label Printer handler for Production Test   
*             
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "hermes.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "list.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"

/* Freescale includes. */
#include "fsl_debug_console.h"

// Other includes
#include "LabelPrinter.h"
#include "utilities.h"	// for crc16Calc()
#include "devices.h"	// for DEVICE_TYPE_HUB
#include "leds.h"
#include "hermes-app.h"	// for READ_BUTTON()
#include "hermes-time.h"
#include "board.h"
#include "NetworkInterface.h"	// for NetworkInterface_IsActive()

// This file supports the interaction with the Label Printer as part of
// the manufacturing process.
//
// The steps involved are (taken from the Sure Petcare Wiki)
//    Opens TCP socket to address 192.168.0.1, port 1800
//    Create text string "%d\r\n\r\nserial_number=%s&mac_address=%s&product_id=1", length, serial number, mac address where length is the total length of the string in bytes. Not clear if includes null terminator.
//    Calculate 16bit CRC of text string.
//    Send CRC to TCP socket, high byte first
//    Send text string to TCP socket.
//    Hub Label Printer will respond with the text string "Okay"
//    If the Label Printer does NOT respond with Okay, the Hub will stop with a single red LED illuminated.
//    The Hub then cycles through the four LEDs in turn, advancing every 250ms
//    The LEDs are cycled until the button is pressed for more than 250ms
//    The green LEDs are turned on.
//    The string "Button press\r\n" is written to the TCP socket
//    The PRODUCT_CONFIGURATION is changed to PRODUCT_CONFIGURED. Doing this makes Label Printing a one-time process.
//
// The final "Button press" string causes the Label Printer to print out the label for that specific Hub2. 

#define PRINT_LBL	false

#if PRINT_LBL
#define lbl_printf(...)		zprintf(CRITICAL_IMPORTANCE, __VA_ARGS__)
#define lbl_flush			DbgConsole_Flush
#else
#define lbl_printf(...)
#define lbl_flush()
#endif

typedef enum
{
	LABEL_START,
	LABEL_WAIT_CONNECTED,
	LABEL_OPEN_SOCKET,
	LABEL_IDENTIFY,
	LABEL_WAIT_OKAY,
	LABEL_CYCLE_LEDS,
	LABEL_BUTTON_PRESS,
	LABEL_FAIL,
	LABEL_DONE,
} LABEL_PRINT_STATE;

LABEL_PRINT_STATE label_print_state;

#define LBL_SERVER_1	192
#define LBL_SERVER_2	168
#define LBL_SERVER_3	0
#define LBL_SERVER_4	1
#define LBL_PORT		1800

extern PRODUCT_CONFIGURATION product_configuration;	// This is a RAM copy of the product info from Flash.

// test function, invoked by typing 'labelreset' from CLI
void restart_label_print(void)	// this is a bit naughty, but only needed for testing...
{
	if( (LABEL_DONE == label_print_state) || (LABEL_WAIT_CONNECTED == label_print_state))
	{
		zprintf(MEDIUM_IMPORTANCE,"Restarting label printer state machine\r\n");
		label_print_state = LABEL_START;
	}
	else
	{
		zprintf(MEDIUM_IMPORTANCE,"NOT restarting label printer state machine as it's busy!\r\n");
	}			
}

/**************************************************************
 * Function Name   : label_task
 * Description     : This task is run when the product is in PRODUCT_TESTED configuration
 *                 : which is set when the board has been successfully programmed and
 *                 : tested. It handles communication with the Label Printer to emit
 *                 : the label for the bottom of the unit and the packaging. 
 *                 : The process exchanges the serial numbers etc. with the Label Printer,
 *                 : then tests the LEDs and pushbutton.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void label_task(void *pvParameters)
{
	Socket_t 					lblSocket;
	struct freertos_sockaddr	address;
	char 						idstring[128];
	uint32_t					timeout = pdMS_TO_TICKS(5000); 
	uint32_t					len;
	uint16_t 					CRC;
	uint8_t						byte;
	char 						reply[128];
	uint8_t						led_cycle_state = 0;	// used to cycle around colour LEDs.
	uint32_t					led_cycle_timer;
	uint32_t					start_timer;
	uint8_t						old_button_state;
	uint32_t 					button_timer;
	char *						finish_string = "Button press\r\n";
	char 						mac_string[] = "0123456789abcdef";
	
	label_print_state = LABEL_START;
	
	zprintf(CRITICAL_IMPORTANCE,"Label task starting...\r\n");

	while(1)
	{
		switch (label_print_state)
		{
			case LABEL_START:
				LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_RED, LED_PATTERN_SOLID, 0);
				start_timer = get_microseconds_tick();
				label_print_state = LABEL_WAIT_CONNECTED;
				break;
			case LABEL_WAIT_CONNECTED:	// wait for the phy to be connected. Actually don't!	
				if( (get_microseconds_tick() - start_timer) > 1000000)	// wait one second for Ethernet to come up
					label_print_state = LABEL_OPEN_SOCKET;
				break;
			case LABEL_OPEN_SOCKET:	// open a socket to 192.168.0.1 on port 1800
				lblSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
				if (FREERTOS_INVALID_SOCKET != lblSocket)
				{
					address.sin_addr = FreeRTOS_inet_addr_quick( LBL_SERVER_1, LBL_SERVER_2,LBL_SERVER_3 ,LBL_SERVER_4 );	
					address.sin_port = FreeRTOS_htons(LBL_PORT);					
					if( 0== FreeRTOS_connect( lblSocket, &address, sizeof(address)))
					{
						lbl_printf("Connected to printer\r\n");					
						label_print_state = LABEL_IDENTIFY;
					}
					else
					{	// couldn't connect to printer
						lbl_printf("Could not connect to printer\r\n");								
						label_print_state = LABEL_FAIL;
					}
				}
				else	// couldn't open socket
				{
					lbl_printf("Couldn't open socket\r\n");				
					label_print_state = LABEL_FAIL;
				}
				break;
			case LABEL_IDENTIFY:	// Send the ID string to the Label Printer
				sprintf(mac_string, "0000%02X%02X%02X%02X%02X%02X",	product_configuration.ethernet_mac[0],product_configuration.ethernet_mac[1], \
																	product_configuration.ethernet_mac[2],product_configuration.ethernet_mac[3], \
																	product_configuration.ethernet_mac[4],product_configuration.ethernet_mac[5]);
				sprintf(idstring,"serial_number=%s&mac_address=%s&product_id=%1d", \
								product_configuration.serial_number, \
								mac_string,
								DEVICE_TYPE_HUB);
				len=strlen(idstring);	// how long is it _without_ leading length and \r\n\r\n

				sprintf(idstring,"%d\r\n\r\nserial_number=%s&mac_address=%s&product_id=%1d", \
								len, \
								product_configuration.serial_number, \
								mac_string,
								DEVICE_TYPE_HUB);
				// Now we need to amend len to account for the %d\r\n\r\n at the beginning
				if (len>95)		// this is a hideous kludge. 95 is 99-strlen("\r\n\r\n")
					len+=7;
				else
					len+=6;				
				CRC = CRC16((uint8_t *)idstring, len, 0xcccc);
				byte = CRC>>8;
				FreeRTOS_send(lblSocket, &byte, sizeof(byte), 0);
				byte = CRC & 0xff;
				FreeRTOS_send(lblSocket, &byte, sizeof(byte), 0);
				if( len!= FreeRTOS_send(lblSocket, &idstring, len, 0) )
				{
					lbl_printf("Failed to send to socket\r\n");						
					label_print_state = LABEL_FAIL;
				}
				else
				{
					lbl_printf("Sent %s to printer\r\n",idstring);					
					label_print_state = LABEL_WAIT_OKAY;
				}
				break;	
			case LABEL_WAIT_OKAY:	// wait for the Label Printer to respond with "Okay"
				len = FreeRTOS_recv(lblSocket, reply, sizeof(reply), 0);
				if( strcmp(reply,"Okay")==0)
				{
					led_cycle_state = 0;
					led_cycle_timer = get_microseconds_tick();
					LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_GREEN, LED_PATTERN_SOLID_LEFT, 250);
					old_button_state = !BUTTON_PRESSED;					
					label_print_state = LABEL_CYCLE_LEDS;
				}
				else
				{
					lbl_printf("Timeout waiting for OKAY\r\n");						
					label_print_state = LABEL_FAIL;
				}
				break;	
			case LABEL_CYCLE_LEDS:
				if( (get_microseconds_tick()-led_cycle_timer) > 250000)	// advance the LED
				{
					led_cycle_timer = get_microseconds_tick();

					switch (led_cycle_state)
					{
						case 0:
							LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_GREEN, LED_PATTERN_SOLID_LEFT, 0);
							led_cycle_state++;						
							break;
						case 1:
							LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_RED, LED_PATTERN_SOLID_LEFT, 0);
							led_cycle_state++;
							break;
						case 2:
							LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_GREEN, LED_PATTERN_SOLID_RIGHT, 0);
							led_cycle_state++;
							break;
						case 3:
							LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_RED, LED_PATTERN_SOLID_RIGHT, 0);
							led_cycle_state = 0;
							break;
						default:
							led_cycle_state = 0;
							break;
					}
				}
				if( (!BUTTON_PRESSED == old_button_state) && (BUTTON_PRESSED == READ_BUTTON()) )
				{
					old_button_state = BUTTON_PRESSED;
					button_timer = get_microseconds_tick();
				}
				if( BUTTON_PRESSED != READ_BUTTON() )
				{
					old_button_state = !BUTTON_PRESSED;
				}
				if( (250000<(get_microseconds_tick()-button_timer)) && (BUTTON_PRESSED == old_button_state) )
				{
					product_configuration.product_state = PRODUCT_CONFIGURED;					
					write_product_configuration();	
					LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_GREEN, LED_PATTERN_SOLID, 0);	
					FreeRTOS_send(lblSocket, finish_string, strlen(finish_string)+1, 0);
					lbl_printf("Label printing done\r\n");							
					label_print_state = LABEL_DONE;
				}
				break;	
			case LABEL_FAIL:
				LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_RED, LED_PATTERN_SOLID, 0);
				lbl_printf("Test FAILED\r\n");					
				label_print_state = LABEL_DONE;
				break;
			case LABEL_DONE:		
			default:	
				// wait here for ever
				break;
		}
	}
}

