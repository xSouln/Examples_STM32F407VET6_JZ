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
* Filename: flashManager.c   
* Author:   Tony Thurgood
* Purpose:  19/9/2019  
*   
* Flash Manager methods to handle qSPI flash memory and NV Store
*            
**************************************************************************/

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* FreeRTOS+IP includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "queue.h"

/*Other includes*/
#include "fsl_flexspi.h"
#include "app.h"
#include "Devices.h"
#include "stdbool.h"
#include "flashManager.h"
#include "fsl_flexspi_ram.h"
#include "fsl_cache.h"
#include "credentials.h"
#include "utilities.h"

extern TaskHandle_t xHermesTestTaskHandle;      // declared in hermes.c
extern TaskHandle_t xFM_hermesFlashTaskHandle;     //      "
extern PRODUCT_CONFIGURATION product_configuration;                             // This is a RAM copy of the product info from Flash.

uint8_t flash_page_read_buf[FLASH_PAGE_SIZE];

// Shared mailboxes
QueueHandle_t xNvStoreMailboxSend;
QueueHandle_t xNvStoreMailboxResp;

// global declarations
static uint32_t targetBankBase;

// Local Functions:
static status_t hermesFlashWriteCredential(SEND_TO_FM_MSG* msg);

/*******************************************************************************
 * Flash Manager Initialisation
 ******************************************************************************/
/* Define the standard flexspi configuration */
flexspi_device_config_t deviceconfig = {
    .flexspiRootClk       = 133000000,
    .flashSize            = FLASH_SIZE,
    .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval           = 2,
    .CSHoldTime           = 3,
    .CSSetupTime          = 3,
    .dataValidTime        = 0,
    .columnspace          = 0,
    .enableWordAddress    = 0,
    .AWRSeqIndex          = 0,
    .AWRSeqNumber         = 0,
    .ARDSeqIndex          = NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD,
    .ARDSeqNumber         = 1,
    .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};

/* Define the flexspi commands in the lookup table */
uint32_t customLUT[CUSTOM_LUT_LENGTH] = {
    /* Normal read mode -SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x03, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Fast read mode - SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x0B, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, 0x08, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Fast read quad mode - SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xEB, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x06, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x04),

    /* Read extend parameters */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x81, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Write Enable */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x06, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Erase Sector  */
    [4 * NOR_CMD_LUT_SEQ_IDX_ERASESECTOR] =
//        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xD7, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),      // for ISSI
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x20, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),        // for MX and ISSI

    /* Page Program - single mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x02, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Page Program - quad mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x32, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read ID */
    [4 * NOR_CMD_LUT_SEQ_IDX_READID] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x9F, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Enable Quad mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x01, kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04),

    /* Enter QPI mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_ENTERQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x35, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Exit QPI mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_EXITQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0xF5, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read status register */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUSREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x05, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Erase whole chip */
    [4 * NOR_CMD_LUT_SEQ_IDX_ERASECHIP] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xC7, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
};






/**************************************************************
 * Function Name   : FM_hermesFlashInit
 * Description     : Sets up Flash Manager
 * Inputs          : 
 * Outputs         : none
 * Returns         : void
 **************************************************************/
status_t FM_hermesFlashInit(void)
{
    static status_t nvStoreStatus;
	status_t ret = kStatus_Success;
	xNvStoreMailboxSend = xQueueCreate(1, sizeof(SEND_TO_FM_MSG));   // incoming mailbox queue for NV requests 
	xNvStoreMailboxResp = xQueueCreate(1, sizeof(RESP_FROM_FM_MSG)); // outgoing mailbox queue for NV responses

        if (product_configuration.product_state == PRODUCT_CONFIGURED)
        {
            nvStoreStatus = FM_crcIntegrityCheck();                                // Normal start-up, so check NV store for corruption - NOT sure what we are doing with this?
            if (!nvStoreStatus)
			//zprintf(LOW_IMPORTANCE,"\r\nNV Store CRC status is good. \r\n");
			ret = kStatus_Success;
            else
			ret = kStatus_Fail;
			//zprintf(CRITICAL_IMPORTANCE,"\r\n\nNV Store CRC FAIL - kStatus_Fail \r\n\n");   // <<<< Now what?
        }
	return ret;
}

/**************************************************************
 * Function Name   : FM_hermesFlashTask
 * Description     : FreeRtos do forever task - State m/c driven by incoming msgs
 * Inputs          : pvParameters - std parameter
 * Outputs         : none
 * Returns         : void
 **************************************************************/
void FM_hermesFlashTask(void *pvParameters)
{
    status_t		status = kStatus_Fail;
    SEND_TO_FM_MSG	nvmIncomingMsg;
	uint32_t		i;
    
    while( 1 )
    {
		/* Check the queue for incoming messages.   */
		if( xQueueReceive( xNvStoreMailboxSend, &nvmIncomingMsg, pdMS_TO_TICKS(5) ) == pdPASS )
		{
			/* Service the incoming message, type and action */
			switch( nvmIncomingMsg.type )
			{
				case FM_PRODUCT_CONFIG:
					if( nvmIncomingMsg.action == FM_PUT )
					{
						/* Data string shoulld be checked for length, but strlen() can't be used, because 00 is a valid char */
						if( nvmIncomingMsg.dataLength == sizeof(PRODUCT_CONFIGURATION) ) 
						{
                            status = FM_hermesFlashPageUpdate(&nvmIncomingMsg, FLASH_BASE+(FM_SECTOR_ID*FLASH_SECTOR_SIZE_BYTES));
							/* Flush the cache, so subsequent read will be from flash */
							DCACHE_InvalidateByRange((uint32_t)(FLASH_BASE+(FM_SECTOR_ID*FLASH_SECTOR_SIZE_BYTES)), nvmIncomingMsg.dataLength);
							FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
							FM_crcUpdateAreaCode(FM_CRC_MAC_SERIAL);
						} else
						{
							FM_hermesFlashNotifyResponse(kStatus_Fail, nvmIncomingMsg.xClientTaskHandle);
						}
					} else
					{ 	/* Get PRODUCT_CONFIG from NV Store */
						FM_hermesFlashElementRead(nvmIncomingMsg.ptrToBuf, (FLASH_BASE+(FM_SECTOR_ID*FLASH_SECTOR_SIZE_BYTES)), nvmIncomingMsg.dataLength, nvmIncomingMsg.xClientTaskHandle);
					}
					break;
				
				case FM_DEVICE_STATS:
					if( nvmIncomingMsg.action == FM_PUT )
					{
						FM_hermesFlashUpdateDevStats(&nvmIncomingMsg, (FLASH_BASE + (FM_SECTOR_STATS * FLASH_SECTOR_SIZE_BYTES)) );
						/* Flush the cache, so subsequent read will be from flash */
						DCACHE_InvalidateByRange((uint32_t)(FLASH_BASE + (FM_SECTOR_STATS * FLASH_SECTOR_SIZE_BYTES)), nvmIncomingMsg.dataLength);
						FM_crcUpdateAreaCode(FM_CRC_DEV_STATS);
					} else
					{ 	/* Return the Device Stats from NV area */
						FM_hermesFlashReadDevStats(&nvmIncomingMsg, (FLASH_BASE + (FM_SECTOR_STATS * FLASH_SECTOR_SIZE_BYTES)) );
					}
					break;
				
				case FM_FACTORY_TEST:
					if( nvmIncomingMsg.action == FM_PUT )
					{
						/* Data string should be checked for length, but strlen() can't be used, because 00 is a valid char */
						if( nvmIncomingMsg.dataLength == sizeof(FACTORY_TEST_DATA) ) 
						{
                            status = FM_hermesFlashPageUpdate(&nvmIncomingMsg, FLASH_BASE+(FM_SECTOR_TEST_DATA*FLASH_SECTOR_SIZE_BYTES));
							/* Flush the cache, so subsequent read will be from flash */
							DCACHE_InvalidateByRange((uint32_t)(FLASH_BASE+(FM_SECTOR_TEST_DATA*FLASH_SECTOR_SIZE_BYTES)), nvmIncomingMsg.dataLength);
							FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
							FM_crcUpdateAreaCode(FM_CRC_FT_DATA);
						} else
						{
							FM_hermesFlashNotifyResponse(kStatus_Fail, nvmIncomingMsg.xClientTaskHandle);
						}
					} else
					{ 	/* Get TEST_DATA from NV Store */
						FM_hermesFlashElementRead(nvmIncomingMsg.ptrToBuf, (FLASH_BASE+(FM_SECTOR_TEST_DATA*FLASH_SECTOR_SIZE_BYTES)), nvmIncomingMsg.dataLength, nvmIncomingMsg.xClientTaskHandle);
					}
					break;
				
				case FM_AES_DATA:
					if( nvmIncomingMsg.action == FM_PUT )
					{
						if( nvmIncomingMsg.dataLength == sizeof(AES_CONFIG) ) 
						{
                            status = FM_hermesFlashPageUpdate(&nvmIncomingMsg, FLASH_BASE+(FM_SECTOR_AES*FLASH_SECTOR_SIZE_BYTES));
							/* Flush the cache, so subsequent read will be from flash */
							DCACHE_InvalidateByRange((uint32_t)(FLASH_BASE+(FM_SECTOR_AES*FLASH_SECTOR_SIZE_BYTES)), nvmIncomingMsg.dataLength);
							FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
							FM_crcUpdateAreaCode(FM_CRC_KEYS);
						} else
						{
							FM_hermesFlashNotifyResponse(kStatus_Fail, nvmIncomingMsg.xClientTaskHandle);
						}
					} else
					{ 	/* Get PRODUCT_CONFIG from NV Store */
						FM_hermesFlashElementRead(nvmIncomingMsg.ptrToBuf, (FLASH_BASE+(FM_SECTOR_AES*FLASH_SECTOR_SIZE_BYTES)), nvmIncomingMsg.dataLength, nvmIncomingMsg.xClientTaskHandle);
					}
					break;

				case FM_PERSISTENT_DATA:
					if( nvmIncomingMsg.action == FM_UPDATE )
					{
						if( nvmIncomingMsg.dataLength == sizeof(PERSISTENT_DATA) ) 
						{	
							status = FM_hermesFlashPageUpdate(&nvmIncomingMsg, FM_PERSISTENT_PAGE_ADDR);
							/* Flush the cache, so subsequent read will be from flash */
							DCACHE_InvalidateByRange((uint32_t)FM_PERSISTENT_PAGE_ADDR, nvmIncomingMsg.dataLength);
							FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
							FM_crcUpdateAreaCode(FM_CRC_PERSISTENT);
						} else
						{
							FM_hermesFlashNotifyResponse(kStatus_Fail, nvmIncomingMsg.xClientTaskHandle);
						}
					}
					break;
				
				case FM_ERASE_SECTOR:
                    if ((nvmIncomingMsg.sectorNum > FM_SECTOR_EO_STARTUP) && (nvmIncomingMsg.sectorNum <= FM_SECTOR_PERSISTENT))             // Exclude startup code and persistent data for now
					{
						status = FM_hermesFlashEraseSector(nvmIncomingMsg.sectorNum * SECTOR_SIZE);
						FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
					} else
					{
						FM_hermesFlashNotifyResponse(kStatus_Fail, nvmIncomingMsg.xClientTaskHandle);
					}
					break;
				
				case FM_TEST:                                           // Used for dev testing
					FM_crcUpdateAreaCode(FM_CRC_PERSISTENT);
					break;
					
				case FM_CREDENTIAL:
					status = hermesFlashWriteCredential(&nvmIncomingMsg);
					FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
					break;
					
				case FM_GENERIC_WRITE:
					status = kStatus_Success;
					while( (kStatus_Success == status) && (nvmIncomingMsg.dataLength > 0) )
					{
						status = FM_hermesFlashPageWrite(nvmIncomingMsg.offsetSectorAddr - FLASH_BASE, nvmIncomingMsg.ptrToBuf);
						__MEMORY_BARRIER();
				        DCACHE_InvalidateByRange(nvmIncomingMsg.offsetSectorAddr, FLASH_PAGE_SIZE_BYTES);							
						// Read it back via FlexSPI to avoid translation caused by BEE
						FM_hermesFlashPageRead(nvmIncomingMsg.offsetSectorAddr - FLASH_BASE, flash_page_read_buf);												
						for( i=0; i<FLASH_PAGE_SIZE_BYTES; i++ )
						{
							if( flash_page_read_buf[i] != nvmIncomingMsg.ptrToBuf[i] )
							{
								status = kStatus_Fail;
							}
						}
						if( kStatus_Fail == status) zprintf(CRITICAL_IMPORTANCE,"Verify fail\r\n");
						nvmIncomingMsg.dataLength -= FLASH_PAGE_SIZE_BYTES;
						nvmIncomingMsg.offsetSectorAddr += FLASH_PAGE_SIZE_BYTES;
						nvmIncomingMsg.ptrToBuf += FLASH_PAGE_SIZE_BYTES;
					}
					FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
					break;
					
				case FM_GENERIC_ERASE:
					status = kStatus_Success;
					while( (kStatus_Success == status) && (nvmIncomingMsg.dataLength > 0) )
					{
						status = FM_hermesFlashEraseSector(nvmIncomingMsg.offsetSectorAddr - FLASH_BASE);
						nvmIncomingMsg.dataLength -= FLASH_SECTOR_SIZE_BYTES;
						nvmIncomingMsg.offsetSectorAddr += FLASH_SECTOR_SIZE_BYTES;
					}
					FM_hermesFlashNotifyResponse(status, nvmIncomingMsg.xClientTaskHandle);
					break;
					
				
			} //switch nvmIncomingMsg.type
		} // if( xQueueReceive...
    } // while loop
}

bool hermesFlashRequestErase(uint8_t* target, uint32_t size, bool await)
{
	SEND_TO_FM_MSG	flash_erase_message = {FM_PUT, FM_GENERIC_ERASE, NULL, size, 0, (int32_t)target, xTaskGetCurrentTaskHandle()};
	uint32_t		notifyValue			= xQueueSend(xNvStoreMailboxSend, &flash_erase_message, portMAX_DELAY);
	
	if( true == await )
	{
		if( (xTaskNotifyWait(0, 0, &notifyValue, portMAX_DELAY ) != pdTRUE) ||
	    	(notifyValue != FM_ACK) )	// nominal time to wait for notification, or use portMAX_DELAY if blocking
		{
			return false;
		}
	}
	
	return true;
}

bool hermesFlashRequestWrite(uint8_t* to_write, uint8_t* target, uint32_t size, bool await)
{
	SEND_TO_FM_MSG	flash_write_message = {FM_PUT, FM_GENERIC_WRITE, to_write, size, 0, (int32_t)target, xTaskGetCurrentTaskHandle()};
	uint32_t		notifyValue			= xQueueSend(xNvStoreMailboxSend, &flash_write_message, portMAX_DELAY);
	
	if( true == await )
	{
		if( (xTaskNotifyWait(0, 0, &notifyValue, portMAX_DELAY ) != pdTRUE) ||
	    	(notifyValue != FM_ACK) )	// nominal time to wait for notification, or use portMAX_DELAY if blocking
		{
			return false;
		}
	}
	
	return true;
	
}

bool hermesFlashRequestCredentialWrite(uint8_t* credential, void* target, uint32_t data_size)
{
	SEND_TO_FM_MSG	cred_message	= {FM_PUT, FM_CREDENTIAL, credential, data_size, 0, (int32_t)target, xTaskGetCurrentTaskHandle()};
	uint32_t		notifyValue		= xQueueSend(xNvStoreMailboxSend, &cred_message, portMAX_DELAY);
	
	if( (xTaskNotifyWait(0, 0, &notifyValue, portMAX_DELAY) != pdTRUE) ||
	   	(notifyValue != FM_ACK) )
	{
		return false;
	}
	
	return true;
}

static status_t hermesFlashWriteCredential(SEND_TO_FM_MSG* msg)
{
	status_t	status	= kStatus_Success;
	
	uint8_t* page_buffer = pvPortMalloc(FLASH_PAGE_SIZE_BYTES);
	if( NULL == page_buffer )
	{
		FM_hermesFlashNotifyResponse(kStatus_Fail, msg->xClientTaskHandle);
		return false;
	}
	
	status = FM_hermesFlashEraseSector(msg->offsetSectorAddr - FLASH_BASE);
	
	uint32_t buff_progress = 4;
	uint32_t data_progress = 0;
	uint32_t to_take;
	
	memset(page_buffer, 0, FLASH_PAGE_SIZE_BYTES);
	memcpy(page_buffer, &msg->dataLength, sizeof(msg->dataLength));
	
	while( (data_progress < msg->dataLength) && (status == kStatus_Success) )
	{
		to_take = msg->dataLength - data_progress;
		if( to_take > FLASH_PAGE_SIZE_BYTES - buff_progress )
		{
			to_take = FLASH_PAGE_SIZE_BYTES - buff_progress;
		}
		memcpy(&page_buffer[buff_progress], &msg->ptrToBuf[data_progress], to_take);	
		status = FM_hermesFlashPageWrite(msg->offsetSectorAddr - FLASH_BASE, page_buffer);
		
		msg->offsetSectorAddr += FLASH_PAGE_SIZE_BYTES;
		buff_progress = 0;
		data_progress += to_take;
		memset(page_buffer, 0, FLASH_PAGE_SIZE_BYTES);
	}
	
	vPortFree(page_buffer);
	return status;
}

/********************************************/
/**     Flash memory service functions     **/
/********************************************/


/**************************************************************
 * Function Name   : FM_hermesFlashCrcElementModify
 * Description     : Update a CRC element - read 2 pages - erase sector - modify CRC data - write 2 page
 * Inputs          : incoming_msg_buffer, target field address, data length
 * Outputs         : none
 * Returns         : status_t
 **************************************************************/
status_t FM_hermesFlashCrcElementModify(uint8_t* p_incomingData, uint32_t elementAddr, uint16_t dataLength)
{
    static uint8_t* p_ramSinglePageData;
    static uint8_t* p_ramSecondPageData;
    status_t status = kStatus_Fail;
    uint32_t elementOffset = elementAddr % FLASH_PAGE_SIZE_BYTES;
    uint32_t pageBase = elementAddr & PAGE_BOUNDARY_MASK;                       // page boundary
    uint32_t sectorBase = elementAddr & SECTOR_BOUNDARY_MASK;                   // sector boundary
    
    /*  This function has been modified to specifically handle the CRC update (1 word) in the Persistent store,
        where there are currently two pages of separate data structures...
            page 1 - Stores the CRCs for all the NV sectors.
            page 2 - Store the characterisation of Hermes e.g. led brightness, for POR continuity.
        Since a page modification requires a whole sector erase, the integrity of the second page must be maintained. */
    
    /* Copy pages into ram */
    p_ramSinglePageData = (uint8_t*) pvPortMalloc( FLASH_PAGE_SIZE_BYTES * sizeof(uint8_t) );    // create buffer for single page update
    p_ramSecondPageData = (uint8_t*) pvPortMalloc( FLASH_PAGE_SIZE_BYTES * sizeof(uint8_t) );    // create buffer for second page store
    if (p_ramSinglePageData && p_ramSecondPageData)
    {
        uint8_t* p_nvReadAddr = (uint8_t*)pageBase;                    
        FM_hermesFlashCopyPage(p_ramSinglePageData, p_nvReadAddr);                 // get 1st page from flash
        p_nvReadAddr = (uint8_t*)FM_PERSISTENT_PAGE_ADDR;                    
        FM_hermesFlashCopyPage(p_ramSecondPageData, p_nvReadAddr);                 // get 2nd page from flash
        status = FM_hermesFlashEraseSector(sectorBase - FLASH_BASE);               // erase the target sector
        
        if (status == kStatus_Success)
        {
            for (uint16_t i=0; i<dataLength; i++)                               // Overwrite the local data element with new data
            {
                *(p_ramSinglePageData+i+elementOffset) = *(p_incomingData+i);
            }
            
            status = FM_hermesFlashPageWrite(pageBase-FLASH_BASE, p_ramSinglePageData);
            status = FM_hermesFlashPageWrite(FM_PERSISTENT_PAGE_ADDR-FLASH_BASE, p_ramSecondPageData);

        }
        vPortFree((void*)p_ramSinglePageData);
        vPortFree((void*)p_ramSecondPageData);
    }
    return status;
}  


/**************************************************************
 * Function Name   : FM_hermesFlashPageUpdate
 * Description     : Update a page within a sector
 * Inputs          : incoming_msg_buffer, absolute page address
 * Outputs         : CRITICAL_IMPORTANCE zprintf indicates failure
 * Returns         : status_t
 **************************************************************/
status_t FM_hermesFlashPageUpdate(SEND_TO_FM_MSG* p_incomingMsg, uint32_t pageAddr)
{
    uint16_t i;
    static uint8_t* p_ramPageData;
    static uint8_t* p_ramSectorData;
    status_t status = kStatus_Success;
    uint32_t sectorBase = pageAddr & SECTOR_BOUNDARY_MASK;                      // sector boundary
    uint8_t pageNum = (pageAddr & 0xf00)>>8;
    bool success = true;

    // get the new incoming data page into our temp buffer
    p_ramPageData = (uint8_t*) pvPortMalloc( FLASH_PAGE_SIZE_BYTES * sizeof(uint8_t) );
    if (p_ramPageData)
    {
        for (i=0; i<FLASH_PAGE_SIZE_BYTES; i++)
        {
            *(p_ramPageData+i) = 0xff;
        }
        for (i=0; i<p_incomingMsg->dataLength; i++)
        {
            *(p_ramPageData+i) = *(p_incomingMsg->ptrToBuf+i);
        }

    
        // Now copy existing data + new page into buffer
        p_ramSectorData = (uint8_t*) pvPortMalloc( FLASH_PAGE_SIZE_BYTES * FLASH_PAGES_PER_SECTOR );  
        if (p_ramSectorData)
        {

            for (i=0; i<FLASH_PAGES_PER_SECTOR; i++)
            {
                if (i == pageNum)
                    FM_hermesFlashCopyPage(p_ramSectorData+(i*FLASH_PAGE_SIZE_BYTES), p_ramPageData);
                else
                    FM_hermesFlashCopyPage(p_ramSectorData+(i*FLASH_PAGE_SIZE_BYTES), (uint8_t*) sectorBase+(i*FLASH_PAGE_SIZE_BYTES));
            }
            
            status = FM_hermesFlashEraseSector(sectorBase - FLASH_BASE);        // erase the target sector
            if (status == kStatus_Success)
            {
                for (uint8_t i=0; i<FLASH_PAGES_PER_SECTOR; i++)
                {
                    /* Write new pages to NV Store */
                    status = FM_hermesFlashPageWrite(((sectorBase-FLASH_BASE)+(i*FLASH_PAGE_SIZE_BYTES)), p_ramSectorData+(i*FLASH_PAGE_SIZE_BYTES));
                    if (status != kStatus_Success)
                    {
                        success = false;
                        break;
                    }
                }
                if (success == false)
                {
                    zprintf(CRITICAL_IMPORTANCE,"FAILED write flash page %d, sector %0x\r\n", i, sectorBase);
                    return kStatus_Fail;
                }
    		}
            else
            {
                zprintf(CRITICAL_IMPORTANCE,"FAILED to erase sector %0x %d\r\n", sectorBase);
                return kStatus_Fail;
            }
        }
        else
        {
            zprintf(CRITICAL_IMPORTANCE,"FAILED TO pvPortMalloc 4K buffer for updating\r\n");
            vPortFree((void*)p_ramPageData);                                    // this one did succeed, so free up the memory
            return kStatus_Fail;
        }
    }
	else
    {
		zprintf(CRITICAL_IMPORTANCE,"FAILED TO pvPortMalloc page buffer for updating\r\n");
        return kStatus_Fail;
    }
    
    vPortFree((void*)p_ramSectorData);
    vPortFree((void*)p_ramPageData);
    return status;
}  



/**************************************************************
 * Function Name   : FM_hermesFlashUpdateDevStats
 * Description     : Update the Device Stats in NV Store.
 * Inputs          : incoming_msg_buffer, field address
 * Outputs         : none
 * Returns         : void
 **************************************************************/
void FM_hermesFlashUpdateDevStats(SEND_TO_FM_MSG* p_incomingMsg, uint32_t elementAddr)
{
    static uint8_t* p_ramMultiPageData;

    status_t status = kStatus_Fail;
    uint8_t numOfPages;
    
    /* Copy a pages into ram */
    p_ramMultiPageData = (uint8_t*) pvPortMalloc( FLASH_PAGE_SIZE_BYTES * 16 ); // buffer to handle multi page updates 
    if (p_ramMultiPageData)
    {
        /* determine how many pages this input data will require */
        if (p_incomingMsg->dataLength % FLASH_PAGE_SIZE_BYTES)
            numOfPages = (p_incomingMsg->dataLength/FLASH_PAGE_SIZE_BYTES)+1;
        else
            numOfPages = p_incomingMsg->dataLength/FLASH_PAGE_SIZE_BYTES; 

        for (uint8_t i=0; i<numOfPages; i++)
        {
            /* Copy pages into ram */
            FM_hermesFlashCopyPage(p_ramMultiPageData+(i*FLASH_PAGE_SIZE_BYTES), p_incomingMsg->ptrToBuf+(i*FLASH_PAGE_SIZE_BYTES));
        }

        status = FM_hermesFlashEraseSector(elementAddr - FLASH_BASE);              // erase the target sector
        if (status == kStatus_Success)
        {
            for (uint8_t i=0; i<numOfPages; i++)
            {
                /* Write new pages to NV Store */
                status = FM_hermesFlashPageWrite(((elementAddr-FLASH_BASE)+(i*FLASH_PAGE_SIZE_BYTES)), p_ramMultiPageData+(i*FLASH_PAGE_SIZE_BYTES));
            }
        }
        vPortFree((void*)p_ramMultiPageData);
    }
	else
		zprintf(HIGH_IMPORTANCE,"FAILED TO pvPortMalloc 4K BUFFER FOR WRITING DEV STATS\r\n");
    FM_hermesFlashNotifyResponse(status, p_incomingMsg->xClientTaskHandle);
}  



/**************************************************************
 * Function Name   : FM_hermesFlashElementRead
 * Description     : Read data element in NV Store.
 * Inputs          : incoming_msg_buffer, field address, data length, TaskHandle_t
 * Outputs         : The read data is o/p via msg ptr, Notify response
 * Returns         : void
 **************************************************************/
void FM_hermesFlashElementRead(uint8_t* p_incomingMsg, uint32_t elementAddr, uint16_t dataLength, TaskHandle_t xClientTaskHandle)
{
    uint8_t* flash_read_ptr = (uint8_t*)( elementAddr );
    
    for (uint16_t i=0; i<dataLength; i++)
    {
        *(p_incomingMsg+i) = *(flash_read_ptr+i);
    }
    
    FM_hermesFlashNotifyResponse(kStatus_Success, xClientTaskHandle);
}



/**************************************************************
 * Function Name   : FM_hermesFlashReadDevStats
 * Description     : Read the Device Stats in NV Store.
 * Inputs          : incoming_msg_buffer, field address
 * Outputs         : The read data is o/p via msg ptr
 * Returns         : void
 **************************************************************/
void FM_hermesFlashReadDevStats(SEND_TO_FM_MSG* p_incomingMsg, uint32_t elementAddr)
{
    uint8_t* flash_read_ptr = (uint8_t*)( elementAddr );
    for (uint16_t i=0; i<p_incomingMsg->dataLength; i++)
    {
        *(p_incomingMsg->ptrToBuf+i) = *(flash_read_ptr+i);
    }
    
    FM_hermesFlashNotifyResponse(kStatus_Success, p_incomingMsg->xClientTaskHandle);
}



/**************************************************************
 * Function Name   : FM_hermesFlashPageWrite
 * Description     : Write a ram data page to NV Store.
 * Inputs          : page address (e.g. 0x003fc000), ptr to ram data
 * Outputs         : none
 * Returns         : status of page write
 **************************************************************/
status_t FM_hermesFlashPageWrite(uint32_t pageAddr, uint8_t *p_ramSinglePageData)
{
    status_t status = kStatus_Fail;
    taskENTER_CRITICAL();
	__MEMORY_BARRIER();
    status = flexspi_nor_flash_page_program(FLEXSPI, pageAddr, (const uint32_t*)p_ramSinglePageData);
    taskEXIT_CRITICAL();
    
    return status;
}

/**************************************************************
 * Function Name   : FM_hermesFlashPageRead
 * Description     : Read a Flash Page to RAM. Note this avoids the effects
 *                 : of the BEE and it's memory translation and decryption.
 * Inputs          : page address (e.g. 0x003fc000), ptr to ram data
 * Outputs         : none
 * Returns         : status of page read
 **************************************************************/
status_t FM_hermesFlashPageRead(uint32_t pageAddr, uint8_t *p_ramSinglePageData)
{
    status_t status = kStatus_Fail;
    taskENTER_CRITICAL();
	__MEMORY_BARRIER();
	status = flexspi_nor_flash_page_read(FLEXSPI, pageAddr, (const uint32_t*)p_ramSinglePageData);
    taskEXIT_CRITICAL();
    return status;
}

/**************************************************************
 * Function Name   : FM_hermesFlashSectorWrite
 * Description     : Write 16 pages of ram data page to target sector
 * Inputs          : sector address (e.g. 0x60200000), ptr to ram data
 * Outputs         : none
 * Returns         : status of page write
 **************************************************************/
status_t FM_hermesFlashSectorWrite(uint32_t sectorAddr, uint8_t *p_fwData)
{
    status_t status = kStatus_Fail;
    uint32_t pageNum;
    
    for (pageNum = 0; pageNum < FLASH_SECTOR_SIZE_BYTES; pageNum += FLASH_PAGE_SIZE_BYTES, p_fwData += FLASH_PAGE_SIZE_BYTES)
    {
        status = FM_hermesFlashPageWrite(sectorAddr+pageNum-FLASH_BASE, p_fwData);
        if (status != kStatus_Success)
            break;
    }
    
    return status;
}



/**************************************************************
 * Function Name   : FM_hermesFlashEraseSector
 * Description     : Erase a sector. There are 1024 4k sectors.
 * Inputs          : Offset addr of sector e.g. 0x003FC000
 * Outputs         : none
 * Returns         : status_t
 **************************************************************/
/* NOTE: block erase 64k is faster ...nxp has no routine for this? */
status_t FM_hermesFlashEraseSector(uint32_t sectorAddr)
{
    status_t status = kStatus_Fail;
    taskENTER_CRITICAL();
    status = flexspi_nor_flash_erase_sector(FLEXSPI, sectorAddr);
    taskEXIT_CRITICAL();
    return status;
}  



/**************************************************************
 * Function Name   : FM_hermesFlashCopyPage
 * Description     : Copy a page from flash to ram buffer.
 * Inputs          : Ptr to ram buffer, Page address start
 * Outputs         : The 256k data buffer is filled
 * Returns         : void
 **************************************************************/
void FM_hermesFlashCopyPage(uint8_t* p_ramSinglePageData, uint8_t* p_nvReadAddr)
{
    uint16_t i;
    for (i=0; i<256; i++)
    {
        *(p_ramSinglePageData+i) = *(p_nvReadAddr+i);
    }
}  


// This will probably be deprecated in favour of TaskNotify below
/**************************************************************
 * Function Name   : FM_hermesFlashResponseMsg
 * Description     : Construct a reply message.
 * Inputs          : ACK/NAK, length of data handled (as a check), echo the requested action
 * Outputs         : Format message struct
 * Returns         : void
 **************************************************************/
void FM_hermesFlashResponseMsg(status_t status, uint32_t dataLength, FM_STORE_ACTION action)
{
    static RESP_FROM_FM_MSG nvmResponseMsg;
    
    if( status == kStatus_Success )
    {
        nvmResponseMsg.action = FM_ACK;
        nvmResponseMsg.action |= action;
        nvmResponseMsg.dataLength = dataLength;
    } else
    {
        nvmResponseMsg.action = FM_NAK;
        nvmResponseMsg.action |= action;
    }
    xQueueSend(xNvStoreMailboxResp, &nvmResponseMsg, 0);
}  



/**************************************************************
 * Function Name   : FM_hermesFlashNotifyResponse
 * Description     : Reply with a Notification.
 * Inputs          : ACK/NAK, the recipient's Task Handle 
 * Outputs         : TaskNotification
 * Returns         : void
 **************************************************************/
void FM_hermesFlashNotifyResponse(status_t status, TaskHandle_t p_ClientTaskHandle)
{
    if( status == kStatus_Success )
    {
        xTaskNotify(p_ClientTaskHandle, FM_ACK, eSetValueWithOverwrite );
    } else
    {
        xTaskNotify(p_ClientTaskHandle, FM_NAK, eSetValueWithOverwrite );
    }
    
}  



/**************************************************************
 * Function Name   : FM_crcIntegrityCheck
 * Description     : Check NV store pages against saved crc's.
 * Inputs          : void
 * Outputs         : none
 * Returns         : Success/Fail
 **************************************************************/
#define GET_NV_CRC(x)   ((uint16_t*)( FLASH_BASE+(FM_SECTOR_PERSISTENT*FLASH_SECTOR_SIZE_BYTES)+(x)))
status_t FM_crcIntegrityCheck(void)
{
    status_t crcStatus = kStatus_Success;
    uint16_t crcAreaVal;
    uint16_t* p_crcVal;
    uint32_t i;

    for (i=FM_CRC_DEV_STATS; i<(FM_CRC_PERSISTENT); i++)                        // From FM_SECTOR_STATS(1016) to (1022)
    {
        crcAreaVal = FM_crcCalcAreaCode((CRC_DATA_AREA)i);                         // calculate CRC each sector                           
        p_crcVal = GET_NV_CRC((i-FM_CRC_DEV_STATS)*2);                          // Get the stored CRC, passing the offset store posn
        if (*p_crcVal == 0xffff)
        {
//            zprintf(HIGH_IMPORTANCE,"\r\nSector %d = 0xffff\r\n",i);            // not yet been stored, so ignore
        }
        else
        {
            if ( crcAreaVal != *p_crcVal )                                      // compare calculated with stored value
            {
                crcStatus = kStatus_Fail;
                zprintf(CRITICAL_IMPORTANCE,"Sector %d CRC failed... %x\r\n",i, *p_crcVal);
            }
            else
            {
//                zprintf(HIGH_IMPORTANCE,"Sector %d CRC is ok... %x\r\n",i, *p_crcVal);
            }
        }
    }
    return crcStatus;
}  

/**************************************************************
 * Function Name   : FM_store_persistent_data
 * Description     : Stores a parameter in the persistent store
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void FM_store_persistent_data(PERSISTENT_STORE_INDEX index, uint32_t param)
{					   
    static SEND_TO_FM_MSG persistentMessage;
    static PERSISTENT_DATA persistentDataRecord;
    uint32_t notifyValue;
	
    PERSISTENT_DATA *pNvPersistent = (PERSISTENT_DATA*)FM_PERSISTENT_PAGE_ADDR; // Persistent data page
    memcpy(&persistentDataRecord,pNvPersistent,sizeof(PERSISTENT_DATA));        // get existing settings from the NV page

	switch (index)
	{
		case PERSISTENT_BRIGHTNESS:
			persistentDataRecord.brightness = (LED_MODE)param;                             // update the stored value
			break;
		case PERSISTENT_PRINTLEVEL:
			persistentDataRecord.printLevel = (uint8_t)param;
			break;
		case PERSISTENT_RF_CHANNEL:
			persistentDataRecord.RF_channel = (uint8_t)param;
			break;
		default:
			break;
	}

    // now send new data to FM
    persistentMessage.ptrToBuf	        = (uint8_t *)&persistentDataRecord;
    persistentMessage.type		        = FM_PERSISTENT_DATA;
    persistentMessage.dataLength	    = sizeof(PERSISTENT_DATA);
    persistentMessage.action		    = FM_UPDATE;
    persistentMessage.xClientTaskHandle = xTaskGetCurrentTaskHandle();		
	
    xQueueSend(xNvStoreMailboxSend, &persistentMessage, 0);

	if( xTaskNotifyWait(0, 0, &notifyValue, portMAX_DELAY ) == pdTRUE )
	{
		if( notifyValue == FM_ACK )
		{
			zprintf(LOW_IMPORTANCE,"Product persistent data written\r\n");
		} else
		{
			zprintf(HIGH_IMPORTANCE,"Product persistent data write failed\r\n");
		}
	} else
	{
		zprintf(HIGH_IMPORTANCE,"Notification of Product persistent data Write did not arrive\r\n");
	}
	DbgConsole_Flush();
}

/**************************************************************
 * Function Name   : FM_crcUpdateAreaCode
 * Description     : Store the CRC for given area.
 * Inputs          : Area type
 * Outputs         : Update CRC code
 * Returns         : Flash write status
 **************************************************************/
status_t FM_crcUpdateAreaCode(CRC_DATA_AREA dataArea)
{
    status_t status = kStatus_Fail;
    uint16_t crcVal;
    uint16_t* p_crcVal = &crcVal;
    
    crcVal = FM_crcCalcAreaCode(dataArea);                                         // get crc for sector
    
    switch(dataArea)
    {
        case FM_CRC_DEV_STATS:
            status = FM_hermesFlashCrcElementModify((uint8_t*)p_crcVal, (FLASH_BASE+(FM_CRC_PERSISTENT*FLASH_SECTOR_SIZE_BYTES))+FM_CRC_DEV_STATS_POSN, sizeof(uint16_t));
        
        break;
          
        case FM_CRC_MAC_SERIAL:
            status = FM_hermesFlashCrcElementModify((uint8_t*)p_crcVal, (FLASH_BASE+(FM_CRC_PERSISTENT*FLASH_SECTOR_SIZE_BYTES))+FM_CRC_MAC_SERIAL_POSN, sizeof(uint16_t));
        
        break;
          
        case FM_CRC_FT_DATA:
            status = FM_hermesFlashCrcElementModify((uint8_t*)p_crcVal, (FLASH_BASE+(FM_CRC_PERSISTENT*FLASH_SECTOR_SIZE_BYTES))+FM_CRC_FT_DATA_POSN, sizeof(uint16_t));
        
        break;
          
        case FM_CRC_KEYS:
            status = FM_hermesFlashCrcElementModify((uint8_t*)p_crcVal, (FLASH_BASE+(FM_CRC_PERSISTENT*FLASH_SECTOR_SIZE_BYTES))+FM_CRC_KEYS_POSN, sizeof(uint16_t));
        
        break;
          
        case FM_CRC_PERSISTENT:
            status = FM_hermesFlashCrcElementModify((uint8_t*)p_crcVal, (FLASH_BASE+(FM_CRC_PERSISTENT*FLASH_SECTOR_SIZE_BYTES))+FM_CRC_PERSISTENT_POSN, sizeof(uint16_t));
        
        break;


        default:
        
        break;
    }
    DCACHE_InvalidateByRange(FLASH_BASE+(FM_SECTOR_PERSISTENT*FLASH_SECTOR_SIZE_BYTES), FLASH_PAGE_SIZE_BYTES);


    return status;
}    



/**************************************************************
 * Function Name   : FM_crcCalcAreaCode
 * Description     : Calculate CRC for given area.
 * Inputs          : Area type
 * Outputs         : none
 * Returns         : 16-bit CRC code
 **************************************************************/
uint16_t FM_crcCalcAreaCode(CRC_DATA_AREA dataArea)
{
    return CRC16( (uint8_t*)(FLASH_BASE+(dataArea * FLASH_SECTOR_SIZE_BYTES)), FLASH_SECTOR_SIZE_BYTES ,0);
}  



