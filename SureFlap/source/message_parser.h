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
* Filename: message_parser.h   
* Author:   Chris Cowdery 28/10/2019
* Purpose:  Parses messages received from Server, and despatch them.
*             
**************************************************************************/


#ifndef __MESSAGE_PARSER_H__
#define __MESSAGE_PARSER_H__

typedef enum
{
  MSG_NONE = 0,
  MSG_SET_ONE_REG = 1,  //WEB sends a single register value
  MSG_SET_REG_RANGE = 2,  //WEB sends a range of register values
  MSG_SEND_REG_RANGE = 3,  //WEB asks the HUB/DEVICE to send a range of register values
  MSG_REG_VALUES = 4,  //HUB/DEVICE reports a range of register values to WEB
  MSG_SEND_REG_DUMP = 5,  //WEB tels HUB/DEVICE to send a full register dump (to register queue)
  MSG_REPROGRAM_HUB = 6,  //WEB tels HUB to reprogram itself
  MSG_FULL_REG_SET = 7,  //WEB sends full set of register values for HUB/DEVICE to use
  MSG_MOVEMENT_EVENT = 8,
  MSG_GET_REG_RANGE = 9,  //WEB sends a range of register values
  MSG_HUB_UPTIME = 10,
  MSG_FLASH_SET = 11,
  MSG_HUB_REBOOT = 12,
  MSG_HUB_VERSION_INFO = 13,
  MSG_WEB_PING = 14,  //ping from web
  MSG_HUB_PING = 15,  //also sent on channel change
  MSG_LAST_WILL = 16,
  MSG_SET_DEBUG_MODE = 17,
  MSG_DEBUG_DATA = 18,
  MSG_CHANGE_SIGNING_KEY = 20,	
  MSG_HUB_THALAMUS = 127,  //NB used in both directions
  MSG_HUB_THALAMUS_MULTIPLE = 126,  //NB currently only used from hiub to Xively
  MSG_REG_VALUES_INDEX_ADDED = 0x84     // This is magic - it is 0x80 | MSG_REG_VALUES
} MSG_TYPE;

// These are COMMAND types for messages to/from Devices. Most are not used.
typedef enum{
	COMMAND_INVALID =  0,
//Messages to exchange register values
	COMMAND_GET_REG,          // 1
	COMMAND_SET_REG,          // 2
	COMMAND_GET_REG_R,        // 3
	COMMAND_SET_REG_R,        // 4
//Messages for SPI Slave Module interaction
	COMMAND_GET_FIRMWARE_VERSION,   // 5
	COMMAND_GET_FIRMWARE_VERSION_R, // 6
	COMMAND_GET_HARDWARE_VERSION,   // 7
	COMMAND_GET_HARDWARE_VERSION_R,	// 8
	COMMAND_STATUS,                 // 9  // poll for status
	COMMAND_STATUS_R,               // 10
//SPI Response message (holds error code)
	COMMAND_ACK,                    // 11
//Error response message (holds error code)
	COMMAND_ERROR_REPORT,           // 12
    COMMAND_CLAIM_REGISTER_SET,     // 13
    COMMAND_FREE_REGISTER_SET,      // 14
    COMMAND_ADD_REGISTER_SET,       // 15
    COMMAND_SEND_REGISTER_SET,      // 16
    COMMAND_SEND_MSG,               // 17
    COMMAND_SEND_MSG_R,             // 18
    COMMAND_THALAMUS,              // 19
    COMMAND_THALAMUS_MULTIPLE,     // 20
    COMMAND_TEST_MSG,               // 21

  //Messages for RF control
    COMMAND_SET_PAIRING_NORMAL=32,  // 32
    COMMAND_SET_PAIRING_MANUAL,     // 33
    COMMAND_RESET_SURENET,          // 34
    COMMAND_UNPAIR_DEVICE,          // 35
    COMMAND_CHECK_DATA,             // 36
	COMMAND_PROGRAMMER_DATA,        // 37 return values needed by programmer, currently just MAC address
	COMMAND_PROGRAMMER_DATA_R       // 38		
}DEVICE_COMMANDS;

//Dummy register values to indicate special commands which cannot be split for transmission or need special handling
#define MOVEMENT_DUMMY_REGISTER 1024
//NB there can only be an LSB change between the next two commands
//THALLAMUS_MULTIPLE is only used from device to server
#define THALAMUS_DUMMY_REGISTER 0x2000
#define THALAMUS_MULTIPLE_DUMMY_REGISTER 0x2001
#define HUB_SET_DEBUG_MODE_DUMMY_REGISTER 1025

#define MAX_MESSAGE_BYTES 88  //NB currently less than maximum that could fir in a packet, set according to longest Thallamus command
#define MAX_REGISTERS_TO_DEVICE 8   //split messages for device into blocks of 8

#define TRIGGER_DEVICE_DETACH 0xCC  // writing this value to the device register 0x00 (PRODUCT_TYPE) triggers the 
                                    // Hub to detach the device.


void process_MQTT_message_from_server(char *message,char *subtopic);

#endif
