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
* Filename: hermes.c   
* Author:   Chris Cowdery
* Purpose:   
*   
* Main start point for Hermes firmware (after low level startup)
*            
**************************************************************************/
// Project wide includes that can affect anything!
#include "hermes.h"
#include <stdarg.h>	// used by zprintf
#include <stdio.h>	// for vsprintf
/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "timers.h"
#include "hermes-time.h"
/* FreeRTOS-TCP includes */
#include "FreeRTOS_IP.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
//#include "fsl_debug_console_conf.h"
#include "fsl_log.h"    // for LOG_Readline - entry into CLI
#include "fsl_pwm.h"
#include "board.h"
#include "fsl_wdog.h"
#include "fsl_pwm.h"
#include "fsl_xbara.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_flexspi_ram.h"
#include "fsl_iomuxc.h"
#include "fsl_cache.h"

// Other includes
#include "FreeRTOS_CLI.h"
#include "SureNet-interface.h"
#include "hermes-app.h"
#include "MQTT.h"
#include "SNTP.h"
#include "leds.h"
#include "Hermes-test.h"
#include "flashManager.h"
#include "resetHandler.h"
#include "HTTP_Helper.h"
#include "myMAC.h"		// just needed for initialisation
#include "Hermes-app.h"
#include "LabelPrinter.h"
#include "SureNet.h"	// for INITAL_CHANNEL
#include "HubFirmwareUpdate.h"
#include "BankManager.h"
#include "Signing.h"
#include "fault.h"
#include "Babel.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void shell_task(void *pvParameters);
void init_watchdog();

/* The CLI commands are defined in CLI-commands.c. */
void vRegisterBasicCLICommands(void);
void vRegisterCLICommands( void );

static const uint8_t ucIPAddress_printer[ 4 ] = { 192,168,0,2 };
static const uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 };
static const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 };
static const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 };
static const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 };
static uint8_t initial_RF_channel;
BaseType_t xTCstatus;
static WDOG_Type *wdog_base = WDOG1;

TaskHandle_t xHermesTestTaskHandle;
TaskHandle_t xFM_hermesFlashTaskHandle;
TaskHandle_t xStartupTaskHandle;
TaskHandle_t xHTTPPostTaskHandle;
TaskHandle_t xHFUTaskHandle;

extern QueueHandle_t xNvStoreMailboxSend;
extern QueueHandle_t xNvStoreMailboxResp;

//Global variable with some useful product information within. Initialised from Flash on boot.
bool shouldIPackageFlag = false;
bool shouldICryptFlag = false;
bool amILocked = true;
bool global_message_trace_flag = false;	// turned on by messagedump CLI command
PRODUCT_CONFIGURATION product_configuration;	// This is a RAM copy of the product info from Flash.
// local functions
void write_product_configuration(void);
static void StartUpTask(void *pvParameters);
static void watchdog_task(void *pvParameters);
static void service_watchdog(void);
uint8_t printLevel = LOW_IMPORTANCE;
void Core_detect(void);
bool immediate_firmware_update = false;
uint64_t	rfmac;	// global because a pointer to this variable is used by the RF Stack
bool rf_channel_override = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */

// This is a bit of debug code that can be uncommented out (along with it's
// enabling code in BOARD_ConfigInterrupts())
//void BUTTON_IRQ_HANDLER(void)   // overrides weakly linked handler in startup_MIMXRT1021.s
//{
//	LED_SET_3(LEDS_ON);
//	while(1);
//}
int Init(void)
{ 
	flexspi_nor_flash_init(FLEXSPI);    // Enables all 4Mb flash

	BOARD_ConfigMPU();  // Set permissions on memories
    BOARD_InitPins();
    BOARD_ConfigGPIOPins();
    BOARD_BootClockRUN();
	enable_spread_spectrum();
	turn_off_unneccessary_clocks();

    BOARD_InitDebugConsole();
    BOARD_ConfigTimers();
    BOARD_ConfigInterrupts();
    BOARD_ConfigSPI();      // should be checked for kStatus_Success
    BOARD_ConfigPwm();
    BOARD_ConfigTRNG();	// random number generator

#if USE_LEDS_FOR_DEBUG
	// temporary for debug - connects GPIO2 to LEDs
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_23_GPIO2_IO23,0U); // PWMLED4 - Green
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_24_GPIO2_IO24,0U); // PWMLED3 - Red
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_25_GPIO2_IO25,0U); // PWMLED2 - Green
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_26_GPIO2_IO26,0U); // PWMLED1 - Red
	LED_SET_3(LEDS_OFF);
#endif

#ifdef DEBUG
#warning DEBUG build is deprecated. DO NOT USE
#endif
	
    /* enable watchdog - set for 16 secs. Only active in Release configuration*/
    init_watchdog();

	BM_ResolveBankUse();	// work out whether we are in Bank A or Bank B

	if( 0==GPIO_PinRead(BUTTON_GPIO, BUTTON_GPIO_PIN))
	{
		uint32_t buttondown = get_microseconds_tick();
		while(0==GPIO_PinRead(BUTTON_GPIO, BUTTON_GPIO_PIN));
		if( (get_microseconds_tick()-buttondown)<5*usTICK_SECONDS)
			immediate_firmware_update = true;	// tact pressed at power up, so let's just do f/w update
		else
			rf_channel_override=true;	// meant for blocking_test
	}

    // Set up LED driver
    led_driver_init();

	/* Register the command line commands with the CLI */
	vRegisterBasicCLICommands();

	// Initialise the Flash Manager
	if( kStatus_Fail == FM_hermesFlashInit())
	{
		while(1);	// panic??
	}

    if (xTaskCreate(StartUpTask, "Startup", STARTUP_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xStartupTaskHandle) != pdPASS)
    {
        while(1);
	}

	vTaskStartScheduler();
	for (;;)
        ;
}

/**************************************************************
 * Function Name   : StartUpTask
 * Description     : This handles the retrieval of config data from flash
 *                 : and then the startup of all the other tasks.
 *                 : When complete, it kills itself.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
static void StartUpTask(void *pvParameters)
{
	SEND_TO_FM_MSG	nvNvmMessage;    // Outgoing message	to Flash Manager
    uint32_t		notifyValue;
    PERSISTENT_DATA *pNvPersistent = (PERSISTENT_DATA*)FM_PERSISTENT_PAGE_ADDR; // Persistent data page
	BANK_DESCRIPTOR	new_descriptor;
	extern const volatile BANK_DESCRIPTOR*	CurrentDescriptor;
	BANK_DESCRIPTOR *OtherBankDescriptor;
	uint32_t		i;

	// the first thing we need is the flash handler so we can get info from the Flash
    if( xTaskCreate(FM_hermesFlashTask, "Hermes_Flash_Manager", FLASH_MANAGER_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xFM_hermesFlashTaskHandle) != pdPASS )
    {
        zprintf(CRITICAL_IMPORTANCE, "Hermes Flash Manager task creation failed!.\r\n");
	}

	// First activity is to check if we were reset via a watchdog trigger, if so, update
	// the bank descriptor.
	// Note this might benefit from being done earlier, i.e. before this task starts, but that
	// would mean using a new interface to the flash memory as the Flash Manager is task-based.

	if( (0 != (SRC_SRSR_WDOG_RST_B_MASK & SRC->SRSR)) )
	{	// watchdog has fired.

		// Copy bank descriptor into RAM so we can update it before writing it back.
		memcpy(&new_descriptor, (uint8_t *)CurrentDescriptor, sizeof(BANK_DESCRIPTOR));

		new_descriptor.watchdog_resets++;
		if( new_descriptor.watchdog_resets >= MAX_WATCHDOG_RESETS)
		{ // we should roll back here, but only if the other bank has a valid and older revision available
			OtherBankDescriptor = CurrentDescriptor->opposite;
			if( (OtherBankDescriptor->bank_mark > BANK_MARK_UNINITIALISED) &&	// valid version
			    (OtherBankDescriptor->bank_mark < CurrentDescriptor->bank_mark) ) // older than this version
			{
				zprintf(CRITICAL_IMPORTANCE,"Max watchdog resets exceeded - rolling back (i.e. use other bank)\r\n");
				new_descriptor.bank_mark = BANK_MARK_ROLLED_BACK;	// we don't want to use this bank any more
			}
			// else cannot roll back
			else
				zprintf(LOW_IMPORTANCE,"Cannot roll back\r\n");
		}

		// Update the hash!
		new_descriptor.descriptor_hash = 0;
		for( i=4; i<sizeof(BANK_DESCRIPTOR)/sizeof(uint32_t); i++ )
		{
			new_descriptor.descriptor_hash ^= *(((uint32_t*)&new_descriptor)+i);
		}

		// write the descriptor back to flash
		hermesFlashRequestErase((uint8_t*)CurrentDescriptor, sizeof(BANK_DESCRIPTOR), true);
		hermesFlashRequestWrite((uint8_t*)&new_descriptor, (uint8_t*)CurrentDescriptor, sizeof(BANK_DESCRIPTOR), true);
		DCACHE_InvalidateByRange((uint32_t)CurrentDescriptor, sizeof(BANK_DESCRIPTOR));
		DbgConsole_Flush();

		// if we have set the bank mark to ROLLED_BACK, we will reset the core and flip to the other bank
		if( BANK_MARK_ROLLED_BACK == new_descriptor.bank_mark)
		{
			SRC->SRSR = SRC_SRSR_WDOG_RST_B_MASK;	// clear status bit in reset cause register.
			NVIC_SystemReset();
		}
	}

	// Now read product_configuration from flash
	nvNvmMessage.ptrToBuf	= (uint8_t *)&product_configuration;
	nvNvmMessage.type		= FM_PRODUCT_CONFIG;
	nvNvmMessage.dataLength	= sizeof(PRODUCT_CONFIGURATION);
	nvNvmMessage.action		= FM_GET;
	nvNvmMessage.xClientTaskHandle = xTaskGetCurrentTaskHandle();

    xQueueSend(xNvStoreMailboxSend, &nvNvmMessage, 0);	// get device table into RAM

    if( xTaskNotifyWait(0, 0, &notifyValue, portMAX_DELAY) == pdTRUE )
    {
		if( notifyValue == FM_ACK )
		{	// Got the values into product_configuration
        	printLevel = MEDIUM_IMPORTANCE;                                     // Limit the CLI messages when communicating with Programmer
			if( (product_configuration.sanity_state==PRODUCT_NOT_CONFIGURED) )//||
//				(myRFMAC != product_configuration.rf_mac) ||
//				(0 != strcmp(mySecretSerial, (char const *)product_configuration.secret_serial)) ||
//				(0 != strcmp(mySerial, (char const *)product_configuration.serial_number)) )	// **** THESE THREE LINES TEMPORARY FOR DEVELOPMENT TO KEEP PRODUCT_CONFIGURATION SYNCHRONISED WITH MYMAC.H
			{	// we need to put some sensible defaults in place
				// we will only overwrite the MAC addresses and serials if they are 0xff's. Otherwise we will
				// leave them as they were.
				if( (product_configuration.ethernet_mac[0] == 0xff) &&
					(product_configuration.rf_mac == 0xffffffffffffffff) &&
					(product_configuration.rf_pan_id == 0xffff))
				{	// we are pretty sure that the flash area is actually blank so we can put some sensible values in it
					uint8_t ethmac[6] = myEthMAC;
					memcpy(product_configuration.ethernet_mac, ethmac, 6);	// retrieve ethernet mac
					strcpy((char*)product_configuration.serial_number, mySerial);	// serial number
					strcpy((char*)product_configuration.secret_serial, mySecretSerial);
					product_configuration.rf_mac = myRFMAC;
					product_configuration.rf_pan_id = SUREFLAP_PAN_ID;
				}
				// but we put sensible values in this lot regardless
				product_configuration.sanity_state = PRODUCT_CONFIGURED;
				product_configuration.product_state = PRODUCT_BLANK;	// leave product_state ready for programming / test / label steps
				product_configuration.rf_mac_mangle = true;
				product_configuration.spare1 = 0;
				product_configuration.spare2 = 0;
				product_configuration.spare3 = 0;
				for( i=0; i<DERIVED_KEY_LENGTH; i++) { product_configuration.DerivedKey[i] = 0xff; }
				write_product_configuration();
				FM_store_persistent_data(PERSISTENT_BRIGHTNESS, (uint32_t)LED_MODE_NORMAL);
				FM_store_persistent_data(PERSISTENT_PRINTLEVEL, (uint32_t)printLevel);
				FM_store_persistent_data(PERSISTENT_RF_CHANNEL, (uint32_t)INITIAL_CHANNEL);
			}
		}
	}

	if( true == product_configuration.rf_mac_mangle)
	{ // we mangle it by putting 0xfffe in the middle, and use the 6 byte Ethernet one around it
	// This is because Pet Doors reject Hub MAC addresses that do not have 0xfffe in the middle
		((uint8_t *)&rfmac)[7] = product_configuration.ethernet_mac[0];
		((uint8_t *)&rfmac)[6] = product_configuration.ethernet_mac[1];
		((uint8_t *)&rfmac)[5] = product_configuration.ethernet_mac[2];
		((uint8_t *)&rfmac)[4] = 0xFF;
		((uint8_t *)&rfmac)[3] = 0xFE;
		((uint8_t *)&rfmac)[2] = product_configuration.ethernet_mac[3];
		((uint8_t *)&rfmac)[1] = product_configuration.ethernet_mac[4];
		((uint8_t *)&rfmac)[0] = product_configuration.ethernet_mac[5];
	}	// the apparent swapping of endianness is because the RF mac is stored as a uint64_t
		// but the Ethernet mac is stored as a byte array.
	else
	{
		rfmac = product_configuration.rf_mac;
	}

	// This must come before TCP initialisation as it sets up some mailboxes used by TCP/IP
    hermes_app_init();

	if( false == rf_channel_override)	// swap RF channels if button held down for >5 secs
	{
	initial_RF_channel = pNvPersistent->RF_channel;
	}
	else
	{
		if( RF_CHANNEL1 == pNvPersistent->RF_channel)
			initial_RF_channel = RF_CHANNEL3;
		else
			initial_RF_channel = RF_CHANNEL1;
	}

	switch (product_configuration.product_state)
	{
		case PRODUCT_NOT_CONFIGURED:	// should never get here!!
			shouldICryptFlag = false;
			shouldIPackageFlag = false;
        	printLevel = LOW_IMPORTANCE;
        	zprintf(MEDIUM_IMPORTANCE,"\r\n\r\nSure Petcare Hub - 'Hermes'\r\n---------------------------\r\n");
			zprintf(MEDIUM_IMPORTANCE,"PRODUCT NOT CONFIGURED\r\n");
			if( xTaskCreate(shell_task, "Shell_task", SHELL_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Shell task creation failed!\r\n");
			}
			if( xTaskCreate(watchdog_task, "watchdog_task", WATCHDOG_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Watchdog task creation failed! - tick... tick... tick...\r\n");
			}
			break;
		case PRODUCT_BLANK: // This is the state after a reset after firmware programming. It needs to support 'test'
			shouldICryptFlag = false;
			shouldIPackageFlag = false;
        	printLevel = MEDIUM_IMPORTANCE;                                     // Limit the CLI messages when communicating with Programmer
			if( xTaskCreate(shell_task, "Shell_task", SHELL_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Shell task creation failed!\r\n");
			}
			FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, product_configuration.ethernet_mac, pdFALSE, NORMAL_TASK_PRIORITY );
            /* The call to surenet_init() is now controlled by the Programmer, via CLI */
			if( xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
    		{
				zprintf(CRITICAL_IMPORTANCE,"LED task creation failed!\r\n");
    		}
			if (xTaskCreate(hermesTestTask, "Hermes_Test", TEST_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHermesTestTaskHandle) != pdPASS)
    		{
				zprintf(CRITICAL_IMPORTANCE,"Hermes Test task creation failed!.\r\n");
			}
			if( xTaskCreate(watchdog_task, "watchdog_task", WATCHDOG_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Watchdog task creation failed! - tick... tick... tick...\r\n");
			}
			break;
		case PRODUCT_TESTED:	// Now I want to talk to the Label Printer
		  	shouldICryptFlag = false;
			shouldIPackageFlag = false;
        	printLevel = LOW_IMPORTANCE;
        	zprintf(MEDIUM_IMPORTANCE,"\r\n\r\nSure Petcare Hub - 'Hermes'\r\n---------------------------\r\n");
			zprintf(MEDIUM_IMPORTANCE,"PRODUCT_TESTED - expecting Label Printer\r\n");
			if( xTaskCreate(shell_task, "Shell_task", SHELL_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Shell task creation failed!\r\n");
			}
			FreeRTOS_IPInit( ucIPAddress_printer, ucNetMask, ucIPAddress_printer, ucIPAddress_printer, product_configuration.ethernet_mac, pdFALSE, HIGH_TASK_PRIORITY );
			if( xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"LED task creation failed!\r\n");
			}
			if( xTaskCreate(label_task, "Label Printer task", LABEL_PRINTER_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Label printer creation failed!\r\n");
			}
			if( xTaskCreate(watchdog_task, "watchdog_task", WATCHDOG_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Watchdog task creation failed! - tick... tick... tick...\r\n");
			}
			break;
		case PRODUCT_CONFIGURED:	// everything set up, now I am ready for a customer
			GenerateSharedSecret(SHARED_SECRET_CURRENT);	// generate a truly random Shared Secret to be shared with the Server
			GenerateDerivedKey(DERIVED_KEY_CURRENT);	// Used for signing data transfers between Hub and Server.
					
			//Delay boot-up by 2 seconds.  The LEDs will be off for this period, giving us a visual indication if the device ever reboots.
			vTaskDelay(pdMS_TO_TICKS(2000));

			BABEL_set_aes_key(product_configuration.serial_number);
			BABEL_aes_encrypt_init();

		  	shouldIPackageFlag = SHOULD_I_PACKAGE;
			shouldICryptFlag = false;//true for AES Encryption

#ifdef DEBUG
        	printLevel = LOW_IMPORTANCE;
#else
			printLevel = CRITICAL_IMPORTANCE;
#endif
        	zprintf(CRITICAL_IMPORTANCE,"\r\n\r\nSure Petcare Hub - 'Hermes'\r\n---------------------------\r\n");
#ifdef DEBUG
			zprintf(CRITICAL_IMPORTANCE,"DEBUG build - NOT FOR RELEASE!\r\n");
#endif
			// Initialise Hub Firmware Update - this requires the Secret Serial to have been read
			// from flash into product_configuration.secret_serial for key calculation
			HFU_init();
			if( xTaskCreate(watchdog_task, "watchdog_task", WATCHDOG_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Watchdog task creation failed! - tick... tick... tick...\r\n");
			}
			if( false == immediate_firmware_update )
			{	// normal operation
				DisplayResetStatus();
				Core_detect();
				set_led_brightness(get_led_brightness(),false);     // from nvm
				update_led_view();
				// CONSIDER NOT STARTING THIS TASK FOR RELEASE BUILD...
				if( xTaskCreate(shell_task, "Shell_task", SHELL_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE,"Shell task creation failed!\r\n");
				}
				FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, product_configuration.ethernet_mac, pdTRUE, HIGH_TASK_PRIORITY );

				// Start SureNet

				surenet_init(&rfmac, product_configuration.rf_pan_id,initial_RF_channel); // This initialises stack variables and starts the surenet task.

				HTTPPostTask_init();

				SNTP_Init();
				if( xTaskCreate(SNTP_Task, "SNTP Task", SNTP_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE, "SNTP Task creation failed!\r\n");
				}

				if( xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE,"LED task creation failed!\r\n");
				}

				if( xTaskCreate(hermes_app_task, "Hermes Application", HERMES_APPLICATION_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE, "Hermes Application task creation failed!\r\n");
				}

				if( xTaskCreate(MQTT_Task, "MQTT", MQTT_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE,"MQTT Task creation failed!\r\n");
				}

				if (xTaskCreate(hermesTestTask, "Hermes_Test", TEST_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHermesTestTaskHandle) != pdPASS)
				{
					zprintf(CRITICAL_IMPORTANCE,"Hermes Test task creation failed!.\r\n");
				}

				if (xTaskCreate(HTTPPostTask, "HTTP Post", HTTP_POST_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHTTPPostTaskHandle) != pdPASS)
				{
					zprintf(CRITICAL_IMPORTANCE,"HTTP Post task creation failed!.\r\n");
				}
				if (xTaskCreate(HFU_task, "Hub Firmware Update", HFU_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHFUTaskHandle) != pdPASS)
				{
					zprintf(CRITICAL_IMPORTANCE,"Hub Firmware Update task creation failed!.\r\n");
				}
			} else	// 	immediate_firmware_update == true
			{		// So we don't need to do a lot here
				LED_Request_Pattern(LED_MODE_NORMAL, COLOUR_RED, LED_PATTERN_SOLID, 0);

				// CONSIDER NOT STARTING THIS TASK FOR RELEASE BUILD...
				if( xTaskCreate(shell_task, "Shell_task", SHELL_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE,"Shell task creation failed!\r\n");
				}
				FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, product_configuration.ethernet_mac, pdTRUE, HIGH_TASK_PRIORITY );

				HTTPPostTask_init();

				SNTP_Init();	// This is required as Signing requires UTC to be correct
				if( xTaskCreate(SNTP_Task, "SNTP Task", SNTP_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE, "SNTP Task creation failed!\r\n");
				}
				
				if( xTaskCreate(led_task, "led_task", LED_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
				{
					zprintf(CRITICAL_IMPORTANCE,"LED task creation failed!\r\n");
				}
				if (xTaskCreate(HTTPPostTask, "HTTP Post", HTTP_POST_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHTTPPostTaskHandle) != pdPASS)
				{
					zprintf(CRITICAL_IMPORTANCE,"HTTP Post task creation failed!.\r\n");
				}
				if (xTaskCreate(HFU_task, "Hub Firmware Update", HFU_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHFUTaskHandle) != pdPASS)
				{
					zprintf(CRITICAL_IMPORTANCE,"Hub Firmware Update task creation failed!.\r\n");
				}

				// Now we need to wait for DHCP, then trigger a SNTP fetch, and finally
				// kick off a Hub Firmware Update
				extern EventGroupHandle_t	xConnectionStatus_EventGroup;
				xEventGroupWaitBits(xConnectionStatus_EventGroup, CONN_STATUS_NETWORK_UP, false, false, portMAX_DELAY );
				while(false == SNTP_AwaitUpdate(true, portMAX_DELAY));			
				HFU_trigger(true);
			}
			break;
		default:	// Not sure what to do here - something's gone wrong with Flash
        	printLevel = LOW_IMPORTANCE;
        	zprintf(MEDIUM_IMPORTANCE,"\r\n\r\nSure Petcare Hub - 'Hermes'\r\n---------------------------\r\n");
			zprintf(MEDIUM_IMPORTANCE,"CONFIGURATION CORRUPTED\r\n");
			if( xTaskCreate(shell_task, "Shell_task", SHELL_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Shell task creation failed!\r\n");
			}
			if( xTaskCreate(watchdog_task, "watchdog_task", WATCHDOG_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
			{
				zprintf(CRITICAL_IMPORTANCE,"Watchdog task creation failed! - tick... tick... tick...\r\n");
			}
			break;
	}

	vTaskDelete(NULL);	// my work is done, now I fly to Switzerland.
}

/**************************************************************
 * Function Name   : set_product_state
 * Description     : Sets product state and writes to flash. To be called
 *                 : by production tester and label printer
 * Inputs          : one of PRODUCT_BLANK, PRODUCT_TESTED or PRODUCT_CONFIGURED
 * Outputs         :
 * Returns         :
 **************************************************************/
void set_product_state(PRODUCT_STATE state)
{
	product_configuration.product_state = state;
	write_product_configuration();
}

/**************************************************************
 * Function Name   : Sanitise Product Configuration
 * Description     : Writes PRODUCT_NOT_CONFIGURED to sanity_state, which
 *                 : will cause the NVM to be refreshed with sane values
 *                 : on the next processor reset. Useful if the format of
 *                 : NVM has changed and it needs initialising
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void sanitise_product_config(void)
{
	SEND_TO_FM_MSG	nvNvmMessage;    // Outgoing message	to Flash Manager
    uint32_t		notifyValue;

	nvNvmMessage.ptrToBuf	= (uint8_t *)&product_configuration;
	nvNvmMessage.type		= FM_PRODUCT_CONFIG;
	nvNvmMessage.dataLength	= sizeof(PRODUCT_CONFIGURATION);
	nvNvmMessage.action		= FM_GET;
	nvNvmMessage.xClientTaskHandle = xTaskGetCurrentTaskHandle();

    xQueueSend(xNvStoreMailboxSend, &nvNvmMessage, 0);	// get device table into RAM

    if( xTaskNotifyWait(0, 0, &notifyValue, portMAX_DELAY) == pdTRUE )
    {
		if( notifyValue == FM_ACK )
		{	// Got the values into product_configuration
			product_configuration.sanity_state = PRODUCT_NOT_CONFIGURED;
			product_configuration.product_state = PRODUCT_NOT_CONFIGURED;
			write_product_configuration();	// write 0xff into configuration
		}
	}
}

/**************************************************************
 * Function Name   : write_product_configuration
 * Description     : Store product_configuration in NV Store
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void write_product_configuration(void)
{
	SEND_TO_FM_MSG nvNvmMessage;    // Outgoing message
    uint32_t notifyValue;
	uint8_t i;
	uint8_t *rfmac = (uint8_t *)&product_configuration.rf_mac;

	zprintf(LOW_IMPORTANCE,"Writing product configuration:\r\n");
	zprintf(LOW_IMPORTANCE,"RF MAC : ");
	for( i=0; i<8; i++ )
		zprintf(LOW_IMPORTANCE,"%02X ",rfmac[i]);
	zprintf(LOW_IMPORTANCE,"\r\nPANID :%04X\r\n",product_configuration.rf_pan_id);
	zprintf(LOW_IMPORTANCE,"Serial number : %s\r\n",product_configuration.serial_number);
	zprintf(LOW_IMPORTANCE,"Ethernet MAC : ");
	for( i=0; i<6; i++ )
		zprintf(LOW_IMPORTANCE,"%02X ",product_configuration.ethernet_mac[i]);
	zprintf(LOW_IMPORTANCE,"\r\n");

	nvNvmMessage.ptrToBuf = (uint8_t *)&product_configuration;
	nvNvmMessage.dataLength = sizeof(PRODUCT_CONFIGURATION);
	nvNvmMessage.type = FM_PRODUCT_CONFIG;
	nvNvmMessage.action = FM_PUT;
	nvNvmMessage.xClientTaskHandle = xTaskGetCurrentTaskHandle();

	xQueueSend(xNvStoreMailboxSend, &nvNvmMessage, 0);	// Write product configuration

	if( xTaskNotifyWait(0, 0, &notifyValue, portMAX_DELAY ) == pdTRUE )
	{
		if( notifyValue == FM_ACK )
		{
			zprintf(MEDIUM_IMPORTANCE,"Product configuration written\r\n");
		} else
		{
			zprintf(HIGH_IMPORTANCE,"Product configuration write failed\r\n");
		}
	} else
	{
		zprintf(HIGH_IMPORTANCE,"Notification of Product configuration Write did not arrive\r\n");
	}
	DbgConsole_Flush();
}

// quick and dirty task written by Chris to give a shell-like Debug interface.
// uses FreeRTOS CLI
// It gets timesliced by other tasks of the same priority, and pre-empted by higher priority tasks.
char output[256];
static void shell_task(void *pvParameters)
{
    BaseType_t result;
    char input[256];
    vTaskDelay(pdMS_TO_TICKS( 1000 ));  // just wait for other initialisation messages to be emitted
    zprintf(MEDIUM_IMPORTANCE,"\r\nEnter 'help' for help\r\n");
    while(1)
    {
        zprintf(CRITICAL_IMPORTANCE, "HERMES >");
        LOG_ReadLine((uint8_t *)input, sizeof(input));
        zprintf(CRITICAL_IMPORTANCE, "\r\n");
        do
        {
		    result = FreeRTOS_CLIProcessCommand( input, output, sizeof(output) );  // process input and generate some output
			if( strlen(output)>256)
			{
				zprintf(CRITICAL_IMPORTANCE,"CLI Output too long! - system may become unstable");
				output[sizeof(output)-1]='\0';	// we still want to know what the output was for debugging purposes
			}
            zprintf(CRITICAL_IMPORTANCE, output); // print the partial output
			DbgConsole_Flush();
        } while( result == pdTRUE ); // keep looping around until all output has been generated
        vTaskDelay(pdMS_TO_TICKS( 100 ));   // give up CPU for 100ms
    }
}

/**************************************************************
 * Function Name   : watchdog_task
 * Description     : Kicks the 'dog every 5 seconds
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
static void watchdog_task(void *pvParameters)
{ 	
	uint32_t				mqtt_reboot_timestamp;
	MQTT_CONNECTION_STATE	mqtt_connection_state;
	
	mqtt_reboot_timestamp = get_microseconds_tick();	
	
    while(1)
    {
		service_watchdog();
		mqtt_connection_state = get_mqtt_connection_state();
		// This is brutal. If we fall outside MQTT_STATE_CONNECTED for too
		// long, we just reboot the Hub !!
		if( (MQTT_STATE_CONNECTED == mqtt_connection_state) || 
		    (MQTT_STATE_STOP == mqtt_connection_state))
		{
			mqtt_reboot_timestamp = get_microseconds_tick();
		}
		else
		{
			if( (get_microseconds_tick()-mqtt_reboot_timestamp) > MQTT_REBOOT_TIMEOUT)
			{
				zprintf(CRITICAL_IMPORTANCE,"MQTT detected lack of traffic - restarting system...\r\n");
				DbgConsole_Flush();
				NVIC_SystemReset(); // never returns
			}
		}
				
        vTaskDelay(pdMS_TO_TICKS( 5000 ));   // give up CPU for 5 seconds
    }
}


/* Initialise watchdog - only do once, at startup */
void init_watchdog(void)
{
	wdog_base = WDOG1;
#ifdef DEBUG
	WDOG_Deinit(wdog_base);
#else
	wdog_config_t config;
    WDOG_GetDefaultConfig(&config);
	config.workMode.enableDebug = true;
    config.timeoutValue = WDOG_TIMEOUT_VALUE;
    config.interruptTimeValue = WDOG_INT_TIME_VALUE;
    config.enableInterrupt = true;
    WDOG_Init(wdog_base, &config);
#endif
}

static void service_watchdog(void)
{
#ifndef DEBUG
    WDOG_Refresh(wdog_base);
#endif
}

#ifndef DEBUG
/* For completeness... probably not required */
/* Provides early warning of reset */
void WDOG1_IRQHandler(void)
{
     /* Clear WTIS interrupt flag.*/
    WDOG1->WICR |= 1<<14;
	uart_puts("Watchdog about to fire...\r\n");
}
#endif

//#define TIMESTAMP_ZPRINTF_OUTPUT	// Note this breaks Alohomora, so should only be used with a terminal
#define ZBUFFER_SIZE	384
void zprintf(ZPRINTF_IMPORTANCE importanceLevel, const char *arg0, ...)
{
	static uint32_t zprintfCorrelationIDCounter = 0;
	uint32_t 			len;
    va_list 			args;	
	char 				*zbuffer;
	HERMES_UART_HEADER	*header;	

	if( 0 == strlen(arg0) )
	{
		return;
	}

    if( (importanceLevel <= MAX_PRINT_LEVEL) && (importanceLevel >= printLevel) )
	{
		if(shouldIPackageFlag == true)
		{
    		va_start(args, arg0);
			len = vsnprintf(NULL,0,arg0,args) + 1;	// find out how long the string would be, excluding terminating \0
			va_end(args);	
#ifdef TIMESTAMP_ZPRINTF_OUTPUT
			len += 9;	// allow for extra HH:MM:SS that would be prepended.
#endif
			if( len > ZBUFFER_SIZE) {return;}	// string too long

			header = pvPortMalloc(len+sizeof(HERMES_UART_HEADER));
	
			if( NULL == header) {return;}	// drop output if cannot allocate buffer

			zbuffer = (char *)header + sizeof(HERMES_UART_HEADER);
			
			len = 0;
#ifdef TIMESTAMP_ZPRINTF_OUTPUT
			len += time_string(zbuffer);
			zbuffer += len;
#endif			
			
			va_start(args, arg0);
			len += vsprintf(zbuffer, arg0, args);
			va_end(args);	
			
			header->sync_word = 0x6863615A;
			if(importanceLevel == NOT_ENCRYPT_IMPORTANCE)
			{
				header->type = MESSAGE_GREETING;
			}
			else
			{
				header->type = MESSAGE_RESPONSE;
			}

			header->check = header->type ^ 0xFF;

			if(shouldICryptFlag == true)	// this is probably broken
			{
				uint32_t TotalWidth = (((strlen(zbuffer) + 15)/16)*16);

				if(TotalWidth == 0)
				{
					header->correlation_ID++;
				}
				else
				{
					sprintf(zbuffer,"%-*s",TotalWidth,zbuffer);
				}
			}

			header->size = len;
			header->timestamp = get_microseconds_tick();
			header->correlation_ID = zprintfCorrelationIDCounter++;
			LOG_Push((uint8_t*)header,sizeof(HERMES_UART_HEADER)+len);	// single push of whole message
			vPortFree(header);	
		}
		else
		{	// Not packaging, so straight printout
			va_start(args, arg0);
			len = vsnprintf(NULL,0,arg0,args) + 1;	// find out how long the string would be, excluding terminating \0
			va_end(args);	
#ifdef TIMESTAMP_ZPRINTF_OUTPUT
			len += 9;	// allow for extra HH:MM:SS that would be prepended.
#endif
			if( len > ZBUFFER_SIZE) {return;}
			zbuffer = pvPortMalloc(len);
			if( NULL == zbuffer) {return;}		
			len = 0;
#ifdef TIMESTAMP_ZPRINTF_OUTPUT
			len += time_string(zbuffer);
#endif
			va_start(args, arg0);
			len += vsprintf(zbuffer+len, arg0, args);
			va_end(args);				
			LOG_Push((uint8_t*)zbuffer,len);
			vPortFree(zbuffer);
		}
	}
}
/**************************************************************
 * Function Name   : Core_detect
 * Description     : Check revision of CPU core
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void Core_detect(void)
{
  uint32_t cpuid = SCB->CPUID;
  uint32_t var, pat;

//  printf("CPUID %08X DEVID %03X REVID %04X\n", cpuid, DBGMCU->IDCODE & 0xFFF, (DBGMCU->IDCODE >> 16) & 0xFFFF);

  zprintf(LOW_IMPORTANCE,"Detected Core: ");

  pat = (cpuid & 0x0000000F);
  var = (cpuid & 0x00F00000) >> 20;

  if ((cpuid & 0xFF000000) == 0x41000000) // ARM
  {
    switch((cpuid & 0x0000FFF0) >> 4)
    {
      case 0xC20 : zprintf(LOW_IMPORTANCE,"Cortex M0 r%dp%d\r\n", var, pat); break;
      case 0xC60 : zprintf(LOW_IMPORTANCE,"Cortex M0+ r%dp%d\r\n", var, pat); break;
      case 0xC21 : zprintf(LOW_IMPORTANCE,"Cortex M1 r%dp%d\r\n", var, pat); break;
      case 0xC23 : zprintf(LOW_IMPORTANCE,"Cortex M3 r%dp%d\r\n", var, pat); break;
      case 0xC24 : zprintf(LOW_IMPORTANCE,"Cortex M4 r%dp%d\r\n", var, pat); break;
      case 0xC27 : zprintf(LOW_IMPORTANCE,"Cortex M7 r%dp%d\r\n", var, pat); break;

      default : zprintf(LOW_IMPORTANCE,"Unknown CORE\r\n");
    }
  }
  else
    zprintf(LOW_IMPORTANCE,"Unknown CORE IMPLEMENTER\r\n");
}

/**************************************************************
 * Function Name   : Diagnostic function that just turns on an LED combination and hangs up
 * Description     :
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void led_hang(uint8_t val)
{
	// Switch LED pins from PWM to GPIO so we can directly manipulate them
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_23_GPIO2_IO23,0U); // PWMLED4 - Green
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_24_GPIO2_IO24,0U); // PWMLED3 - Red
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_25_GPIO2_IO25,0U); // PWMLED2 - Green
	IOMUXC_SetPinMux(IOMUXC_GPIO_EMC_26_GPIO2_IO26,0U); // PWMLED1 - Red

  	GPIO2->DR &= 0xf87fffff;
	GPIO2->DR |= (val<<23);

	while(1);
}

/**************************************************************
 * Function Name   : initSureNetByProgrammer
 * Description     : Used by the Programmer to control SureNet initialisation.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void initSureNetByProgrammer(void)
{
    surenet_init(&product_configuration.rf_mac, product_configuration.rf_pan_id, initial_RF_channel); // This initialises stack variables and starts the surenet task.
}

/**************************************************************
 * Function Name   : connectToServer
 * Description     : Used by the test framework to register a
 *                 : blank mode Hub2 with SurePetCare server.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void connectToServer(void)
{
    HFU_init();
    HTTPPostTask_init();
    SNTP_Init();
    if( xTaskCreate(SNTP_Task, "SNTP Task", SNTP_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
    {
        zprintf(CRITICAL_IMPORTANCE, "SNTP Task creation failed!\r\n");
    }

    if( xTaskCreate(hermes_app_task, "Hermes Application", HERMES_APPLICATION_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
    {
        zprintf(CRITICAL_IMPORTANCE, "Hermes Application task creation failed!\r\n");
    }

    if( xTaskCreate(MQTT_Task, "MQTT", MQTT_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS )
    {
        zprintf(CRITICAL_IMPORTANCE,"MQTT Task creation failed!\r\n");
    }

    if (xTaskCreate(HTTPPostTask, "HTTP Post", HTTP_POST_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHTTPPostTaskHandle) != pdPASS)
    {
        zprintf(CRITICAL_IMPORTANCE,"HTTP Post task creation failed!.\r\n");

    }

    if (xTaskCreate(HFU_task, "Hub Firmware Update", HFU_TASK_STACK_SIZE, NULL, NORMAL_TASK_PRIORITY, &xHFUTaskHandle) != pdPASS)
    {
        zprintf(CRITICAL_IMPORTANCE,"Hub Firmware Update task creation failed!.\r\n");
    }

}

/**************************************************************
 * Function Name   : hermes_rand()
 * Description     : Thread safe version of rand()
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
int hermes_rand(void)
{
	int result;
	portENTER_CRITICAL();
	result = rand();
	portEXIT_CRITICAL();
	return result;
}

/**************************************************************
 * Function Name   : hermes_srand()
 * Description     : Thread safe version of srand()
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void hermes_srand(int seed)
{
	portENTER_CRITICAL();
	srand(seed);
	portEXIT_CRITICAL();
}

void mem_dump(uint8_t *addr, uint32_t len)
{
#define LINELEN 16
	uint32_t j;
	uint8_t c;
	uint8_t *end_addr = addr+len;
	zprintf(LOW_IMPORTANCE,"\r\n");
	while(addr<end_addr)
	{
		zprintf(LOW_IMPORTANCE,"%08x: ",(uint32_t)addr);
		zprintf(LOW_IMPORTANCE,"%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ", \
			*addr,*(addr+1),*(addr+2),*(addr+3),*(addr+4),*(addr+5),*(addr+6),*(addr+7),	
			*(addr+8),*(addr+9),*(addr+10),*(addr+11),*(addr+12),*(addr+13),*(addr+14),*(addr+15));

		for (j=0; j<LINELEN; j++)
		{
			c = *(addr+j);
			if ((c>31) && (c<127))
				zprintf(LOW_IMPORTANCE,"%c",c);
			else
				zprintf(LOW_IMPORTANCE,".");		
		}
		zprintf(LOW_IMPORTANCE,"\r\n");
		DbgConsole_Flush();
		addr+=LINELEN;
	}
}
