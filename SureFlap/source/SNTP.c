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
* Filename: SNTP.c
* Author:   Tom Monkhouse 02/04/2020
* Purpose:  Handles retrieving SNTP from an internet Time Server.
*
**************************************************************************/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"
#include "FreeRTOS_Sockets.h"

#include "hermes-time.h"
#include "timer_interface.h"
#include "NetworkInterface.h"
#include "tls\network_platform.h"
#include "compiler.h"

#include "wolfssl/ssl.h"

#include "SNTP.h"

static bool	SNTP_GetTime(void);
EventGroupHandle_t	xSNTP_EventGroup;

void SNTP_Init(void)
{
	xSNTP_EventGroup = xEventGroupCreate();
}

void SNTP_Task(void *pvParameters)
{
    EventBits_t xEventBits;

	while( true )
	{
		xEventBits = xEventGroupWaitBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_REQUESTED, pdFALSE, pdFALSE, SNTP_AUTO_INTERVAL);
		if( 0 != (SNTP_EVENT_UPDATE_REQUESTED & xEventBits) )
		{
			SNTP_GetTime();
		}
	};
}

bool SNTP_IsTimeValid(void)
{
	return (0 != (SNTP_EVENT_TIME_VALID & xEventGroupGetBits(xSNTP_EventGroup)));
}

bool SNTP_DidUpdateFail(void)
{
	return (0 != (SNTP_EVENT_UPDATE_FAILED & xEventGroupGetBits(xSNTP_EventGroup)));
}

bool SNTP_AwaitUpdate(bool MakeRequest, TickType_t TimeToWait)
{
	EventBits_t xEventBits = xEventGroupGetBits(xSNTP_EventGroup);

	if( (true == MakeRequest) && (0 == (SNTP_EVENT_UPDATE_UNDERWAY & xEventBits)) )
	{
		xEventGroupClearBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_FAILED | SNTP_EVENT_TIME_VALID);
		xEventGroupSetBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_REQUESTED);
	}

	xEventBits = xEventGroupWaitBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_FAILED | SNTP_EVENT_TIME_VALID, pdFALSE, pdFALSE, TimeToWait);
	if( (0 == TimeToWait) || (0 != (SNTP_EVENT_TIME_VALID & xEventBits)) )
	{
		return true;
	}
	return false;
}

static bool SNTP_GetTime(void)
{
	Socket_t 		sntpSocket;
	uint32_t		rx_timeout = 1000; // TAM Check for the best value of this.
	bool			success = false;

	xEventGroupClearBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_FAILED | SNTP_EVENT_TIME_VALID);
	xEventGroupSetBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_UNDERWAY);

	sntp_printf("\r\n\t@@@@@@@ Starting SNTP @@@@@@@\r\n");

	do
	{
		struct freertos_sockaddr	address;
		address.sin_port = 0;

		sntp_printf(SNTP_LINE "Setting up Socket | ");
		if( FREERTOS_INVALID_SOCKET == (sntpSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP)) ) break;

		address.sin_port = 0u;
		int32_t result = FreeRTOS_bind( sntpSocket, &address, sizeof(address) );
		if( 0 == result )
		{
			sntp_printf("Success!\r\n");
		} else
		{
			sntp_printf("Failure: %d!\r\n", result);
			break;
		}
		FreeRTOS_setsockopt( sntpSocket, 0, FREERTOS_SO_RCVTIMEO, &rx_timeout, sizeof(rx_timeout) );
		FreeRTOS_setsockopt( sntpSocket, 0, FREERTOS_SO_SNDTIMEO, &rx_timeout, sizeof(rx_timeout) );

		sntp_printf(SNTP_LINE "Resolving URL: %s | ", SNTP_SERVER);

		uint32_t address_len	= sizeof(address);
		address.sin_addr = FreeRTOS_gethostbyname(SNTP_SERVER);
		address.sin_port = FreeRTOS_htons(SNTP_PORT);

		if( address.sin_addr == 0 )
		{
			sntp_printf("Failed!\r\n");
			break;
		}
		sntp_printf("Found at %08x.\r\n", address.sin_addr);
		sntp_printf(SNTP_LINE "Writing Request | ");

		NTP_PACKET	request;
		memset(&request, 0, sizeof(request));
		request.flags.versionNumber = 3;
		request.flags.mode = 3;
		request.orig_ts_secs = swap32(SNTP_EPOCH);

		if( sizeof(request) != FreeRTOS_sendto(sntpSocket, &request, sizeof(request), 0, &address, sizeof(address)) )
		{
			sntp_printf("Failed!\r\n");
			break;
		}
		sntp_printf("Success!\r\n");
		sntp_printf(SNTP_LINE "Awaiting Response | ");

		result = FreeRTOS_recvfrom(sntpSocket, &request, sizeof(request), 0, &address, &address_len);
		if( result == sizeof(request) )
		{
			if( request.stratum == 0 )
			{	// Invalid response, maybe a KoD.
				sntp_printf("SNTP received an invalid response: %.4s\r\n", request.ref_identifier);
				success = false;
				break;
			}

			request.tx_ts_secs = swap32(request.tx_ts_secs) - SNTP_EPOCH; // Flip endianess and subtract epoch.
			if( request.tx_ts_fraq & 0x80 ){ request.tx_ts_secs++; } // Fraction is 32-bit big-endian, so 0x80 represents 0.5.
			sntp_printf("Received: %d seconds since epoch.\r\n", request.tx_ts_secs);
			set_utc(request.tx_ts_secs);

			volatile struct tm time;
			time = *hermes_gmtime(&request.tx_ts_secs);
			sntp_printf(SNTP_LINE "Year: %d | Month: %d | Day: %d | Hour: %d | Minute: %d | Second: %d\r\n", time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
			success = true;
		} else
		{
			sntp_printf("Failed. Received %d/%d bytes.\r\n", result, sizeof(request));
		}
	} while( false ); // Just for non-returning breaks.

	xEventGroupClearBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_REQUESTED | SNTP_EVENT_UPDATE_UNDERWAY); // Signal that we're done. Note: doesn't mean the time is good.
	if( true == success )
	{
		xEventGroupSetBits(xSNTP_EventGroup, SNTP_EVENT_TIME_VALID);	// Signal that the time is good.
	} else
	{
		xEventGroupSetBits(xSNTP_EventGroup, SNTP_EVENT_UPDATE_FAILED);	// ...Or that it's not.
	}
	FreeRTOS_closesocket(sntpSocket);

	sntp_printf("\t@@@@@@@ SNTP Complete @@@@@@@\r\n");
	sntp_flush();
	return success;
}