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
* takes the various structures which can appear in a synapse message and puts them in a union
* includes additional structures from other thalamus source, and modifies to keep msvc happy
*            
**************************************************************************/// taken from Thalamus synapse.h and modified

#ifndef _SYNAPSE_
#define _SYNAPSE_

#ifdef __GNUC__
#define PACKED_STRUCT struct __attribute__((packed))
#define PACKED_UNION union __attribute__((packed))
#define PACKED_ENUM	enum __attribute__((packed))
#else
#define PACKED_STRUCT __packed struct
#define PACKED_UNION __packed union
#define PACKED_ENUM enum
#endif

#define MQTT_SYNAPSE_PRINTS		false

#if MQTT_SYNAPSE_PRINTS
#define mqtt_synapse_printf(...)	zprintf(CRITICAL_IMPORTANCE, __VA_ARGS__)
#else
#define mqtt_synapse_printf(...)
#endif


#define SYNAPSE_PLACEHOLDER_ARBITRARY_AMOUNT	16
#define BICAMERAL_CHECK_SIZE					16
#define BICAMERAL_KEY_SIZE						16
#define BICAMERAL_BLOCK_SIZE					128
#define SYNAPSE_KEY_LENGTH						16
#define SURE_KEY_LENGTH							16u
#define SYNAPSE_PLACEHOLDER_DEBUG_MESSAGE_SIZE	63
#define RFID_MICROCHIP_NUMBER_LENGTH			6u
#define HIPPO_NUM_CURFEW_INSTANCES				4
#define MEDULLA_MAX_STATS_IN_SET				10u
#define CONFIG_NUM_WEIGHT_FRAMES				2
#define PARIETAL_MAX_DATA_LENGTH				32u
#define OCCIPITAL_NUM_SAMPLES_IGNORED			2
#define OCCIPITAL_NUM_SAMPLES_NEUTRAL			8		
#define OCCIPITAL_NUM_SAMPLES_SIGNAL			16	
#define OCCIPITAL_NUM_SAMPLES_TOTAL				(OCCIPITAL_NUM_SAMPLES_NEUTRAL + OCCIPITAL_NUM_SAMPLES_SIGNAL + OCCIPITAL_NUM_SAMPLES_IGNORED)
#define LID_DEBUG_REPORT_SAMPLES				27


typedef PACKED_ENUM
{ // This enum should always be contiguous.
	MESSAGE_ACK,							// 0	00
	MESSAGE_TRIGGER_EVENT,					// 1	01
	MESSAGE_STREAM_CONTROL,					// 2	02
	MESSAGE_RESTART,						// 3	03
	MESSAGE_UNLOCK_DOWNLOAD,				// 4	04
	MESSAGE_FIRMWARE_BLOCK,					// 5	05
	MESSAGE_FIRMWARE_DOWNLOAD_COMPLETE,		// 6	06
	MESSAGE_SET_TIME,						// 7	07
	MESSAGE_DEBUG_COMMAND,					// 8	08
	MESSAGE_CHANGE_SETTING,					// 9	09
	MESSAGE_EVENT_CONTROL,					// 10	0A
	MESSAGE_PRODUCT_INFO,					// 11	0B
	MESSAGE_ENVIRONMENT,					// 12	0C
	MESSAGE_STATE_CONTROL,					// 13	0D
	MESSAGE_DOWNLOAD_RESULT,				// 14	0E
	MESSAGE_DEBUG,							// 15	0F
	MESSAGE_RF_STATS,						// 16	10
	MESSAGE_CHANGE_PROFILE,					// 17	11
	MESSAGE_TIMED_ACCESS,					// 18	12
	MESSAGE_ANIMAL_ACTIVITY,				// 19	13
	MESSAGE_ANIMAL_PRESENT,					// 20	14
	MESSAGE_MICROCHIP_READ,					// 21	15
	MESSAGE_BUTTON_PRESS,					// 22	16
	MESSAGE_STATISTICS,						// 23	17
	MESSAGE_WEIGHTS,						// 24	18
	MESSAGE_ONE_SHOT,						// 25	19
	MESSAGE_DEBUG_REPORT,					// 26	1A
	MESSAGE_BACKSTOP,						// Should always be the last member of the contiguous values in this enum.
	MESSAGE_FORCE_SIZE = 0xFFFF				// Force this to be at least 16 bits.
} SYNAPSE_MESSAGE_TYPE;						// N.B. Values whose first byte is 0x7F (127) are reserved for Pings.

typedef PACKED_ENUM
{
	SURE_STATUS_OK = 0,				 // 0
	SURE_STATUS_ERROR,               // 1*
	SURE_STATUS_INVALID_ARGUMENT,    // 2*
	SURE_STATUS_ESCAPE,              // 3
	SURE_STATUS_FAILURE,             // 4*
	SURE_STATUS_TOO_EARLY,           // 5
	SURE_STATUS_BAD_TRANSITION,      // 6
	SURE_STATUS_BAD_CONTROL,         // 7
	SURE_STATUS_BAD_PARITY,          // 8*
	SURE_STATUS_BAD_CHECK,           // 9
	SURE_STATUS_BAD_SETTING,         // 10
	SURE_STATUS_END_OF_BUFFER,       // 11
	SURE_STATUS_TIMEOUT,             // 12
} SURE_STATUS;

typedef union
{
	uint8_t array[BICAMERAL_CHECK_SIZE];
	struct
	{
		uint32_t	a;
		uint32_t	b;
		uint32_t	c;
		uint32_t	d;
	};
} BICAMERAL_CHECK;

typedef union
{
	uint8_t array[BICAMERAL_KEY_SIZE];
	struct
	{
		uint32_t	a;
		uint32_t	b;
		uint32_t	c;
		uint32_t	d;
	};
} BICAMERAL_KEY;

typedef PACKED_ENUM
{
	SURE_DAY_FORBIDDEN = 0,
	SUNDAY,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SURE_DAY_BACKSTOP
} SURE_DAY;

typedef PACKED_ENUM
{
	BICAMERAL_TEST_IMAGE,
	BICAMERAL_CONFIRM_IMAGE,
	BICAMERAL_SYSTEM_RESET
} BICAMERAL_COMPLETION_ACTION;

typedef uint8_t	SYNAPSE_KEY[SYNAPSE_KEY_LENGTH];

typedef PACKED_STRUCT
{
	uint8_t		lot_number[3];
	uint8_t		wafer_number;
	uint8_t		lot_number_continued[4];
	uint32_t	unique_id;
} SURE_CHIP_UID;

typedef uint8_t SURE_KEY[SURE_KEY_LENGTH];

typedef PACKED_ENUM
{
	SURE_PRODUCT_UNKNOWN,
	SURE_PRODUCT_HUB,
	SURE_PRODUCT_REPEATER,
	SURE_PRODUCT_PET_DOOR,
	SURE_PRODUCT_IMPF,
	SURE_PRODUCT_PROGRAMMER,
	SURE_PRODUCT_IDSCF,
	SURE_PRODUCT_FEEDER_LITE,
	SURE_PRODUCT_POSEIDON,
} SURE_PRODUCT; // BEWARE: Max x bits.

typedef PACKED_ENUM 
{
	// settings for the feeder lite
	MPF2_TRAINING_MODE,				// 00 0 = normal operation
	MPF2_RESCAN_TIME_MS,				// 01 time between rfid scans while cat is feeding
	MPF2_OCCIPITAL_BLOCK,			// 02 how long before a blocked sensor triggers a recalibration
	MPF2_CUSTOM_MODES,				// 03 a bitfield of modes
	MPF2_ARB_CLOSE_DELAY,			// 04 arbitrary close delay, if the custom mode is set
	MPF2_SETTING_LEARN_MODE,			// 05 Learn Mode! Not actually written to EEPROM.
	MPF2_SETTING_LID_FULL_DISTANCE,	// 06 Time in milliseconds for the lid to open/close.
	MPF2_SETTING_WEIGH_POSITION,
	MPF2_SETTING_LID_OPEN_PWM_CYCLES,
	MPF2_SETTING_BOOST_TEMPERATURE,
	MPF2_SETTING_OCC_TOP_OUTPUT,
	MPF2_SETTING_OCC_BOTTOM_OUTPUT
} MPF2_SETTINGS;

typedef PACKED_ENUM
{
	// settings for the impf
	IMPF_CAL_OFFSET_LEFT,				// 00 4 x 32 bit values define the calibration
	IMPF_CAL_OFFSET_RIGHT,				// 01
	IMPF_CAL_SCALE_LEFT,            	// 02
	IMPF_CAL_SCALE_RIGHT,            	// 03
	IMPF_CALIBRATE,						// 04 have we calibrated?
	IMPF_TRAINING_MODE,					// 05 0 = normal operation
	IMPF_D_PLATE_WEIGHT_CG,				// 06 the weight of a d-plate
	IMPF_FULL_PLATE_WEIGHT_CG,			// 07 and the full plate
	IMPF_D_BOWL_WEIGHT_CG,				// 08 the weight of half bowl
	IMPF_FULL_BOWL_WEIGHT_CG,			// 09 the weight of a full bowl
	IMPF_TARGET_WEIGHT_LEFT_CG,			// 0a target food weights
	IMPF_TARGET_WEIGHT_RIGHT_CG,		// 0b all weights in centigrams
	IMPF_BOWL_COUNT,					// 0c how many bowls do we have?
	IMPF_CAT_CLOSE_DELAY_MS,			// 0d how long after cat leaves should the lid close?
	IMPF_RESCAN_TIME_MS,				// 0e time between rfid scans while cat is feeding
	IMPF_LED_BRIGHTNESS,				// 0f scales LED brightness
	IMPF_TIME_TO_DIM,					// 10 how long after opening do we dim?
	IMPF_LID_AUTOCLOSE,					// 11 and how long before we close the lid?
	IMPF_SETTING_RF_INTERVAL,			// 12 how often does the rf run?
	IMPF_OPTO_BLOCK_DELAY,				// 13 how long before a blocked sensor triggers a recalibration
	IMPF_CUSTOM_MODES,					// 14 a bitfield of modes
	IMPF_ARB_CLOSE_DELAY,				// 15 arbitrary close delay, if the custom mode is set
	IMPF_SETTING_LEARN_MODE,			// 16 Learn Mode! Not actually written to EEPROM.
	IMPF_SETTING_LEFT_TARE,				// 17 Zeroing information for the Left Bowl.
	IMPF_SETTING_RIGHT_TARE,			// 18 Zeroing information for the Right Bowl.
	IMPF_SETTING_LID_FULL_DISTANCE,		// 19 Expected time in milliseconds for the lid to complete a movement.
	IMPF_SETTING_WEIGH_POSITION,		// 1a When to perform a weigh; make it larger for slow/cold motors
	IMPF_SETTING_LID_OPEN_PWM_CYCLES,	// 1b How many lid cycles at open_pwm power before starting PID control
	IMPF_SETTING_BOOST_TEMPERATURE,		// 1c When is it too cold to work properly?
} IMPF_SETTINGS;

typedef PACKED_STRUCT
{
	uint32_t		seconds : 6;
	uint32_t		minutes : 6;
	uint32_t		hours : 5;
	uint32_t		days : 5;
	uint32_t		months : 4;
	uint32_t		years : 6;
} SURE_DATE_TIME;

typedef uint8_t	RFID_MICROCHIP_NUMBER[RFID_MICROCHIP_NUMBER_LENGTH];

typedef PACKED_ENUM
{
	RFID_TYPE_FDXA,
	RFID_TYPE_FDXB,
	RFID_TYPE_TROVAN,
	RFID_TYPE_COLLAR_TAG,
	RFID_TYPE_AVID,
	RFID_TYPE_FDXB_125,
	RFID_TYPE_FDXA_133,
	RFID_TYPE_BACKSTOP
} RFID_TYPE;

typedef PACKED_STRUCT
{
	RFID_MICROCHIP_NUMBER	number;
	RFID_TYPE				type;
} RFID_MICROCHIP;

typedef PACKED_ENUM
{
	HIPPO_PROFILE_NONE,			// 00
	HIPPO_PROFILE_UNRECOGNISED,	// 01
	HIPPO_PROFILE_PERMITTED,	// 02
	HIPPO_PROFILE_SAFE,			// 03
	HIPPO_PROFILE_NOT_PERMITTED,// 04
	HIPPO_PROFILE_INTRUDER,		// 05
	HIPPO_PROFILE_BACKSTOP 		// 06 - Should always be last member of enumeration.
} HIPPO_SPECIAL_PROFILES;

typedef PACKED_ENUM
{
	SYNAPSE_ADD_PROFILE,
	SYNAPSE_REMOVE_PROFILE,
	SYNAPSE_MODIFY_PROFILE
} SYNAPSE_CHANGE_PROFILE_ACTION;

typedef PACKED_STRUCT
{
	SURE_DATE_TIME			lock_time;
	SURE_DATE_TIME			unlock_time;
	HIPPO_SPECIAL_PROFILES	special_profile;
} HIPPO_CURFEW;

typedef PACKED_STRUCT
{
	HIPPO_CURFEW	curfews[HIPPO_NUM_CURFEW_INSTANCES];
} HIPPO_CURFEW_SET;

typedef PACKED_ENUM
{
	AZIMUTH_ANGLED_OUTWARDS,
	AZIMUTH_ANGLED_INWARDS,
	AZIMUTH_ANGLED_NEUTRAL,
	AZIMUTH_ANGLED_BACKSTOP
} AZIMUTH_ANGLE;

typedef PACKED_ENUM
{
	CONFIG_FUSIFORM_BUTTON_LID,
	CONFIG_FUSIFORM_BUTTON_LEARN,
	CONFIG_FUSIFORM_BUTTON_TRAIN,
	CONFIG_FUSIFORM_BUTTON_BACKSTOP
} CONFIG_FUSIFORM_BUTTON;

typedef PACKED_STRUCT
{
	uint8_t					channel;		/*was CONFIG_FUSIFORM_BUTTON */
	uint8_t					active;
	uint32_t				timestamp;
	uint32_t				duration;
} FUSIFORM_INPUT_EVENT;

typedef PACKED_ENUM
{
	CONFIG_STAT_LOCK_CYCLES,
	CONFIG_STAT_MICROCHIP_TYPES,
	CONFIG_STAT_TEMPERATURE_HIGH,
	CONFIG_STAT_TEMPERATURE_LOW,
	CONFIG_STAT_RFID_PULSES_INSIDE,
	CONFIG_STAT_RFID_PULSES_OUTSIDE,
	// feeder specific lid
	CONFIG_STAT_LID_CYCLES,
	CONFIG_STAT_CURRENT_LID_DISTANCE,
	CONFIG_STAT_USER_OPENS,
	CONFIG_STAT_CAT_OPENS,
	CONFIG_STAT_INTRUDER_CLOSES,
	// feeder specific optical
	CONFIG_STAT_OPTO_NUMBER_OF_TRIGGERS,
	CONFIG_STAT_OPTO_TRIGGER_DURATION,
	CONFIG_STAT_OPTO_MACHINE_STATE,
	CONFIG_STAT_OPTICAL_RECALIBRATES,
	CONFIG_STAT_OPTICAL_REFERENCE_1,
	CONFIG_STAT_OPTICAL_REFERENCE_2,
	// feeder specific buttons
	CONFIG_STAT_BUTTON_OPEN_PUSHES,
	CONFIG_STAT_BUTTON_TRAINING_PUSHES,
	CONFIG_STAT_BUTTON_LEARN_PUSHES,
	CONFIG_STAT_BUTTON_NOISE_LEVEL,
	// feeder specific RFID
	CONFIG_STAT_RFID_SUCCESSFUL_SCANS,
	CONFIG_STAT_RFID_UNSUCCESSFUL_SCANS,
	// feeder specific odds and ends
	CONFIG_STAT_INITS,
	CONFIG_STAT_GENERIC_VOLTS_AT_RESET,
	CONFIG_STAT_GENERIC_TIME_SINCE_RESET,
	CONFIG_STAT_RECAL_COUNT,
	CONFIG_STAT_BACKSTOP
} CONFIG_STATS;

typedef PACKED_STRUCT
{
	CONFIG_STATS	stat;
	uint32_t		value;
} MEDULLA_STATISTIC_PAIR;

typedef PACKED_STRUCT
{
	uint8_t					count;
	MEDULLA_STATISTIC_PAIR	stats[MEDULLA_MAX_STATS_IN_SET];
} MEDULLA_STAT_SET;

typedef PACKED_ENUM
{
	CONFIG_WEIGHT_CONTEXT_PET_OPENED,			// 0
	CONFIG_WEIGHT_CONTEXT_PET_CLOSED,			// 1
	CONFIG_WEIGHT_CONTEXT_INTRUDER_CLOSED,		// 2
	CONFIG_WEIGHT_CONTEXT_DUBIOUS_CLOSED,		// 3
	CONFIG_WEIGHT_CONTEXT_USER_OPENED,			// 4
	CONFIG_WEIGHT_CONTEXT_USER_CLOSED,			// 5
	CONFIG_WEIGHT_CONTEXT_USER_ZEROED,			// 6
	CONFIG_WEIGHT_CONTEXT_BACKSTOP				// 7
} CONFIG_WEIGHT_CONTEXT;

typedef PACKED_STRUCT
{
	int32_t		initial;
	int32_t		final;
} PROP_WEIGHT_FRAME; 

typedef PROP_WEIGHT_FRAME PROP_WEIGHT_FRAME_SET[CONFIG_NUM_WEIGHT_FRAMES];

typedef PACKED_ENUM
{
	PARIETAL_READ_VALUE,
	PARIETAL_UPDATE_VALUE,
	PARIETAL_COMMIT_VALUES,
	PARIETAL_READ_BACKUP,
	PARIETAL_READ_MIRROR,
	PARIETAL_READ_MIRROR_BACKUP,
	PARIETAL_COMMAND_BACKSTOP
} PARIETAL_COMMAND;

typedef PACKED_ENUM
{
	PARIETAL_MAC_ADDRESS,
	PARIETAL_SERIAL_NUMBER,
	PARIETAL_SECRET_CODE,
	PARIETAL_HARDWARE_VERSION,
	PARIETAL_ONE_SHOT_BACKSTOP,
} PARIETAL_ONE_SHOT_SETTINGS;

typedef uint8_t SYNAPSE_ONE_SHOT_DATA[PARIETAL_MAX_DATA_LENGTH];

typedef PACKED_STRUCT
{
	uint8_t					count;
	SYNAPSE_ONE_SHOT_DATA	data;
} SYNAPSE_ONE_SHOT;

typedef PACKED_ENUM
{
	TASK_ID_HOUSEKEEPING, 	// 0 This should always be first.
	TASK_ID_EGO,			// 1
	TASK_ID_FUSIFORM,		// 2
	TASK_ID_AZIMUTH,		// 3
	TASK_ID_RFID,			// 4
	TASK_ID_OCCIPITAL,		// 5
	TASK_ID_LOCOMOTIVE,		// 6
	TASK_ID_HIPPOCAMPUS,	// 7
	TASK_ID_LUCIFERASE,		// 8
	TASK_ID_PROPRIOCEPTION,	// 9
	TASK_ID_LID,			// 10
	TASK_ID_OPERCULARIS,	// 11
	TASK_ID_MEDULLA,		// 12
	TASK_ID_SYNAPSE, 		// 13 Should be near the end.
	TASK_ID_BACKSTOP 		// 14 This should always be last.
} TASK_ID;

typedef PACKED_ENUM
{
	CONFIG_OCCIPITAL_TOP,
	CONFIG_OCCIPITAL_BOTTOM,
	CONFIG_OCCIPITAL_BACKSTOP
} CONFIG_OCCIPITAL_SIDE; 

typedef PACKED_STRUCT
{
	CONFIG_OCCIPITAL_SIDE	side;
	uint32_t				signal_area;
	uint32_t				neutral_area;
	uint32_t				threshold;
	uint32_t				noise_threshold;
	int32_t					reference;
	uint16_t				samples[OCCIPITAL_NUM_SAMPLES_TOTAL - OCCIPITAL_NUM_SAMPLES_IGNORED];
} OCCIPITAL_DEBUG_REPORT;

typedef PACKED_STRUCT
{
	uint32_t		level_1;
	uint32_t		level_2;
	AZIMUTH_ANGLE	decision;
} AZIMUTH_REPORT; 

typedef PACKED_STRUCT
{
	uint32_t	idle_reference;			
	uint32_t	idle_reference_scaled;	
	uint32_t	active_threshold;		
	uint32_t	idle_threshold;			
	uint32_t	recal_countdown;		
} AZIMUTH_THRESHOLDS; 

typedef PACKED_STRUCT
{
	AZIMUTH_REPORT		normal_report;
	AZIMUTH_THRESHOLDS	thresholds_1;
	AZIMUTH_THRESHOLDS	thresholds_2;
} AZIMUTH_DEBUG_REPORT;

typedef PACKED_ENUM
{
	CONFIG_RFID_MAIN,
	CONFIG_RFID_BACKSTOP
} CONFIG_RFID_SIDE; 

typedef PACKED_STRUCT
{
	CONFIG_RFID_SIDE	side;
	bool				success;
	uint32_t			amp_before_ramp;
	uint32_t			adjusted_target_amp;
	uint32_t			peak_amp;
	uint32_t			amp_after_scan;
	uint32_t			pulses_required;
	RFID_MICROCHIP		chip;
} RFID_DEBUG_REPORT;

typedef PACKED_ENUM
{
	LOCOMOTIVE_STATE_LOCKED,
	LOCOMOTIVE_STATE_DRIVING,
	LOCOMOTIVE_STATE_PULSING,
	LOCOMOTIVE_STATE_UNLOCKED,
	LOCOMOTIVE_STATE_BACKSTOP
} LOCOMOTIVE_STATE;

typedef PACKED_STRUCT
{
	int32_t	actual_weight;	// the DAC sample
	int32_t	offset;			// as observed/calculated
	int32_t	scale;			// calibrated at the factory
	int32_t	hundredths;		// calculated weight in hundredths of a gram
	int32_t food_weight;
	int32_t	tare;			// the difference from zero
	int32_t	plate;			// what does the plate weigh?
	int32_t	overridden_weight;	// When we cannot get a Steady weight, our measurement is stored here.
	bool 	low_gain;		// low_gain for testing in the programmer
	bool	pending;		// A measurement is yet to be finalised.
	bool	steady;			// A steady measurement was possible, or not requested.
	int32_t : 8;       	// just for kidding, I mean padding
} PROP_WEIGHT; 

typedef PROP_WEIGHT PROPRIOCEPTION_REPORT[CONFIG_NUM_WEIGHT_FRAMES];

typedef PACKED_ENUM
{
	LID_DEBUG_STOP,
	LID_DEBUG_OPEN,
	LID_DEBUG_CLOSE,
	LID_DEBUG_SAMPLE,
	LID_DEBUG_BACKSTOP
} LID_DEBUG_CONTROL;

typedef PACKED_STRUCT
{
	//uint16_t	time;
	uint16_t	sample;
} LID_SPEED_SAMPLE;

typedef PACKED_STRUCT
{
	LID_DEBUG_CONTROL	request;
	uint32_t			full_duration;
	LID_SPEED_SAMPLE	samples[LID_DEBUG_REPORT_SAMPLES];
} LID_DEBUG_REPORT;

typedef PACKED_ENUM
{
	LUC_LED_OFF,
	LUC_LED_RED,
	LUC_LED_GREEN,
	LUC_LED_AMBER
} LUC_LED_COLOUR;

typedef PACKED_ENUM
{
	// PA
	CONFIG_IO_A0,
	CONFIG_IO_USER_BUTTONS_1,
	CONFIG_IO_A2,
	CONFIG_IO_A3,
	CONFIG_IO_FXDA_IN,
	CONFIG_IO_FXDB_IN,
	CONFIG_IO_TROVAN_IN,
	CONFIG_IO_ANTENNA_AMP,
	CONFIG_IO_A8,
	CONFIG_IO_UART_TX,
	CONFIG_IO_UART_RX,
	CONFIG_IO_A11,
	CONFIG_IO_A12,
	CONFIG_IO_SWD_IO,
	CONFIG_IO_SWD_CLK,
	CONFIG_IO_LED_1_RED,

	// PB
	CONFIG_IO_IR_PD_BOTTOM,
	CONFIG_IO_IR_PD_TOP,
	CONFIG_IO_TUNE_CTL,
	CONFIG_IO_SLIDE_SWITCH_1,
	CONFIG_IO_SLIDE_SWITCH_2,
	CONFIG_IO_SLIDE_SWITCH_3,
	CONFIG_IO_I2C_SCL,
	CONFIG_IO_I2C_SDA,
	CONFIG_IO_RF_SLP_TR,
	CONFIG_IO_RF_nRST,
	CONFIG_IO_RF_SPI_SCK,
	CONFIG_IO_ANALOG_PWR_EN,
	CONFIG_IO_B12,
	CONFIG_IO_RES_CTL_NEG,
	CONFIG_IO_RES_CTL_POS,
	CONFIG_IO_B15,

	// PC
	CONFIG_IO_THERMISTOR,
	CONFIG_IO_VBAT_MONITOR,
	CONFIG_IO_SPI_MISO,
	CONFIG_IO_SPI_MOSI,
	CONFIG_IO_MOTOR_SENSE,
	CONFIG_IO_C5,
	CONFIG_IO_IR_LED_BOTTOM,
	CONFIG_IO_IR_LED_TOP,
	CONFIG_IO_PD_PWR_EN,
	CONFIG_IO_C9,
	CONFIG_IO_LED_1_GREEN,
	CONFIG_IO_LED_2_RED,
	CONFIG_IO_LED_2_GREEN,
	CONFIG_IO_RF_nSEL,
	CONFIG_IO_C14,
	CONFIG_IO_C15,

	// PD
	CONFIG_IO_D0,
	CONFIG_IO_D1,
	CONFIG_IO_D2,
	CONFIG_IO_D3,
	CONFIG_IO_D4,
	CONFIG_IO_D5,
	CONFIG_IO_D6,
	CONFIG_IO_D7,
	CONFIG_IO_D8,
	CONFIG_IO_D9,
	CONFIG_IO_D10,
	CONFIG_IO_D11,
	CONFIG_IO_D12,
	CONFIG_IO_D13,
	CONFIG_IO_D14,
	CONFIG_IO_D15,

	// PE
	CONFIG_IO_E0,
	CONFIG_IO_E1,
	CONFIG_IO_E2,
	CONFIG_IO_E3,
	CONFIG_IO_SLIDE_SWITCH_EN,
	CONFIG_IO_E5,
	CONFIG_IO_E6,
	CONFIG_IO_E7,
	CONFIG_IO_E8,
	CONFIG_IO_STIMULUS,
	CONFIG_IO_TROVAN_DEMOD,
	CONFIG_IO_MOTOR_1,
	CONFIG_IO_MOTOR_2,
	CONFIG_IO_E13,
	CONFIG_IO_E14,
	CONFIG_IO_E15,

	// PH
	CONFIG_IO_CLK_32MHZ_IN,
	CONFIG_IO_H1,
	CONFIG_IO_RF_IRQ,
	CONFIG_IO_CLK_EN,

	CONFIG_IO_BACKSTOP
} CONFIG_IO_FUNCTION; 

typedef PACKED_STRUCT
{
	const CONFIG_IO_FUNCTION	red_pin;
	const CONFIG_IO_FUNCTION	green_pin;
	LUC_LED_COLOUR				colour;
	bool						flashing;
	uint32_t 					led_number;
} LUC_PHYSICAL_LED;

typedef PACKED_ENUM
{
	EGO_REPORT_OVERALL,
	EGO_REPORT_BACKSTOP
} EGO_REPORT_TYPE; 

typedef PACKED_STRUCT
{
	EGO_REPORT_TYPE		report_type;
} EGO_DEBUG_REPORT;

typedef union
{
	OCCIPITAL_DEBUG_REPORT	occipital;
	AZIMUTH_DEBUG_REPORT	azimuth;
	RFID_DEBUG_REPORT		rfid;
	LOCOMOTIVE_STATE		locomotive;
	PROPRIOCEPTION_REPORT	weight;
	LID_DEBUG_REPORT		lid;
	LUC_PHYSICAL_LED		leds;
	EGO_DEBUG_REPORT		ego;
} SYNAPSE_DEBUG_REPORT;








// Message Specifics:

typedef PACKED_STRUCT
{
	SYNAPSE_MESSAGE_TYPE	request_type;
	SURE_STATUS				status;
} SYNAPSE_MESSAGE_ACK;
/******* MESSAGE: Ack *******
* Description:	Acknowledges a message, with an error code. Should be the response to almost every message.
* Arguments:	None
* Fields:		request_type - SYNAPSE_MESSAGE_TYPE - The message type being acknowledged.
				status - SURE_STATUS - What went wrong, if anything.
*******/

typedef PACKED_STRUCT
{
	SYNAPSE_MESSAGE_TYPE	triggered_event;
	uint8_t					arguments[SYNAPSE_PLACEHOLDER_ARBITRARY_AMOUNT];
} SYNAPSE_MESSAGE_TRIGGER_EVENT;
/******* MESSAGE: Trigger Event *******
* Description:	Triggers the handler/dispatcher for the specified event.
* Arguments:	The triggered event field to be placed in the outgoing message.
* Fields:		triggered_event - SYNAPSE_MESSAGE_TYPE - The event to trigger.
				arguments - uint8_t[] - Variable length, context specific arguments.
*******/

typedef PACKED_STRUCT
{
	bool	enabled;
} SYNAPSE_MESSAGE_STREAM_CONTROL;
/******* MESSAGE: Stream Control *******
* Description:	Sets the general setting for stream enable.
* Arguments:	None
* Fields:		enabled - Boolean - Streams active or not.
*******/

typedef PACKED_STRUCT
{
	uint32_t	key;
} SYNAPSE_MESSAGE_RESTART;
/******* MESSAGE: Restart *******
* Description:	When the correct key is given, restarts the device.
* Arguments:	None
* Fields:		enabled - uint32_t - Only the correct value will cause a restart.
*******/

typedef PACKED_STRUCT
{
	BICAMERAL_CHECK	CHECK;
	BICAMERAL_KEY	KEY;
	uint32_t		target_version;
	uint32_t		num_blocks;
	uint32_t		bicameral_unlock;
} SYNAPSE_MESSAGE_UNLOCK_DOWNLOAD;
/******* MESSAGE: Unlock Download *******
* Description:	Prepares the unit for a firmware download. Can be Triggered to report the current upgrade status, or to check the hash of the current Flash Bank.
* Arguments:	Any non-zero uint8_t will trigger a Hash of the current
* Fields:		CHECK - BICAMERAL_CHECK - MD5 Check of the whole 96k image, including blank spaces.
				KEY - BICAMERAL_KEY - Key used for encryption/decryption of firmware during download.
				target_version - uint32_t - Firmware version to be downloaded.
				num_blocks - uint32_t - Number of 128 byte blocks to be transmitted (does NOT need to be contiguous).
				bicameral_unlock - uint32_t - The correct value here allows the Bicameral Fixed Section to be erased and written to.
*******/

typedef PACKED_STRUCT
{
	uint32_t	destination;
	uint32_t	checksum;
	uint8_t		contents[BICAMERAL_BLOCK_SIZE];
} SYNAPSE_MESSAGE_FIRMWARE_BLOCK;
/******* MESSAGE: Firmware Block *******
* Description:	A block of data in the firmware image.
* Arguments:	None
* Fields:		destination - uint32_t - Address of first byte to be placed.
				checksum - uint32_t - Check for data integrity.
				contents - uint8_t[] - Data to be placed.
*******/

typedef PACKED_STRUCT
{
	BICAMERAL_COMPLETION_ACTION		complete_action;
} SYNAPSE_MESSAGE_FIRMWARE_DOWNLOAD_COMPLETE;
/******* MESSAGE: Firmware Download Complete *******
* Description:	Confirmation that the firmware download has completed. Can be used to test an image, confirm it, or cause a system reset (leading to a Bank Switch).
* Arguments:	None
* Fields:		complete_action - BICAMERAL_COMPLETION_ACTION - Determines whether we commit an image or test it first.
*******/

typedef PACKED_STRUCT
{
	int32_t		time_zone;
	SURE_DAY	week_day;
} SYNAPSE_MESSAGE_SET_TIME;
/******* MESSAGE: Set Time *******
* Description:	Configures the RTC to match the server. Can be Triggered to get the information not included in a timestamp i.e. TimeZone and DayOfWeek.
* Arguments:	None
* Fields:		time_zone - int32_t - Number of minutes offset from UTC standard time (if displayed).
				week_day - SURE_DAY - Day of the week.
*******/

typedef PACKED_STRUCT
{
	SYNAPSE_KEY	key;
} SYNAPSE_MESSAGE_DEBUG_COMMAND;
/******* MESSAGE: Debug Command *******
* Description:	A veriety of debug functionality is accessible with specific keys via this message. First, the debug system must be unlocked by acquiring an Obscurer key, and applying it to all subsequent messages.
* Arguments:	None
* Fields:		key - SYNAPSE_KEY - Specific unlock key value to enable debug messages.
*******/

typedef PACKED_STRUCT
{
	uint8_t				setting;			// the settings are different depending on impf or mpf2 - FIXME
	uint32_t			value;
} SYNAPSE_MESSAGE_CHANGE_SETTING;
/******* MESSAGE: Change Setting *******
* Description:	Sets the specified setting to a particular value. Passed to Ego
* to deal with. Refer to Ego for settings definitions.
* Arguments:	The index of the setting to read and report. EGO_SETTING_INDEX. '255' to start an ACK'd transfer of all settings.
* Fields:		setting - EGO_SETTING_INDEX - The index of the setting to be changed.
				value - uint32_t - The value to be assigned. May be narrowed by Ego.
*******/

typedef PACKED_STRUCT
{
	SYNAPSE_MESSAGE_TYPE	controlled_event;
	bool					enabled;
} SYNAPSE_MESSAGE_EVENT_CONTROL;
/******* MESSAGE: Event Control *******
* Description:	Enables or Disables particular event notifications.
* NB: Some events or settings may override these locks.
* Arguments:	None
* Fields:		controlled_event - SYNAPSE_MESSAGE_TYPE - The type of event to be enabled/disabled. This CAN be a Command type, but is unlikely to have any effect if so.
				enabled - bool - Whether the event in question is enabled.
*******/

typedef PACKED_STRUCT
{
	uint32_t		hardware_revision;
	uint32_t		software_revision;
	uint32_t		thalamus_revision;
	uint32_t		chip_hash;
	uint32_t		settings_hash;
	uint32_t		curfew_hash;
	uint32_t		profiles_filled;
	SURE_CHIP_UID	uid;
	SURE_KEY		reserved;
	SURE_PRODUCT	product_type;
	uint8_t			reserved2;
	bool			banks_flipped;
} SYNAPSE_MESSAGE_PRODUCT_INFO;
/******* MESSAGE: Product Info *******
* Description:	Pulls out information regarding this unit and reports it. This message should be automatically transmitted when first connecting after a reset.
* Arguments:	None
* Fields:		hardware_revision - uint32_t - Programmed at production.
				software_revision - uint32_t - Determined at compile time.
				thalamus_revision - uint32_t - Determined at compile time.
				chip_hash - uint32_t - 32 bit CRC (polynomial: 0x04C11DB7uL) of each [[Datatypes#HIPPO_GENERAL_PROFILE|HIPPO_GENERAL_PROFILE]], bitwise XOR'd together.
				settings_hash - uint32_t - 32 bit CRC of the Ego Settings structure (polynomial: 0x04C11DB7uL).
				curfew_hash - uint32_t - 32 bit CRC (polynomial: 0x04C11DB7uL) of each Curfew Set (1 per Profile), bitwise XOR'd together.
				profiles_filled - uint32_t - Bit field indicating which profiles are active. Does NOT include Override.
				uid - SURE_CHIP_UID - ID unique to the microcontroller.
				reserved - SURE_KEY - Reserved for future use.
				product_type - SURE_PRODUCT	- Unique product identifier.
				reserved2 - uint8_t - Reserved for future use.
				banks_flipped - bool - Whether we are using Bank 2 or not for the current firmware image.
*******/

typedef PACKED_STRUCT
{
	uint32_t		battery_voltage;
	uint32_t		micro_voltage;
	uint32_t		ambient_temperature;
	SURE_DATE_TIME	last_notable_action;
} SYNAPSE_MESSAGE_ENVIRONMENT;
/******* MESSAGE: Environment *******
* Description: 	Reports environmental conditions of the unit and its surroundings.
* Arguments:	None
* Fields: 		battery_voltage - uint32_t - The last measurement of the battery voltage, in mV.
				micro_voltage - uint32_t - The supply voltage of the microcontroller, in mV.
				ambient_temperature - uint32_t - The last measurement of the ambient temperature, in degrees Kelvin.
				last_notable_action - SURE_DATE_TIME - The timestamp the last time the user performed a noteable action.
*******/

typedef PACKED_STRUCT
{
	uint8_t	ego_state;						// another thing with different meaning between impf/mpf2 FIXME
} SYNAPSE_MESSAGE_STATE_CONTROL;
/******* MESSAGE: State Control *******
* Description: 	Project specific message that interacts with Ego to determine the State of the unit.
				Coming in to the unit, this message ought to permit manipulation of the state, or the request of certain actions.
* Arguments:	None
* Fields: 		ego_state - EGO_STATE - Project specific contents for this message. Should contain state data.

*******/

typedef PACKED_STRUCT
{
	uint32_t				address;
	SYNAPSE_MESSAGE_TYPE	type;
	SURE_STATUS				status;
	uint8_t					block_failures;
} SYNAPSE_MESSAGE_DOWNLOAD_RESULT;
/******* MESSAGE: Download Result *******
* Description:	This is transmitted by the device as a response to firmware download commands.
*				After the command is implemented, this message delivers the result and some
*				basic details to allow the Host/Server to forget most of it...
* Arguments:	None
* Fields: 		address - uint32_t - Byte address that the operation was performed at.
				type - SYNAPSE_MESSAGE_TYPE - Type of command implemented. Currently one of: MESSAGE_UNLOCK_DOWNLOAD, MESSAGE_FIRMWARE_BLOCK, MESSAGE_FIRMWARE_DOWNLOAD_COMPLETE.
				status - SURE_STATUS - Whether the operation was successfully completed.
				block_failures - uint8_t - How many times a particular block has failed to verify; should cause the Hub to re-fetch a page at a certain threshold.
*******/

typedef PACKED_STRUCT
{
	uint8_t	length;
	uint8_t	text[SYNAPSE_PLACEHOLDER_DEBUG_MESSAGE_SIZE];
} SYNAPSE_MESSAGE_DEBUG;
/******* MESSAGE: Debug *******
* Description:	A text string, used for debug purposes. By default, this message
*				type is locked until explicitly enabled.
* Arguments:	A null-terminated string to transmit.
* Fields: 		length - uint8_t - Length of string in bytes.
				text - uint8_t[] - Actual full string starts here. Does NOT have to be null terminated.
*******/

typedef PACKED_STRUCT
{
	uint32_t	uplink_time;
	uint32_t	packets_lost;
	uint32_t	retries;
	uint32_t	signal_strength;
} SYNAPSE_MESSAGE_RF_STATS;
/******* MESSAGE: RF Stats *******
* Description:	Data about the health of the RF link.
* Arguments:	None
* Fields: 		uplink_time - uint32_t - Duration of the current link, in seconds.
				packets_lost - uint32_t - Simple count of packets deemed to be lost, in the current link.
				retries - uint32_t - Simple count of number of retries attempted, in the current link.
				signal_strength - uint32_t - The latest estimate of the RF signal strength, in TBD units.
*******/

typedef PACKED_STRUCT
{
	RFID_MICROCHIP					microchip;
	HIPPO_SPECIAL_PROFILES			special_profile;
//	uint8_t							special_profile;
	uint8_t							index;
	SYNAPSE_CHANGE_PROFILE_ACTION	action;
//	uint8_t							action;
} SYNAPSE_MESSAGE_CHANGE_PROFILE;
/******* MESSAGE: Change Profile *******
* Description:	Records, removes or modifies pet information for consideration in decision making.
* Arguments:	Index of the profile to begin retrieval at. 1 byte. '255' to begin an ACK'd transfer of all profiles.
* Fields: 		microchip - RFID_MICROCHIP - The animal added.
				special_profile - HIPPO_SPECIAL_PROFILES - General behaviour for pet. Overriden by permissions.
				index - uint8_t - The index of the pet in our local database. 0 indicates Override behaviour.
				action - SYNAPSE_CHANGE_PROFILE_ACTION - The action to perform with this information. See enum documentation for details.
*******/

typedef PACKED_STRUCT
{
	RFID_MICROCHIP		microchip;
	uint8_t				index;
	HIPPO_CURFEW_SET	curfews;
} SYNAPSE_MESSAGE_TIMED_ACCESS;
/******* MESSAGE: Timed Access *******
* Description:	Decribes a full set of curfews for a particular animal, controlling what profile that animal is evaluated as between particular times.
* Arguments:	Index of the pet profile to lookup. '255' to begin an ACK'd transfer of all profiles' Curfews.
* Fields: 		microchip - RFID_MICROCHIP - The animal being referred to.
				index - uint8_t - The index of the pet in our local database. 0 indicates Override behaviour.
				curfews - HIPPO_CURFEW_SET - The full set of curfews for the animal in question.
*******/

typedef PACKED_STRUCT
{
	uint32_t		temperature;
	uint32_t		duration;
	AZIMUTH_ANGLE	angle;
	uint8_t			side;
	RFID_MICROCHIP	microchip;
	uint8_t			encounter_id;
	uint32_t		ambient_temperature;
} SYNAPSE_MESSAGE_ANIMAL_ACTIVITY;
/******* MESSAGE: Animal Activity *******
* Description:	Indicates activity from the animal. A transition in a Flap-type product; feeding in a Bowl-type product.
* Arguments:	None
* Fields: 		temperature - uint32_t - The temperature byte read (from T Chips).
				duration - uint32_t - Length of time between beginning and completeion of movement. Seconds.
				angle - AZIMUTH_ANGLE - Indicates the status of the door. Only applicable in flap type products.
				side - uint8_t - Indicates the side an animal is on when interacting.
				microchip - RFID_MICROCHIP - The animal performing the activity.
				encounter_id - uint8_t - Locally unique ID for the encounter during which the activity took place.
				ambient_temperature - uint32_t - Last measured ambient temperature, in Kelvin.
*******/

typedef PACKED_STRUCT
{
	uint32_t 	duration;
	uint8_t		direction;
	uint8_t		encounter_id;
} SYNAPSE_MESSAGE_ANIMAL_PRESENT;
/******* MESSAGE: Animal Present *******
* Description:	Indicates that we have detected the presence of an animal. Transmitted when first detected, and when no longer detected.
* Arguments:	None
* Fields: 		duration - uint32_t - Length of time animal had been detected in milliseconds. '0' indicates the start of detection.
				direction - uint8_t - Indicates the "side" of the device that the animal occupies. Should this be an enum?
				encounter_id - uint8_t - Locally unique ID for the encounter during which the activity took place.
*******/

typedef PACKED_STRUCT
{
	uint32_t				temperature;
	RFID_MICROCHIP			microchip;
	HIPPO_SPECIAL_PROFILES	profile;
	uint8_t 				side;
	uint8_t					encounter_id;
	uint32_t				ambient_temperature;
} SYNAPSE_MESSAGE_MICROCHIP_READ;
/******* MESSAGE: Microchip Read *******
* Description:	Triggered when a microchip is successfully read.
* Arguments:	None
* Fields: 		temperature - uint32_t - The temperature measured by the chip, if enabled.
				microchip - RFID_MICROCHIP - The animal detected.
				profile - HIPPO_SPECIAL_PROFILES - The profile that the unit has evaluated the animal as.
				side - uint8_t - Indicates which side the read was conducted on.
				encounter_id - uint8_t - Locally unique ID for the encounter during which the activity took place.
				ambient_temperature - uint32_t - Last measured ambient temperature, in Kelvin.
*******/

typedef PACKED_STRUCT
{
	FUSIFORM_INPUT_EVENT	input_event;
} SYNAPSE_MESSAGE_BUTTON_PRESS;
/******* MESSAGE: Button Press *******
* Description:	Triggered by a button press/UI change. Also used by the server to "press" a button.
* Arguments:	None
* Fields: 		input_event - FUSIFORM_INPUT_EVENT - The button press to be simulated.
*******/

typedef PACKED_STRUCT
{
	MEDULLA_STAT_SET	stat_set;
} SYNAPSE_MESSAGE_STATISTICS;
/******* MESSAGE: Statistics *******
* Description:	Contains product statistics accrued since the last reset.
* Arguments:	One byte indicating the number of statistics requested, followed by up to 15 CONFIG_STAT values.
* Fields: 		stat_set - MEDULLA_STAT_SET - A list of statistic pairs and a count.
*******/

typedef PACKED_STRUCT
{
	RFID_MICROCHIP			microchip;
//	CONFIG_WEIGHT_CONTEXT	context;
	uint8_t					context;
	uint16_t				duration;
	uint8_t					num_elements;
	PROP_WEIGHT_FRAME_SET	weights;
	uint8_t					encounter_id;
	uint8_t					temperature;
	uint32_t				ambient_temperature;
} SYNAPSE_MESSAGE_WEIGHTS;
/******* MESSAGE: Weights *******
* Description:	Indicates either the currently measured weight (absolute) or a
*				change in weight (relative) on either side. If only one bowl is
*				set, only the left field is populated.
* Arguments:	None
* Fields: 		microchip - RFID_MICROCHIP -	The ID of the animal feeding. If we believe it is human-activated, this should be ignored.
				context - CONFIG_WEIGHT_CONTEXT - Indicates the type of transaction e.g. animal feeding or owner loading.
				duration - uint16_t - The time in seconds between the start of the event and the end.
				num_elements - uint8_t - The number of weight pairs being reported in the following field.
				weights - PROP_WEIGHT_FRAME_SET - The start, end, and offset weights for each "side", in order.
				encounter_id - uint8_t - Count indicating the uniqueish ID of the current encounter.
				temperature - uint8_t - If a temperature is read from a chip, it is included here.
				ambient_temperature - uint32_t - Last measured ambient temperature, in Kelvin.
*******/

typedef PACKED_STRUCT
{
	PARIETAL_COMMAND			command;
	PARIETAL_ONE_SHOT_SETTINGS	setting;
	SYNAPSE_ONE_SHOT			one_shot;
} SYNAPSE_MESSAGE_ONE_SHOT;
/******* MESSAGE: One Shot *******
* Description:	Configures the Parietal one-shot settings. Thalamus keeps the
*				settings in Flash and a buffered version in RAM. This message
*				either reads from Flash, writes to RAM, or commits the full
*				contents of the RAM variable to Flash (one time only).
* Arguments:	The PARIETAL_ONE_SHOT_SETTING we're interested in.
* Fields:		command - PARIETAL_COMMAND - What to do! Writes, reads, or commits One Shot values.
				setting - PARIETAL_ONE_SHOT_SETTINGS - The One Shot to perform the command on.
				one_shot - SYNAPSE_ONE_SHOT - A structure containing the value of the One Shot and a count of its size.
*******/

typedef PACKED_STRUCT
{
	TASK_ID					task;
	SYNAPSE_DEBUG_REPORT	report;
} SYNAPSE_MESSAGE_DEBUG_REPORT;
/******* MESSAGE: Debug Report *******
* Description:	Allows high-detail reports from individual modules for debugging and
*				testing within the Programmer and over RF.
* Arguments:	None
* Fields:		task - TASK_ID - Indicates which module the report originates from.
				report - SYNAPSE_DEBUG_REPORT - Union of all debug enabled module reports.
*******/

typedef union
{
	SYNAPSE_MESSAGE_ACK							ack;							// 0x00
	SYNAPSE_MESSAGE_TRIGGER_EVENT				trigger_event;					// 0x01	
	SYNAPSE_MESSAGE_STREAM_CONTROL				stream_control;					// 0x02
	SYNAPSE_MESSAGE_RESTART						restart;						// 0x03
	SYNAPSE_MESSAGE_UNLOCK_DOWNLOAD				unlock_download;				// 0x04
	SYNAPSE_MESSAGE_FIRMWARE_BLOCK				firmware_block;					// 0x05
	SYNAPSE_MESSAGE_FIRMWARE_DOWNLOAD_COMPLETE	firmware_download_complete;		// 0x06
	SYNAPSE_MESSAGE_SET_TIME					set_time;						// 0x07
	SYNAPSE_MESSAGE_DEBUG_COMMAND				debug_command;					// 0x08
	SYNAPSE_MESSAGE_CHANGE_SETTING				change_setting;					// 0x09
	SYNAPSE_MESSAGE_EVENT_CONTROL				event_control;					// 0x0a
	SYNAPSE_MESSAGE_PRODUCT_INFO				product_info;					// 0x0b
	SYNAPSE_MESSAGE_ENVIRONMENT					environment;					// 0x0c
	SYNAPSE_MESSAGE_STATE_CONTROL				state_control;					// 0x0d
	SYNAPSE_MESSAGE_DOWNLOAD_RESULT				download_result;				// 0x0e
	SYNAPSE_MESSAGE_DEBUG						debug;							// 0x0f
	SYNAPSE_MESSAGE_RF_STATS					rf_stats;						// 0x10
	SYNAPSE_MESSAGE_CHANGE_PROFILE				change_profile;					// 0x11
	SYNAPSE_MESSAGE_TIMED_ACCESS				timed_access;					// 0x12
	SYNAPSE_MESSAGE_ANIMAL_ACTIVITY				animal_activity;				// 0x13
	SYNAPSE_MESSAGE_ANIMAL_PRESENT				animal_present;					// 0x14
	SYNAPSE_MESSAGE_MICROCHIP_READ				microchip_read;					// 0x15
	SYNAPSE_MESSAGE_BUTTON_PRESS				button_press;					// 0x16
	SYNAPSE_MESSAGE_STATISTICS					statistics;						// 0x17
	SYNAPSE_MESSAGE_WEIGHTS						weights;						// 0x18
	SYNAPSE_MESSAGE_ONE_SHOT					one_shot;						// 0x19
	SYNAPSE_MESSAGE_DEBUG_REPORT				debug_report;					// 0x1a	
} SYNAPSE_MESSAGE_PAYLOAD;


typedef PACKED_STRUCT
{
	uint16_t				type;			// 2 bytes
	uint16_t				correlation_ID;	// 2
	uint32_t				timestamp;		// 4
//	uint8_t					check;			// 1
//	uint8_t					data_size;		// 1
//	uint16_t				padding;		// 2
} SYNAPSE_RF_HEADER;						// = 8 bytes total

typedef union
{
	SYNAPSE_MESSAGE_PAYLOAD smp;
	uint8_t					smpbyte[256];
} PAYLOAD;

typedef PACKED_STRUCT
{	
	SYNAPSE_RF_HEADER	header;
	PAYLOAD				payload;
} SYNAPSE_RF_MSG;


// and the prototypes for MQTT_Synapse helper routines

void set_unit (SURE_PRODUCT type);
void set_header (int8_t * str);
void set_payload (int8_t * str, uint8_t count);
void print_tod (uint32_t time);
void message_handler (uint16_t type);
void message_environment_handler (void);
void message_rf_stats_handler (void);
void button_press_handler (void);
void debug_message_handler (void);
void change_setting_handler (void);
void change_profile_handler (void);
void message_weights_handler (void);
void debug_command_handler (void);
void message_product_info_handler (void);
void message_timed_access_handler (void);
void message_animal_activity_handler (void);
void sure_date_time_to_string (SURE_DATE_TIME sdt, char * str);
void sure_date_time_to_time_string (SURE_DATE_TIME sdt, char * str);

#endif