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
* Filename: flashManager.h   
* Author:   Tony Thurgood
* Purpose:  19/9/2019  
*   
* Flash Manager methods to handle qSPI flash memory and NV Store
*            
**************************************************************************/

#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include "fsl_common.h"
#include "task.h"
#include "leds.h"

// Hermes Flash Memory Map
#define FLASH_BASE              0x60000000                                      // Cortex M7 memory map
#define FLASH_TOTAL_SIZE_BYTES  4194304                                         // currently 4Mbytes
#define FLASH_SECTOR_SIZE_BYTES 4096
#define FLASH_PAGE_SIZE_BYTES   256
#define FLASH_PAGES_PER_SECTOR  16
#define FLASH_NUM_SECTORS       (FLASH_TOTAL_SIZE_BYTES/FLASH_SECTOR_SIZE_BYTES)// 1024 sectors
#define PAGE_BOUNDARY_MASK      0xFFFFFF00
#define SECTOR_BOUNDARY_MASK    0xFFFFF000


/* FLEXSPI - Peripheral instance base addresses */
/** Peripheral FLEXSPI base address */
#define FLEXSPI_BASE            (0x402A8000u)

/** Peripheral FLEXSPI base pointer */
#define FLEXSPI                 ((FLEXSPI_Type *)FLEXSPI_BASE)

// NV Store addresses - Note: Arm CM7 defines ext memory map offset (qspi flash) at 0x6000,0000
#define FM_SECTOR_PERSISTENT    (FLASH_NUM_SECTORS-1)                           // 3FF000 - 1023 Last sector, Persistent data
// spare                                                                        // 3FE000 - 1022
#define FM_SECTOR_AES           (FLASH_NUM_SECTORS-3)                           // 3FD000 - 1021
#define FM_SECTOR_ID            (FLASH_NUM_SECTORS-4)                           // 3FC000 - 1020                                
#define FM_SECTOR_TEST_DATA     (FLASH_NUM_SECTORS-5)                           // 3FB000 - 1019                                
#define FM_SECTOR_MQTT_KEY      (FLASH_NUM_SECTORS-6)                           // 3FA000 - 1018
#define FM_SECTOR_MQTT_CERT     (FLASH_NUM_SECTORS-7)                           // 3F9000 - 1017
#define FM_SECTOR_STATS         (FLASH_NUM_SECTORS-8)                           // 3F8000 - 1016

#define FM_SECTOR_EO_STARTUP    8                                               // IVT and hermes-main
#define FM_MAC_LEN              8
#define FM_SN_LEN               6
#define FM_SN_OFFSET            8                                               // from page boundary

// Second page address of Persistent store, used for housekeeping
#define FM_PERSISTENT_PAGE_ADDR ((FLASH_BASE + (FM_SECTOR_PERSISTENT*FLASH_SECTOR_SIZE_BYTES)) + (1*FLASH_PAGE_SIZE_BYTES))


// Mailbox data types for handling NV Store
typedef enum
{
    FM_DEVICE_STATS,
    FM_PRODUCT_CONFIG,
    FM_FACTORY_TEST,
    FM_AES_DATA,
    FM_PERSISTENT_DATA,
    FM_UPDATE_FW,
    FM_ERASE_SECTOR,
    FM_TEST,
	FM_GENERIC_WRITE,
	FM_GENERIC_ERASE,
	FM_CREDENTIAL,
}FM_STORE_DATA_TYPE;


typedef enum
{
    FM_NAK = 1 << 0,
    FM_ACK = 1 << 1,
    FM_GET = 1 << 2,
    FM_PUT = 1 << 3,
    FM_ERASE = 1 << 4,
    FM_UPDATE = 1 << 5,
    FM_FW_BLOCK = 1 << 6,
    FM_FW_ERASE_BANK = 1 << 7,
    FM_FW_CHECK = 1 << 8,
    FM_FW_FLIP = 1 << 9,
}FM_STORE_ACTION;


typedef struct 
{
    FM_STORE_ACTION		action;
	FM_STORE_DATA_TYPE	type;
	uint8_t*			ptrToBuf;
	int32_t				dataLength;   
	int16_t				sectorNum;   
	int32_t				offsetSectorAddr;	// offset from base... 0,000 to 1f7,000
    TaskHandle_t		xClientTaskHandle;	// identify who sent this message
}SEND_TO_FM_MSG;


typedef struct 
{
    FM_STORE_ACTION action;
	FM_STORE_DATA_TYPE type;
	uint8_t *ptrToBuf;
	int16_t dataLength;   
}RESP_FROM_FM_MSG;


typedef enum
{
    FM_CRC_DEV_STATS        = 1016,
    FM_CRC_DEV_STATS_POSN   = 0,
    FM_CRC_MQTT_CERT        = 1017,
    FM_CRC_MQTT_CERT_POSN   = 2,
    FM_CRC_MQTT_KEY         = 1018,
    FM_CRC_MQTT_KEY_POSN    = 4,
    FM_CRC_FT_DATA          = 1019,
    FM_CRC_FT_DATA_POSN     = 6,
    FM_CRC_MAC_SERIAL       = 1020,
    FM_CRC_MAC_SERIAL_POSN  = 8,
    FM_CRC_KEYS             = 1021,
    FM_CRC_KEYS_POSN        = 10,
// 1022 - spare    
    FM_CRC_PERSISTENT       = 1023,
    FM_CRC_PERSISTENT_POSN  = 14,
}CRC_DATA_AREA;


typedef enum
{
    FM_UFW_NULL,
    FM_UFW_FIRST_BLOCK,
    FM_UFW_IN_PROGRESS,
    FM_UFW_DONE,
}UFW_STATE;


// data definitions for Programmer test results
typedef struct
{
    uint32_t standbyTest : 1; 
    uint32_t greenTest : 1; 
    uint32_t redTest : 1; 
    uint32_t rfTest : 1; 
}TEST_PASS;


typedef union
{
    TEST_PASS testPass; 
    uint32_t all;                                                               // so we can track the pass/fails in a word32 
}PASS_RESULTS;	


typedef struct
{
	uint32_t		testRevNum;
	uint32_t		standby_mA;
	uint32_t		green_mA;
	uint32_t		red_mA;
    PASS_RESULTS    passResults;                                                // I don't expect to see a failure
}FACTORY_TEST_DATA;


typedef struct
{
	uint64_t		aes_high;
	uint64_t		aes_low;
} AES_CONFIG;


// used to store current Hermes settings for POR continuity
typedef struct
{
	LED_MODE		brightness;
	uint8_t		    printLevel;
	uint8_t			RF_channel;
	uint8_t			spare_1_2;
	uint8_t			spare_1_3;
	uint8_t			spare_1_4;
	uint32_t		spare_2;
}PERSISTENT_DATA;

typedef enum
{
	PERSISTENT_BRIGHTNESS,
	PERSISTENT_PRINTLEVEL,
	PERSISTENT_RF_CHANNEL,
} PERSISTENT_STORE_INDEX;

// Prototypes
status_t FM_hermesFlashInit(void);
void FM_hermesFlashTask(void *pvParameters);
void FM_hermesFlashResponseMsg(status_t status, uint32_t dataLength, FM_STORE_ACTION action);
status_t FM_hermesFlashCrcElementModify(uint8_t* p_incomingMsg, uint32_t elementAddr, uint16_t dataLength);
void FM_hermesFlashUpdateDevStats(SEND_TO_FM_MSG* p_incomingMsg, uint32_t elementAddr);
void FM_hermesFlashElementRead(uint8_t* p_incomingMsg, uint32_t elementAddr, uint16_t dataLength, TaskHandle_t xClientTaskHandle);
void FM_hermesFlashReadDevStats(SEND_TO_FM_MSG* p_nvmIncomingMsg, uint32_t elementAddr);
void FM_hermesFlashCopyPage(uint8_t* nvPageData, uint8_t* p_nvReadAddr);
status_t FM_hermesFlashPageWrite(uint32_t pageAddr, uint8_t *p_ramSinglePageData);      
status_t FM_hermesFlashPageRead(uint32_t pageAddr, uint8_t *p_ramSinglePageData);
status_t FM_hermesFlashSectorWrite(uint32_t sectorAddr, uint8_t *p_fwData);
status_t FM_hermesFlashEraseSector(uint32_t sector_num);
uint16_t FM_crcCalcAreaCode(CRC_DATA_AREA dataArea);
status_t FM_crcUpdateAreaCode(CRC_DATA_AREA dataArea);
status_t FM_crcIntegrityCheck(void);
void FM_hermesFlashNotifyResponse(status_t status, TaskHandle_t p_ClientTaskHandle);
status_t FM_hermesFlashPageUpdate(SEND_TO_FM_MSG* p_incomingMsg, uint32_t pageAddr);
void FM_store_persistent_data(PERSISTENT_STORE_INDEX index, uint32_t param);

// External Exposure
bool	hermesFlashRequestErase(uint8_t* target, uint32_t size, bool await);
bool	hermesFlashRequestWrite(uint8_t* to_write, uint8_t* target, uint32_t size, bool await);
bool	hermesFlashRequestCredentialWrite(uint8_t* credential, void* target, uint32_t data_size);

#endif  // FLASH_MANAGER_H


