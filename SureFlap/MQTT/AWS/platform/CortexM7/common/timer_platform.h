/*
 * File:   timer_platform.h
 * Author: tom monkhouse
 *
 * Created on 26 June 2019, 16:13
 */

#ifndef TIMER_PLATFORM_H
#define	TIMER_PLATFORM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "timer_interface.h"

#define AWS_EXTRA_QOS1_HANDLE_TIME	10	// Extra time added to timeout on QoS1 handling to allow ACKs to be sent.
	
struct Timer
{
	uint32_t	start_time;
	uint32_t	duration;
};
#define EMPTY_TIMER	{0, 0}

#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_PLATFORM_H */

