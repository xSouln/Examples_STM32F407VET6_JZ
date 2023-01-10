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
* Filename: Hermes-time.h   
* Author:   Chris Cowdery 04/04/2019
* Purpose:   
*   
* Delays - some delay functions based on GPT1. Note that if operating in an OS context,
* you should use vTaskDelay() and friends instead. This is meant for low level code.
* Also has functionality for UTC, which is seconds since the Epoch.
* Systick is used to drive UTC as we get a regular interrupt.
*            
**************************************************************************/

#ifndef __HERMES_TIME_H__
#define __HERMES_TIME_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define usTICK_SECONDS 		1000000
#define usTICK_MILLISECONDS	1000
#define usTICK_MINUTE		(usTICK_SECONDS * 60)
#define usTICK_HOUR			(usTICK_MINUTE * 60)

typedef struct
{
	uint8_t		day;
	uint8_t		hour;
	uint8_t		minute;
	uint8_t		second;
} HERMES_TIME_GMT;

void set_utc_to_compile_time(void);
void delay_us(uint32_t delay);
void delay_ms(uint32_t delay);
uint32_t get_microseconds_tick(void);
void get_UTC_ms(uint64_t *p);
uint32_t get_UTC(void);
uint32_t get_UpTime(void);
void set_utc(uint32_t val);
bool get_gmt(uint32_t utc_secs, HERMES_TIME_GMT* out);
struct tm *hermes_gmtime(time_t *tod);
int time_string(char *str);

#endif
