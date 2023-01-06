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
* Filename: CLI-Commands-Simulator.c    
* Author:   Neil Barnes
* Purpose:     
*             
*             
*
**************************************************************************/

#include "MQTT.h"	// Can hold the MQTT_SERVER_SIMULATED #define, if it's not Project-defined.

#ifdef MQTT_SERVER_SIMULATED
/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"

#include "MQTT-Simulator.h"
#include "CLI-Commands-Simulator.h"

static portBASE_TYPE prvSetRegisterCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvSetTraining3Command	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvSetFastRFCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvSetSettingCommand 	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvGenericCommand 		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvHubCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvSettingsCommand 	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvProfilesCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvCurfewsCommand		(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvInfoCommand			(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE prvPingDeviceCommand	(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);


/* Structure that defines the "setreg" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvSetRegisterCommandDefinition =
{
	( const char * const ) "setreg",
	( const char * const ) "setreg:         Set a named register to a desired value\r\n",
	prvSetRegisterCommand, /* The function to run. */
	2 /* Two parameters should be entered. */
};

/* Structure that defines the "settraining3" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvSetTraining3CommandDefinition =
{
	( const char * const ) "train3",
	( const char * const ) "train3:         Set a paired IMPF to training mode 3, enter last 4 digits of MAC\r\n",
	prvSetTraining3Command, /* The function to run. */
	1 	/* one parameter should be entered. */
};

/* Structure that defines the "fastrf" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvSetFastRFCommandDefinition =
{
	( const char * const ) "fastrf",
	( const char * const ) "fastrf:         Set a paired IMPF fast RF mode, enter last 4 digits of MAC\r\n",
	prvSetFastRFCommand, /* The function to run. */
	1 	/* one parameter should be entered. */
};

/* Structure that defines the "setting" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvSetSettingCommandDefinition =
{
	( const char * const ) "setting",
	( const char * const ) "setting:        Set a device setting, enter last digits of MAC, setting address, setting value\r\n",
	prvSetSettingCommand, /* The function to run. */
	3 	/* three parameters should be entered. */
};

/* Structure that defines the "generic" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvGenericCommandDefinition =
{
	( const char * const ) "generic",
	( const char * const ) "generic:        Send a generic message, MAC, message_type, then message in comma separated hex, no spaces\r\n",
	prvGenericCommand, /* The function to run. */
	3 	/* three parameters should be entered. */
};

/* Structure that defines the "hub" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvHubCommandDefinition =
{
	( const char * const ) "hub",
	( const char * const ) "hub:            Send a message to hub, data as comma separated hex, no spaces\r\n",
	prvHubCommand, /* The function to run. */
	1 	/* three parameters should be entered. */
};

/* Structure that defines the "settings" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvSettingsCommandDefinition =
{
	( const char * const ) "settings",
	( const char * const ) "settings:       Request the settings from a paired device: enter short mac\r\n",
	prvSettingsCommand, /* The function to run. */
	1 	/* three parameters should be entered. */
};

/* Structure that defines the "profiles" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvProfilesCommandDefinition =
{
	( const char * const ) "profiles",
	( const char * const ) "profiles:       Request the profiles from a paired device: enter short mac\r\n",
	prvProfilesCommand, /* The function to run. */
	1 	/* three parameters should be entered. */
};

/* Structure that defines the "curfew" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvCurfewsCommandDefinition =
{
	( const char * const ) "curfews",
	( const char * const ) "curfews:        Request the curfews from a paired device: enter short mac\r\n",
	prvCurfewsCommand, /* The function to run. */
	1 	/* three parameters should be entered. */
};

/* Structure that defines the "info" command */
// THIS IS TEMPORARY FOR TESTING ONLY
CLI_Command_Definition_t prvInfoCommandDefinition =
{
	( const char * const ) "info",
	( const char * const ) "info:           Get info from a paired device: enter short mac\r\n",
	prvInfoCommand, /* The function to run. */
	1 	/* one parameters should be entered. */
};

/* Structure that defines the "pingd" command */
CLI_Command_Definition_t prvPingDeviceCommandDefinition =
{
	( const char * const ) "pingd",
	( const char * const ) "pingd:          Ping a device as if from the server; enter MAC\r\n",
	prvPingDeviceCommand, /* The function to run. */
	2 	/* two parameters should be entered. */
};

void vRegisterSimulatorCLICommands(void)
{
	FreeRTOS_CLIRegisterCommand	(&prvSetRegisterCommandDefinition);
	FreeRTOS_CLIRegisterCommand (&prvSetTraining3CommandDefinition);
	FreeRTOS_CLIRegisterCommand (&prvSetFastRFCommandDefinition);
 	FreeRTOS_CLIRegisterCommand (&prvSetSettingCommandDefinition);
 	FreeRTOS_CLIRegisterCommand (&prvGenericCommandDefinition);
 	FreeRTOS_CLIRegisterCommand (&prvHubCommandDefinition);
 	FreeRTOS_CLIRegisterCommand (&prvSettingsCommandDefinition);
 	FreeRTOS_CLIRegisterCommand (&prvProfilesCommandDefinition);
 	FreeRTOS_CLIRegisterCommand (&prvCurfewsCommandDefinition);
 	FreeRTOS_CLIRegisterCommand (&prvInfoCommandDefinition);
    FreeRTOS_CLIRegisterCommand	(&prvPingDeviceCommandDefinition); 
}

// Set Register Command
static portBASE_TYPE prvSetRegisterCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    static portBASE_TYPE 	xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be one byte long
	int						reg;
	int						val;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy(pcParameter,(const char *)pcParameterString,xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	reg = atoi (pcParameter);

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                2,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy(pcParameter,(const char *)pcParameterString,xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	val = atoi (pcParameter);

	MQTT_set_register_message (MSG_SET_ONE_REG, (HUB_REGISTER_TYPE) reg, (uint8_t) val);

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvHubCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	// send a generic message to a hub; uses comma separated hex with no spaces
	// e.g. 	hub 1 18 3 <-- set single reg, led reg, mode 3
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[128];   			// could be a long one
	char *					ptr;

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 1 = the ascii string. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	// remove all the commas from the list and replace with spaces
	ptr = pcParameter;
	while (*ptr != '\0')
	{
		if (',' == *ptr)
		{
			*ptr = ' ';
		}
		ptr++;
	}

	MQTT_generic_hub_msg (pcParameter);

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// Set training mode 3 command
static portBASE_TYPE prvSetTraining3Command( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    portBASE_TYPE		 	xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);
	MQTT_set_training_mode_3 (mac);
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// Set fastrf command (change rf response setting to 500 instead of 5000
static portBASE_TYPE prvSetFastRFCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);
	MQTT_set_fastrf (mac);
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvSetSettingCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	// generic write access to device settings; setting number and value must be in decimal
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];
	uint64_t				mac;
	uint16_t				address;
	int32_t					value;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 1 = short mac address. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 2 = setting number. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	address = atoi (pcParameter);

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 3 = setting value. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	value = atoi (pcParameter);

	MQTT_set_setting (mac, address, value);

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvGenericCommand (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	// send a generic message to a paired device; uses comma separated hex with no spaces
	// e.g. 	generic 402a 9 05,03,00,00,00 = set settings 5 to 3 = set training mode 3; on 70b3d5f9c000402a
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[128];   			// could be a long one
	uint64_t				mac;
	uint16_t				type;
	char *					ptr;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 1 = short mac address. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 2 = message type. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	type = strtol (pcParameter, NULL, 16);

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 3 = the hex string. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */

	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	// remove all the commas from the list and replace with spaces
	ptr = pcParameter;
	while (*ptr != '\0')
	{
		if (',' == *ptr)
		{
			*ptr = ' ';
		}
		ptr++;
	}

	MQTT_generic_device_msg (mac, type, pcParameter);

	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvCurfewsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);
	MQTT_request_curfews (mac);
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvSettingsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);
	MQTT_request_settings (mac);
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvProfilesCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);
	MQTT_request_profiles (mac);
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

static portBASE_TYPE prvInfoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;
    portBASE_TYPE 			xParameterNumber = 1;
    portBASE_TYPE 			xParameterStringLength;
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;

    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1)
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);
	MQTT_get_info (mac);
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

// Ping device command
static portBASE_TYPE prvPingDeviceCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    int8_t 					*pcParameterString;  
    portBASE_TYPE		 	xParameterNumber = 1; 
    portBASE_TYPE 			xParameterStringLength;  
    char 					pcParameter[20];    // should only really need to be a few bytes long
	uint64_t				mac;
	int8_t					type;
    
    pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,		/* The command string itself. */
							                                xParameterNumber++,		/* Return the next parameter. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1) 
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	mac = read_mac_address (pcParameter);

	pcParameterString = (int8_t *) FreeRTOS_CLIGetParameter (pcCommandString,			/* The command string itself. */
							                                xParameterNumber++,			/* param 3 = the hex string. */
                            							    &xParameterStringLength);	/* Store the parameter string length. */
	if (xParameterStringLength > sizeof(pcParameter) -1) 
	{
		xParameterStringLength = sizeof(pcParameter) -1;   // clip it
	}
    strncpy (pcParameter, (const char *)pcParameterString, xParameterStringLength);
    pcParameter[xParameterStringLength]='\0';
	type = strtol (pcParameter, NULL, 16);	
	
	MQTT_ping_device_message (mac, type);
	
	*pcWriteBuffer='\0';    // ensure we don't return with duff info in write buffer (because it does get output!)
    return pdFALSE;
}

#endif	// MQTT_SERVER_SIMULATED