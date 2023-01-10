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
* Filename: message_parser.c
* Author:   Chris Cowdery 28/10/2019
* Purpose:  Parses messages received from Server, and despatch them.
*
**************************************************************************/

#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* Other includes */
#include "hermes-time.h"
#include "hermes-app.h"
#include "SureNet-Interface.h"
#include "message_parser.h"
#include "server_buffer.h"
#include "device_buffer.h"
#include "RegisterMap.h"
#include "utilities.h"
#include "devices.h"    // to get the max size of the Device Table.
#include "HubFirmwareUpdate.h"
#include "credentials.h"

// Main entry point is process_MQTT_message_from_server()

// hideous globals
extern uint8_t bootRN;

// local typedefs
typedef enum
{
    PARSE_COMMAND,
    PARSE_INDEX,
    PARSE_ADDRESS,
    PARSE_NUM_VALUES,
    PARSE_VALUES,
} PARSER_STATE;

typedef enum
{
    MESSAGE_FOR_HUB,
    MESSAGE_FOR_DEVICE,
} MESSAGE_DESTINATION;

// local functions
void TranslateToDevice(uint64_t mac, uint16_t address, uint8_t *values, bool sendGet, uint32_t numVals);
uint32_t msg_string_to_bytes(char *str, char *header, MSG_TYPE *cmd, int32_t *index, uint16_t *address, uint16_t *numValues, uint8_t *vals, uint32_t maxVals);

/**************************************************************
 * Function Name   : process_message_from_server
 * Description     : Takes a message from the server, which comprises a topic
 *                 : and a message. Parses the message and despatches
 *                 : a call to either handle it locally, or pass
 *                 : it on to a Device.
 * Inputs          : *message is a pointer to the message, and *subtopic is a pointer
 *                 : to either "messages" or "1122334455667788" MAC address
 * Outputs         :
 * Returns         :
 **************************************************************/
extern bool global_message_trace_flag;
void process_MQTT_message_from_server(char *message,char *subtopic)
{
    uint8_t 			values[MAX_MESSAGE_BYTES];
    char 				header[16];
    MSG_TYPE 			command;
    int32_t 			index;
    uint16_t 			address;
    uint16_t 			numValues;		// DNB was uint8_t
    uint64_t 			DeviceMAC;
	uint8_t 			ping_value;
    MESSAGE_DESTINATION message_destination;
    uint8_t 			i;
    uint8_t 			numAdd;

	// Check MQTT signature. Note that this call removes the signature from
	// the incoming message string.
	if( NULL != strstr(message,"Hub offline (Last Will)"))
	{ // The signature on this message will be wrong because it will have been signed during the previous connection
		return;
	}

	if( false == check_recd_mqtt_signature(message,subtopic))
	{
		zprintf(MEDIUM_IMPORTANCE,"Dropping received MQTT message with bad signature : %s\r\n",message);
		return;
	}

	// need to do a basic sanity check for malformed messages as the Server could
	// in theory send us any old nonsense! (e.g. Hub offline (Last Will) - which
	// will not parse satisfactorily
	// The first part of the message should be:
	// nnnnnnnn oiir blah blah
	// We check for the space between nnnnnnnn and oiir and between oiir and the first blah
	if( (message[8] != ' ') || (message[13] != ' ') )
	{
		return;
	}

    // first thing is to see if it's a reflected message
    if (server_buffer_process_reflected_message(message) == false)
    {   // it probably wasn't a reflected message, so continue to process it.
		// If the Server has sent a repeated reflected message, we can detect this,
		// because the oiir part of the message should take the form 0iir if
		// the message is reflected (1iir for Hub sourced, and 2iir for Chevron sourced)
		if( '0' == message[9])
		{
			return;	// bail out - this is a message that was originally sourced by us!
		}

		if( true == global_message_trace_flag)
		{
			zprintf(CRITICAL_IMPORTANCE,"Received MQTT message: %s | %s\r\n",subtopic,message);
		}

        // parse the message and convert it from a string to numbers.
        msg_string_to_bytes(message,header,&command,&index,&address,&numValues,values,MAX_MESSAGE_BYTES);

        // Decide what the destination of the message is
        if (strcmp(subtopic,"messages")==0)
        {
            message_destination=MESSAGE_FOR_HUB;
        }
        else
        {
            message_destination=MESSAGE_FOR_DEVICE;

			uint8_t* dMAC = (uint8_t*)&DeviceMAC;
			sscanf(subtopic, "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", dMAC, dMAC+1, dMAC+2, dMAC+3, dMAC+4, dMAC+5, dMAC+6, dMAC+7);
        }

        switch (message_destination)
        {
            case MESSAGE_FOR_HUB:
                switch (command)
                {
                    case MSG_HUB_VERSION_INFO:
						if((numValues==1)&&(values[0]==0))  //else just reflected message from hub
						{
							trigger_send_hub_version_info();
							zprintf(LOW_IMPORTANCE, "Received version info request\r\n");
						}
                        break;
                    case MSG_REPROGRAM_HUB:
						// BootRN is a token which is generated randomly every time the Hub reboots. This number is passed
						// to the Server in response to a MSG_HUB_VERSION_INFO. The Server then sends it as a parameter of
						// the MSG_REPROGRAM_HUB command. This mechanism is to prevent spurious f/w update processes
						// occurring if the MSG_REPROGRAM_HUB is sent multiple times by the Server, specifically once
						// before the f/w update (which triggers it), and once after it's completed and rebooted. It means
						// the second one will be ignored.
//                  		if((numValues==1)&&(values[0]==bootRN))  //this will detect and fail if hub rebooted since bootRN was generated
                  		if(values[0]==bootRN)  //this will detect and fail if hub rebooted since bootRN was generated
						{
							zprintf(LOW_IMPORTANCE, "Hub firmware update starting...\r\n");
							HFU_trigger(false);
						}
                        break;
                    case MSG_FLASH_SET:
                        zprintf(LOW_IMPORTANCE, "MSG_FLASH_SET not supported\r\n");
						// in hub 1, this is used for label printing and possibly for some handshaking between application and bootloader
                        break;
                    case MSG_HUB_REBOOT:
                        zprintf(LOW_IMPORTANCE, "Rebooting hub...\r\n");
                        NVIC_SystemReset(); // never returns
                        break;
                    case MSG_WEB_PING:	// This triggers the Hub to ping a Device, and report the response back to the server
                        zprintf(LOW_IMPORTANCE, "Received Ping request from Server\r\n");
						DeviceMAC = 0;
						ping_value = 0;
						if (numValues==1)
						{
							ping_value = values[0];
							surenet_ping_device(DeviceMAC,ping_value);
						}
						else if (numValues == 9)	// new feature to specify MAC address of Device.
						{
							ping_value = values[0];
							DeviceMAC = ((uint64_t)values[1])<<56 | ((uint64_t)values[2])<<48 | \
										((uint64_t)values[3])<<40 | ((uint64_t)values[4])<<32 | \
										((uint64_t)values[5])<<24 | ((uint64_t)values[6])<<16 | \
										((uint64_t)values[7])<<8  | ((uint64_t)values[8]);
							surenet_ping_device(DeviceMAC,ping_value);
						}
                        break;
                    case MSG_SET_ONE_REG:
                    case MSG_SET_REG_RANGE:
                        HubReg_SetValues(address, numValues, values, true);
                        break;
                    case MSG_SEND_REG_RANGE:    // get the Hub to send values back to the Server
                        HubReg_GetValues(address, numValues);
                        break;
                    case MSG_SET_DEBUG_MODE:
                        if( numValues == 1 )
						{
                            HubReg_SetDebugMode(NULL, values[0]);
						}
                        break;
                    case MSG_HUB_UPTIME:
                        zprintf(LOW_IMPORTANCE, "MSG_HUB_UPTIME not supported\r\n");
						// This isn't used in Hub 1
                        break;
					case MSG_REG_VALUES:
						zprintf(MEDIUM_IMPORTANCE, "MSG_REG_VALUES not supported by Hub\r\nMessage: %s\r\n",message);
						break;
					case MSG_CHANGE_SIGNING_KEY:
						key_copy_pending_to_current();
						break;
					case MSG_NONE:
						// This probably means a message with a different format has been received, e.g. a text error message
						zprintf(MEDIUM_IMPORTANCE, "MSG_NONE received by Hub\r\nMessage: %s Subtopic: %s\r\n",message, subtopic);
						break;
                    default:
						zprintf(MEDIUM_IMPORTANCE, "Unrecognised command received for Hub :0x%04X\r\n",command);
                        break;
                }
                break;

            case MESSAGE_FOR_DEVICE:
                switch(command)
                {
                    case MSG_HUB_THALAMUS:
                        TranslateToDevice(DeviceMAC,THALAMUS_DUMMY_REGISTER,values,false,numValues);
                        break;
                    case MSG_SET_ONE_REG:
                    case MSG_SET_REG_RANGE:
                        i = 0;
                        while(numValues>0)
                        {
                          numAdd = (numValues>MAX_REGISTERS_TO_DEVICE) ? MAX_REGISTERS_TO_DEVICE : numValues;
                          TranslateToDevice(DeviceMAC, address+i, &values[i], false, numAdd);  //TODO check how ID used mqtt_publishes[idx].message_id);
                          numValues -= numAdd;
                          i += numAdd;
                        }
                        break;
                    case MSG_SEND_REG_RANGE:
                        TranslateToDevice(DeviceMAC,address,NULL,true,numValues);
                        break;
                    case MSG_REG_VALUES:
                        zprintf(LOW_IMPORTANCE, "MSG_REG_VALUES not supported for devices\r\n");
                        break;
                    default:
						zprintf(MEDIUM_IMPORTANCE, "Unrecognised message received for device\r\nMessage: %s Subtopic: %s\r\n",message, subtopic);
                        break;
                }
                break;

        }
    }
}


/**************************************************************
 * Function Name   : msg_string_to_bytes
 * Description     : split a message into its components
 *                 : example 10100203 02c0 132 42 33 3 8a 15 ab
 *                 : some messages may be non standard e.g. last will messages
 * Inputs          : str is the input string
 *                 : header and vals are in hex the rest are decimal
 *                 : maVals limits the number of values to be returned
 * Outputs         : *header is the index header e.g. 02c0 in the example,
 *                 : is present if there is is a block of 4 chars at the start
 *                 : may not be present
 *                 : *cmd is the cmd type e.g. 132 in the example
 *                 : *index is the index, only present if cmd is 132
 *                 : *address is 33 in the example
 *                 : *numValues is 3 in the example
 *                 : *vals is 8a 15 ab in the example
 * Returns         : number of values stored in vals
 **************************************************************/
uint32_t msg_string_to_bytes(char *str, 			// the supplied string to convert
							 char *header, 			// the header (will contain UTC and message ID
							 MSG_TYPE *cmd, 		// the type of message we're encoding
							 int32_t *index, 		// the index count, if used
							 uint16_t *address, 	// the target register number
							 uint16_t *numValues, 	// the number of values declared in str
							 uint8_t *vals, 		// a list of the values actually found
							 uint32_t maxVals)		// the maximum possible number of values
{
    uint32_t len = strlen(str);
    uint32_t i,j;
    uint8_t numVals = 0;							// how many values we have found
    uint16_t val;
    bool valid;
    bool expectHex = false;

	PARSER_STATE parser_state = PARSE_COMMAND;

    *index = -1;
    *cmd = MSG_NONE;
    i=0;
//see if there is a utc and header, will have both or neither. Format = "00000000 0000"
    while((str[i]!=' ')&&(i<9)) i++;  //look for 8 character utc
    if(i==8)  //have read 8 byte UTC
    {
        i++;  //go past space
        while((str[i]!=' ')&&(i<14)) i++;  //look for 4 character header
    }

    if(i==13)
    {     // Have read 8 byte UTC and 4 byte header, so copy them into *header
        if(header!=NULL)
        {
            for(j=0;j<13;j++)
            {
                *header++ = str[j];
            }
            *header = 0;  //null terminate header string
            i++;  //advance to first character beyond space
        }
    }
    else
    {   // No 8 byte UTC and 4 byte header
        i=0;  //reset to start of message
        if(header!=NULL)
        {
            *header = '\0';  //return null header string
        }
    }

	while((numVals < maxVals) && (i < len))
	{ // One iteration of this while loop gets and processes one value.
		val = 0;
		valid = false;
		while((str[i]!=' ')&&(i<len))
		{   // One iteration of this while loop reads the next value from the input string, either hex or decimal.
			if((str[i]>='0')&&(str[i]<='9'))
			{
				valid = true;
				if((parser_state!=PARSE_VALUES) && (false==expectHex))  //can other cmds also send hex values in first two locations?
				{
				  val = val*10 + (str[i] - '0');  //cmd address and numValues are decimal
				}
				else
				{
				  val = val*16 + (str[i] - '0');  //register values are hex
				}
			}
			else if(((parser_state==PARSE_VALUES) || (true==expectHex)) && (str[i]>='a') && (str[i]<='f'))
			{
				valid = true;
				val = val*16 + 10 + (str[i] - 'a');
			}
			else if(((parser_state==PARSE_VALUES) || (true==expectHex)) && (str[i]>='A') && (str[i]<='F'))
			{
				valid = true;
				val = val*16 + 10 + (str[i] - 'A');
			}
			else
			{
				zprintf(LOW_IMPORTANCE, "Text Message Received: %s\r\n", str);
				break;
			}
			i++;
		}
		i++;  //consume separating space
		if(valid==true)
		{
			switch(parser_state)
			{
				case PARSE_COMMAND:
					if (val == MSG_REG_VALUES_INDEX_ADDED)
					{
						*cmd = MSG_REG_VALUES;
					}
					else
					{
						*cmd = (MSG_TYPE)val;
					}
					if ((val==MSG_MOVEMENT_EVENT) || (val==MSG_HUB_THALAMUS) || (val==MSG_HUB_THALAMUS_MULTIPLE))
					{
						expectHex = true;
					}
					if ((val==MSG_HUB_THALAMUS) ||
						(val==MSG_HUB_THALAMUS_MULTIPLE) ||
						(val==MSG_SET_DEBUG_MODE) ||
						(MSG_WEB_PING == val))
					{
						parser_state = PARSE_VALUES;  //no address or numVals
					}
					else if (val==MSG_REG_VALUES_INDEX_ADDED)
					{
						parser_state = PARSE_INDEX;
					}
					else
					{
						parser_state = PARSE_ADDRESS;
					}
					break;

				case PARSE_INDEX:
					*index = val;
					parser_state = PARSE_ADDRESS;
					break;
				case PARSE_ADDRESS:
					*address = val;
					if(*cmd==MSG_SET_ONE_REG)
					{
						*numValues = 1;
						parser_state = PARSE_VALUES;  //No numValues for cmd=1 (setting a single register value)
					}
					else
					{
						parser_state = PARSE_NUM_VALUES;
					}
				break;

				case PARSE_NUM_VALUES:
					*numValues = val;
					parser_state = PARSE_VALUES;
				break;

				case PARSE_VALUES:  // keep churning away pulling in values
					*vals++ = val;
					numVals++;
					break;

				default:
					break;
			}
		}
		else
		{
			break;
		}
	}

    // These are special cases
    if(*cmd==MSG_HUB_THALAMUS)
    {   // MSG_HUB_THALAMUS is a list of values, with a 'magic' register address
        *address = THALAMUS_DUMMY_REGISTER;
        *numValues = numVals;
    }
    if(*cmd==MSG_HUB_THALAMUS_MULTIPLE)
    { // Same idea as above
        *address = THALAMUS_MULTIPLE_DUMMY_REGISTER;
        *numValues = numVals;
    }
    if(*cmd==MSG_SET_DEBUG_MODE)
    {
        *address = HUB_SET_DEBUG_MODE_DUMMY_REGISTER;
        *numValues = numVals;
    }
	if (MSG_WEB_PING == *cmd)
	{
	  *numValues = numVals;
	}
    return numVals;
}

/**************************************************************
 * Function Name   : TranslateToDevice
 * Description     : Translates a message into T_MESSAGE format and
 *                 : passes it to the Device Buffer.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void TranslateToDevice(uint64_t mac, uint16_t address, uint8_t *values, bool sendGet, uint32_t numVals)
{
    T_MESSAGE message_for_device;
    uint8_t i;
    uint8_t *pM;

#define NT_HEADER			4		/* length of non thalamus header (command + length words) */
#define NT_REGISTER			2		/* length of required register */
#define NT_PARAMS			2		/* size of each parameter */
#define NT_PARAM_COUNT		2		/* size of parameter count word */
#define NT_COMPATIBILITY	2		/* the aparently unused final zero in the TX message */

    if (address == THALAMUS_DUMMY_REGISTER)
    {   // THALAMUS message, which is just a list of bytes.
        message_for_device.command = COMMAND_THALAMUS;
        message_for_device.length = numVals + T_MESSAGE_HEADER_SIZE + T_MESSAGE_PARITY_SIZE;
        for(i = 0; i < numVals; i++)
        {
            message_for_device.payload[i] = values[i];
        }
    }
    else
    { // Not THALAMUS message, so it is a register message for a Pet Door.
        message_for_device.payload[0] = (address>>8) & 0xFF;		// big endian
        message_for_device.payload[1] = address & 0xFF;
        message_for_device.payload[2] = (numVals >> 8) & 0xFF;		// big endian
        message_for_device.payload[3] = numVals & 0xFF;
        if (sendGet == true)
        { // Request is for the Device to send back a list of registers
            message_for_device.command = COMMAND_GET_REG;
            message_for_device.length =  NT_HEADER + NT_REGISTER + NT_PARAMS + NT_COMPATIBILITY;  //10; //5;
            message_for_device.payload[4] = 0; //values[0];
        }
        else
        {   // This is a batch of register values to be sent to a device.
            // Special case to be detected - if a magic number 0xCC is written to
            // PRODUCT_TYPE register, then we trap that and trigger a device disconnect.
            if ((address==HR_DEVICE_TYPE) && (values[0]==TRIGGER_DEVICE_DETACH))
            {
                surenet_unpair_device(mac);
                return; // break out as we don't want to do any more processing of this message
            }
            message_for_device.command = COMMAND_SET_REG;			// little endian
            message_for_device.length = NT_HEADER + NT_REGISTER + NT_PARAMS + (numVals) + 1;	// little endian
            for (i = 0; i < numVals; i++)
            {
                // the parameters are individual bytes, not words
				message_for_device.payload[NT_HEADER + i] = (values[i]  & 0xff);
            }
			message_for_device.payload[NT_HEADER + (NT_PARAMS * i)] = 0;	// the compatibility zero ???
        }
    }

    // Now add the parity to the end
	pM = (uint8_t*)(&message_for_device) + message_for_device.length-1;		  //set pointer to look at end of message ready for the parity
	*pM = GetParity((int8_t*)(&message_for_device), message_for_device.length-1);	  //calculate parity and drop it in at the end of the array

	// check for duplications before adding
    if (device_message_is_new(mac,&message_for_device)==true)
    {
        device_buffer_add(mac, &message_for_device);  // queue the message
    }
}

