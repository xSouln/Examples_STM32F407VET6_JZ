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
* Filename: MQTT-Simulator.c   
* Author:   Nailed Barnacle
* Purpose:   
*   
* private routines to decode and display thalamus messages
* intended for use only my MQTT
* derived from the original version in Arion (windows command line version)
*            
**************************************************************************/

#include "MQTT.h"	// Can hold the MQTT_SERVER_SIMULATED #define, if it's not Project-defined.
	 
#ifdef MQTT_SERVER_SIMULATED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include "MQTT-Synapse.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"

#include "task.h"
#include "queue.h"

#include "hermes-app.h"
#include "fsl_debug_console.h"
#include "server_buffer.h"
#include "message_parser.h"
#include "hermes-time.h"
#include "Devices.h"
#include "RegisterMap.h"

// this defaults to impf, but when a product info message is received it is set to the correct part
SURE_PRODUCT	unit = SURE_PRODUCT_IMPF;

PAYLOAD				payload = { MESSAGE_ACK };

int					pos;
int					line_count = 0;


typedef struct
{
	uint8_t		seconds;
	uint8_t		minutes;
	uint8_t		hours;
	uint8_t		days;
	uint8_t		months;
	uint8_t		years;
} TIMESTAMP;


SYNAPSE_RF_MSG message;

void set_unit(SURE_PRODUCT type)
{
	// set the unit type so it can be accessed by that which needeth to know
	unit = type;
}

void set_header(int8_t* str)
{
	// copy eight bytes into header
	memcpy ((SYNAPSE_RF_HEADER *) &message.header, (void *) str, 8);
}

void set_payload(int8_t* str, uint8_t count)
{
	memcpy ((uint8_t *) &message.payload, (void *) str, count);
}

void print_tod(uint32_t time)
{
	// print the time of day from the message timestamp
	// it's different between the connected and non-connected products
	// it's also hard to see how we'd ever see a non-connected product here, but...
	switch (unit)
	{
		case SURE_PRODUCT_FEEDER_LITE:
			// we have an incrementing seconds count
			mqtt_synapse_printf("%d:%02d:%02d:%02d ", time / 86400, (time % 86400) / 3600, (time % 3600) / 60, time % 60 );
			break;
		case SURE_PRODUCT_IMPF:
		case SURE_PRODUCT_POSEIDON:
		case SURE_PRODUCT_IDSCF:
			// we have a proper time of day, as a bit field
			// which we will for now ignore
			break;
		default:
			mqtt_synapse_printf("------ ");
			break;
	}
}

void message_handler(uint16_t type)
{
	// the selector that decides which of the message handlers to choose
	// so the correct message is output
	//mqtt_synapse_printf("%ld ", header.synapse.timestamp);
	print_tod (message.header.timestamp);
	switch (type)
	{
		case 	MESSAGE_BUTTON_PRESS:
			button_press_handler();
			break;
		case 	MESSAGE_DEBUG:
			debug_message_handler();
			break;
		case 	MESSAGE_CHANGE_SETTING:
			change_setting_handler();
			break;
		case 	MESSAGE_CHANGE_PROFILE:
			change_profile_handler();
			break;
		case 	MESSAGE_WEIGHTS:
			message_weights_handler();
			break;
		case 	MESSAGE_DEBUG_COMMAND:
			debug_command_handler();
			break;
		case 	MESSAGE_ENVIRONMENT:
			message_environment_handler();
			break;
		case 	MESSAGE_RF_STATS:
			message_rf_stats_handler();
			break;
		case 	MESSAGE_PRODUCT_INFO:
			message_product_info_handler();
			break;
		case 	MESSAGE_ACK:
			// an ack serves to acknowledge an instruction we have given
			// for testing purposes we can ignore it, I think
			// (we have already echoed it before we get this far so the stack is happy)
			// mqtt_synapse_printf("Unimplemented thalamus message handler: ACK\r\n");
			mqtt_synapse_printf("Ack: ignored\r\n");
			break;
		case 	MESSAGE_TIMED_ACCESS:
			message_timed_access_handler();
			break;
		case 	MESSAGE_ANIMAL_ACTIVITY:
			message_animal_activity_handler();
			break;

		case 	MESSAGE_TRIGGER_EVENT:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Trigger Event\r\n");
			break;
		case 	MESSAGE_STREAM_CONTROL:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Stream Control\r\n");
			break;
		case 	MESSAGE_RESTART:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Restart at file\r\n");
			break;
		case 	MESSAGE_UNLOCK_DOWNLOAD:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Unlock Download\r\n");
			break;
		case 	MESSAGE_FIRMWARE_BLOCK:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Firmware Block\r\n");
			break;
		case 	MESSAGE_FIRMWARE_DOWNLOAD_COMPLETE:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Download Complete\r\n");
			break;
		case 	MESSAGE_SET_TIME:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Set Time\r\n");
			break;
		case 	MESSAGE_EVENT_CONTROL:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Event Control\r\n");
			break;
		case 	MESSAGE_STATE_CONTROL:
			mqtt_synapse_printf("Unimplemented thalamus message handler: State Control\r\n");
			break;
		case 	MESSAGE_DOWNLOAD_RESULT:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Download Result\r\n");
			break;
		case 	MESSAGE_ANIMAL_PRESENT:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Animal Present\r\n");
			break;
		case 	MESSAGE_MICROCHIP_READ:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Microchip Read\r\n");
			break;
		case 	MESSAGE_STATISTICS:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Message Statistics\r\n");
			break;
		case 	MESSAGE_ONE_SHOT:
			mqtt_synapse_printf("Unimplemented thalamus message handler: One Shot\r\n");
			break;
		case 	MESSAGE_DEBUG_REPORT:
			mqtt_synapse_printf("Unimplemented thalamus message handler: Debug Report\r\n");
			break;
		default:
			mqtt_synapse_printf("Unrecognised thalamus message header: 0x%02x\r\n", type);
			break;
	}
}

const char* button_names[] = {
	{ "Lid    " },
	{ "Learn  " },
	{ "Train  " },
	{ "Connect" },
	{ "Tare   " },
};

void button_press_handler(void)
{
	// show the state of the buttons...
	mqtt_synapse_printf("Button:\r\n\t%s:\t", button_names[message.payload.smp.button_press.input_event.channel]);
	if( 1 == message.payload.smp.button_press.input_event.active )
	{
		mqtt_synapse_printf("pressed");
	} else
	{
		mqtt_synapse_printf("released");
	}
	mqtt_synapse_printf("\r\n\tTimestamp: \t%ld.%03ld\r\n\tDuration: \t%ld\r\n",
						message.payload.smp.button_press.input_event.timestamp / 1000,
						message.payload.smp.button_press.input_event.timestamp % 1000,
						message.payload.smp.button_press.input_event.duration);
}

void debug_message_handler(void)
{
	// display the debug message - it's always text
	// info only: state events have [event] and state changes <state>
	// but... we want to indent between prox start/end events
	static bool prox = false;
	mqtt_synapse_printf("Debug: ");
	if( true == prox )
	{
		mqtt_synapse_printf("  ");
	}
	mqtt_synapse_printf("%.*s\n", message.payload.smp.debug.length, message.payload.smp.debug.text);
	if( 0 == (strncmp((const char *)message.payload.smp.debug.text, "[PROX_START]", message.payload.smp.debug.length)) )
	{
		prox = true;
	}
	if( 0 == (strncmp((const char *)message.payload.smp.debug.text, "[PROX_END]", message.payload.smp.debug.length)) )
	{
		prox = false;
	}
}

const char* mpf2_settings[] = {
	{ "Training mode" },
	{ "Rescan time ms" },
	{ "Occipital block" },
	{ "Custom modes" },
	{ "Arb close delay" },
	{ "Learn mode" },
	{ "Lid full distance" },
	{ "Weigh position" },
	{ "Open PWM cycles" },
	{ "Boost temperature" },
	{ "OCC top output" },
	{ "OCC bottom output" }
};	// why would this ever happen? It's not connected...

const char* impf_settings[] = {
	{ "Cal offset left" },
	{ "Cal offset right" },
	{ "Cal scale left" },
	{ "Cal scale right" },
	{ "Calibrate" },
	{ "Training mode" },
	{ "D plate weight" },
	{ "Full plate weight" },
	{ "D bowl weight" },
	{ "Full bowl weight" },
	{ "Target weight left" },
	{ "Target weight right" },
	{ "Bowl count" },
	{ "Cat close delay ms" },
	{ "Rescan time ms" },
	{ "Led brightness" },
	{ "Time to dim" },
	{ "Lid autoclose" },
	{ "RF interval" },
	{ "Opto block delay" },
	{ "Custom modes" },
	{ "Arb close delay" },
	{ "Learn mode" },
	{ "Left tare" },
	{ "Right tare" },
	{ "Lid full distance" },
	{ "Weight position" },
	{ "Lid open PWM cycles" },
	{ "Boost temperature" },
	{ "Backstop" },
};

const char* idsf_settings[] = {
	{ "Learn Mode" },
	{ "Unrecognised" },
	{ "Recalibrate timeout" },
	{ "Lockdown" },
	{ "Enable off frequencies" },
	{ "Curiosity level inside" },
	{ "Curiosity level outside" },
	{ "Metal mode" },
	{ "Fast locking" },
	{ "Fail safe" },
	{ "Proximity power" },
	{ "RF interval" },
	{ "Funk antenna" },
	{ "Backstop" },
};

const char* poseidon_settings[] = {
	{ "Cal offset left" },
	{ "Cal scale left" },
	{ "Calibrate" },
	{ "Weight interval" },
	{ "Removed min" },
	{ "Added min" },
	{ "Check pet come back" },
	{ "Weight reservoir min" },
	{ "Weight period log" },
	{ "Tare" },
	{ "Check reservoir" },
	{ "Weight 2" },
	{ "Weight 3" },
	{ "Rescan time" },
	{ "Custom modes" },
	{ "Learn mode" },
	{ "RFID types" },
	{ "RFID 1" },
	{ "RF interval" },
	{ "Proximity recalibrate" },
	{ "Proximity threshold" },
	{ "Recal threshold" },
	{ "Recal time" },
	{ "Prox end delay" },
	{ "Ref filter weight" },
	{ "Stddev filter weight rising" },
	{ "Stddev filter weight falling" },
	{ "Delta stddev_min" },
	{ "Spare 1" },
	{ "Spare 2" },
	{ "Backstop" },
};
		
const char** setting_ptr;

void change_setting_handler(void)
{
	// returns the settings for a single value

	switch (unit)
	{
		case SURE_PRODUCT_IMPF:
			setting_ptr = impf_settings;	
			break;
		case SURE_PRODUCT_IDSCF:
			setting_ptr = idsf_settings;
			break;
		case SURE_PRODUCT_POSEIDON:
			setting_ptr = poseidon_settings;
			break;
		case	SURE_PRODUCT_UNKNOWN:
		case	SURE_PRODUCT_HUB:
		case	SURE_PRODUCT_REPEATER:
		case	SURE_PRODUCT_PET_DOOR:
		case	SURE_PRODUCT_PROGRAMMER:
		case	SURE_PRODUCT_FEEDER_LITE:
		default:
			// do nothing
			break;
	}
	mqtt_synapse_printf("Change Setting: %s is 0x%08x\r\n", setting_ptr[message.payload.smp.change_setting.setting], message.payload.smp.change_setting.value);
}

const char* rfid_types[RFID_TYPE_BACKSTOP+1] = 
{
	"FDXA",
	"FDXB",
	"TROVAN",
	"COLLAR TAG",
	"AVID",
	"FDXB_125",
	"FDXA_133",
	"Not a chip",
};

const char* special_profiles[HIPPO_PROFILE_BACKSTOP] = 
{
	"None",
	"Unrecognised",
	"Permitted",
	"Safe",
	"Not permitted",
	"Intruder"
};

const char* action_types[3] =
{
	"Add profile",
	"Remove profile",
	"Modify profile"
};

void change_profile_handler(void)
{
	// we've added or deleted a new profile; show the message
	mqtt_synapse_printf("Change profile:\r\n");
	mqtt_synapse_printf("\tChip Number:\t%02x%02x%02x%02x%02x%02x (%s)\r\n", message.payload.smp.change_profile.microchip.number[0],
						message.payload.smp.change_profile.microchip.number[1],
						message.payload.smp.change_profile.microchip.number[2],
						message.payload.smp.change_profile.microchip.number[3],
						message.payload.smp.change_profile.microchip.number[4],
						message.payload.smp.change_profile.microchip.number[5],
						rfid_types[message.payload.smp.change_profile.microchip.type]);
	mqtt_synapse_printf("\tSpecial profile:\t%s\r\n", special_profiles[message.payload.smp.change_profile.special_profile]);
	mqtt_synapse_printf("\tIndex: \t%d\r\n", message.payload.smp.change_profile.index);
	mqtt_synapse_printf("\tAction:\t%s\r\n", action_types[message.payload.smp.change_profile.action]);

}

const char* context[CONFIG_WEIGHT_CONTEXT_BACKSTOP] =
{
	"Pet opened",
	"Pet closed",
	"Intruder closed",
	"Dubious closed",
	"User opened",
	"User closed",
	"User zeroed",
	//"Learn opened",
};

void message_weights_handler(void)
{
	// show the weights data
	// there's quite a lot of it...
	mqtt_synapse_printf("Weights:\r\n");
	mqtt_synapse_printf("\tMicrochip:\r\n");
	mqtt_synapse_printf("\t\tChip Number:\t%02x%02x%02x%02x%02x%02x (%s)\r\n", message.payload.smp.weights.microchip.number[0],
		   message.payload.smp.weights.microchip.number[1],
		   message.payload.smp.weights.microchip.number[2],
		   message.payload.smp.weights.microchip.number[3],
		   message.payload.smp.weights.microchip.number[4],
		   message.payload.smp.weights.microchip.number[5],
		   rfid_types[message.payload.smp.weights.microchip.type]);
	mqtt_synapse_printf("\t\tContext:\t%s\r\n", context[message.payload.smp.weights.context]);
	mqtt_synapse_printf("\tDuration:\t%d\r\n", message.payload.smp.weights.duration);
	for (uint32_t entry = 0; entry < message.payload.smp.weights.num_elements; entry++)
	{
		mqtt_synapse_printf("\tSensor %d:\r\n", entry);
		mqtt_synapse_printf("\t\tInitial: %d\r\n", message.payload.smp.weights.weights[entry].initial);
		mqtt_synapse_printf("\t\tFinal:   %d\r\n", message.payload.smp.weights.weights[entry].final);
	}
	mqtt_synapse_printf("\tEncounter ID: %d\r\n", message.payload.smp.weights.encounter_id);
	mqtt_synapse_printf("\tTemperature: %d\r\n", message.payload.smp.weights.temperature);
	mqtt_synapse_printf("\tAmbient temp: %d\r\n", message.payload.smp.weights.ambient_temperature);

}

void debug_command_handler(void)
{
	// debug command just spits out a stream of hex
	mqtt_synapse_printf("Debug Command:\ta\r\n\t\t\tKey:");
	for (uint32_t q = 0; q < 16; q++)
	{
		mqtt_synapse_printf(" %02x", message.payload.smp.debug_command.key[q]);
	}
	mqtt_synapse_printf("\r\n");
}

void message_environment_handler(void)
{
	// the environment message sings of voltages and time
	char datestring[24];
	
	mqtt_synapse_printf("Environment:\r\n");
	mqtt_synapse_printf("\tBattery voltage:      %dmV\r\n", message.payload.smp.environment.battery_voltage);
	mqtt_synapse_printf("\tMicro voltage:        %dmV\r\n", message.payload.smp.environment.micro_voltage);
	mqtt_synapse_printf("\tAmbient Temperature:  %dmV\r\n", message.payload.smp.environment.ambient_temperature);
	sure_date_time_to_string(message.payload.smp.environment.last_notable_action, datestring);
	mqtt_synapse_printf("\tLast big action time: %s\r\n", datestring);
}

void message_rf_stats_handler(void)
{
	// rf stats! we need more rf stats!
	mqtt_synapse_printf("RF Stats:\r\n");
	mqtt_synapse_printf("\tUplink time:     0x%x\r\n", message.payload.smp.rf_stats.uplink_time);
	mqtt_synapse_printf("\tPackets lost:    0x%x\r\n", message.payload.smp.rf_stats.packets_lost);
	mqtt_synapse_printf("\tRetries:         0x%x\r\n", message.payload.smp.rf_stats.retries);
	mqtt_synapse_printf("\tSignal strength: 0x%x\r\n", message.payload.smp.rf_stats.signal_strength);
}
	
const char* product_id[] = {
	{"Unknown"},
	{"Hub"},
	{"Repeater"},
	{"Pet door"},
	{"IMPF"},
	{"Programmer"},
	{"IDSF"},
	{"Feeder Lite"},
	{"Poseiden"}
};
	
void message_product_info_handler(void)
{
	mqtt_synapse_printf("Product info:\r\n");
	mqtt_synapse_printf("\tHardware revision: %d\r\n", message.payload.smp.product_info.hardware_revision);
	mqtt_synapse_printf("\tSoftware revision: %d\r\n", message.payload.smp.product_info.software_revision);
	mqtt_synapse_printf("\tThalamus revision: %d\r\n", message.payload.smp.product_info.thalamus_revision);
	mqtt_synapse_printf("\tChip hash:         0x%08x\r\n", message.payload.smp.product_info.chip_hash);
	mqtt_synapse_printf("\tSettings hash:     0x%08x\r\n", message.payload.smp.product_info.settings_hash);
	mqtt_synapse_printf("\tCurfew hash:       0x%08x\r\n", message.payload.smp.product_info.curfew_hash);
	mqtt_synapse_printf("\tProfiles filled:   %d\r\n", message.payload.smp.product_info.profiles_filled);
	mqtt_synapse_printf("\tUid:               %c%c%c, 0x%02x, %c%c%c%c, 0x%08x\r\n",
						message.payload.smp.product_info.uid.lot_number[0],
						message.payload.smp.product_info.uid.lot_number[1],
						message.payload.smp.product_info.uid.lot_number[2],
						message.payload.smp.product_info.uid.wafer_number,
						message.payload.smp.product_info.uid.lot_number_continued[0],
						message.payload.smp.product_info.uid.lot_number_continued[1],
						message.payload.smp.product_info.uid.lot_number_continued[2],
						message.payload.smp.product_info.uid.lot_number_continued[3 ],
						message.payload.smp.product_info.uid.unique_id);
	//SURE_KEY		reserved;
	mqtt_synapse_printf("\tProduct:           %s\r\n", product_id[message.payload.smp.product_info.product_type]);
	// we save the product type so later messages can be decoded correctly
	unit = message.payload.smp.product_info.product_type;
	//uint8_t			reserved2;
	mqtt_synapse_printf("\tBanks flipped:     %d\r\n", message.payload.smp.product_info.banks_flipped);
}

void message_timed_access_handler(void)
{
	SURE_DATE_TIME	sdt;
	char 			time[10];
	
	mqtt_synapse_printf("Timed access:\r\n");
	mqtt_synapse_printf("\tMicrochip:\r\n");
	mqtt_synapse_printf("\t\tChip Number:\t%02x%02x%02x%02x%02x%02x (%s)\r\n", message.payload.smp.timed_access.microchip.number[0],
						message.payload.smp.timed_access.microchip.number[1],
						message.payload.smp.timed_access.microchip.number[2],
						message.payload.smp.timed_access.microchip.number[3],
						message.payload.smp.timed_access.microchip.number[4],
						message.payload.smp.timed_access.microchip.number[5],
						rfid_types[message.payload.smp.timed_access.microchip.type]);
	mqtt_synapse_printf("\tIndex:\t%d\r\n", message.payload.smp.timed_access.index);
	mqtt_synapse_printf("\tCurfews:\r\n");
	
	for( uint8_t q = 0; q < HIPPO_NUM_CURFEW_INSTANCES; q++ )
	{
		mqtt_synapse_printf("\t(%d)\tlock:\t\t", q);
		
		sdt = message.payload.smp.timed_access.curfews.curfews[q].lock_time;
		sure_date_time_to_time_string(sdt, time);
		
		mqtt_synapse_printf("%s\r\n", time);
		mqtt_synapse_printf("\t\tunlock:\t\t");
		
		sdt = message.payload.smp.timed_access.curfews.curfews[q].unlock_time;
		sure_date_time_to_time_string(sdt, time);
		
		mqtt_synapse_printf("%s\r\n", time);
		mqtt_synapse_printf("\t\tprofile:\t%s\r\n", special_profiles[message.payload.smp.timed_access.curfews.curfews[q].special_profile]);
	}
}
	
void message_animal_activity_handler(void)
{
	mqtt_synapse_printf("Animal activity:\r\n");
	mqtt_synapse_printf("\tTemperature:\t%d\r\n", message.payload.smp.animal_activity.temperature);
	mqtt_synapse_printf("\tDuration:   \t%d\r\n", message.payload.smp.animal_activity.duration);
	mqtt_synapse_printf("\tAngle:	   \t%d\r\n", message.payload.smp.animal_activity.angle);
	mqtt_synapse_printf("\tSide:       \t%d\r\n", message.payload.smp.animal_activity.side);
	mqtt_synapse_printf("\tChip Number:\t%02x%02x%02x%02x%02x%02x (%s)\r\n", message.payload.smp.animal_activity.microchip.number[0],
						message.payload.smp.animal_activity.microchip.number[1],
						message.payload.smp.animal_activity.microchip.number[2],
						message.payload.smp.animal_activity.microchip.number[3],
						message.payload.smp.animal_activity.microchip.number[4],
						message.payload.smp.animal_activity.microchip.number[5],
						rfid_types[message.payload.smp.animal_activity.microchip.type]);
	mqtt_synapse_printf("Encounter id: \t%d\r\n", message.payload.smp.animal_activity.encounter_id);
	mqtt_synapse_printf("Ambient temp: \t%d\r\n", message.payload.smp.animal_activity.ambient_temperature);
}

void sure_date_time_to_string(SURE_DATE_TIME sdt, char* str)
{
	// convert the suredate compressed time to a string

	const char* monname[] = {{"Jan"},{"Feb"},{"Mar"},{"Apr"},{"May"},{"Jun"},{"Jul"},{"Aug"},{"Sep"},{"Oct"},{"Nov"},{"Dec"}};

	sprintf(str, "%d %s, %4d %02d:%02d:%02d", sdt.days, monname[sdt.months], 2000 + sdt.years, sdt.hours, sdt.minutes, sdt.seconds);
}

void sure_date_time_to_time_string(SURE_DATE_TIME sdt, char* str)
{
	// convert the suredate compressed time to a string without the date
	sprintf(str, "%02d:%02d:%02d", sdt.hours, sdt.minutes, sdt.seconds);
}

#endif	// MQTT_SERVER_SIMULATED