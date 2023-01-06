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
* Filename: Hermes-time.c   
* Author:   Chris Cowdery 04/04/2019
* Purpose:   
*   
* Delays - some delay functions based on GPT1. Note that if operating in an OS context,
* you should use vTaskDelay() and friends instead. This is meant for low level code.
* Also has functionality for UTC, which is seconds since the Epoch.
* Systick is used to drive UTC as we get a regular interrupt.
*            
**************************************************************************/
#include <stdio.h>
/* FreeRTOS includes. */
#include "FreeRTOS.h"

#include "board.h"
#include "time.h"
#include "hermes-time.h"
#include "time.h"
#include "fsl_device_registers.h"
#include "fsl_gpt.h"

// no longer used now we have SNTP to set the time.
void set_utc_to_compile_time(void)
{
	uint32_t year = (__DATE__[7] - '0') * 1000 +
					(__DATE__[8] - '0') * 100 +
					(__DATE__[9] - '0') * 10 +
					(__DATE__[10] - '0') - 1970;
	uint32_t month = 0;
	if( strncmp(__DATE__, "Feb", 3) == 0 ) month = 1;
	if( strncmp(__DATE__, "Mar", 3) == 0 ) month = 2;
	if( strncmp(__DATE__, "Apr", 3) == 0 ) month = 3;
	if( strncmp(__DATE__, "May", 3) == 0 ) month = 4;
	if( strncmp(__DATE__, "Jun", 3) == 0 ) month = 5;
	if( strncmp(__DATE__, "Jul", 3) == 0 ) month = 6;
	if( strncmp(__DATE__, "Aug", 3) == 0 ) month = 7;
	if( strncmp(__DATE__, "Sep", 3) == 0 ) month = 8;
	if( strncmp(__DATE__, "Oct", 3) == 0 ) month = 9;
	if( strncmp(__DATE__, "Nov", 3) == 0 ) month = 10;
	if( strncmp(__DATE__, "Dec", 3) == 0 ) month = 11;

	uint32_t day = __DATE__[4];
	if( day >= '0' ){ day = (day - '0') * 10; }
	else day = 0;
	day += __DATE__[5] - '0';

	uint32_t hour = (__TIME__[0] - '0') * 10 +
					(__TIME__[1] - '0');
	uint32_t minute =	(__TIME__[3] - '0') * 10 +
						(__TIME__[4] - '0');
	uint32_t second =	(__TIME__[6] - '0') * 10 +
						(__TIME__[7] - '0');

	uint32_t new_utc = year * 12; // Year to month ratio.
	new_utc += month;
	new_utc *= 30; // Rough month to day ratio.
	new_utc += day;
	new_utc *= 24; // Day to hour ratio.
	new_utc += hour;
	new_utc *= 60; // Hour to minute ratio.
	new_utc += minute;
	new_utc *= 60; // Minute to second ratio.
	new_utc += second;

	set_utc(new_utc);
}

volatile uint64_t UTC_ms = 0;
volatile uint32_t UTC=0; // Counts once per second. When server is connected, will be set to seconds since Unix Epoch
volatile uint32_t UpTime=0;  // Counts once per second. Epoch is when the unit was last reset.

// Since GPT1 is counting at 1MHz, a delay measured in microseconds is very easy!
// Note actually counting at 62.5MHz/63 = 992KHz.
void delay_us(uint32_t delay)
{
    uint32_t start = GPT_GetCurrentTimerCount(GPT1);

    while( (GPT_GetCurrentTimerCount(GPT1) - start) < delay );
}

void delay_ms(uint32_t delay)
{
    delay_us(delay * 1008);	// calibrated for 992KHz timer
}

uint32_t get_microseconds_tick(void)
{
    return GPT_GetCurrentTimerCount(GPT1);
}

// This will occur in the Systick Interrupt context
// called at the Systick rate.
void vApplicationTickHook(void)
{
    static uint32_t div = 0;
    div++;
	UTC_ms++;
    if( div >= configTICK_RATE_HZ )
    {
        div = 0;
        UTC++;
        UpTime++;
    }
}

void get_UTC_ms(uint64_t *p)
{
    portENTER_CRITICAL();
    *p = UTC_ms;
    portEXIT_CRITICAL();	
}

// This could occur in any context, but since it's a
// 32bit value, we don't need to worry about any inconsistencies.
uint32_t get_UTC(void)
{
    return UTC;
}

// This could occur in any context, but since it's a
// 32bit value, we don't need to worry about any inconsistencies.
uint32_t get_UpTime(void)
{
    return UpTime;
}

// Set UTC. This will occur when the Hub connects to the server.
// We do have to be careful about interrupts here as we could
// get overwritten if not careful by increment_UTC()
void set_utc(uint32_t val)
{
    portENTER_CRITICAL();
    UTC = val;
	UTC_ms = val * 1000ull;
    portEXIT_CRITICAL();
}

/**************************************************************
 * Function Name   : get_gmt
 * Description     : convert utc seconds to time
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
bool get_gmt(uint32_t utc_secs, HERMES_TIME_GMT* out)
{
  struct tm x;
  time_t cstamp = utc_secs;       /* 1 */

	x = *hermes_gmtime(&cstamp);        /* 2 */

  out->day = x.tm_mday;
  out->hour = x.tm_hour;
  out->minute = x.tm_min;
  out->second = x.tm_sec;

  if(x.tm_year>15) return true;  //sanity check in case time not synchronised yet
  return false;
}

struct tm *hermes_gmtime(time_t *tod)
{
	struct tm *retval;
	portENTER_CRITICAL();
	retval = gmtime(tod);
	portEXIT_CRITICAL();
	return retval;
}

/**************************************************************
 * Function Name   : time_string
 * Description     : Generates current time of day as HH:MM:SS string
 * Inputs          : pointer to string for result
 * Outputs         : number of chars written
 * Returns         :
 **************************************************************/
int time_string(char *str)
{
	HERMES_TIME_GMT time;
	get_gmt(UTC, &time);
	return sprintf(str,"%02d:%02d:%02d ",time.hour,time.minute,time.second);
}
