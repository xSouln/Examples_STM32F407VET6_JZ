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
* Filename: HTTP_Helper.h
* Author:   Tom Monkhouse 21/02/2020
* Purpose:
*
* Provides blocking and non-blocking interface to HTTP Post request function.
*
**************************************************************************/

#ifndef __HTTP_HELPER_H_
#define __HTTP_HELPER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "hermes.h"

#include "wolfssl/ssl.h"

#define PRINT_HTTP	false
#define HTTP_LINE	"\t+ "
#define HTTP_PORT	443

#if PRINT_HTTP
#define http_printf(...)		zprintf(TERMINAL_IMPORTANCE, __VA_ARGS__)
#else
#define http_printf(...)
#endif

typedef struct	// struct for making HTTP Post Requests
{
	char*				URL;
	char*				resource;
	char*				contents;
	char*				response_buffer;
	uint32_t			response_size;
	TaskHandle_t		xClientTaskHandle;
	DERIVED_KEY_SOURCE	tx_key_source;
	DERIVED_KEY_SOURCE	rx_key_source;
	int32_t*			encrypted_data;	// indicates if x-enc was present, and it's value
	uint32_t*			bytes_read;		// how many bytes were actually read back, excluding the header
} HTTP_POST_Request_params;

typedef struct
{
	char*		buffer;
	uint32_t	buffer_length;
	int32_t		content_length;	// -1 means no content length was found in the header
	int64_t		message_time;
	uint32_t	activity_timestamp;
	char		signature[SIGNATURE_LENGTH_ASCII + 1];
	char		received_signature[SIGNATURE_LENGTH_ASCII + 1];
	bool		got_signature;
	bool		signature_matches;
	bool		chunked;
	bool		reject_message;
	bool		got_update;
	bool		server_error;
	int32_t		encrypted_data;
} HTTP_RESPONSE_DATA;
#define DEFAULT_HTTP_RESPONSE_DATA	{NULL, 0, -1, 0, 0, {0}, {0}, false, false, false, false, false, false, -1}

typedef struct
{
	WOLFSSL_CTX*	context;
	WOLFSSL*		session;
	Socket_t		socket;
	int32_t			result;
	uint32_t		bytes_read;
} HTTP_CONNECTION;
#define DEFAULT_HTTP_CONNECTION	{NULL, NULL, 0, 0, 0}

void HTTPPostTask(void *pvParameters);
void HTTPPostTask_init(void);
bool HTTP_POST_Request(char* URL, char* resource, char* contents, char* response_buffer, \
	uint32_t response_size, bool wait, DERIVED_KEY_SOURCE tx_key, DERIVED_KEY_SOURCE rx_key, \
	int32_t *encrypted_data, uint32_t *bytes_read);

#endif
