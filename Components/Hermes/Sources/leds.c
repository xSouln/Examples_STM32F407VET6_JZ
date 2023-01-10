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
* Filename: leds.c   
* Author:   Zach Cohen 05/11/2019, Mr. T.A. Monkhouse, Esq. 17/03/2020, Chris Cowdery 19/05/2020
* Purpose:  LED Driver  
*             
**************************************************************************/
//
// Some commentary is useful to describe how the LED brightnesses are handled.
// Background:
// - Each LED is driven via PWM as a percentage.
// - LED operation is controlled by a pattern. A pattern is made up of one
//   or more stages. 
// - A stage describes an LED combination (e.g. LEFT_LED), a percentage brightness
//   as one of BRIGHTNESS_FULL,NORMAL,OFF, some timing information, and what
//   to do at the end of the stage (e.g. move on, loop to start or stop)
// The Hermes Application calls the LED system with a call to LED_Start_Pattern()
// which specifies the LED_MODE (one of NORMAL,DIM or OFF), the pattern (one of
// SOLID,PULSE,ALTERNATE,FLASH etc), the colour (one of RED,GREEN or ORANGE)
// and a duration if it's a temporary pattern. This function loads up led_current_pattern
// with the parameters for the first stage of the requested pattern and some
// extra parameters such as the requested LED_MODE.
// LED_Progress_Stage() is then called which determines which LEDs should be on.
// It sets their brightness as defined in the pattern definition (i.e. one of
// BRIGHTNESS_FULL,NORMAL,OFF). This brightness is then modified according to
// the LED_MODE parameter that the pattern was started with. LED_NORMAL does
// nothing, LED_OFF turns off the LEDs and LED_DIM multiplies the brightness
// by DIMMING_FACTOR as a percentage. The results of these calculations are
// placed in the 4 byte array (one byte per LED) led_target_brightness[].
// The LED task calls LED_Implement_Pattern() which essentially calls
// LED_Approach_Target(). This compares led_target_brightness[] with 
// led_current_brightness[], modifies the current brightness appropriately if they
// differ and applies the percentage brightness values to the PWM drivers.
//
// LED_PATTERN_SOLID has a brightness of BRIGHTNESS_NORMAL
// LED_PATTERN_MAX is the same as SOLID but with BRIGHTNESS_FULL
// All of the other patterns (which are flashes of various sorts) are BRIGHTNESS_FULL.
// So four brightnesses are possible for solid outputs:
// - LED_MODE_NORMAL, LED_PATTERN_MAX = 100%  (BRIGHTNESS_FULL)
// - LED_MODE_NORMAL, LED_PATTERN_SOLID = 20% (BRIGHTNESS_NORMAL)
// - LED_MODE_DIM, LED_PATTERN_MAX = 5%       (BRIGHTNESS_FULL * DIMMING_FACTOR)
// - LED_MODE_DIM, LED_PATTERN_SOLID = 1%     (BRIGHTNESS_NORMAL * DIMMING_FACTOR)
// And two brightnesses are possible for flashing outputs, e.g.:
// - LED_MODE_NORMAL, LED_PATTERN_ALTERNATE = 100%
// - LED_MODE_DIM, LED_PATTERN_ALTERNATE = 5%

// Driver task for LEDs
#include <stdlib.h>
#include "hermes.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

// Other includes
#include "leds.h"

QueueHandle_t xLedMailbox;

// These are the default brightnesses of the various LED patterns.
// If the pattern is initiated with LED_MODE==LED_MODE_DIM, then
// the brightness is multiplied by the DIMMING_FACTOR.
#define BRIGHTNESS_FULL		100
#define BRIGHTNESS_NORMAL	20	// Percent 
#define BRIGHTNESS_OFF		0

#define DIMMING_FACTOR		5	// Percent

/******* Static Functions *******/
static bool		LED_Progress_Stage(bool initial);
static void 	LED_Start_Pattern(LED_MODE new_mode, LED_COLOUR new_colour, \
				LED_PATTERN_TYPE new_pattern, uint32_t new_duration);
static bool		LED_Approach_Target(void);
static uint32_t	LED_Implement_Pattern(void);
												// side,brightness %,delay,approach_rate,behaviour
static LED_PATTERN_STAGE	solid_pattern_def			= {	 BOTH_LEDS, BRIGHTNESS_NORMAL,0,	1,	LED_STAGE_STOP};
static LED_PATTERN_STAGE	alt_pattern_def[] 			= { {LEFT_LED, 	BRIGHTNESS_FULL,1000,100,LED_STAGE_MOVE_ON},
		 													{RIGHT_LED,	BRIGHTNESS_FULL,1000,100,LED_STAGE_LOOP_TO_START} };
static LED_PATTERN_STAGE	pulse_pattern_def[]			= {	{BOTH_LEDS,	BRIGHTNESS_FULL,0,	1,	LED_STAGE_MOVE_ON},
		 													{BOTH_LEDS,	BRIGHTNESS_OFF,	0,	1,	LED_STAGE_LOOP_TO_START} };
static LED_PATTERN_STAGE	flash_pattern_def[]			= {	{BOTH_LEDS, BRIGHTNESS_FULL,1000,100,LED_STAGE_MOVE_ON},
															{BOTH_LEDS, BRIGHTNESS_OFF,	1000,100,LED_STAGE_LOOP_TO_START} };
static LED_PATTERN_STAGE	fast_flash_pattern_def[]	= {	{BOTH_LEDS, BRIGHTNESS_FULL,250,100,LED_STAGE_MOVE_ON},
															{BOTH_LEDS, BRIGHTNESS_OFF,	250,100,LED_STAGE_LOOP_TO_START} };
static LED_PATTERN_STAGE	solid_left_def				= { LEFT_LED, 	BRIGHTNESS_FULL, 0, 100, LED_STAGE_STOP };
static LED_PATTERN_STAGE	solid_right_def				= { RIGHT_LED,  BRIGHTNESS_FULL, 0, 100, LED_STAGE_STOP };
static LED_PATTERN_STAGE	max_pattern_def				= {	 BOTH_LEDS, BRIGHTNESS_FULL,0,	1,	 LED_STAGE_STOP};
static LED_PATTERN_STAGE	vfast_flash_pattern_def[]	= {	{BOTH_LEDS, BRIGHTNESS_FULL,100,100,LED_STAGE_MOVE_ON},
															{BOTH_LEDS, BRIGHTNESS_OFF,	100,100,LED_STAGE_LOOP_TO_START} };

static const LED_PATTERN_TYPE_DEF	led_pattern_defs[] = 
{
	{ LED_PATTERN_SOLID, 	 sizeof(solid_pattern_def)/sizeof(LED_PATTERN_STAGE),	&solid_pattern_def },
	{ LED_PATTERN_ALTERNATE, sizeof(alt_pattern_def)/sizeof(LED_PATTERN_STAGE),		alt_pattern_def },
	{ LED_PATTERN_PULSE,	 sizeof(pulse_pattern_def)/sizeof(LED_PATTERN_STAGE),	pulse_pattern_def},
	{ LED_PATTERN_FLASH,	 sizeof(flash_pattern_def)/sizeof(LED_PATTERN_STAGE),	flash_pattern_def},
	{ LED_PATTERN_FAST_FLASH,sizeof(fast_flash_pattern_def)/sizeof(LED_PATTERN_STAGE),	fast_flash_pattern_def},
	{ LED_PATTERN_SOLID_LEFT,sizeof(solid_left_def)/sizeof(LED_PATTERN_STAGE), 		&solid_left_def},
	{ LED_PATTERN_SOLID_RIGHT,sizeof(solid_right_def)/sizeof(LED_PATTERN_STAGE),  	&solid_right_def},
	{ LED_PATTERN_MAX, 	 	 sizeof(solid_pattern_def)/sizeof(LED_PATTERN_STAGE),	&max_pattern_def },
	{ LED_PATTERN_VFAST_FLASH,sizeof(vfast_flash_pattern_def)/sizeof(LED_PATTERN_STAGE),	vfast_flash_pattern_def},
};

static const LED_BRIGHTNESS	led_side_masks[]	= {{ LED_BOTH_MASK }, { LED_LEFT_MASK }, { LED_RIGHT_MASK }};
static const LED_BRIGHTNESS	led_colour_masks[]	= {{ LED_RED_MASK }, { LED_GREEN_MASK }, { LED_ORANGE_MASK }, { LED_OFF_MASK }};
static const LED_BRIGHTNESS	led_colour_divs[]	= {{ LED_RED_DIVS }, { LED_GREEN_DIVS }, { LED_ORANGE_DIVS }, { LED_OFF_DIVS }};

static LED_BRIGHTNESS	led_current_brightness	= { 0 }; // Starts off.
static LED_BRIGHTNESS	led_target_brightness	= { 0 }; // Derived from Pattern, or driven directly.
static LED_PATTERN		led_current_pattern		= {COLOUR_BACKSTOP, LED_MODE_NORMAL, NULL, LED_PATTERN_NO_STAGE, NULL, EMPTY_TIMER, false, false, EMPTY_TIMER};
static LED_PATTERN		led_fallback_pattern;

const char* led_mode_strings[]		= { "off", "dim", "normal" };
const char* led_pattern_strings[] 	= { "solid", "alternate", "pulse", "flash", "fastflash" , "left", "right", "max", "vfast"};
const char*	led_colour_strings[] 	= { "red", "green", "orange"};

/**************************************************************
 * Function Name   : LED_Start_Pattern
 * Description     : Start off a new pattern on the LEDs. A pattern with limited duration
 *                 : will 'stack' the current pattern, and override it. The system will
 *                 : fall back when the limited duration expires to where it was before.
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
static void LED_Start_Pattern(LED_MODE new_mode, LED_COLOUR new_colour, LED_PATTERN_TYPE new_pattern, uint32_t new_duration)
{
	LED_PATTERN *pattern_to_update;
	
	// Three possible scenarios:
	// 1. duration == limited, so an override. Stack current pattern, and set new one.
	// 2. duration == 0, and no override active. Set current pattern to requested one.
	// 3. duration == 0, and override active. Set stacked pattern to requested one.
	if( 0 == new_duration)
	{	// This is a permanent pattern, so check if there is a limited duration one active now.
		if( true == led_current_pattern.limited_life)
		{ // this means that there is a short duration pattern active, so we just need to update the fallback pattern.
			pattern_to_update = &led_fallback_pattern;

		} else
		{
			led_current_pattern.limited_life = false;
			pattern_to_update = &led_current_pattern;
		}
	}else if (false == led_current_pattern.limited_life)
	{ 	// Only do this temporary pattern if there isn't one already in progress.
		// Copy the current pattern into the fallback pattern.
		memcpy(&led_fallback_pattern, &led_current_pattern,sizeof(LED_PATTERN));
		led_current_pattern.limited_life = true;
		countdown_ms(&led_current_pattern.life_timer, new_duration);
		pattern_to_update = &led_current_pattern;
	}
	
	if( COLOUR_BACKSTOP > new_colour ){ pattern_to_update->colour = new_colour; }
	if( LED_PATTERN_BACKSTOP > new_pattern)
	{
		pattern_to_update->pattern = &led_pattern_defs[new_pattern];
	}
	
	pattern_to_update->mode	 		= new_mode;
	pattern_to_update->stage_index 	= 0;
	pattern_to_update->stage 		= pattern_to_update->pattern->stages;
	pattern_to_update->waiting 		= false;
	
	// Don't start pattern if just updating fallback pattern
	if (&led_fallback_pattern != pattern_to_update)
	{
		LED_Progress_Stage(true);
	}
}

static bool LED_Progress_Stage(bool initial)
{
	if( (NULL == led_current_pattern.pattern) || (LED_PATTERN_NO_STAGE == led_current_pattern.stage_index) ){ return false; }
	if( false == initial )
	{
		switch( led_current_pattern.stage->behaviour )
		{
			case LED_STAGE_MOVE_ON:	
				led_current_pattern.stage_index++;
				break;
				
			case LED_STAGE_LOOP_TO_START:
				led_current_pattern.stage_index = 0;
				break;
				
			case LED_STAGE_STOP:
			default:
				return false;
				break;
		}
	}
	
	if( led_current_pattern.stage_index >= led_current_pattern.pattern->num_stages )
	{
		led_current_pattern.stage_index = LED_PATTERN_NO_STAGE;
		return false;
	}
	
	led_current_pattern.stage = &led_current_pattern.pattern->stages[led_current_pattern.stage_index];
	
	led_target_brightness.full = led_colour_masks[led_current_pattern.colour].full;
	led_target_brightness.full &= led_side_masks[led_current_pattern.stage->side].full;
	
	for( uint32_t i = 0; i<LED_NUM_PHYSICALS; i++ )
	{
		uint32_t calculated_target_brightness;
		
		calculated_target_brightness = led_current_pattern.stage->brightness;
		switch (led_current_pattern.mode)
		{
			case LED_MODE_DIM: // reduce target brightness if in DIM mode.
				calculated_target_brightness = calculated_target_brightness * DIMMING_FACTOR / 100;
				break;
			case LED_MODE_OFF:
				calculated_target_brightness = 0;
				break;			
			case LED_MODE_NORMAL:
			default:
				break;
		}
		led_target_brightness.bytes[i] &= (uint8_t)calculated_target_brightness;
	}
	
	return true;
}

static bool LED_Approach_Target(void)
{
	uint32_t	i;
	int32_t		diff;
	LED_BRIGHTNESS	altered_brightness;
	
	if( led_current_brightness.full == led_target_brightness.full ){ return true; }
	
	for( i=0; i<LED_NUM_PHYSICALS; i++ )
	{
		diff = led_current_brightness.bytes[i] - led_target_brightness.bytes[i];
		if( 0 != diff )
		{
			if( (NULL == led_current_pattern.pattern) ||
			   	(NULL == led_current_pattern.stage) ||
				(abs(diff) < led_current_pattern.stage->approach_rate) )
			{
				led_current_brightness.bytes[i] = led_target_brightness.bytes[i];
			} else
			{
				if( diff > 0 )
				{
					led_current_brightness.bytes[i] -= led_current_pattern.stage->approach_rate;
				} else
				{
					led_current_brightness.bytes[i] += led_current_pattern.stage->approach_rate;
				}
			}
		}
		if( led_current_brightness.bytes[i] > LED_MAX_BRIGHTNESS ){ led_current_brightness.bytes[i] = LED_MAX_BRIGHTNESS; }
		altered_brightness.bytes[i] = led_current_brightness.bytes[i] / led_colour_divs[led_current_pattern.colour].bytes[i];
	}
	/*
	PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, altered_brightness.bytes[LED_LEFT_GREEN]);
	PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmB, kPWM_SignedCenterAligned, altered_brightness.bytes[LED_RIGHT_GREEN]);
	PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmB, kPWM_SignedCenterAligned, LED_MAX_BRIGHTNESS-altered_brightness.bytes[LED_LEFT_RED]);
	PWM_UpdatePwmDutycycle(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, LED_MAX_BRIGHTNESS-altered_brightness.bytes[LED_RIGHT_RED]);
	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1 | kPWM_Control_Module_2, true);
	*/
	if( led_current_brightness.full == led_target_brightness.full ){ return true; }
	
	return false;
}

static uint32_t LED_Implement_Pattern(void)
{
	static Timer	led_update_timer 	= EMPTY_TIMER;
	bool 			progressing			= false;
	uint32_t		next_delay			= LED_DELAY_NO_PROGRESS;
	
	if( (true == led_current_pattern.limited_life) &&
	   	(true == has_timer_expired(&led_current_pattern.life_timer)) )
	{	// revert back to fallback display mode
		memcpy(&led_current_pattern, &led_fallback_pattern, sizeof(LED_PATTERN));
		LED_Start_Pattern(led_current_pattern.mode, led_current_pattern.colour, led_current_pattern.pattern->type, 0);
		next_delay = LED_UPDATE_INTERVAL;
	} else if( true == led_current_pattern.waiting )
	{
		if( true == has_timer_expired(&led_current_pattern.stage_timer) )
		{
			progressing = LED_Progress_Stage(false);
			if( true == progressing )
			{
				LED_Approach_Target();
				next_delay = LED_UPDATE_INTERVAL;
			} else
			{
				next_delay = LED_DELAY_NO_PROGRESS;
			}
			led_current_pattern.waiting = false;
		} else
		{
			next_delay = left_ms(&led_current_pattern.stage_timer);
		}
	} else if( true == has_timer_expired(&led_update_timer) )
	{
		if( true == LED_Approach_Target() ) 
		{	// Stage has reached its destination; check for a delay.
			if( (NULL == led_current_pattern.pattern) ||
			   	(NULL == led_current_pattern.stage) )
			{	// Nothing to do.
				next_delay = LED_DELAY_NO_PROGRESS;
			} else if( 0 == led_current_pattern.stage->delay )
			{
				progressing = LED_Progress_Stage(false); // False return mean we cannot progress...
				if( true == progressing )
				{
					countdown_ms(&led_update_timer, LED_UPDATE_INTERVAL);
					next_delay = LED_UPDATE_INTERVAL;
				} else
				{
					next_delay = LED_DELAY_NO_PROGRESS;
				}
			} else
			{
				countdown_ms(&led_current_pattern.stage_timer, led_current_pattern.stage->delay);
				next_delay = led_current_pattern.stage->delay;
				led_current_pattern.waiting = true;
			}
		} else
		{
			countdown_ms(&led_update_timer, LED_UPDATE_INTERVAL);
			next_delay = LED_UPDATE_INTERVAL;
		}
	} else
	{
		next_delay = left_ms(&led_update_timer);
	}
	
	return next_delay;
}

// This is called from other parts of the project
void LED_Request_Pattern(LED_MODE mode, LED_COLOUR colour, LED_PATTERN_TYPE pattern, uint32_t duration)
{
	LED_PATTERN_REQUEST	pattern_to_set = {mode, colour, pattern, duration};
	
	xQueueSend(xLedMailbox, &pattern_to_set, 0);
}

void led_task(void *pvParameters)
{
	LED_PATTERN_REQUEST	incoming_pattern;
	uint32_t			to_delay = LED_UPDATE_INTERVAL;
	
	while(true)
	{
		if( LED_DELAY_NO_PROGRESS == to_delay )
		{	// No pattern to progress and nothing in the mailbox. Just wait in messagebox.
			to_delay = portMAX_DELAY;
		} else
		{	// Otherwise, we've got work to do.
			to_delay = LED_Implement_Pattern();
		}
		
		if( pdPASS == xQueueReceive(xLedMailbox, &incoming_pattern, to_delay) )
		{
			LED_Start_Pattern(	incoming_pattern.mode, incoming_pattern.colour, \
								incoming_pattern.pattern_type, incoming_pattern.duration);
			to_delay = LED_UPDATE_INTERVAL;
		}
	};
}

void led_driver_init(void)
{
    xLedMailbox = xQueueCreate(3, sizeof(LED_PATTERN_REQUEST));
}
