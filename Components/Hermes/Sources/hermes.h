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
* Filename: hermes.h   
* Author:   Chris Cowdery
* Purpose:   
*   
*            
**************************************************************************/
#ifndef __HERMES_H__
#define __HERMES_H__

#include "Components.h"

#include <stdint.h>
#include "debug.h"	// for LED based debugging for when all else fails!
#include "Signing.h"

#define SHOULD_I_PACKAGE	true
#define MAX_WATCHDOG_RESETS	5	// Number of watchdog resets required to cause a bank switch

#define HUB_HARDWARE_REVISION_MAJOR 3
#define HUB_HARDWARE_REVISION_MINOR 0	// this is populated with the binary weighted hardware resistor values on the PCB

#define GPT1_ONE_SECOND 1000000

// Stringification!
#define XSTR(s)	STR(s)
#define STR(s)	#s
#define GREETING_STRING "cjctamzjvc"


#define __MEMORY_BARRIER()	do                                 	\
							{                                  	\
								__DSB();						\
								if (SCB->CCR & SCB_CCR_DC_Msk)	\
								{								\
									SCB_CleanDCache();			\
								}								\
								__ISB();						\
							} while(0)

								
typedef struct
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
} CONTEXT;

typedef enum
{
	PRODUCT_BLANK,					// Unit has default values in PRODUCT_CONFIGURATION
	PRODUCT_TESTED,					// Unit programmed and tested in the Programmer, but not seen the Label Printer yet
	PRODUCT_CONFIGURED,				// Unit been interrogated by the Label Printer and is now ready for shipping.
	PRODUCT_NOT_CONFIGURED = 0xff,	// used to indicate a non-configured unit, so some interrogation by the production equipment is allowed.
} PRODUCT_STATE;

typedef struct
{
	uint8_t			ethernet_mac[6];
	uint64_t		rf_mac;
	uint16_t		rf_pan_id;	
	uint8_t			serial_number[13];	// H001-0000000\0
	PRODUCT_STATE	product_state;
	PRODUCT_STATE	sanity_state;	// used to detect an uninitialised flash
	uint8_t			secret_serial[33];
	bool			rf_mac_mangle;
	uint8_t			spare1;
	uint8_t			spare2;
	uint8_t			spare3;
	uint8_t			DerivedKey[DERIVED_KEY_LENGTH];
} PRODUCT_CONFIGURATION;

#define SUREFLAP_PAN_ID	0x3421

/* Default IP address configuration.  Used in ipconfigUSE_DNS is set to 0, or
ipconfigUSE_DNS is set to 1 but a DNS server cannot be contacted. */
#define configIP_ADDR0		192
#define configIP_ADDR1		168
#define configIP_ADDR2		0
#define configIP_ADDR3		10

/* Default gateway IP address configuration.  Used in ipconfigUSE_DNS is set to
0, or ipconfigUSE_DNS is set to 1 but a DNS server cannot be contacted. */
#define configGATEWAY_ADDR0	192
#define configGATEWAY_ADDR1	168
#define configGATEWAY_ADDR2	0
#define configGATEWAY_ADDR3	1

/* Default DNS server configuration.  OpenDNS addresses are 208.67.222.222 and
208.67.220.220.  Used in ipconfigUSE_DNS is set to 0, or ipconfigUSE_DNS is set
to 1 but a DNS server cannot be contacted.*/
#define configDNS_SERVER_ADDR0 	192
#define configDNS_SERVER_ADDR1 	168
#define configDNS_SERVER_ADDR2 	0
#define configDNS_SERVER_ADDR3 	1
//#define configDNS_SERVER_ADDR0 	208uL
//#define configDNS_SERVER_ADDR1 	67uL
//#define configDNS_SERVER_ADDR2 	222uL
//#define configDNS_SERVER_ADDR3 	222uL
#define configDNS_SERVER_FULL_IP	((configDNS_SERVER_ADDR3 << 24) + (configDNS_SERVER_ADDR2 << 16) + (configDNS_SERVER_ADDR1 << 8) + configDNS_SERVER_ADDR0)

/* Default netmask configuration.  Used in ipconfigUSE_DNS is set to 0, or
ipconfigUSE_DNS is set to 1 but a DNS server cannot be contacted. */
#define configNET_MASK0		255
#define configNET_MASK1		255
#define configNET_MASK2		255
#define configNET_MASK3		0

/* DHCP has an option for clients to register their hostname.  */
#define HUB2_HOSTNAME       "Sure Petcare Hub"

/* Task priorities. */
// We will only have two priorities, one for ISR's and one for other tasks. This
// means that the scheduler will round-robin the other tasks when they each yield,
// giving each one some processing time.

#define ISR_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define NORMAL_TASK_PRIORITY    1
#define HIGH_TASK_PRIORITY		3

#define MIN_PRINT_LEVEL 0
#define MAX_PRINT_LEVEL 11

#define TERMINAL_PRINT_ENABLED	false

typedef enum
{
	LOW_IMPORTANCE	= 2,
	MEDIUM_IMPORTANCE	= 5,
	HIGH_IMPORTANCE		= 9,	// Unexpected!
	CRITICAL_IMPORTANCE	= 10,	// What the absolute fuck?
	TERMINAL_IMPORTANCE = 10,
	NOT_ENCRYPT_IMPORTANCE	= 11,	//0xFF	// Print to debugger Terminal - faults if not debugging!
} ZPRINTF_IMPORTANCE;

typedef enum
{ // This enum should always be contiguous.
	MESSAGE_ACK,
	MESSAGE_NACK,
	MESSAGE_COMMAND,
	MESSAGE_RESPONSE,
	MESSAGE_GREETING,
	MESSAGE_BACKSTOP,	
}HERMES_MESSAGE_TYPE; // N.B. Values whose first byte is 0x7F (127) are reserved for Pings.


typedef struct
{
	uint32_t				sync_word;//zach
	HERMES_MESSAGE_TYPE		type;
	uint8_t					check; //inverse of first two bytes
  	uint16_t				padding;
	uint32_t				size; //length of the package in bytes
	uint32_t                timestamp; //quando (ms count)
	uint32_t				correlation_ID; 
	
}HERMES_UART_HEADER;


typedef struct
{
  HERMES_UART_HEADER 	header_in;
  char  				payload[256]; //being naughty like chris. Coded by Thom
}HERMES_MESSAGE_RECEIVED;
	

typedef enum
{
	DEAL_NOT_ENOUGH_BYTES,
	DEAL_INVALID_HEADER = -1,
}MESSAGE_DEAL_RESULT;

// Task stack sizes in words
#define ipISR_TASK_STACK_SIZE_WORDS	(0x400/4) //(0x800/4)
#define	NETWORK_WATCHDOG_TASK_STACK_SIZE (0x200/4) //(0x400/4)
#define STARTUP_TASK_STACK_SIZE (0x400/4)
#define FLASH_MANAGER_TASK_STACK_SIZE (0x400/4) //(0x800/4)
#define SHELL_TASK_STACK_SIZE (0x800/4) //(0x1000/4)
#define WATCHDOG_TASK_STACK_SIZE (0x200/4) //(0x400/4)
#define LED_TASK_STACK_SIZE (0x200/4) //(0x800/4)
#define LABEL_PRINTER_TASK_STACK_SIZE (0x800/4)
#define TEST_TASK_STACK_SIZE (0x400/4) //(0x800/4)
#define SNTP_TASK_STACK_SIZE (0x300/4) //(0x400/4)
#define MQTT_TASK_STACK_SIZE (0x1800/4) //(0x2000/4)
#define HTTP_POST_TASK_STACK_SIZE (0x800/4) //
#define HFU_TASK_STACK_SIZE (0x400/4) //(0x800/4)
#define HERMES_APPLICATION_TASK_STACK_SIZE (0x800/4) //
#define rfISR_TASK_STACK_SIZE (0x600/4) //(0x800/4)
#define SURENET_TASK_STACK_SIZE (0x800/4) //(0x2000/4)

#define HUB_SEND_TIME_UPDATES_FROM_DEVICES_EVERY_MINUTE	0x01			// default is every hour
#define HUB_SEND_HUB_STATUS_UPDATES_EVERY_MINUTE 		0x02			// default is every hour
#define HUB_SEND_TIME_UPDATES_FROM_DEVICES				0x04

int hermes_app_start(void);
void mem_dump(uint8_t *addr, uint32_t len);	// utility function
void zprintf(ZPRINTF_IMPORTANCE test, const char *_Restrict, ...);
void set_product_state(PRODUCT_STATE state);
void sanitise_product_config(void);
void write_product_configuration(void);
void hermes_srand(int seed);	// thread safe
int hermes_rand(void);			// thread safe
void initSureNetByProgrammer(void);
void connectToServer(void);





#endif //__HERMES_H__



