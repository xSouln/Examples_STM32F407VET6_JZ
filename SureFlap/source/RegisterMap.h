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
* Filename: RegisterMap.h
* Author:   Chris Cowdery 14/10/2019
* Purpose:  Handles the Hub Register Map.
*
*
*
**************************************************************************/

#ifndef __REGISTER_MAP_H__
#define __REGISTER_MAP_H__

#include "Devices.h"
#include "hermes.h"

#define PRINT_REG_MAP	false

#if PRINT_REG_MAP
#define regmap_printf(...)		zprintf(TERMINAL_IMPORTANCE, __VA_ARGS__)
#else
#define regmap_printf(...)
#endif

// r = read only, w = write only (reading is meaningless), rw = read & write
typedef enum
{
  HR_DEVICE_TYPE = 0,  				// 0 r - DEVICE_TYPE_HUB = 1
  HR_NUM_REGISTERS_LOW,				// 1 r - number of registers in the hub
  HR_NUM_REGISTERS_HIGH,			// 2
  HR_HUB_FIRMWARE_MAJOR_LOW,    	// 3 r - low byte of firmware major revision
  HR_HUB_FIRMWARE_MAJOR_HIGH,   	// 4 r - high byte of firmware major revision
  HR_HUB_FIRMWARE_MINOR_LOW,    	// 5 r - low byte of firmware minor revision
  HR_HUB_FIRMWARE_MINOR_HIGH,   	// 6 r - high byte of firmware minor revision
  HR_HUB_HARDWARE_MAJOR_LOW,    	// 7 r - low byte of hardware major revision
  HR_HUB_HARDWARE_MAJOR_HIGH,   	// 8 r - high byte of hardware major revision
  HR_HUB_BOOTLOADER_MAJOR,      	// 9 r - major bootloader revision
  HR_HUB_BOOTLOADER_MINOR = 10, 	// 10 r - minor bootloader revision // 10
  HR_HUB_MAX_MESSAGE_SIZE_LOW,  	// 11 r - low byte of maximum message size. Not used
  HR_HUB_MAX_MESSAGE_SIZE_HIGH, 	// 12 r - high byte of maximum message size. Not used
  HR_HUB_FIRMWARE_CHECKSUM_LOW, 	// 13 r - low byte of modulo 65536 checksum of firmware image
  HR_HUB_FIRMWARE_CHECKSUM_HIGH,	// 14 r - low byte of modulo 65536 checksum of firmware image
  HR_PAIRING_STATE,             	// 15 rw - Puts hub into pairing mode - 0=PAIRING_OFF, 1=PAIRING_DISTANCE, 2=PAIRING_MANUAL //15
  HR_DEBUG_MODE,                	// 16 rw - control debug output
  HR_PAIRING_TIMER_VALUE,       	// 17 not used
  HR_LEDS_MODE,                 	// 18 rw ? Enumerated type LED_MODE controlling LED behaviour (e.g. none, alert, normal)
  HR_REMOTE_REBOOT,             	// 19 w - 0xdd / 221 = cause the hub to reset and restart
  HR_CLEAR_PAIRING_TABLE = 20,  	// 20 w - 255 = cause the pairing table to be cleared. 255 clears all pair
  HR_INITIATE_CHANNEL_HOP,      	// 21 w - 0-31 = channel no, 255 = initiate a channel hop. Note it might end up back on the same channel
  HR_DELETE_CONNECTION_TABLE_ENTRY,	// 22 Delete entry in pairing table
  HR_SURENET_CHANNEL_NOISE_FLOOR,  	// 23 r - channel noise floor for current MiWi channel // 20
  HR_SURENET_CHANNEL_NUMBER,       	// 24 r - channel being used by SureNet
  HR_APPLICATION_ERROR_LOW, 		// 25 r - low byte of most recent application error
  HR_APPLICATION_ERROR_HIGH,		// 26 r - high byte of most recent application error
  HR_HUB_UPTIME_LOW,          		// 27 r - Note that _UPTIME_LOW must be read first to trigger an atomic update of these four registers
  HR_HUB_UPTIME_BITS_15_8,			// 28
  HR_HUB_UPTIME_BITS_23_16,			// 29
  HR_HUB_UPTIME_HIGH = 30,			// 30 r - hub uptime in seconds (2^32 seconds = 136 years - I won't care then)
  HR_HUB_BANK_A_MARK,				// 31 r - These four are selected parts of the Bank Descriptors
  HR_HUB_BANK_A_WDOG_RESETS,		// 32 r
  HR_HUB_BANK_B_MARK,         		// 33 r
  HR_HUB_BANK_B_WDOG_RESETS,		// 34 r
  HR_HUB_BANK_A_HASH_LOW,			// 35 NOT IMPLEMENTED
  HR_HUB_BANK_A_HASH_BITS_15_8,		// 36
  HR_HUB_BANK_A_HASH_BITS_23_16,	// 37
  HR_HUB_BANK_A_HASH_HIGH,			// 38 amount of free space on heap. We don't use the heap....
  HR_HUB_BANK_B_HASH_LOW,			// 39
  HR_HUB_BANK_B_HASH_BITS_15_8 = 40,// 40
  HR_HUB_BANK_B_HASH_BITS_23_16,	// 41
  HR_HUB_BANK_B_HASH_HIGH,			// 42 Number of writes to non-volatile memory
  HR_DEVICE_TABLE_SIZE,				// 43 r - number of rows in pairing table
  HR_DEVICE_TABLE_NUMBER_CONNECTIONS, // 44 r - number of valid pairs in pairing table
// pairing table must go at the end
  HR_DEVICE_TABLE,            		// 45 r - pairing table, first byte is at register address 42, last is at 42+(16*10)-1 = 201
  HR_DEVICE_TABLE_END = HR_DEVICE_TABLE+(sizeof(DEVICE_STATUS)*MAX_NUMBER_OF_DEVICES)-1,
  HR_LAST_ELEMENT,
  //HR_HUB_HRS,
  //HR_HUB_MINS,
} HUB_REGISTER_TYPE;

#define HR_GET_DYNAMIC_SIZE()	(HR_DEVICE_TABLE + (sizeof(DEVICE_STATUS) * hubRegisterBank[HR_DEVICE_TABLE_SIZE].value))

#define MAX_REGISTERS_PER_MESSAGE		128
#define MAX_PAIR_ENTRIES_PER_MESSAGE	(MAX_REGISTERS_PER_MESSAGE / sizeof(DEVICE_STATUS))

typedef struct
{
    uint8_t 	(*read_handler_function)(uint16_t);
    void 		(*write_handler_function)(uint16_t,uint8_t);
    uint8_t		value;
    bool		updateWebFlag;
    bool		end_of_block;
	bool		include_in_hash;
} T_HUB_REGISTER_ENTRY;

//LED mode commands sent by server to hub. Interpreted as a brightness
typedef enum
{
  LED_DISPLAY_OFF,
  LED_DISPLAY_NORMAL,
  LED_DISPLAY_STEADY,  // not used no flashing LEDs, just static ones
  LED_DISPLAY_ALERT,   // not used
  LED_DISPLAY_DIM,
} LED_DISPLAY;

#define LED_MODE_DISPLAY_MASK 0xf //lower nibble contains basic display mode (e.g. normal dim or off)
#define LED_MODE_SUCCESS_BIT 0x80 //Or with any of the modes sent by the server to display success then switch to one of the LED_DISPLAY modes

#define	RESET_PROCESSOR	0xdd
#define RESET_PROCESSOR_AND_USE_8_BYTE_RF_MAC	0x80	// Use proper RF MAC as programmed into Flash
#define	RESET_PROCESSOR_AND_USE_6_BYTE_RF_MAC	0x81	// Use 6 byte Ethernet MAC with 0xfffe padding for RF MAC

void		HubReg_Handle_Messages(void);
void		HubReg_Dump(void);
void		HubReg_Init(void);
void		HubReg_SetValues(uint16_t address, uint16_t numValues, uint8_t* values, bool set_update_flag);
void		HubReg_GetValues(uint16_t address, uint16_t numValues);
void		HubReg_SetDebugMode(uint16_t address, uint8_t value);
void		HubReg_Check_Full(void);
void		HubReg_Send_All(void);
void		HubReg_SetPairingMode(PAIRING_REQUEST mode);
void		HubReg_Refresh_All(void);
uint32_t	HubReg_Get_Integer(uint16_t address);

#endif
