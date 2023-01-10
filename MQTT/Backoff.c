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
* Filename: Backoff.c   
* Author:   Tom Monkhouse 29/10/2019
* Purpose:   
*   
*            
**************************************************************************/
#include "Components.h"
#include <stdlib.h>

#include "Backoff.h"
#include "hermes-time.h"

BACKOFF_RESULT Backoff_GetStatus(BACKOFF* backoff)
{
	uint32_t now = get_microseconds_tick();

	if( NULL == backoff->specs )
	{
		return BACKOFF_READY;
	}

	if( backoff->retries >= backoff->specs->max_retries )
	{
		return BACKOFF_FAILED;
	}
	uint32_t milliseconds_since = (now - backoff->last_retry_tick) / 1000;

	if( milliseconds_since < backoff->retry_delay_ms )
	{
		return BACKOFF_WAITING;
	}

	if( backoff->retries == backoff->specs->max_retries - 1 )
	{
		return BACKOFF_FINAL_ATTEMPT;
	}

	return BACKOFF_READY;
}

BACKOFF_RESULT Backoff_Progress(BACKOFF* backoff)
{
	if( NULL == backoff->specs )
	{
		return BACKOFF_FAILED;
	}

	backoff->last_retry_tick = get_microseconds_tick();
	backoff->retry_delay_ms = backoff->specs->base_retry_interval_ms;

	uint8_t	retries = backoff->retries;
	while( 0 < retries-- )
	{
		backoff->retry_delay_ms *= backoff->specs->retry_mult_base;
	}

	int32_t retry_jitter = 0;
	//RNG_HandleTypeDef hrng;
	HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t*)&retry_jitter);

	backoff->retry_delay_ms += retry_jitter % backoff->specs->retry_jitter_max_ms;

	if( backoff->specs->max_retries > backoff->retries )
	{
		backoff->retries++;
	}

	if( backoff->specs->max_retries > backoff->retries )
	{
		return BACKOFF_FAILED;
	}

	return BACKOFF_WAITING;
}

void Backoff_Reset(BACKOFF* backoff)
{
	backoff->retries = 0;
	backoff->last_retry_tick = get_microseconds_tick();
	backoff->retry_delay_ms = 0;//backoff->specs->base_retry_interval_ms;
}
