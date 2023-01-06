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
* Filename: SNTP.h   
* Author:   Tom Monkhouse 02/04/2020
* Purpose:  Handles retrieving SNTP from an internet Time Server.
*           
**************************************************************************/

#ifndef __SNTP_H__
#define __SNTP_H__

#define SNTP_SERVER		"pool.ntp.org"
#define SNTP_PORT		123
#define SNTP_EPOCH 		(86400ul * (365ul * 70ul + 17ul))	// Epoch starts: 01-Jan-1970 00:00:00

#define SNTP_AUTO_INTERVAL	portMAX_DELAY // Don't auto-update - wait for a signal to. Used to be pdMS_TO_TICKS(1000*60*15)

#define PRINT_SNTP		false
#define SNTP_LINE		"\t@ "

#if PRINT_SNTP
#define sntp_printf(...)	zprintf(CRITICAL_IMPORTANCE, __VA_ARGS__)
#define sntp_flush			DbgConsole_Flush
#else
#define sntp_printf(...)
#define sntp_flush()
#endif

typedef enum
{
	SNTP_EVENT_UPDATE_REQUESTED	= (1<<0),
	SNTP_EVENT_UPDATE_UNDERWAY	= (1<<1),
	SNTP_EVENT_TIME_VALID		= (1<<2),
	SNTP_EVENT_UPDATE_FAILED	= (1<<3),
} SNTP_EVENT_BITS;

// Defines the structure of an NTP packet
typedef struct
{
	struct
	{
		uint8_t mode			: 3;// NTP mode
		uint8_t versionNumber 	: 3;// SNTP version number
		uint8_t leapIndicator	: 2;// Leap second indicator
	} flags;						// Flags for the packet

	uint8_t		stratum;			// Stratum level of local clock
	uint8_t		poll;				// Poll interval
	uint8_t		precision;			// Precision (seconds to nearest power of 2)
	uint32_t	root_delay;			// Root delay between local machine and server
	uint32_t	root_dispersion;	// Root dispersion (maximum error)
	uint32_t	ref_identifier;		// Reference clock identifier
	uint32_t	ref_ts_secs;		// Reference timestamp (in seconds)
	uint32_t	ref_ts_fraq;		// Reference timestamp (fractions)
	uint32_t	orig_ts_secs;		// Origination timestamp (in seconds)
	uint32_t	orig_ts_fraq;		// Origination timestamp (fractions)
	uint32_t	recv_ts_secs;		// Time at which request arrived at sender (seconds)
	uint32_t	recv_ts_fraq;		// Time at which request arrived at sender (fractions)
	uint32_t	tx_ts_secs;			// Time at which request left sender (seconds)
	uint32_t	tx_ts_fraq;			// Time at which request left sender (fractions)
} NTP_PACKET;

void 	SNTP_Init(void);
void	SNTP_Task(void* pvParameters);
bool	SNTP_IsTimeValid(void);
bool	SNTP_DidUpdateFail(void);
bool	SNTP_AwaitUpdate(bool MakeRequest, TickType_t TimeToWait);

#endif
