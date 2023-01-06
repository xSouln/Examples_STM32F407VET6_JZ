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
* Filename: CLI-Commands.c
* Author:   Chris Cowdery
* Purpose:
*
*
*
**************************************************************************/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "fsl_debug_console.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "queue.h"
#include "fsl_pwm.h"
#include "board.h"
#include "fsl_phy.h"

// external functions we might call as a result of a CLI command.
#include "SureNet-interface.h"
#include "SureNet.h"
#include "Devices.h"
#include "Hermes-test.h"
#include "RegisterMap.h"
#include "leds.h"
#include "CLI-commands-Simulator.h"
#include "MQTT.h"
#include "hermes-time.h"
#include "BuildNumber.h"
#include "Device_Buffer.h"
#include "Server_Buffer.h"
#include "MQTT.h"
#include "Hermes-app.h"
#include "LabelPrinter.h"
#include "fsl_flexspi_ram.h"
#include "Hermes-Startup.h"
#include "BankManager.h"
#include "Babel.h"
#include "HubFirmwareUpdate.h"
#include "utilities.h"

#include "wolfssl/wolfcrypt/hash.h"

uint32_t uAppendString(char *pcDest, char *pcStr);
uint64_t read_mac_address (char * tmac);

static portBASE_TYPE prvTaskStatsCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvRunTimeStatsCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvVersionCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvBankCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvWDTestCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvIfConfigCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvPingCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvPairCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvPairTableCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvForgetDeviceCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvUnPairCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvFaultCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvTestCaseCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvDumpHubRegistersCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvLEDCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvAwakeCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvPublishCommand		(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);
static portBASE_TYPE prvPrintLevelCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvServerBufferCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvDeviceBufferCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvMQTTStatusCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvSetProductStateCommand  (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvResetCommand  		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvLabelResetCommand  	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvSanitiseCommand  	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvCrashCommand  		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvPinSpeedCommand  	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvTranscribeCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvHFUCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvRFMACMangleCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvNetStatCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvKeyDumpCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvKeyUpdateCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvChecksumCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvMessageDumpCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvHRMFullDumpCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
#if SURENET_ACTIVITY_LOG
static portBASE_TYPE prvSNLogCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
#endif

static portBASE_TYPE prvGreetingCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);//vRegisterBasicCLICommands
static portBASE_TYPE prvUnlockCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);//vRegisterBasicCLICommands



extern QueueHandle_t xLedMailbox;
extern QueueHandle_t xRemoveFromPairingTable;
extern QueueHandle_t xUnpairDeviceMailbox;
extern QueueHandle_t xOutgoingMQTTMessageMailbox;
extern uint8_t printLevel;


bool isCLIUnlocked = false;

/* Structure that defines the "checksum" command line command. */
CLI_Command_Definition_t prvChecksumCommandDefinition =
{
	( const char * const ) "checksum", /* The command string to type. */
	( const char * const ) "checksum      : (A|a|B|b|0xStart,0xLen)Calculates checksum for Flash regions\r\n",
	prvChecksumCommand, /* The function to run. */
	1 /* No parameters are expected. */
};

/* Structure that defines the "messagedump" command line command. */
CLI_Command_Definition_t prvMessageDumpCommandDefinition =
{
	( const char * const ) "messagedump", /* The command string to type. */
	( const char * const ) "messagedump   : on | off - Dump messages as they come through the system\r\n",
	prvMessageDumpCommand, /* The function to run. */
	1 /* One parameter is expected. */
};

/* Structure that defines the "keydump" command line command. */
CLI_Command_Definition_t prvKeyDumpCommandDefinition =
{
	( const char * const ) "keydump", /* The command string to type. */
	( const char * const ) "keydump       : Dumps the keys used for signing\r\n",
	prvKeyDumpCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

/* Structure that defines the "keyupdate" command line command. */
CLI_Command_Definition_t prvKeyUpdateCommandDefinition =
{
	( const char * const ) "keyupdate", /* The command string to type. */
	( const char * const ) "keyupdate     : Forces a key update with the Server\r\n",
	prvKeyUpdateCommand, /* The function to run. */
	0 /* No parameters are expected. */
};


/* Structure that defines the "run-time-stats" command line command.   This
generates a table that shows how much run time each task has */
CLI_Command_Definition_t prvRunTimeStatsCommandDefinition =
{
	( const char * const ) "run-time-stats", /* The command string to type. */
	( const char * const ) "run-time-stats: Displays a table showing how much processing time each FreeRTOS task has used\r\n",
	prvRunTimeStatsCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
CLI_Command_Definition_t prvTaskStatsCommandDefinition =
{
	( const char * const ) "task-stats", /* The command string to type. */
	( const char * const ) "task-stats:     Displays a table showing the state of each FreeRTOS task\r\n",
	prvTaskStatsCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

/* Structure that defines the "version" command */
CLI_Command_Definition_t prvVersionCommandDefinition =
{
	( const char * const ) "version",
	( const char * const ) "version:        Displays version information\r\n",
	prvVersionCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "bank" command */
CLI_Command_Definition_t prvBankCommandDefinition =
{
	( const char * const ) "bank",
	( const char * const ) "bank:           Displays Bank Descriptors\r\n",
	prvBankCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

#if SURENET_ACTIVITY_LOG
/* Structure that defines the "snlog" command */
CLI_Command_Definition_t prvSNLogCommandDefinition =
{
	( const char * const ) "snlog",
	( const char * const ) "snlog:          SureNet Log dump\r\n",
	prvSNLogCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};
#endif

/* Structure that defines the "wdtest" command */
CLI_Command_Definition_t prvWDTestCommandDefinition =
{
	( const char * const ) "wdtest",
	( const char * const ) "wdtest:         Stops the watchdog being kicked\r\n",
	prvWDTestCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "sanitise" command */
CLI_Command_Definition_t prvSanitiseCommandDefinition =
{
	( const char * const ) "sanitise",
	( const char * const ) "sanitise:       Writes 0xFF to NVM so next reset installs sane values. Note this will attempt to preserve the programmer installed mac / serial / secret\r\n",
	prvSanitiseCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "crash" command */
CLI_Command_Definition_t prvCrashCommandDefinition =
{
	( const char * const ) "crash",
	( const char * const ) "crash:          Accesses an unclocked peripheral and crashes the system\r\n",
	prvCrashCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "version" command */
CLI_Command_Definition_t prvIfConfigCommandDefinition =
{
	( const char * const ) "ifconfig",
	( const char * const ) "ifconfig:       Displays Ethernet information\r\n",
	prvIfConfigCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "netstat" command */
CLI_Command_Definition_t prvNetStatCommandDefinition =
{
	( const char * const ) "netstat",
	( const char * const ) "netstat:        Displays TCP/IP information\r\n",
	prvNetStatCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

/* Structure that defines the "ping" command */
CLI_Command_Definition_t prvPingCommandDefinition =
{
	( const char * const ) "ping",
	( const char * const ) "ping:           Ping remote host\r\n",
	prvPingCommand, /* The function to run. */
	1 /* One parameter should be entered. */
};

CLI_Command_Definition_t prvResetCommandDefinition =
{
	( const char * const ) "reset",
	( const char * const ) "reset:          Reset processor\r\n",
	prvResetCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};

CLI_Command_Definition_t prvLabelResetCommandDefinition =
{
	( const char * const ) "labelreset",
	( const char * const ) "labelreset:     Reset Label Printing state machine to beginning\r\n",
	prvLabelResetCommand, /* The function to run. */
	0 /* No parameters should be entered. */
};


/* Structure that defines the "pairmode" command */
CLI_Command_Definition_t prvPairCommandDefinition =
{
	( const char * const ) "pairmode",
	( const char * const ) "pairmode:       Turns pairing mode on or off (on/off)\r\n",
	prvPairCommand, /* The function to run. */
	1 /* One parameter should be entered. */
};

/* Structure that defines the "pairtable" command */
CLI_Command_Definition_t prvPairTableCommandDefinition =
{
	( const char * const ) "pairtable",
	( const char * const ) "pairtable:      Display Pairing Table\r\n",
	prvPairTableCommand, /* The function to run. */
	0 /* No parameter should be entered. */
};

/* Structure that defines the "printlevel" command */
CLI_Command_Definition_t prvPrintLevelCommandDefinition =
{
	( const char * const ) "printlevel",
	( const char * const ) "printlevel:     Sets the print level priority 0 (not important) - 10 (critical)\r\n",
	prvPrintLevelCommand, /* The function to run. */
	-1 /* One parameter should be entered. */
};

/* Structure that defines the "setproductstate" command */
CLI_Command_Definition_t prvSetProductStateCommandDefinition =
{
	( const char * const ) "setproductstate",
	( const char * const ) "setproductstate:Sets the Product State. Values are 0 (PRODUCT_BLANK),1 (PRODUCT_TESTED) or 2 (PRODUCT_CONFIGURED).\r\n",
	prvSetProductStateCommand, /* The function to run. */
	1 /* One parameter should be entered. */
};

/* Structure that defines the "pairtable" command */
CLI_Command_Definition_t prvForgetDeviceCommandDefinition =
{
	( const char * const ) "forgetdev",
	( const char * const ) "forgetdev:      Removes device from table with MAC input\r\n",
	prvForgetDeviceCommand, /* The function to run. */
	1 /* One parameter should be entered. */
};

/* Structure that defines the "unpair" command */
CLI_Command_Definition_t prvUnPairCommandDefinition =
{
	( const char * const ) "unpair",
	( const char * const ) "unpair:         unpair a named MAC\r\n",
	prvUnPairCommand, /* The function to run. */
	1 /* One parameter should be entered. */
};

/* Structure that defines the "fault" command */
CLI_Command_Definition_t prvFaultCommandDefinition =
{
	( const char * const ) "fault",
	( const char * const ) "fault:          Cause a HardFault by jumping to an invalid address\r\n",
	prvFaultCommand, /* The function to run. */
	0 /* No parameter should be entered. */
};

/* Structure that defines the "serverbuf" command */
CLI_Command_Definition_t prvServerBufferCommandDefinition =
{
	( const char * const ) "serverbuf",
	( const char * const ) "serverbuf:      Dump contents of buffer containing message for server\r\n",
	prvServerBufferCommand, /* The function to run. */
	0 /* No parameter should be entered. */
};

/* Structure that defines the "devicebuf" command */
CLI_Command_Definition_t prvDeviceBufferCommandDefinition =
{
	( const char * const ) "devicebuf",
	( const char * const ) "devicebuf:      Dump contents of buffer containing message from Devices\r\n",
	prvDeviceBufferCommand, /* The function to run. */
	0 /* No parameter should be entered. */
};

/* Structure that defines the "mqttstat" command */
CLI_Command_Definition_t prvMQTTStatusCommandDefinition =
{
	( const char * const ) "mqttstat",
	( const char * const ) "mqttstat:       Display status of MQTT engine\r\n",
	prvMQTTStatusCommand, /* The function to run. */
	0 /* No parameter should be entered. */
};

/* Structure that defines the "mqttstat" command */
CLI_Command_Definition_t prvRFMACMangleCommandDefinition =
{
	( const char * const ) "rfmacmangle",
	( const char * const ) "rfmacmangle:    (on|off) Mangle the RF MAC address to support Pet Doors (resets unit)\r\n",
	prvRFMACMangleCommand, /* The function to run. */
	1 /* one parameter should be entered. */
};


/* Structure that defines the "led" command */
CLI_Command_Definition_t prvLEDCommandDefinition =
{
	( const char * const ) "led",
	( const char * const ) "led:            (off|dim|normal) (solid|alternate|pulse|flash|fastflash) (red|green|orange)\r\n",
	prvLEDCommand, /* The function to run. */
	-1 /* Any number of parameters could be entered. */
};

CLI_Command_Definition_t prvPublishCommandDefinition =
{
	(const char* const) "publish",
	(const char* const) "publish:        Publish a text message under the base topic/messages.\r\n",
	prvPublishCommand,
	-1	// Any number of "parameters" when the messages has spaces.
};

CLI_Command_Definition_t prvTranscribeCommandDefinition =
{
	(const char* const) "transcribe",
	(const char* const) "transcribe:     Encrypt and transcribe Bank A to Bank B.\r\n",
	prvTranscribeCommand,
	-1	// Support any number of parameters.
};

/* Structure that defines the "greeting" command */
CLI_Command_Definition_t prvGreetingCommandDefinition =
{
	( const char * const ) GREETING_STRING,
	( const char * const ) "",
	prvGreetingCommand, /* The function to run. */
	-1 /* No parameters should be entered. */
};

CLI_Command_Definition_t prvUnlockCommandDefinition =
{
	( const char * const ) "clienablekey",
	( const char * const ) "",
	prvUnlockCommand, /* The function to run. */
	1 /* No parameters should be entered. */
};

CLI_Command_Definition_t prvHFUCommandDefinition =
{
	(const char* const) "hfu",
	(const char* const) "hfu:            Trigger a Hub Firmware Update.\r\n",
	prvHFUCommand,
	0	// Support any number of parameters.
};

/* Structure that defines the "testcase" command */
CLI_Command_Definition_t prvTestCaseCommandDefinition =
{
	( const char * const ) "testcase",
	( const char * const ) "testcase:       Run a test case, enter \"testcase help\"\r\n",
	prvTestCaseCommand, /* The function to run. */
	1 /* One parameter should be entered. */
};

/* Structure that defines the "hubreg" command */
CLI_Command_Definition_t prvDumpHubRegistersCommandDefinition =
{
	( const char * const ) "hubreg",
	( const char * const ) "hubreg:         Dump Hub Register Map\r\n",
	prvDumpHubRegistersCommand, /* The function to run. */
	0 /* No parameter should be entered. */
};

/* Structure that defines the "awake" command */
CLI_Command_Definition_t prvAwakeCommandDefinition =
{
	( const char * const ) "awake",
	( const char * const ) "awake:          Set the amount of device awake notifications: 0-2\r\n",
	prvAwakeCommand, /* The function to run. */
	1 	/* one parameters should be entered. */
};

/* Structure that defines the "pinspeed" command */
CLI_Command_Definition_t prvPinSpeedCommandDefinition =
{
	( const char * const ) "pinspeed",
	( const char * const ) "pinspeed:       Set the speed of the SPI and Ethernet pins. pinspeed 0-2,0-2 where 0=slow,1=med,2=fastest\r\n",
	prvPinSpeedCommand, /* The function to run. */
	2 	/* one parameters should be entered. */
};

CLI_Command_Definition_t prvHRMFullDumpCommandDefinition =
{
	( const char * const ) "hrm_full_dump",
	( const char * const ) "hrm_full_dump:  Transmit a full single-message register dump over MQTT.\r\n",
	prvHRMFullDumpCommand,
	0
};

/*-----------------------------------------------------------*/

void vRegisterBasicCLICommands(void)
{
  //greeting command
  //unlock commands - if i's unlocked, call vRegisterCLICommands

  	FreeRTOS_CLIRegisterCommand(&prvGreetingCommandDefinition);//greeting command
	FreeRTOS_CLIRegisterCommand(&prvUnlockCommandDefinition);//unlock command
	vRegisterSimulatorCLICommands();

}

void vRegisterCLICommands( void )
{
	/* Register all the command line commands defined immediately above. */
	FreeRTOS_CLIRegisterCommand(&prvFaultCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvCrashCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvPinSpeedCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvPublishCommandDefinition);//ask Tom
	FreeRTOS_CLIRegisterCommand(&prvForgetDeviceCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvBankCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvWDTestCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvTaskStatsCommandDefinition );
	FreeRTOS_CLIRegisterCommand(&prvRunTimeStatsCommandDefinition );
    FreeRTOS_CLIRegisterCommand(&prvVersionCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvIfConfigCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvPingCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvPairCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvPairTableCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvUnPairCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvDumpHubRegistersCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvTestCaseCommandDefinition);
    FreeRTOS_CLIRegisterCommand(&prvPrintLevelCommandDefinition);
    FreeRTOS_CLIRegisterCommand(&prvServerBufferCommandDefinition);
    FreeRTOS_CLIRegisterCommand(&prvDeviceBufferCommandDefinition);
    FreeRTOS_CLIRegisterCommand(&prvMQTTStatusCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvSetProductStateCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvResetCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvLabelResetCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvTranscribeCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvHFUCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvLEDCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvSanitiseCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvRFMACMangleCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvNetStatCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvKeyDumpCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvKeyUpdateCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvChecksumCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvMessageDumpCommandDefinition);
	FreeRTOS_CLIRegisterCommand(&prvHRMFullDumpCommandDefinition);
#if SURENET_ACTIVITY_LOG
	FreeRTOS_CLIRegisterCommand(&prvSNLogCommandDefinition);
#endif
	vRegisterSimulatorCLICommands();

}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	TaskStatus_t *pxTaskStatusArray;
	UBaseType_t uxArraySize, x;
	uint32_t start,end;
	uint32_t *ptr;
	uint32_t percent_unused;

	char *states[]={"Running","Ready","Blocked","Suspend","Deleted","Invalid"};

	zprintf(CRITICAL_IMPORTANCE,"Task                        State  Pri Task Stk Sz  Unused  TCB loc.    Stack start % Stk unused\r\n***********************************************************************************************\r\n");

	/* Take a snapshot of the number of tasks in case it changes while this
	function is executing. */
	uxArraySize = uxTaskGetNumberOfTasks();

	/* Allocate an array index for each task.  NOTE!  if
	configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
	equate to NULL. */
	pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) ); /*lint !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack and this allocation allocates a struct that has the alignment requirements of a pointer. */

	if( pxTaskStatusArray != NULL )
	{
			/* Generate the (binary) data. */
		uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, NULL );

		/* Create a human readable table from the binary data. */
		for( x = 0; x < uxArraySize; x++ )
		{
			zprintf(CRITICAL_IMPORTANCE,"%s",pxTaskStatusArray[ x ].pcTaskName );

			if( strlen(pxTaskStatusArray[ x ].pcTaskName)<24)
			{
				for( uint8_t j=0; j<24-strlen(pxTaskStatusArray[ x ].pcTaskName); j++)
					zprintf(CRITICAL_IMPORTANCE," ");
			}
			// These two are a bit of a hack, because the struct definition
			// for xHandle is private to Tasks.c, so we have to index into it blindly.
			start = *((uint32_t *)pxTaskStatusArray[x].xHandle+12);
			end = *((uint32_t *)pxTaskStatusArray[x].xHandle+18);

			/* Write the rest of the string. */
			zprintf(CRITICAL_IMPORTANCE, "\t%s\t%u\t%u\t0x%04X\t0x%04X\t0x%08X\t0x%08X", \
				states[pxTaskStatusArray[ x ].eCurrentState],\
				( unsigned int ) pxTaskStatusArray[ x ].uxCurrentPriority, \
				( unsigned int ) pxTaskStatusArray[ x ].xTaskNumber, \
				end-start, \
				( unsigned int ) pxTaskStatusArray[ x ].usStackHighWaterMark*sizeof( StackType_t ), \
				( unsigned int ) pxTaskStatusArray[ x ].xHandle, \
				start); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
			DbgConsole_Flush();
			percent_unused = (100*(pxTaskStatusArray[ x ].usStackHighWaterMark*sizeof( StackType_t )))/(end-start);
			zprintf(CRITICAL_IMPORTANCE,"  %d\r\n",percent_unused);
			DbgConsole_Flush();
		}
		/* Free the array again.  NOTE!  If configSUPPORT_DYNAMIC_ALLOCATION
		is 0 then vPortFree() will be #defined to nothing. */
		vPortFree( pxTaskStatusArray );
		zprintf(CRITICAL_IMPORTANCE, "Total Heap = 0x%08X  Free Heap = 0x%08X  Min free Heap = 0x%08X\r\n",configTOTAL_HEAP_SIZE, xPortGetFreeHeapSize(),xPortGetMinimumEverFreeHeapSize());
	}
	*pcWriteBuffer='\0';	// stop previous content from being printed
	return pdFALSE;
}
/*-----------------------------------------------------------*/
static portBASE_TYPE prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	/* Generate a table of task stats. */
	TaskStatus_t *pxTaskStatusArray;
	UBaseType_t uxArraySize, x;
	uint32_t ulTotalTime, ulStatsAsPercentage;

	zprintf(CRITICAL_IMPORTANCE,"Task                    Abs Time    %% Percent Time\r\n*************************************************\r\n");
	DbgConsole_Flush();
	/* Take a snapshot of the number of tasks in case it changes while this
	function is executing. */
	uxArraySize = uxTaskGetNumberOfTasks();;

	/* Allocate an array index for each task.  NOTE!  If
	configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
	equate to NULL. */
	pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) ); /*lint !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack and this allocation allocates a struct that has the alignment requirements of a pointer. */

	if( pxTaskStatusArray != NULL )
	{
		/* Generate the (binary) data. */
		uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalTime );

		/* For percentage calculations. */
		ulTotalTime /= 100UL;

		/* Avoid divide by zero errors. */
		if( ulTotalTime > 0UL )
		{
			/* Create a human readable table from the binary data. */
			for( x = 0; x < uxArraySize; x++ )
			{
				/* What percentage of the total run time has the task used?
				This will always be rounded down to the nearest integer.
				ulTotalRunTimeDiv100 has already been divided by 100. */
				ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalTime;

				/* Write the task name to the string, padding with
				spaces so it can be printed in tabular form more
				easily. */
				zprintf(CRITICAL_IMPORTANCE,"%s ", pxTaskStatusArray[ x ].pcTaskName );
				if( strlen(pxTaskStatusArray[ x ].pcTaskName)<20)
				{
					for( uint8_t j=0; j<20-strlen(pxTaskStatusArray[ x ].pcTaskName); j++)
						zprintf(CRITICAL_IMPORTANCE," ");
				}

				if( ulStatsAsPercentage > 0UL )
				{
					zprintf(CRITICAL_IMPORTANCE, "\t%u\t\t%u\r\n", ( unsigned int ) pxTaskStatusArray[ x ].ulRunTimeCounter, ( unsigned int ) ulStatsAsPercentage ); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
				}
				else
				{	/* If the percentage is zero here then the task has
					consumed less than 1% of the total run time. */
					zprintf(CRITICAL_IMPORTANCE, "\t%u\t\t<1\r\n", ( unsigned int ) pxTaskStatusArray[ x ].ulRunTimeCounter ); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
				}
				DbgConsole_Flush();
			}
		}
		/* Free the array again.*/
		vPortFree( pxTaskStatusArray );
	}
	*pcWriteBuffer='\0';	// stop previous content from being printed
	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvFaultCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
    void (*fn_ptr)()=(void (*)())0x30000000;    // no memory at this address

    fn_ptr();

	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvHFUCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	pcWriteBuffer += sprintf(pcWriteBuffer,"Starting Hub Firmware Update\r\n");
    HFU_trigger(false);
	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvResetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	// Reset MCU
	NVIC_SystemReset(); // never returns

	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvWDTestCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

#ifdef DEBUG
	pcWriteBuffer += sprintf(pcWriteBuffer,"Watchdog disabled in DEBUG configuration\r\n");
#else
	portENTER_CRITICAL();
	while(1);
//	portEXIT_CRITICAL();	// never gets here
//	*pcWriteBuffer='\0';	// stop previous content from being printed
#endif
//	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvLabelResetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	*pcWriteBuffer='\0';	// stop previous content from being printed
	restart_label_print();

	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvCrashCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	*pcWriteBuffer='\0';	// stop previous content from being printed
	CLOCK_ControlGate(kCLOCK_Gpio1,kCLOCK_ClockNotNeeded);
	volatile uint32_t t = GPIO1->DR;	// will trigger an exception
	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvSanitiseCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	*pcWriteBuffer='\0';	// stop previous content from being printed
	sanitise_product_config();

	return pdFALSE;
}

#if SURENET_ACTIVITY_LOG
void dump_surenet_log(void);
/*-----------------------------------------------------------*/
static portBASE_TYPE prvSNLogCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	*pcWriteBuffer='\0';	// stop previous content from being printed
	dump_surenet_log();
	return pdFALSE;
}
#endif

/*-----------------------------------------------------------*/
static portBASE_TYPE prvKeyDumpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	*pcWriteBuffer='\0';	// stop previous content from being printed
	key_dump();
	return pdFALSE;
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvKeyUpdateCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	*pcWriteBuffer='\0';	// stop previous content from being printed
	send_shared_secret();
	return pdFALSE;
}

/*-----------------------------------------------------------*/
extern PRODUCT_CONFIGURATION product_configuration;
extern const BANK_DESCRIPTOR	BankA_Descriptor;
extern const BANK_DESCRIPTOR	BankB_Descriptor;
extern uint64_t	rfmac;
static portBASE_TYPE prvVersionCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	zprintf(CRITICAL_IMPORTANCE,"Hermes Version Information:\r\nBuild time  : "__DATE__" "__TIME__"\r\n");
	zprintf(CRITICAL_IMPORTANCE,"Compiled by : %s\r\n", MYUSERNAME);
	zprintf(CRITICAL_IMPORTANCE,"Kernel      : FreeRTOS "tskKERNEL_VERSION_NUMBER"\r\n" \
				"RF          : %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n" \
				"Eth         : %02X:%02X:%02X:%02X:%02X:%02X\r\n", \
				(uint8_t)(rfmac>>56),(uint8_t)(rfmac>>48),(uint8_t)(rfmac>>40),(uint8_t)(rfmac>>32), \
                (uint8_t)(rfmac>>24),(uint8_t)(rfmac>>16),(uint8_t)(rfmac>>8),(uint8_t)(rfmac), \
				product_configuration.ethernet_mac[0],product_configuration.ethernet_mac[1], \
                product_configuration.ethernet_mac[2],product_configuration.ethernet_mac[3], \
				product_configuration.ethernet_mac[4],product_configuration.ethernet_mac[5]);

	if( true == product_configuration.rf_mac_mangle)
		zprintf(CRITICAL_IMPORTANCE,"RFMAC mangle: true\r\n");
	else
		zprintf(CRITICAL_IMPORTANCE,"RFMAC mangle: false\r\n");

	zprintf(CRITICAL_IMPORTANCE,"Hub serial  : %s\r\n"\
				"PAN ID      : %04X\r\n", \
				product_configuration.serial_number, \
				product_configuration.rf_pan_id	);

	zprintf(CRITICAL_IMPORTANCE,"Buildnumber : %d.%d\r\n",\
				SVN_REVISION,BUILD_MARK);

	zprintf(CRITICAL_IMPORTANCE,"Build Hash  : %s\r\n",\
				VERSION_HASH);

	zprintf(CRITICAL_IMPORTANCE,"Config.     : ");
	if (product_configuration.product_state == PRODUCT_NOT_CONFIGURED)
		zprintf(CRITICAL_IMPORTANCE,"PRODUCT_NOT_CONFIGURED\r\n");
	else if (product_configuration.product_state == PRODUCT_TESTED)
		zprintf(CRITICAL_IMPORTANCE,"PRODUCT_TESTED\r\n");
	else if (product_configuration.product_state == PRODUCT_BLANK)
		zprintf(CRITICAL_IMPORTANCE,"PRODUCT_BLANK\r\n");
	else
		zprintf(CRITICAL_IMPORTANCE,"PRODUCT_CONFIGURED\r\n");
#ifdef DEBUG
	zprintf(CRITICAL_IMPORTANCE,"Build conf. : DEBUG\r\n");
#else
	zprintf(CRITICAL_IMPORTANCE,"Build conf. : RELEASE\r\n");
#endif

	if( BM_BANK_A == BM_GetCurrentBank() )
	{
		zprintf(CRITICAL_IMPORTANCE, "Bank A      : Active (%s)\r\n", BankA_Descriptor.encrypted ? "Encrypted" : "Plain");
		zprintf(CRITICAL_IMPORTANCE, "Bank B      : %s (%s)\r\n",	(BankB_Descriptor.bank_mark > 0 ? "Present" : "Invalid"),
								 												(BankB_Descriptor.encrypted ? "Encrypted" : "Plain"));
	} else
	{
		zprintf(CRITICAL_IMPORTANCE, "Bank A      : %s (%s)\r\n", (BankA_Descriptor.bank_mark > 0 ? "Present" : "Invalid"),
								 											  BankA_Descriptor.encrypted ? "Encrypted" : "Plain");
		zprintf(CRITICAL_IMPORTANCE, "Bank B      : Active (%s)\r\n", (BankB_Descriptor.encrypted ? "Encrypted" : "Plain"));
	}

	zprintf(CRITICAL_IMPORTANCE,"Active bank : BANK ");
	BM_BANK bank = BM_GetCurrentBank();
	if( BM_BANK_A == bank)
		zprintf(CRITICAL_IMPORTANCE,"A\r\n");
	if( BM_BANK_B == bank)
		zprintf(CRITICAL_IMPORTANCE,"B\r\n");
	if( BM_BANK_UNKONWN == bank)
		zprintf(CRITICAL_IMPORTANCE,"Unknown\r\n");

	zprintf(CRITICAL_IMPORTANCE,"END OF VERSION\r\n");

	*pcWriteBuffer='\0';	// stop previous content from being printed
	return pdFALSE;
}


static portBASE_TYPE prvGreetingCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	extern bool shouldICryptFlag;
	extern bool shouldIPackageFlag;
	int8_t 			*pcParameterString;
    portBASE_TYPE	xParameterNumber = 1;
    portBASE_TYPE 	xParameterStringLength;
    char 			pcParameter[20];    // should only really need to be a few bytes long
    int8_t 		packageValue;


    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		 	/*The command string itself. */
							                                xParameterNumber,		 	/*Return the next parameter. */
                            							    &xParameterStringLength);	/*Store the parameter string length. */


	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}

    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

	if(*pcParameterString == '0')
    {
		shouldIPackageFlag = false;
	}
	else if(*pcParameterString == '1')
    {
		shouldIPackageFlag = true;
	}

	char* firstChar = pcWriteBuffer;

	configASSERT( pcWriteBuffer );

	if (product_configuration.product_state == PRODUCT_NOT_CONFIGURED)
	{
	  pcWriteBuffer += sprintf(pcWriteBuffer,"PRODUCT: NOT_CONFIGURED; ");
	}
	else if (product_configuration.product_state == PRODUCT_TESTED)
	{
	  pcWriteBuffer += sprintf(pcWriteBuffer,"PRODUCT: TESTED; ");
	}
	else if (product_configuration.product_state == PRODUCT_BLANK)
	{
	  pcWriteBuffer += sprintf(pcWriteBuffer,"PRODUCT: BLANK; ");
	}
	else
	{
	  pcWriteBuffer += sprintf(pcWriteBuffer,"PRODUCT: CONFIGURED; ");
	}


	//send actual product config state
	pcWriteBuffer += sprintf(pcWriteBuffer,"%s %d %d \r\n",product_configuration.serial_number,shouldICryptFlag,shouldIPackageFlag);

	if(false == isCLIUnlocked)
	{
		vRegisterCLICommands();//unlock the cli list
		isCLIUnlocked = true;
	}
	else
	{
		pcWriteBuffer += sprintf(pcWriteBuffer,"\r\nCLI is already enabled\r\n");
	}

	zprintf(NOT_ENCRYPT_IMPORTANCE,firstChar);
	firstChar[0] = '\0';
	//indication of if encrypted, packaged up, and locked/unlocked
	return pdFALSE;
}

static portBASE_TYPE prvUnlockCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	extern bool amILocked;
  	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	portBASE_TYPE	xParameterNumber = 1;
	portBASE_TYPE 	xParameterStringLength;


	FreeRTOS_CLIGetParameter(	pcCommandString,		/* The command string itself. */
								xParameterNumber,		/* Return the next parameter. */
								&xParameterStringLength );	/* Store the parameter string length. */


    //strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    //pcParameter[xParameterStringLength]='\0';

	*pcWriteBuffer = '\0';    // ensure we don't return with duff info in write buffer (because it does get output!)

//if unlocked...
	amILocked = false;
	pcWriteBuffer += sprintf(pcWriteBuffer,"\r\nUNLOCKED\r\n");
	//vRegisterCLICommands();//unlock the cli list
//else
	//amILocked = true;
	return pdFALSE;
}

extern const BANK_DESCRIPTOR	BankA_Descriptor, BankB_Descriptor;
void dump_bank_desc(const BANK_DESCRIPTOR *desc);
static portBASE_TYPE prvBankCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	zprintf(CRITICAL_IMPORTANCE, "Bank Descriptors\r\n");
	zprintf(CRITICAL_IMPORTANCE, "Bank A\r\n");
	dump_bank_desc(&BankA_Descriptor);
	zprintf(CRITICAL_IMPORTANCE, "\r\nBank B\r\n");
	dump_bank_desc(&BankB_Descriptor);
    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

void dump_bank_desc(const BANK_DESCRIPTOR *desc)
{

	zprintf(CRITICAL_IMPORTANCE, "descriptor_hash : 0x%08x\r\nbank_mark       : ",desc->descriptor_hash);
	if( desc->bank_mark == BANK_MARK_ROLLED_BACK)
		zprintf(CRITICAL_IMPORTANCE, "BANK_MARK_ROLLED_BACK\r\n");
	else if( desc->bank_mark == BANK_MARK_UPDATING)
		zprintf(CRITICAL_IMPORTANCE, "BANK_MARK_UPDATING\r\n");
	else if( desc->bank_mark == BANK_MARK_UNINITIALISED)
		zprintf(CRITICAL_IMPORTANCE, "BANK_MARK_UNINITIALISED\r\n");
	else
		zprintf(CRITICAL_IMPORTANCE, "%d\r\n",desc->bank_mark);
	zprintf(CRITICAL_IMPORTANCE, "encrypted       : ");
	if( desc->encrypted == true)
		zprintf(CRITICAL_IMPORTANCE, "true\r\n");
	else
		zprintf(CRITICAL_IMPORTANCE, "false\r\n");
	zprintf(CRITICAL_IMPORTANCE, "watchdog resets : %d\r\n",desc->watchdog_resets);
}

void CLI_PingRespCallback(void)
{
	zprintf(CRITICAL_IMPORTANCE,"CLI Ping reply OK\r\n");
}

BaseType_t vSendPing(uint32_t ulIPAddress )
{
    uint16_t usRequestSequenceNumber, usReplySequenceNumber;

 	FreeRTOS_SendPingRequest( ulIPAddress, 8, 100 / portTICK_PERIOD_MS, CLI_PingRespCallback );

    return pdPASS;
}

// ping command
static portBASE_TYPE prvPingCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t *pcParameterString;
    static portBASE_TYPE xParameterNumber = 1;
    portBASE_TYPE xParameterStringLength;
    char pcParameter[20];    // ought to be long enough for an IP address
	uint32_t ulIPAddress;

    pcParameterString = ( int8_t * ) FreeRTOS_CLIGetParameter
                            (
                                pcCommandString,		/* The command string itself. */
                                xParameterNumber,		/* Return the next parameter. */
                                &xParameterStringLength	/* Store the parameter string length. */
                            );
    if (xParameterStringLength>sizeof(pcParameter)-1) xParameterStringLength=sizeof(pcParameter)-1;   // clip it

    strncpy(pcParameter,(const char *)pcParameterString,xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

    /* The pcIPAddress parameter holds the destination IP address as a string in
    decimal dot notation (for example, "192.168.0.200").  Convert the string into
    the required 32-bit format. */
    ulIPAddress = FreeRTOS_inet_addr( (char const *)pcParameter );

    vSendPing(ulIPAddress);

    *pcWriteBuffer = '\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

extern uint32_t		m_bank_a_start;
extern uint32_t		m_bank_b_start;
extern uint32_t		m_bank_a_size;
extern uint32_t		m_bank_b_size;

void calc_checksum(uint8_t *addr, uint32_t length)
{
	char hash[WC_SHA256_DIGEST_SIZE];	// result
	uint16_t crc;
	zprintf(CRITICAL_IMPORTANCE,"Calculating checksum from 0x%08x for 0x%08x bytes\r\n",addr,length);
	if( (addr < (uint8_t *)0x60000000) || ((addr + length) > (uint8_t *)0x603fffff) )
	{
		zprintf(CRITICAL_IMPORTANCE,"Address out of range\r\n");
		return;
	}
	if( length<256 )
	{
		zprintf(CRITICAL_IMPORTANCE,"Length too short\r\n");
		return;
	}

	wc_Sha256Hash((const byte *)addr, length, (byte *)hash);

	zprintf(CRITICAL_IMPORTANCE,"SHA256 Hash = ");
	for( int i=0; i<WC_SHA256_DIGEST_SIZE; i++) {zprintf(CRITICAL_IMPORTANCE,"%02X",hash[i]);}
	zprintf(CRITICAL_IMPORTANCE,"\r\n");

	crc = CRC16(addr, length, 0xcccc);
	zprintf(CRITICAL_IMPORTANCE,"CRC = 0X%04x\r\n",crc);
}

static portBASE_TYPE prvChecksumCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t *pcParameterString;
    static portBASE_TYPE xParameterNumber = 1;
    portBASE_TYPE xParameterStringLength;
	uint8_t *addr = (uint8_t *)0x60000000;
	uint32_t length =  0x100;
	char *tok;
    char pcParameter[30];    // 0x60000000,0x10000 is probably the longest realistic command
	pcParameterString = ( int8_t * ) FreeRTOS_CLIGetParameter
                            (
                                pcCommandString,		/* The command string itself. */
                                xParameterNumber,		/* Return the next parameter. */
                                &xParameterStringLength	/* Store the parameter string length. */
                            );
    if (xParameterStringLength>sizeof(pcParameter)-1) xParameterStringLength=sizeof(pcParameter)-1;   // clip it

	if( (pcParameterString[0]=='A') || (pcParameterString[0]=='a') )
	{
		zprintf(CRITICAL_IMPORTANCE,"Calculating checksum for bank A\r\n");
		calc_checksum((uint8_t *)&m_bank_a_start, (uint32_t)&m_bank_a_size);
	}

	else if( (pcParameterString[0]=='B') || (pcParameterString[0]=='b') )
	{
		zprintf(CRITICAL_IMPORTANCE,"Calculating checksum for bank B\r\n");
		calc_checksum((uint8_t *)&m_bank_b_start, (uint32_t)&m_bank_b_size);
	}

	else if( (pcParameterString[0]=='0') && (pcParameterString[1]=='x' || (pcParameterString[0]=='X') ) )
	{	// parameter string should be 0xstart,0xlength
		tok = strtok((char *)pcParameterString,",");
		sscanf(tok,"%x",&addr);
		tok = strtok(NULL,",");
		if( NULL != tok) {sscanf(tok,"%x",&length);}
		calc_checksum(addr,length);
	}
    else
        pcWriteBuffer+=uAppendString(pcWriteBuffer,"Unrecognised checksum parameter!\r\n");

    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

extern bool global_message_trace_flag;
static portBASE_TYPE prvMessageDumpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t *pcParameterString;
    static portBASE_TYPE xParameterNumber = 1;
    portBASE_TYPE xParameterStringLength;
    char pcParameter[20];    // ought to be long enough for on/off
	pcParameterString = ( int8_t * ) FreeRTOS_CLIGetParameter
                            (
                                pcCommandString,		/* The command string itself. */
                                xParameterNumber,		/* Return the next parameter. */
                                &xParameterStringLength	/* Store the parameter string length. */
                            );
    if (xParameterStringLength>sizeof(pcParameter)-1) xParameterStringLength=sizeof(pcParameter)-1;   // clip it

    if (strcmp((const char *)pcParameterString,"on")==0)
    {
		global_message_trace_flag = true;
        zprintf(CRITICAL_IMPORTANCE,"Message dumps enabled\r\n");
		DbgConsole_Flush();
    }
    else if (strcmp((const char *)pcParameterString,"off")==0)
    {
		global_message_trace_flag = false;
        zprintf(CRITICAL_IMPORTANCE,"Message dumps disabled\r\n");
		DbgConsole_Flush();
    }
    else
        pcWriteBuffer+=uAppendString(pcWriteBuffer,"Unrecognised messagedump parameter!\r\n");

    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}


static portBASE_TYPE prvRFMACMangleCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t *pcParameterString;
    static portBASE_TYPE xParameterNumber = 1;
    portBASE_TYPE xParameterStringLength;
    char pcParameter[20];    // ought to be long enough for on/off
	pcParameterString = ( int8_t * ) FreeRTOS_CLIGetParameter
                            (
                                pcCommandString,		/* The command string itself. */
                                xParameterNumber,		/* Return the next parameter. */
                                &xParameterStringLength	/* Store the parameter string length. */
                            );
    if (xParameterStringLength>sizeof(pcParameter)-1) xParameterStringLength=sizeof(pcParameter)-1;   // clip it

    if (strcmp((const char *)pcParameterString,"on")==0)
    {
        zprintf(CRITICAL_IMPORTANCE,"Enabling mangling of RF MAC address\r\n");
		DbgConsole_Flush();
		product_configuration.rf_mac_mangle = true;
		write_product_configuration();
		// Reset MCU
		NVIC_SystemReset(); // never returns
    }
    else if (strcmp((const char *)pcParameterString,"off")==0)
    {
        zprintf(CRITICAL_IMPORTANCE,"Disabled mangling of RF MAC address\r\n");
		DbgConsole_Flush();
		product_configuration.rf_mac_mangle = false;
		write_product_configuration();
		// Reset MCU
		NVIC_SystemReset(); // never returns
    }
    else
        pcWriteBuffer+=uAppendString(pcWriteBuffer,"Unrecognised rfmacmangle parameter!\r\n");

    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// pairmode command
extern bool display_pairing_success;
static portBASE_TYPE prvPairCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	PAIRING_REQUEST request = {0,true,PAIRING_REQUEST_SOURCE_CLI};
    int8_t *pcParameterString;
    static portBASE_TYPE xParameterNumber = 1;
    portBASE_TYPE xParameterStringLength;
    char pcParameter[20];    // ought to be long enough for an IP address
    pcParameterString = ( int8_t * ) FreeRTOS_CLIGetParameter
                            (
                                pcCommandString,		/* The command string itself. */
                                xParameterNumber,		/* Return the next parameter. */
                                &xParameterStringLength	/* Store the parameter string length. */
                            );
    if (xParameterStringLength>sizeof(pcParameter)-1) xParameterStringLength=sizeof(pcParameter)-1;   // clip it

    if (strcmp((const char *)pcParameterString,"on")==0)
    {
		request.enable = true;
        surenet_set_hub_pairing_mode(request);
    }
    else if (strcmp((const char *)pcParameterString,"off")==0)
    {
		request.enable = false;
        surenet_set_hub_pairing_mode(request);
    }
    else
        pcWriteBuffer+=uAppendString(pcWriteBuffer,"Unrecognised command!\r\n");

    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvPrintLevelCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 			*pcParameterString;
    portBASE_TYPE	xParameterNumber = 1;
    portBASE_TYPE 	xParameterStringLength;
    char 			pcParameter[20];    // should only really need to be a few bytes long
    int8_t 		numberToChange;

	*pcWriteBuffer='\0';
    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if( '\0' == pcParameterString )
	{	// We've not been given the level, so just report it.
//		sprintf(pcWriteBuffer, "Print Level: %d\r\n\0", printLevel);
		return pdFALSE;
	}

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

    if((*pcParameterString < '0') || (*pcParameterString > '9'))
    {
        sprintf(pcWriteBuffer, "NOT A NUMBER\r\n");
        return pdFALSE;
    }


    numberToChange = atoi((char *)pcParameterString);

    if( (numberToChange <= MAX_PRINT_LEVEL) && (numberToChange >= MIN_PRINT_LEVEL) )
	{
		pcWriteBuffer += sprintf(pcWriteBuffer, "Changing from level %d to level %d\r\n", printLevel, numberToChange);
		printLevel = numberToChange;
	} else
	{
		pcWriteBuffer += sprintf(pcWriteBuffer, "Error. Must be a number between %d and %d\r\n", MIN_PRINT_LEVEL, MAX_PRINT_LEVEL);
	}

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)

	return pdFALSE;
}

extern const char *pin_speed_names[];
static portBASE_TYPE prvPinSpeedCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 			*pcParameterString;
    portBASE_TYPE	xParameterNumber = 1;
    portBASE_TYPE 	xParameterStringLength;
    char 			pcParameter[20];    // should only really need to be a few bytes long
    int8_t 			state;
	PIN_SPEED		spi_pin_speed,eth_pin_speed;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

    spi_pin_speed = (PIN_SPEED)atoi((char *)pcParameter);

	xParameterNumber++;	// skip on to next parameter

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

    eth_pin_speed = (PIN_SPEED)atoi((char *)pcParameter);

	if( (PIN_SPEED_BACKSTOP>spi_pin_speed) && (PIN_SPEED_BACKSTOP>eth_pin_speed))
	{
		configure_high_speed_pins (spi_pin_speed, eth_pin_speed);

		zprintf(MEDIUM_IMPORTANCE,"Set FlexSPI to %s and Ethernet to %s\r\n", \
										pin_speed_names[spi_pin_speed],
										pin_speed_names[eth_pin_speed]);
	}
	else
	{
		zprintf(MEDIUM_IMPORTANCE,"pinspeed: invalid parameter. Idiot\r\n");
	}

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)

	return pdFALSE;
}


static portBASE_TYPE prvSetProductStateCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 			*pcParameterString;
    portBASE_TYPE	xParameterNumber = 1;
    portBASE_TYPE 	xParameterStringLength;
    char 			pcParameter[20];    // should only really need to be a few bytes long
    int8_t 			state;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

    if((*pcParameterString < '0') || (*pcParameterString > '2'))
    {
        sprintf(pcWriteBuffer, "Invalid parameter\r\n");
        return pdFALSE;
    }


    state = atoi((char *)pcParameterString);

	set_product_state((PRODUCT_STATE) state);

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)

	return pdFALSE;
}

static portBASE_TYPE prvForgetDeviceCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    portBASE_TYPE		 	xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;

    pcParameterString = (int8_t*)FreeRTOS_CLIGetParameter( pcCommandString,		/* The command string itself. */
							                               xParameterNumber,		/* Return the next parameter. */
                            							   &xParameterStringLength );	/* Store the parameter string length. */
	if( xParameterStringLength > sizeof(pcParameter) - 1 )
	{
		xParameterStringLength = sizeof(pcParameter) - 1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

    mac = read_mac_address(pcParameter);
    if((mac & 0x0000ffff)==0)
    {
        pcWriteBuffer += sprintf(pcWriteBuffer, "\r\nWILL WIPE ALL\r\n\r\n");
        mac = 0;
    } else
    {
        pcWriteBuffer += sprintf(pcWriteBuffer, "MAC to unpair: %08x %08x\r\n", (uint32_t)(mac >> 32), (uint32_t)(mac & 0xffffffff));
    }

	*pcWriteBuffer = '\0';
    xQueueSend(xRemoveFromPairingTable, &mac, 0);

    return pdFALSE;
}

// unpair command
static portBASE_TYPE prvUnPairCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    int8_t *				pcParameterString;
    static portBASE_TYPE	xParameterNumber = 1;
    portBASE_TYPE			xParameterStringLength;
    char 					pcParameter[20];    // ought to be long enough for an IP address
	uint64_t				mac;

    pcParameterString = ( int8_t * ) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
																xParameterNumber,		/* Return the next parameter. */
																&xParameterStringLength	/* Store the parameter string length. */
																);
    if (xParameterStringLength > sizeof(pcParameter) - 1)
	{
		xParameterStringLength = sizeof(pcParameter) - 1;   // clip it
	}
    strncpy(pcParameter,(const char *)pcParameterString,xParameterStringLength);

	mac = read_mac_address (pcParameter);

	xQueueSend (xUnpairDeviceMailbox, &mac, 0);

    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// pairtable command
static portBASE_TYPE prvPairTableCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    device_table_dump(); // the output of this should go into pcWriteBuffer, one line at a time, until complete.
    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// serverbuf command
static portBASE_TYPE prvServerBufferCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    server_buffer_dump(); // the output of this should go into pcWriteBuffer, one line at a time, until complete.
    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// devicebuf command
static portBASE_TYPE prvDeviceBufferCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    device_buffer_dump(); // the output of this should go into pcWriteBuffer, one line at a time, until complete.
    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// mqttstat command
static portBASE_TYPE prvMQTTStatusCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	mqtt_status_dump();
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}
// hubreg command
static portBASE_TYPE prvDumpHubRegistersCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    HubReg_Dump(); // the output of this should go into pcWriteBuffer, one line at a time, until complete.
    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// Test LED driver
static portBASE_TYPE prvLEDCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	int8_t*				pcParameterString;
	portBASE_TYPE		xParameterNumber = 1;
	portBASE_TYPE		xParameterStringLength;
	uint32_t			i;
	LED_PATTERN_TYPE	pattern = LED_PATTERN_SOLID;
	LED_COLOUR			colour	= COLOUR_GREEN;
	LED_MODE			mode	= LED_MODE_OFF;
	uint32_t			duration = 0;

	pcParameterString = (int8_t*)FreeRTOS_CLIGetParameter( pcCommandString, xParameterNumber, &xParameterStringLength	);

	while( (NULL != pcParameterString) && ('\0' != *pcParameterString) )
	{
		for( i=0; i<LED_MODE_BACKSTOP; i++ )
		{
			if( 0 == strncmp(led_mode_strings[i], (char const*)pcParameterString, xParameterStringLength) )
			{
				mode = (LED_MODE)i;
			}
		}

		for( i=0; i<LED_PATTERN_BACKSTOP; i++ )
		{
			if( 0 == strncmp(led_pattern_strings[i], (char const*)pcParameterString, xParameterStringLength) )
			{
				pattern = (LED_PATTERN_TYPE)i;
			}
		}

		for( i=0; i<COLOUR_BACKSTOP; i++ )
		{
			if( 0 == strncmp(led_colour_strings[i], (char const*)pcParameterString, xParameterStringLength) )
			{
				colour = (LED_COLOUR)i;
			}
		}

		if ((pcParameterString[0]>='0') && (pcParameterString[0]<='9'))
		{
			sscanf((char const *)pcParameterString,"%d",&duration);
		}

		xParameterNumber++;
		pcParameterString = (int8_t*)FreeRTOS_CLIGetParameter( pcCommandString, xParameterNumber, &xParameterStringLength	);
	};

	LED_Request_Pattern(mode, colour, pattern, duration);	// Pass it on.

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
	return pdFALSE;
}



// ifconfig command
extern phy_speed_t phy_speed;
extern phy_duplex_t phy_duplex;

static portBASE_TYPE prvIfConfigCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
    uint32_t pulIPAddress, pulNetMask, pulGatewayAddress, pulDNSServerAddress;

    FreeRTOS_GetAddressConfiguration( &pulIPAddress, &pulNetMask, &pulGatewayAddress, &pulDNSServerAddress );

    zprintf(CRITICAL_IMPORTANCE, "IP Address: ");
	zprintf(CRITICAL_IMPORTANCE, "%d.%d.%d.%d",(pulIPAddress&0xff),((pulIPAddress>>8)&0xff),((pulIPAddress>>16)&0xff),((pulIPAddress>>24)&0xff));
    zprintf(CRITICAL_IMPORTANCE, "\r\nNetmask:    ");
	zprintf(CRITICAL_IMPORTANCE, "%d.%d.%d.%d",(pulNetMask&0xff),((pulNetMask>>8)&0xff),((pulNetMask>>16)&0xff),((pulNetMask>>24)&0xff));
    zprintf(CRITICAL_IMPORTANCE, "\r\nGateway:    ");
	zprintf(CRITICAL_IMPORTANCE, "%d.%d.%d.%d",(pulGatewayAddress&0xff),((pulGatewayAddress>>8)&0xff),((pulGatewayAddress>>16)&0xff),((pulGatewayAddress>>24)&0xff));
    zprintf(CRITICAL_IMPORTANCE, "\r\nDNS Server: ");
	zprintf(CRITICAL_IMPORTANCE, "%d.%d.%d.%d",(pulDNSServerAddress&0xff),((pulDNSServerAddress>>8)&0xff),((pulDNSServerAddress>>16)&0xff),((pulDNSServerAddress>>24)&0xff));
	if( kPHY_Speed10M == phy_speed)
		zprintf(CRITICAL_IMPORTANCE, "\r\nPHY speed:  10Mbit");
	else
		zprintf(CRITICAL_IMPORTANCE, "\r\nPHY speed:  100Mbit");
	if( kPHY_HalfDuplex == phy_duplex)
		zprintf(CRITICAL_IMPORTANCE, "\r\nPHY duplex: Half Duplex");
	else
		zprintf(CRITICAL_IMPORTANCE, "\r\nPHY duplex: Full Duplex");
#if( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
    zprintf(CRITICAL_IMPORTANCE, "\r\nHostname:   ");
    zprintf(CRITICAL_IMPORTANCE, (char*)pcApplicationHostnameHook());
#endif
    zprintf(CRITICAL_IMPORTANCE, "\r\n");

	return pdFALSE;
}

static portBASE_TYPE prvNetStatCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	FreeRTOS_netstat();
	return pdFALSE;
}

uint8_t hex_ascii_to_int (char * ptr)
{
	// convert a pointed-to ascii hex character to its integer (0-f) value
	// non-hex characters just return 0
	// this is exposed in the .h file as MQTT_Simulator also needs it

	uint8_t retval = 0;
	if (*ptr < '0')				// less than '0'
	{
		retval = 0;
	}
	else if (*ptr < 0x3a)		// decimal digit
	{
		retval = *ptr - '0';
	}
	else if (*ptr < 'A')		// between digit and 'A'
	{
		retval = 0;
	}
	else if (*ptr < 'G')		// hex upper case digit
	{
		retval = (*ptr - 'A') + 0x0a;
	}
	else if (*ptr < 'a')		// between 'F' and 'a'
	{
		retval = 0;
	}
	else if (*ptr < 'g')		// hex lower case digit
	{
		retval = (*ptr - 'a') + 0x0a;
	}
	return retval;
}

uint64_t read_mac_address (char * tmac)
{
	// Convert a mac parameter from text to a uint64_t value
	// To avoid typing, if the provided text mac is of fewer than seven characters, assume that the
	// remaining higher bytes are always 0x70b3d5f9c0000000 and simply append (this will work for
	// impf, idscf, and poseidon). Otherwise return the entire entered value.
	// If a * is entered as the mac, return the previously entered mac address
	// By request of Tom, pxx returns the mac address of the xxth entry in the pairing table

	static uint64_t last_mac = 0x0;
	uint64_t		mac = 0x0;
	char *			ptr = tmac;
	uint16_t		row = 0;

	if ('p' == *tmac)
	{
		// user typed a p; what is the associated number?
		row = atoi (&tmac[1]);
		// now get the mac address from the row number
		mac = get_mac_from_index (row);
	}
	else if ('*' == *tmac)
	{
		// user typed a *
		mac = last_mac;
	}
	else if (0 == strtoull(tmac, NULL, 16))
	{
		mac = 0;	// special case to wipe all pairs if parameter is 0 (or 00, or 000 etc.)
	}
	else
	{
		mac = strtoull (ptr, NULL, 16);
		// is the mac entered too big to use the default mac base?
		if (mac <= 0xfffffff)
		{
			// no, so add the mac base
			mac += 0x70b3d5f9c0000000;
		}
		// otherwise we will use the mac as entered
		last_mac = mac;			// save it for next time
	}
	return mac;
}

static portBASE_TYPE prvAwakeCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	// set the display visibility of device awake messages, by zapping a couple of variables in
	// surenet_device_awake_notification_cb
	// input	result
	// 0		show no notifications
	// 1		show notifications where there is data available; otherwise just an alphanumeric character (different
	//			for each device, based on position in pairing table 0-9, a-z, A-X)
	// 2		show notifications for everything - this can get busy!
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[128];   			// could be a long one
	extern 					bool SDAN_QUIET_MODE;
	extern					bool SDAN_VERY_QUIET_MODE;
	uint8_t					mode;

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 1 = the ascii string. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';

	mode = (uint8_t) atoi (pcParameter);
	switch (mode)
	{
		case 2:
			SDAN_QUIET_MODE = false;
			SDAN_VERY_QUIET_MODE = false;
			break;
		case 1:
			SDAN_QUIET_MODE = true;
			SDAN_VERY_QUIET_MODE = false;
			break;
		case 0:
		default:
			SDAN_QUIET_MODE = true;
			SDAN_VERY_QUIET_MODE = true;
			break;
	}


	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvPublishCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString)
{
	MQTT_MESSAGE	message;
	portBASE_TYPE 	xParameterStringLength;
	const char* 	pcParameterString = FreeRTOS_CLIGetParameter( pcCommandString, 1, &xParameterStringLength	);

	memset(&message, '\0', sizeof(message));

	snprintf(message.message, MAX_INCOMING_MQTT_MESSAGE_SIZE_SMALL-1, "%08x 0FF0 %s\0", get_UTC(), pcParameterString);

	xQueueSend(xOutgoingMQTTMessageMailbox, &message, 0);
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// Run test cases defined in Hermes-test.c
static portBASE_TYPE prvTestCaseCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t *pcParameterString;
    static portBASE_TYPE xParameterNumber = 1;
    portBASE_TYPE xParameterStringLength;
    char pcParameter[20];
    char * pToken;
    uint32_t firstParam;
    pcParameterString = ( int8_t * ) FreeRTOS_CLIGetParameter
                            (
                                pcCommandString,		/* The command string itself. */
                                xParameterNumber,		/* Return the next parameter. */
                                &xParameterStringLength	/* Store the parameter string length. */
                            );
    if (xParameterStringLength>sizeof(pcParameter)-1) xParameterStringLength=sizeof(pcParameter)-1;   // clip it

    /* separate the params */
    pToken = strtok ((char*)pcParameterString,",");
    firstParam = atoi(pToken);
    pToken = strtok (NULL,",");

    if (strncmp((const char *)pcParameterString,"help", 4)==0)
    {
        DbgConsole_Flush();
        zprintf(CRITICAL_IMPORTANCE,"1: Write Ethernet MAC - 12 hex chars\te.g. testcase 1,1199aabbEEFF\r\n");
        zprintf(CRITICAL_IMPORTANCE,"2: Get Ethernet MAC from NV Store\te.g. testcase 2\r\n");
        zprintf(CRITICAL_IMPORTANCE,"3: Write RF MAC address - 16 hex chars\te.g. testcase 3,1234567890abcDEF\r\n");
        DbgConsole_Flush();
		zprintf(CRITICAL_IMPORTANCE,"4: Get RF MAC from NV Store\t\te.g. testcase 4\r\n");
        zprintf(CRITICAL_IMPORTANCE,"5: Write Serial Number to NV Store\te.g. testcase 5,(12 x acceptable ascii chars)\r\n");
        zprintf(CRITICAL_IMPORTANCE,"6: Get Serial Number from NV Store\te.g. testcase 6\r\n");
        DbgConsole_Flush();
		zprintf(CRITICAL_IMPORTANCE,"7: Write rf_pan_id - 4 hex chars\te.g. testcase 7,(4 x 0....fF)\r\n");
        zprintf(CRITICAL_IMPORTANCE,"8: Get rf_pan_id from NV Store\t\te.g. testcase 8\r\n");
        zprintf(CRITICAL_IMPORTANCE,"9: Write Device Stats to NV Store\te.g. testcase 9  <<uses pre-defined data>>\r\n");
        DbgConsole_Flush();
		zprintf(CRITICAL_IMPORTANCE,"10: Get Device Stats from NV Store\te.g. testcase 10\r\n");
        zprintf(CRITICAL_IMPORTANCE,"13: Erase a sector\t\t\te.g. testcase 13,9...1022\r\n");         // 8 <> 1023 ...exclude start code and persistent data
        zprintf(CRITICAL_IMPORTANCE,"15: Write aes_high\r\n");
        DbgConsole_Flush();
		zprintf(CRITICAL_IMPORTANCE,"16: Get aes_high\r\n");
        zprintf(CRITICAL_IMPORTANCE,"17: Write aes_low\r\n");
        zprintf(CRITICAL_IMPORTANCE,"18: Get aes_low\r\n");
        DbgConsole_Flush();
//		zprintf(CRITICAL_IMPORTANCE,"19: Store Secret Serial Number\r\n");
//        zprintf(CRITICAL_IMPORTANCE,"20: Get Secret Serial from Store\r\n");
        zprintf(CRITICAL_IMPORTANCE,"22: GET_FT_REV_NUM\r\n");
        DbgConsole_Flush();
		zprintf(CRITICAL_IMPORTANCE,"24: GET_FT_STDBY_MA\r\n");
        zprintf(CRITICAL_IMPORTANCE,"26: GET_FT_GREEN_MA\r\n");
        zprintf(CRITICAL_IMPORTANCE,"28: GET_FT_RED_MA\r\n");
        DbgConsole_Flush();
		zprintf(CRITICAL_IMPORTANCE,"30: GET_FT_PASS_RESULTS\r\n");
        zprintf(CRITICAL_IMPORTANCE,"32: Write the PRODUCT_CONFIGURATION to flash\r\n");
        zprintf(CRITICAL_IMPORTANCE,"40: Read H/W version from resistors\r\n");
        DbgConsole_Flush();
        // Factory Testing Results etc - used by the Programmer
        // 21 = SET_FT_REV_NUM
        // 22 = GET_FT_REV_NUM
        // 23 = SET_FT_STDBY_MA
        // 24 = GET_FT_STDBY_MA
        // 25 = SET_FT_GREEN_MA
        // 26 = GET_FT_GREEN_MA
        // 27 = SET_FT_RED_MA
        // 28 = GET_FT_RED_MA
        // 29 = SET_FT_PASS_RESULTS
        // 30 = GET_FT_PASS_RESULTS
        // 32 = write_product_configuration

        // 40 = Read H/W version from binary resistors
        // 42 = Read UNIQUE_CHIP_ID

        // 50 = Read efuse index address
        // 51 = Burn BEE_KEY1_SEL
        // 52 = Burn BOOT_FUSE_SEL
        // 53 = Burn LOCK_FUSES

        // 73 = Write target RF MAC
        // 74 = Read target RF MAC
        // 75 = Initialise Surenet (RF stack)
        // 76 = Surenet pairing test
        // 77 = Add a MAC (created by testcase 73) to the pairing table.

        // 168 = IP to DHCP and connect to server
        // 192 = TCP/IP socket test
        // 193 = Send network down event
        // 194 = Check link status (cable connected?)

    }
    /* these testcases include a comma separated parameter */
    else if ( (firstParam == 1) || (firstParam == 3) || (firstParam == 5) || (firstParam == 7) || (firstParam == 13) \
            || (firstParam == 15) || (firstParam == 17) || (firstParam == 19) \
            || (firstParam == 21) || (firstParam == 23) || (firstParam == 25) || (firstParam == 27) || (firstParam == 29) \
            || (firstParam == 50) || (firstParam == 73) )
    {
        hermesTestRunCase(firstParam, (int8_t*)pToken);
    }
    /* testcases without additional parameter */
    else if ( (firstParam == 2) || (firstParam == 4) || (firstParam == 6) || (firstParam == 8) || (firstParam == 9) || (firstParam == 10) \
            || (firstParam == 11) || (firstParam == 12) || (firstParam == 16) || (firstParam == 18) || (firstParam == 20) || (firstParam == 22) \
            || (firstParam == 24) || (firstParam == 26) || (firstParam == 28) || (firstParam == 30) || (firstParam == 32) || (firstParam == 42) \
            || (firstParam == 51) || (firstParam == 52) || (firstParam == 53) \
            || (firstParam == 74) || (firstParam == 76) || (firstParam == 77) || (firstParam == 78) \
            || (firstParam == 168) || (firstParam == 192) || (firstParam == 193) || (firstParam == 194) )
    {
        hermesTestRunCase(firstParam, NULL);
    }
    else if (firstParam == 40)
    {
        uint8_t hwVer = readHwVersion();                                        // get from binary weighted resistors
        zprintf(CRITICAL_IMPORTANCE, "HW_VER=%d\r\n", hwVer);
    }
    else if (firstParam == 75)
    {
        initSureNetByProgrammer();                                              // initialise SureNet
    }
    else
        zprintf(CRITICAL_IMPORTANCE,"Enter a number from 1 to 20 and optional parameter... see \"testcase help\" \r\n");

    *pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvHRMFullDumpCommand( char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString )
{
	sprintf(pcWriteBuffer, "Attempting a full register dump.\r\n");
	HubReg_Send_All();
	return pdFALSE;
}

static portBASE_TYPE prvTranscribeCommand( char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString )
{
	int8_t*				pcParameterString;
	portBASE_TYPE		xParameterNumber = 1;
	portBASE_TYPE		xParameterStringLength;

	pcParameterString = (int8_t*)FreeRTOS_CLIGetParameter( pcCommandString, xParameterNumber, &xParameterStringLength	);

	if( (NULL != pcParameterString) && ('\0' != *pcParameterString) )
	{
		if( 't' == *pcParameterString )
		{
			BM_TranscribeBank();
		} else if( 's' == *pcParameterString )
		{
			__MEMORY_BARRIER();
			BM_BankSwitch(true, true);
			__MEMORY_BARRIER();
		} else if( 'r' == *pcParameterString )
		{
			BM_ReportBank();
		} else if( 'c' == *pcParameterString )
		{
			BM_ConfirmBank(false);
		} else
		{
			sprintf(pcWriteBuffer, "No command!\r\n");
		}
	} else
	{
		sprintf(pcWriteBuffer, "No command!\r\n");
	}

	*pcWriteBuffer = '\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
	return pdFALSE;
}
// Appends a string pcStr to an existing string pcDest. Returns how many characters appended
uint32_t uAppendString(char *pcDest, char *pcStr)
{
    strcpy(pcDest,pcStr);
    return strlen(pcStr);
}

