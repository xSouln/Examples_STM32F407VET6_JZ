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
* Filename: leds.h   
* Authors:   Zach Cohen 05/11/2019, Mr. T.A. Monkhouse, Esq. 17/03/2020, Chris Cowdery 19/05/2020
* Purpose:  LED Driver  
*             
**************************************************************************/

#ifndef __LEDS_H__
#define __LEDS_H__

#include "Components.h"
#include "FreeRTOSConfig.h"
#include "AWS.h"

#define LED_MAX_BRIGHTNESS		100		// % Duty Cycle
#define LED_UPDATE_INTERVAL		20		// Milliseconds
#define LED_DELAY_NO_PROGRESS	0xFFFFFFFE

typedef enum
{
	LED_PATTERN_SOLID,
	LED_PATTERN_ALTERNATE,
	LED_PATTERN_PULSE,
	LED_PATTERN_FLASH,
	LED_PATTERN_FAST_FLASH,
	LED_PATTERN_SOLID_LEFT,
	LED_PATTERN_SOLID_RIGHT,
	LED_PATTERN_MAX,
	LED_PATTERN_VFAST_FLASH,
	LED_PATTERN_BACKSTOP
} LED_PATTERN_TYPE;
extern const char* led_pattern_strings[];
	
typedef enum
{
	BOTH_LEDS,
	LEFT_LED,
	RIGHT_LED,
} LED_SIDE;

typedef enum
{
	COLOUR_RED,
	COLOUR_GREEN,
	COLOUR_ORANGE,
	COLOUR_BACKSTOP
} LED_COLOUR;
extern const char*	led_colour_strings[];

typedef enum
{
	LED_LEFT_GREEN,
	LED_LEFT_RED,
	LED_RIGHT_GREEN,
	LED_RIGHT_RED,
	LED_NUM_PHYSICALS
} LED_PHYSICAL_LEDS;

typedef union
{
	int8_t		bytes[LED_NUM_PHYSICALS];
	uint32_t	full;
} LED_BRIGHTNESS;

#define LED_BOTH_MASK	{0xFF, 0xFF, 0xFF, 0xFF}
#define LED_LEFT_MASK	{0xFF, 0xFF, 0x00, 0x00}
#define LED_RIGHT_MASK	{0x00, 0x00, 0xFF, 0xFF}

#define LED_GREEN_MASK	{0xFF, 0x00, 0xFF, 0x00}
#define LED_RED_MASK	{0x00, 0xFF, 0x00, 0xFF}
#define LED_ORANGE_MASK	{0xFF, 0xFF, 0xFF, 0xFF}
#define LED_OFF_MASK	{0x00, 0x00, 0x00, 0x00}

#define LED_GREEN_DIVS	{0x01, 0x01, 0x01, 0x01}
#define LED_RED_DIVS	{0x01, 0x01, 0x01, 0x01}
#define LED_ORANGE_DIVS	{0x05, 0x01, 0x05, 0x01}
#define LED_OFF_DIVS	{0x01, 0x01, 0x01, 0x01}

typedef enum
{
	LED_STAGE_MOVE_ON,
	LED_STAGE_LOOP_TO_START,
	LED_STAGE_STOP
} LED_STAGE_BEHAVIOUR;

typedef struct
{
	LED_SIDE			side;
	uint8_t				brightness;
	uint32_t			delay;		// Wait X milliseconds after achieving brightness target.
	uint8_t				approach_rate;	// How quickly to approach target (at 50Hz update rate).
	LED_STAGE_BEHAVIOUR	behaviour;	// What to do once target is achieved and delay is taken.
} LED_PATTERN_STAGE;
#define LED_PATTERN_NO_STAGE	255

typedef struct
{
	LED_PATTERN_TYPE			type;
	uint8_t						num_stages;
	const LED_PATTERN_STAGE*	stages;
} LED_PATTERN_TYPE_DEF;

typedef enum // This is the brightness level
{
	LED_MODE_OFF,
	LED_MODE_DIM,	
	LED_MODE_NORMAL,
	LED_MODE_BACKSTOP
} LED_MODE;
extern const char*	led_mode_strings[];

typedef struct
{
	LED_COLOUR					colour;
	LED_MODE					mode;
	const LED_PATTERN_TYPE_DEF*	pattern;
	uint8_t						stage_index;
	const LED_PATTERN_STAGE*	stage;
	Timer						stage_timer;
	bool						waiting;
	bool						limited_life;
	Timer						life_timer;
} LED_PATTERN;

typedef struct
{
	LED_MODE			mode;
	LED_COLOUR			colour;
	LED_PATTERN_TYPE	pattern_type;
	uint32_t			duration;
} LED_PATTERN_REQUEST;

void led_driver_init(void);
void LED_Request_Pattern(LED_MODE mode, LED_COLOUR colour, LED_PATTERN_TYPE pattern, uint32_t duration);
void led_task(void *pvParameters);

#endif
