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
* Filename: Hermes-test.c   
* Author:   Tony Thurgood 27/09/2019
* Purpose:   
*   
* Test cases are invoked by the CLI shell commands
*            
**************************************************************************/

#ifdef HERMES_TEST_MODE

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
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
#include "queue.h"

/*Other includes*/
#include "Hermes-test.h"
#include "Devices.h"
#include "hermes-time.h"
#include "SureNet-Interface.h" 
#include "NetworkInterface.h"

#define LOCK_FUSES_0X400	0
#define CHIP_ID_LOW_0X410   1
#define CHIP_ID_HIGH_0X420  2
#define BEE_BOOT_FUSE_0X460	6
#define UART_FUSE_0X470	    7
#define BEE_KEY1_SEL_VAL	0x0000C000
#define BOOT_FUSE_SEL_VAL	0x00000010
#define LOCK_FUSE_VAL	    0x0C203C4C
#define CFG1_FUSE_VAL	    0x0CD3C010
#define CFG2_FUSE_VAL	    0x00000010

// Externals
extern QueueHandle_t xNvStoreMailboxSend;
extern QueueHandle_t xNvStoreMailboxResp;
extern DEVICE_STATUS test_device_status[];                                      // from Hermes_var_init
extern PRODUCT_CONFIGURATION product_configuration;	// This is a RAM copy of the product info from Flash.
extern TaskHandle_t xHermesTestTaskHandle;      // declared in hermes.c
extern TaskHandle_t xFM_hermesFlashTaskHandle;     //      "
extern bool remove_pairing_table_entry(uint64_t mac);

// Globals and statics
SEND_TO_FM_MSG nvTestNvmMessage;                                                // Outgoing message

typedef union
{
  uint8_t configBuffer[100];
  PRODUCT_CONFIGURATION  prodConfig;
  AES_CONFIG  aesConfig;
  FACTORY_TEST_DATA ftConfig;
} prodConfigBuf_t; 
 
prodConfigBuf_t prodConfigBuf;
uint8_t*		pConfigBuffer = prodConfigBuf.configBuffer;
static TEST_ACTION	testAction;
static uint8_t		cliDataBuff[40];
uint64_t pairingTargetRfMac = 0x70b3d5f9c0000de1;                               // initially use default MAC



/**************************************************************
 * Function Name   : hermesTestTask
 * Description     : FreeRtos do forever task - monitor incoming messages
 * Inputs          : pvParameters - std parameter
 * Outputs         : none
 * Returns         : void
 **************************************************************/
void hermesTestTask(void *pvParameters)
{
    uint32_t notifyValue;
    
// Incoming message queue    
    RESP_FROM_FM_MSG nvmResponseMsg;

    while(1)
    {
        uint16_t rxdBytes;
        FM_STORE_ACTION rxd_action;
        
        // Check for incoming messages
        if( xQueueReceive( xNvStoreMailboxResp, &nvmResponseMsg, pdMS_TO_TICKS(5) ) == pdPASS )
        {
            taskENTER_CRITICAL();
            rxd_action = nvmResponseMsg.action;
            if ( rxd_action &= FM_ACK)
            {
                hermes_test_printf("\r\nHermes Test - received ACK for sent message\r\n");
                rxd_action = nvmResponseMsg.action;
                if (rxd_action &= FM_GET)
                {
                    for (rxdBytes=0; rxdBytes<nvmResponseMsg.dataLength; rxdBytes++)
                    {
                        hermes_test_printf(" %01x",*(pConfigBuffer+rxdBytes));
                    }
                    hermes_test_printf("\r\nHermes Test - expected msg length = %d, rx'd length = %d\r\n\n", nvmResponseMsg.dataLength, rxdBytes);
                }
            }
            else
            {
                hermes_test_printf("\r\nHermes Test - received NAK for sent message\r\n");
            }
            taskEXIT_CRITICAL();
            
        }
        
        
        if (xTaskNotifyWait(3, 0, &notifyValue, 100 ) == pdTRUE)                  // nominal time to wait for notification, or use portMAX_DELAY if blocking
        {
            uint32_t flashEvent = notifyValue;
            if (flashEvent == FM_NAK)
            {
				hermes_test_printf("\r\nNAK");
            }
            else if (flashEvent == FM_ACK)
            {
                if ((nvTestNvmMessage.type == FM_PRODUCT_CONFIG && nvTestNvmMessage.action == FM_PUT) || (nvTestNvmMessage.type == FM_AES_DATA && nvTestNvmMessage.action == FM_PUT) \
                     || (nvTestNvmMessage.type == FM_FACTORY_TEST && nvTestNvmMessage.action == FM_PUT) )
                { 	// PRODUCT_CONFIG, FM_AES_DATA and FM_FACTORY_TEST set, does a get first, so don't need the 2nd response
                } 
                else
                {
                    hermesTestProcessResponse();                                       // ACK received, process repsonse
                }
            }
        }
    }
}



/**************************************************************
 * Function Name   : hermesTestRunCase()
 * Description     : Called by CLI commands to run a test case
 * Inputs          : uint8 - Test Case number to run
 *                 : int8_t* - input string
 * Outputs         : Prints to terminal
 * Returns         :
 **************************************************************/
void hermesTestRunCase(uint8_t testCase, int8_t *pcParameterString)
{
    char testCaseStr[100];

    /* Number is entered in the Hermes CLI shell */
    switch (testCase)
    {
        case 1:
        {
        /* Set Ethernet MAC Number */
            uint8_t length = strlen((char*)pcParameterString);                  // get the inccoming data into our buffer
            uint8_t i;
            uint8_t j = 0;
            bool fail = false;

            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            
            length = strlen((char const*)cliDataBuff);
            if (length != 12)
            {
                fail = true;
            }
            
            if (!fail)
            {
                for (i=0; i<6; i++)
                {
                    if ((cliDataBuff[j] > '/') && (cliDataBuff[j] < ':'))
                    {
                        product_configuration.ethernet_mac[i] = (cliDataBuff[j]-48)<<4;           // 0 to 9 = 48 to 57
                    }
                    else if ((cliDataBuff[j] > '@') && (cliDataBuff[j] < 'G'))
                    {
                        product_configuration.ethernet_mac[i] = (cliDataBuff[j]-55)<<4;           // A to F = 65 to 70
                    }
                    else if ((cliDataBuff[j] > 96) && (cliDataBuff[j] < 'g'))
                    {
                        product_configuration.ethernet_mac[i] = (cliDataBuff[j]-87)<<4;           // a to f = 97 to 102
                    }
                    else
                    {
                        fail = true;
                    }
                    j++;
                    if ((cliDataBuff[j] > '/') && (cliDataBuff[j] < ':'))
                    {
                        product_configuration.ethernet_mac[i] |= cliDataBuff[j] - 48;              // 0 to 9 = 48 to 57
                    }
                    else if ((cliDataBuff[j] > '@') && (cliDataBuff[j] < 'G'))
                    {
                        product_configuration.ethernet_mac[i] |= cliDataBuff[j] - 55;              // A to F = 65 to 70
                    }
                    else if ((cliDataBuff[j] > 96) && (cliDataBuff[j] < 'g'))
                    {
                        product_configuration.ethernet_mac[i] |= cliDataBuff[j] - 87;              // a to f = 97 to 102
                    }
                    else
                    {
                        fail = true;
                    }
                    j++;
                }
                hermes_test_printf("Ethernet Mac written.\r\n");
            }
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            
            if (fail)
            {
                hermes_test_printf("FAIL-Ethernet Mac must be 6 octets 0-9, a-f, A-F\r\n");
            }
        }
        break;
        
        case 2:
        {
            /* Get Ethernet MAC Number */    

            uint8_t i;
            uint8_t j = 0;
            uint8_t ethernetMacAscii[13];
            uint8_t *pByte = ethernetMacAscii;
            for(i=0; i<6; i++)
            {
                if ((product_configuration.ethernet_mac[i]>>4) < 10)
                {
                    ethernetMacAscii[j] = (product_configuration.ethernet_mac[i]>>4) + 48;         // 0 to 9 = 48 to 57
                }
                else
                {
                    ethernetMacAscii[j] = (product_configuration.ethernet_mac[i]>>4) + 55;         // a to f = 65 to 70
                }
                j += 1;
                if ((product_configuration.ethernet_mac[i]&0xf) < 10)
                {
                    ethernetMacAscii[j] = (product_configuration.ethernet_mac[i]&0xf) + 48;         // 0 to 9 = 48 to 57
                }
                else
                {
                    ethernetMacAscii[j] = (product_configuration.ethernet_mac[i]&0xf) + 55;         // a to f = 65 to 70
                }
                j++;
            }
            
            for (i=0; i<12; i++)
            {
                hermes_test_printf("%c",pByte[i]);
                i++;
                hermes_test_printf("%c",pByte[i]);
                if (i!=11)
                    hermes_test_printf(":");
            }
            hermes_test_printf("\r\n");
        }
        break;


        case 3:
        {
        /* Set RF MAC Number */
            
            // get the inccoming data into our buffer
            uint8_t length = strlen((char*)pcParameterString);
            uint8_t i;
            uint8_t		cnt		 = 0;
            uint64_t	newRfMac = 0;
            char*	ptr  = (char*)cliDataBuff;

            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';
            
            length  = strlen((char const*)cliDataBuff);
            if (length != 16)
            {
                hermes_test_printf("FAIL-RF Mac must be 8 octets 0-9, a-f, A-F\r\n");
                break;
            }
            
            while (cnt < 0x10 )
            {
                newRfMac <<= 4;
                newRfMac += hex_ascii_to_int(ptr);                              // non hex chars return a zero WHICH IS VALID ...could be -1?
                ptr++;
                cnt++;
            }
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            product_configuration.rf_mac = newRfMac;
            hermes_test_printf("RF Mac written.\r\n");
        }
        break;

        
        case 4:
        {
            /* Get RF MAC Number */    
            uint8_t *pByte = (uint8_t *)&product_configuration.rf_mac;
            int8_t i;
            for (i=8; i>0; i--)
            {
                hermes_test_printf("%02X",pByte[i-1]);
                if (i!=1)
                    hermes_test_printf(":");
            }
            hermes_test_printf("\r\n");
        }
        break;

        
        case 5:
        {
            /* Set Serial Number */ 
            uint8_t length = strlen((char*)pcParameterString);                  // get the inccoming data into our buffer
            uint8_t i;
            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';                                            // terminate the string 
            
            length = strlen((char const*)cliDataBuff);
            if( length != 12 ) 
            {
                hermes_test_printf("FAIL-Serial Number must be 12 ascii chars\r\n");
                break;
            }
            for( i=0; i<13; i++ )
            {
                product_configuration.serial_number[i] = cliDataBuff[i];
            }
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            hermes_test_printf("Serial number written.\r\n");
        }
        break;
        
        
        case 6:
        {
            /* Get Serial Number */
            hermes_test_printf("%s\r\n", product_configuration.serial_number);
        }
        break;


        case 7:
        {
        /* Set RF PanId */
            size_t length = strlen((char*)pcParameterString);                  // get the inccoming data into our buffer
            uint8_t i;
            uint8_t cnt = 0;
            uint32_t newRfPanid = 0;
            char*	ptr		= (char*)cliDataBuff;

            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';                                            // terminate the string to allow string checking
            
            length = strlen((char const*)cliDataBuff);
            if( length != 4 )
            {
                hermes_test_printf("FAIL-RF PanId must be 4 octets 0-9, a-f, A-F\r\n");
                break;
            }
            
            while( cnt < 4 )
            {
                newRfPanid <<= 4;
                newRfPanid += hex_ascii_to_int(ptr);                            // non hex chars return a zero WHICH IS VALID ...could be -1?
                ptr++;
                cnt++;
            }
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            product_configuration.rf_pan_id = newRfPanid;
            hermes_test_printf("RF PanId written.\r\n");
        }
        break;

        
        case 8:
        {
            /* Get RF PanId */    
            hermes_test_printf("%04x\r\n", product_configuration.rf_pan_id);               
        }
        break;

        
        case 9:
        {
        /* write Device Data to NV Store */
            uint16_t len=0;
            uint16_t strux;
            // Initialise a buffer with example Device stats structure    
            uint8_t devStatsBuff[] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, \
                                     1, 0, 0x15, 1, 0xFF, 0xEE, 0xDD, 0xCC,0xBB,0xAA, \
                                     0x99, 0x88, 0x77, 0x66, 0x55, 1, 0x44, 0x33};
            
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Send a SET Device Stats message\r\n");
            
            for (strux=0; strux<(32*sizeof(devStatsBuff)); strux+=sizeof(devStatsBuff))
            {
                for (uint16_t i=0; i<sizeof(devStatsBuff); i++)
                {
                    *(pConfigBuffer+i+strux) = devStatsBuff[i];
                    len++;
                }
            }

            nvTestNvmMessage.ptrToBuf	= pConfigBuffer;
            nvTestNvmMessage.dataLength	= 26*32;
            nvTestNvmMessage.type		= FM_DEVICE_STATS;
            nvTestNvmMessage.action		= FM_PUT;
            nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
        }
        break;

        
        case 10:
        {
        /* Get Device Data */
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Send a GET Device Stats message\r\n");
            nvTestNvmMessage.ptrToBuf	= pConfigBuffer;
            nvTestNvmMessage.type		= FM_DEVICE_STATS;
            nvTestNvmMessage.dataLength	= 26*32;
            nvTestNvmMessage.action		= FM_GET;
            nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
        }
        break;

        
        case 13:
        {
        /* Erase sector */
            testAction = ERASE_SECTOR;                                          // Helps to process the response
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Send an erase sector message\r\n");
            uint32_t sectorNum			= atoi((char*)pcParameterString);
			
            nvTestNvmMessage.type		= FM_ERASE_SECTOR;
            nvTestNvmMessage.sectorNum	= sectorNum;
            nvTestNvmMessage.action		= FM_ERASE;
            nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
        }
        break;
        
        break;
        
        case 15:
        {
        /* Set aes_high */
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Write aes_high\r\n");
            testAction = SET_AES_HIGH;
            
            // get the inccoming data into our buffer
            uint8_t length = strlen((char*)pcParameterString);
            uint8_t i;
            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';                                            // terminate the string to allow string checking
            
            // The aes_high is a AES_CONFIG struct member, so need to get existing info from flash
            getAesConfig();
        }
        break;
        
        case 16:
        {
            /* Get aes_high */    
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Get aes_high\r\n");
            testAction = GET_AES_HIGH;
            getAesConfig();

        }
        break;
        
        case 17:
        {
        /* Set aes_low */
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Write aes_low\r\n");
            testAction = SET_AES_LOW;
            
            // get the inccoming data into our buffer
            uint8_t length = strlen((char*)pcParameterString);
            uint8_t i;
            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';                                            // terminate the string to allow string checking
            
            // The aes_high is a AES_CONFIG struct member, so need to get existing info from flash
            getAesConfig();
        }
        break;
        
        case 18:
        {
            /* Get aes_low */    
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Get aes_low\r\n");
            testAction = GET_AES_LOW;
            getAesConfig();

        }
        break;
        
        case 19:
        {
        /* Set Secret Serial number */
            uint8_t length = strlen((char*)pcParameterString);                  // get the inccoming data into our buffer
            uint8_t i;
            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';                                            // terminate the string to allow string checking
            
            length = strlen((char const*)cliDataBuff);
            if( length != 32 ) 
            {
                hermes_test_printf("FAIL-Secret Serial Number must be 32 chars\r\n");
                break;
            }
            for( i=0; i<33; i++ )
            {
                product_configuration.secret_serial[i] = cliDataBuff[i];
            }
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            hermes_test_printf("Secret serial written.\r\n");
        }
        break;
        
        case 20:
        {
            /* Get Secret Serial number */    
            hermes_test_printf("%s\r\n", product_configuration.secret_serial);
        }
        break;
        
        case 21:
        case 23:
        case 25:
        case 27:
        {
        /* Write factory test data */
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Write factory test Revision Number\r\n");
            if ( testCase == 21)
                testAction = SET_FT_REV_NUM;
            if ( testCase == 23)
                testAction = SET_FT_STDBY_MA;
            if ( testCase == 25)
                testAction = SET_FT_GREEN_MA;
            if ( testCase == 27)
                testAction = SET_FT_RED_MA;
            
            // get incoming data into our buffer
            uint8_t length = strlen((char*)pcParameterString);
            uint8_t i;
            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';                                            // terminate the string to allow string checking
            
            // The factory test Revision Number is a TEST_DATA struct member, so need to get existing info from flash
            getFTdata();
        }
        break;
        
        case 22:
        {
            /* Get factory test Revision Number */    
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - get factory test Revision Number\r\n");
            testAction = GET_FT_REV_NUM;
            getFTdata();

        }
        break;
        
        case 24:
        {
            /* Get factory test standby current */    
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - get factory test standby current\r\n");
            testAction = GET_FT_STDBY_MA;
            getFTdata();

        }
        break;
        
        case 26:
        {
            /* Get factory test Green led current */    
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - get factory test green current\r\n");
            testAction = GET_FT_GREEN_MA;
            getFTdata();

        }
        break;
        
        case 28:
        {
            /* Get factory test Red led current */    
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - get factory test red current\r\n");
            testAction = GET_FT_RED_MA;
            getFTdata();

        }
        break;

        case 29:
        {
        /* Set factory test results */
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Write factory test results\r\n");
            testAction = SET_FT_PASS_RESULTS;
            
            // get incoming data into our buffer
            uint8_t length = strlen((char*)pcParameterString);
            uint8_t i;
            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';                                            // terminate the string to allow string checking
            
            // The factory test Revision Number is a TEST_DATA struct member, so need to get existing info from flash
            getFTdata();
        }
        break;
        
        case 30:
        {
            /* Get factory test results */    
            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - get factory test results\r\n");
            testAction = GET_FT_PASS_RESULTS;
            getFTdata();

    }
        break;

        case 31:
        {
        /* Set the product_state - deprecated*/
//            snprintf(testCaseStr, sizeof(testCaseStr), "\r\nHermes Test - Update the product_state\r\n");
//            testAction = SET_PRODUCT_STATE;
//            
//            // get incoming data into our buffer
//            uint8_t length = strlen((char*)pcParameterString);
//            uint8_t i;
//            for (i=0; i<length; i++)
//            {
//                cliDataBuff[i] = *(pcParameterString+i);
//            }
//            cliDataBuff[i+1] = '\0';                                            // terminate the string to allow string checking
//            
//            getProductConfig();
            }
        break;
            
        case 32:
        {
        /* Write the PRODUCT_CONFIGURATION from ram struct to flash */
            write_product_configuration();
//            hermes_test_printf("...written product configuration\r\n");
        }
        break;
            
        case 42:
        {
        /* get the unique chip id from NXP fuse registers */
            uint32_t val32;
            uint64_t chipId = 0;
            
            ocotp_read_once(OCOTP, CHIP_ID_HIGH_0X420, &val32, 4);
            hermes_test_printf("UID=%08X", val32);
            chipId |= (uint64_t)val32;
            chipId = chipId<<32;
            ocotp_read_once(OCOTP, CHIP_ID_LOW_0X410, &val32, 4);
            hermes_test_printf("%08X\r\n", val32);
            chipId |= (uint64_t)val32;                                          // 64 bit value, if we need it?
        }
        break;
        
            
        case 50:
        {
        /* Read an eFuse index address. The index param is 0-7 */
        /* e.g. to read Cfg1 (BEE and BOOT), index = 6 */
            uint32_t val;
            uint32_t idxNum = atoi((char*)pcParameterString);
            if( idxNum > 7 )
            {
                hermes_test_printf("FAIL-eFuse Index Address must be a digit 0-7\r\n");
                break;
            }
            
            ocotp_read_once(OCOTP, idxNum, &val, 4);
            hermes_test_printf("%X\r\n", val);
        }
        break;

        
        case 51:
        {
        /* Burn BEE_KEY1_SEL = 0000 C000 */
        // Note: The Programmer does this using the spc_flashloader command
            uint32_t val = 0;
            status_t status;
            val |= BEE_KEY1_SEL_VAL;
            status = ocotp_program_once(OCOTP, BEE_BOOT_FUSE_0X460, &val, 4);
            if (status == kStatus_Success)
                hermes_test_printf("Success!\r\n");
            else
                hermes_test_printf("FAIL-BEE_KEY1_FUSE burn error %d\r\n", status);
        }
        break;

        
        case 52:
        {
        /* Burn BOOT_FUSE_SEL	0x0000 0010 */
            uint32_t val = 0;
            status_t status;
            val |= BOOT_FUSE_SEL_VAL;
            status = ocotp_program_once(OCOTP, BEE_BOOT_FUSE_0X460, &val, 4);
            if (status == kStatus_Success)
                hermes_test_printf("Success!\r\n");
            else
                hermes_test_printf("FAIL-BOOT_FUSE burn error %d\r\n", status);
        }
        break;

        
        case 53:
        {
        /* Burn LOCK_FUSES */
            uint32_t val = 0;
            status_t status;
            uint32_t burnFail = 0;
            
            // CFG1 reg 0x460 - fuses
            val = CFG1_FUSE_VAL;
            status = ocotp_program_once(OCOTP, BEE_BOOT_FUSE_0X460, &val, 4);
            if (status == kStatus_Success)
            {
                hermes_test_printf("Blow reg 0x460\r\n");
            
                // CFG2 reg 0x470 - fuses
                val = CFG2_FUSE_VAL;
                status = ocotp_program_once(OCOTP, UART_FUSE_0X470, &val, 4);
                if (status == kStatus_Success)
                {
                    hermes_test_printf("Blow reg 0x470\r\n");
                
                    // LOCK reg 0x400 -  fuses
                    val = LOCK_FUSE_VAL;
                    status = ocotp_program_once(OCOTP, LOCK_FUSES_0X400, &val, 4);
                    if (status == kStatus_Success)
                         hermes_test_printf("Blow reg 0x400\r\n");
                    else
                        burnFail = 0x400;
                }
                else
                    burnFail = 0x470;
            }
            else
                burnFail = 0x460;

            if (burnFail != 0)
                hermes_test_printf("FAIL - FUSE BURN reg %x, error %d\r\n",burnFail, status);
            else
                hermes_test_printf("Success! - all lock fuses blown\r\n");
                
        }
        break;
        
        case 73:
        {
        /* Set pairing target RF MAC */
            // get the inccoming data into our buffer
            uint8_t length = strlen((char*)pcParameterString);
            uint8_t i;
            uint8_t		cnt		 = 0;
            uint64_t	newRfMac = 0;
            char*	ptr  = (char*)cliDataBuff;

            for (i=0; i<length; i++)
            {
                cliDataBuff[i] = *(pcParameterString+i);
            }
            cliDataBuff[i+1] = '\0';
            
            length  = strlen((char const*)cliDataBuff);
            if (length != 16)
            {
                hermes_test_printf("FAIL-RF Mac must be 8 octets 0-9, a-f, A-F\r\n");
                break;
            }
            
            while (cnt < 0x10 )
            {
                newRfMac <<= 4;
                newRfMac += hex_ascii_to_int(ptr);                              // non hex chars return a zero WHICH IS VALID ...could be -1?
                ptr++;
                cnt++;
            }
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            pairingTargetRfMac = newRfMac;
            hermes_test_printf("Target Mac written.\r\n");
        }
        break;
        
        case 74:
        {
            /* Get pairing target RF MAC */    
            uint8_t *pByte = (uint8_t *)&pairingTargetRfMac;
            int8_t i;
            for (i=8; i>0; i--)
            {
                hermes_test_printf("%02X",pByte[i-1]);
                if (i!=1)
                    hermes_test_printf(":");
            }
            hermes_test_printf("\r\n");
        }
        break;

        
//        case 75:
//        initialise SureNet
        
        case 76:
        {
            // Run SureNet pairing test
            extern QueueHandle_t xAssociationSuccessfulMailbox;
            static ASSOCIATION_SUCCESS_INFORMATION assoc_test_info;
            ASSOCIATION_SUCCESS_INFORMATION *test_assc = &assoc_test_info;
            test_assc->association_addr = pairingTargetRfMac;                   // set MAC address with testcase 73
            test_assc->association_dev_type = DEVICE_TYPE_FEEDER;
            test_assc->association_dev_rssi = 0;

            add_mac_to_pairing_table(test_assc->association_addr);              // store MAC in pairing table
            add_device_characteristics_to_pairing_table(test_assc->association_addr,test_assc->association_dev_type,test_assc->association_dev_rssi);    
            store_device_table();	                                            // write result to NVM
            xQueueSend(xAssociationSuccessfulMailbox,test_assc,0);              // put result in mailbox for Surenet-Interface
        }
        break;
            
        case 77:
        {
        /* Add a MAC (created by testcase 73) to the pairing table. */
            static ASSOCIATION_SUCCESS_INFORMATION assoc_test_info;
            ASSOCIATION_SUCCESS_INFORMATION *test_assc = &assoc_test_info;
            test_assc->association_addr = pairingTargetRfMac;                   // set MAC address with testcase 73
            test_assc->association_dev_type = DEVICE_TYPE_FEEDER;
            test_assc->association_dev_rssi = 0;

            add_mac_to_pairing_table(test_assc->association_addr);              // store MAC in pairing table
            add_device_characteristics_to_pairing_table(test_assc->association_addr,test_assc->association_dev_type,test_assc->association_dev_rssi);    
            store_device_table();	                                            // write result to NVM
        }
        break;
            
        case 78:
        {
        /* Wipe pairing table */
            remove_pairing_table_entry(0);
        }
        break;
            
        case 168:
        {
            /* Switch IP to DHCP, start Surenet and complete the server connection*/
            FreeRTOS_SetDHCPMode(pdTRUE);
            xSendEventToIPTask(eNetworkDownEvent);
            initSureNetByProgrammer();
            connectToServer();
        }
        break;
        
        case 192:
        {
            /* TCP/IP Factory test - socket connection */ 
            Socket_t xSocket;
            struct freertos_sockaddr xRemoteAddress;
            int32_t result = 0;
            const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

            hermes_test_printf("Setting up Socket.\r\n");

            xRemoteAddress.sin_port = FreeRTOS_ntohs( 1800 );
            xRemoteAddress.sin_addr = FreeRTOS_inet_addr_quick( 192, 168, 0, 1 );

            /* Create a socket. */
            xSocket = FreeRTOS_socket( FREERTOS_AF_INET,
                                       FREERTOS_SOCK_STREAM,/* FREERTOS_SOCK_STREAM for TCP. */
                                       FREERTOS_IPPROTO_TCP );
            
            if ( xSocket == FREERTOS_INVALID_SOCKET)
            {
                hermes_test_printf("FAILed to create socket\r\n");
                break;
            }
//            configASSERT( xSocket != FREERTOS_INVALID_SOCKET );
            
            result = FreeRTOS_connect(xSocket, &xRemoteAddress, sizeof(xRemoteAddress));
            if( pdFREERTOS_ERRNO_NONE != result )
            {
                hermes_test_printf("FAIL... %d\r\n", result);
            }
            else
            {
                hermes_test_printf("Success!\r\n");
                snprintf(testCaseStr, 22, "     \r\nHi from Hermes");

                FreeRTOS_send(xSocket, testCaseStr, sizeof(testCaseStr), 0);
            }
            vTaskDelay( xDelay );                                               // wait for the Programmer test to run
            FreeRTOS_closesocket( xSocket );
        }
        break;
            
        case 193:
        {
            // Wake up the ethernet for the Programmer, give the IP stack a kick
            xSendEventToIPTask(eNetworkDownEvent);
        }
        break;
            
        case 194:
        {
            BaseType_t cableConnected;
            cableConnected = testLinkStatus();
            if (cableConnected == pdFAIL)
                hermes_test_printf("FAIL - LAN cable not detected.\r\n");
            else
                hermes_test_printf("LAN cable connected.\r\n");
        }
        break;

        
    }
    if ( (testCase == 13) || (testCase > 20 &&  testCase < 31) )                // Testcases that interact with flashmanager via RTOS massage queue
    xQueueSend(xNvStoreMailboxSend, &nvTestNvmMessage, 0);
}


/**************************************************************
 * Function Name   : hermesTestProcessResponse()
 * Description     : Handles the FM response
 * Inputs          : 
 *                 : 
 * Outputs         : 
 * Returns         :
 **************************************************************/
void hermesTestProcessResponse()
{
    switch (testAction)
    {
// This function comleted the action request after reading configs from flash, before modify.
// The product_configuration is now handled by the global struct+methods, so this part is deprecated
//        case SET_ETHERNET_MAC:
//        case GET_ETHERNET_MAC:
//        case SET_RF_MAC:
//        case GET_RF_MAC:
//        case GET_SERIAL_NUMBER:
//        case SET_SECRET_SERIAL:
//        case GET_SECRET_SERIAL:
//        case SET_RF_PANID:
//        case GET_RF_PANID:
//        case SET_PRODUCT_STATE:

      
        case ERASE_SECTOR:
        {
            hermes_test_printf("Sector has been erased!\r\n");               
        }
        break;

        
        case SET_AES_HIGH:
        {
            uint8_t		cnt	= 0;
            uint64_t	newAes = 0;
            bool	fail = false;
            char*	ptr  = (char*)cliDataBuff;
            size_t	len  = strlen((char const*)cliDataBuff);
            if (len != 16)
            {
                hermes_test_printf("FAIL-AES high must be 16 hex chars 0-9, a-f, A-F\r\n");
                break;
            }
            
            while (cnt < 16 )
            {
                newAes <<= 4;
                newAes += hex_ascii_to_int(ptr);                              
                ptr++;
                cnt++;
            }
            if (cnt != 16)
                fail = true;
            
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            
            if (fail == true)
            {
                hermes_test_printf("FAIL-AES high must be 16 hex chars 0-9, a-f, A-F\r\n");
            } else
            {
                prodConfigBuf.aesConfig.aes_high = newAes;
                // now send updated config
                nvTestNvmMessage.ptrToBuf	= (uint8_t *)&prodConfigBuf;
                nvTestNvmMessage.type		= FM_AES_DATA;
                nvTestNvmMessage.dataLength	= sizeof(AES_CONFIG);
                nvTestNvmMessage.action		= FM_PUT;
                nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
				
                xQueueSend(xNvStoreMailboxSend, &nvTestNvmMessage, 0);
            }
        }
        break;

        
        case GET_AES_HIGH:
        {
            uint8_t *pByte = (uint8_t *)&prodConfigBuf.aesConfig.aes_high;
            int8_t i;
            for (i=8; i>0; i--)
                hermes_test_printf("%02X",pByte[i-1]);
            hermes_test_printf("\r\n");
        }
        break;

        
        case SET_AES_LOW:
        {
            uint8_t		cnt	= 0;
            uint64_t	newAes = 0;
            bool	fail = false;
            char*	ptr  = (char*)cliDataBuff;
            size_t	len  = strlen((char const*)cliDataBuff);
            if (len != 16)
            {
                hermes_test_printf("FAIL-AES low must be 16 hex chars 0-9, a-f, A-F\r\n");
                break;
            }
            
            while (cnt < 16 )
            {
                newAes <<= 4;
                newAes += hex_ascii_to_int(ptr);                              
                ptr++;
                cnt++;
            }
            if (cnt != 16)
                fail = true;
            
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            
            if (fail == true)
            {
                hermes_test_printf("FAIL-AES low must be 16 hex chars 0-9, a-f, A-F\r\n");
            } else
            {
                prodConfigBuf.aesConfig.aes_low = newAes;
                // now send updated config
                nvTestNvmMessage.ptrToBuf	= (uint8_t *)&prodConfigBuf;
                nvTestNvmMessage.type		= FM_AES_DATA;
                nvTestNvmMessage.dataLength	= sizeof(AES_CONFIG);
                nvTestNvmMessage.action		= FM_PUT;
                nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
				
                xQueueSend(xNvStoreMailboxSend, &nvTestNvmMessage, 0);
            }
        }
        break;

        
        case GET_AES_LOW:
        {
            uint8_t *pByte = (uint8_t *)&prodConfigBuf.aesConfig.aes_low;
            int8_t i;
            for (i=8; i>0; i--)
                hermes_test_printf("%02X",pByte[i-1]);
            hermes_test_printf("\r\n");
        }
        break;

        
        
// Factory test data        
        case SET_FT_REV_NUM:
        case SET_FT_STDBY_MA:
        case SET_FT_GREEN_MA:
        case SET_FT_RED_MA:
        {
            uint8_t cnt = 0;
            uint32_t newTestData = 0;
            bool	fail	= false;
            char*	ptr		= (char*)cliDataBuff;
            size_t	len		= strlen((char const*)cliDataBuff);
            if( len != 3 )
            {
                hermes_test_printf("FAIL - Test data must be 3 octets 0-9, a-f, A-F\r\n");
                memset(cliDataBuff, 0, sizeof cliDataBuff);                     // clear the buffer for next usage
                break;
            }
            
            while( cnt < 3 )
            {
                newTestData <<= 4;
                newTestData += hex_ascii_to_int(ptr);                           // non hex chars return a zero WHICH IS VALID ...could be -1?
                ptr++;
                cnt++;
            }
            if( cnt != 3 )
                fail = true;
            
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            
            if( fail == true )
            {
                hermes_test_printf("FAIL - Test data must be 3 octets 0-9, a-f, A-F\r\n");
            } else
            {
                // Update the test element with new data
                if (testAction == SET_FT_REV_NUM)
                    prodConfigBuf.ftConfig.testRevNum = newTestData;
                if (testAction == SET_FT_STDBY_MA)
                    prodConfigBuf.ftConfig.standby_mA = newTestData;
                if (testAction == SET_FT_GREEN_MA)
                    prodConfigBuf.ftConfig.green_mA = newTestData;
                if (testAction == SET_FT_RED_MA)
                    prodConfigBuf.ftConfig.red_mA = newTestData;
                
                // now send updated config
                nvTestNvmMessage.ptrToBuf	= (uint8_t *)&prodConfigBuf;
                nvTestNvmMessage.type		= FM_FACTORY_TEST;
                nvTestNvmMessage.dataLength	= sizeof(FACTORY_TEST_DATA);
                nvTestNvmMessage.action		= FM_PUT;
                nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
				
                xQueueSend(xNvStoreMailboxSend, &nvTestNvmMessage, 0);
            }
        }
        break;

        
        case SET_FT_PASS_RESULTS:
        {
            uint8_t cnt = 0;
            uint8_t newDigit = 0;
            uint32_t newTestData = 0;
            bool	fail	= false;
            char*	ptr		= (char*)cliDataBuff;
            size_t	len		= strlen((char const*)cliDataBuff);
            if( len != 5 )
            {
                hermes_test_printf("FAIL - Pass/Fail test data must be 5 boolean digits\r\n");
                memset(cliDataBuff, 0, sizeof cliDataBuff);                     // clear the buffer for next usage
                break;
            }
            
            while( cnt < 5 )
            {
                newTestData <<= 4;
                newDigit = hex_ascii_to_int(ptr);
                if (newDigit > 1)
                    break;
                
                newTestData += newDigit;                            
                ptr++;
                cnt++;
            }
            if( cnt != 5 )
                fail = true;
            
            memset(cliDataBuff, 0, sizeof cliDataBuff);                         // clear the buffer for next usage
            
            if( fail == true )
            {
                hermes_test_printf("FAIL - Pass/Fail test data must be a boolean\r\n");
            } else
            {
                // Update the test element with new data
                prodConfigBuf.ftConfig.passResults.all = newTestData;
                
                // now send updated config
                nvTestNvmMessage.ptrToBuf	= (uint8_t *)&prodConfigBuf;
                nvTestNvmMessage.type		= FM_FACTORY_TEST;
                nvTestNvmMessage.dataLength	= sizeof(FACTORY_TEST_DATA);
                nvTestNvmMessage.action		= FM_PUT;
                nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
				
                xQueueSend(xNvStoreMailboxSend, &nvTestNvmMessage, 0);
            }
        }
        break;
        
        case GET_FT_REV_NUM:
        {
            hermes_test_printf("%03x\r\n", prodConfigBuf.ftConfig.testRevNum);               
        }
        break;

        case GET_FT_STDBY_MA:
        {
            hermes_test_printf("%03x\r\n", prodConfigBuf.ftConfig.standby_mA);               
        }
        break;

        case GET_FT_GREEN_MA:
        {
            hermes_test_printf("%03x\r\n", prodConfigBuf.ftConfig.green_mA);               
        }
        break;

        case GET_FT_RED_MA:
        {
            hermes_test_printf("%03x\r\n", prodConfigBuf.ftConfig.red_mA);               
        }
        break;

        case GET_FT_PASS_RESULTS:
        {
            hermes_test_printf("%05x\r\n", prodConfigBuf.ftConfig.passResults.all);               
        }
        break;
    }
    
}



/**************************************************************
 * Function Name   : getProductConfig
 * Description     : 
 * Inputs          : 
 * Outputs         : none
 * Returns         : void
 **************************************************************/
void getProductConfig()
{ 	// Read product_configuration from flash
	nvTestNvmMessage.ptrToBuf	= pConfigBuffer;
	nvTestNvmMessage.type		= FM_PRODUCT_CONFIG;
	nvTestNvmMessage.dataLength = sizeof(PRODUCT_CONFIGURATION);
	nvTestNvmMessage.action		= FM_GET;
	nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
}



/**************************************************************
 * Function Name   : getAesConfig
 * Description     : 
 * Inputs          : 
 * Outputs         : none
 * Returns         : void
 **************************************************************/
void getAesConfig()
{ 	// Read aes data from flash
	nvTestNvmMessage.ptrToBuf	= pConfigBuffer;
	nvTestNvmMessage.type		= FM_AES_DATA;
	nvTestNvmMessage.dataLength = sizeof(AES_CONFIG);
	nvTestNvmMessage.action		= FM_GET;
	nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
}



/**************************************************************
 * Function Name   : getFTdata
 * Description     : 
 * Inputs          : 
 * Outputs         : none
 * Returns         : void
 **************************************************************/
void getFTdata()
{ 	// Read factory test data from flash
	nvTestNvmMessage.ptrToBuf	= pConfigBuffer;
	nvTestNvmMessage.type		= FM_FACTORY_TEST;
	nvTestNvmMessage.dataLength = sizeof(FACTORY_TEST_DATA);
	nvTestNvmMessage.action		= FM_GET;
	nvTestNvmMessage.xClientTaskHandle = xHermesTestTaskHandle;
}

#endif //HERMES_TEST_MODE
