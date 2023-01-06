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
* Filename: Backoff.h
* Author:   Tom Monkhouse 29/10/2019
* Purpose:   
*   
*            
**************************************************************************/

#ifndef BACKOFF_H
#define	BACKOFF_H

#include <stdint.h>

typedef enum
{
	BACKOFF_READY,
	BACKOFF_WAITING,
	BACKOFF_FAILED,
	BACKOFF_FINAL_ATTEMPT
} BACKOFF_RESULT;

typedef struct
{
	uint32_t	base_retry_interval_ms;
	uint32_t	retry_mult_base;
	uint32_t	retry_jitter_max_ms;
	uint8_t		max_retries;
} BACKOFF_SPECS;

typedef struct
{
	uint32_t	last_retry_tick;
	uint32_t	retry_delay_ms;
	uint8_t		retries;
	const BACKOFF_SPECS*	specs;
} BACKOFF;

BACKOFF_RESULT Backoff_GetStatus(BACKOFF* backoff);
BACKOFF_RESULT Backoff_Progress(BACKOFF* backoff);
void Backoff_Reset(BACKOFF* backoff);

#endif	/* BACKOFF_H */

