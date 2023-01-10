#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "timer_platform.h"
#include "hermes-time.h"

void init_timer(Timer* t)
{
	t->start_time = 0;
	t->duration = 0;
}

bool has_timer_expired(Timer* t)
{
	if( 0u == t->duration ){ return true; }
	uint32_t tick = get_microseconds_tick();
	if( tick - t->start_time > t->duration )
	{
		t->duration = 0;
		t->start_time = 0;
		return true;
	}
	return false;
}

void countdown_ms(Timer* t, uint32_t ms)
{
	t->start_time = get_microseconds_tick();
	t->duration = ms * usTICK_MILLISECONDS;
}

void extend_countdown_ms(Timer* t, uint32_t ms)
{
	if( 0 == t->start_time ){ t->start_time = get_microseconds_tick(); }
	t->duration += ms * usTICK_MILLISECONDS;
}

void countdown_sec(Timer* t, uint32_t sec)
{
	t->start_time = get_microseconds_tick();
	t->duration = sec * usTICK_SECONDS;
}

uint32_t left_ms(Timer* t)
{
	if( 0 == t->duration ){ return 0u; }
	uint32_t tick = get_microseconds_tick();
	if( tick - t->start_time > t->duration )
	{
		t->duration = 0;
		t->start_time = 0;
		return 0;
	}
	int32_t diff = t->duration - (tick - t->start_time);
	return (diff + (usTICK_MILLISECONDS - 1)) / usTICK_MILLISECONDS;
}

