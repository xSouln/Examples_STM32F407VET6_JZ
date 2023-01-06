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
* Filename: HTTP_Helper.c
* Author:   Tom Monkhouse
* Purpose:
*
* Provides blocking and non-blocking interface to HTTP Post request function.
*
**************************************************************************/

/* Standard includes. */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"
#include "semphr.h"
#include "signing.h"
#include "hermes-app.h"
#include "HTTP_Helper.h"
#include "hermes-time.h"
#include "NetworkInterface.h"
#include "tls\network_platform.h"
#include "HubFirmwareUpdate.h"
#include "MQTT.h"

static bool HTTP_Open_Connection(HTTP_CONNECTION* connection, char* URL);
static void HTTP_Kill_Connection(HTTP_CONNECTION* connection);
static bool HTTP_Read_Content(HTTP_CONNECTION* connection, HTTP_RESPONSE_DATA* response_data);
static bool HTTP_Transmit_Message(WOLFSSL* ssl, char* URL, char* resource, char* contents, char* signature, DERIVED_KEY_SOURCE tx_key);
static bool HTTP_Process_Header(WOLFSSL* ssl, HTTP_RESPONSE_DATA* response_data);
static bool HTTP_Calculate_Signature(HTTP_CONNECTION* connection, HTTP_RESPONSE_DATA* response_data, DERIVED_KEY_SOURCE rx_key);

// Mutex to prevent re-entry of HTTP Post function
SemaphoreHandle_t xHTTPPostSemaphoreHandle;

// Shared mailboxes
QueueHandle_t xHTTPPostRequestMailbox;
/**************************************************************
 * Function Name   : HTTP_Post_Task_init
 * Description     : Sets up mailboxes and events for HTTPPostTask
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void HTTPPostTask_init(void)
{
	xHTTPPostRequestMailbox = xQueueCreate(1,sizeof(HTTP_POST_Request_params));   // incoming mailbox queue for HTTP Post requests
	xHTTPPostSemaphoreHandle = xSemaphoreCreateMutex();
}

/**************************************************************
 * Function Name   : HTTPPostTask
 * Description     : This task is a wrapper around HTTP_POST_Request()
 *                 : It essentially converts it to be non-blocking.
 **************************************************************/
void HTTPPostTask(void *pvParameters)
{
	HTTP_POST_Request_params params;
	bool result = true;

	while(1)
	{
		if (xQueueReceive( xHTTPPostRequestMailbox, &params, portMAX_DELAY ) == pdPASS )
		{	// sleep task until we get a request
			if (true == NetworkInterface_IsActive())
			{ // only make a request if the TCP/IP connection is up.
				zprintf(LOW_IMPORTANCE,"HTTP_POST_Request to %s\r\n",params.URL);
				result = HTTP_POST_Request( params.URL, params.resource, params.contents, \
											params.response_buffer, params.response_size, \
											true, params.tx_key_source, params.rx_key_source, \
											params.encrypted_data, params.bytes_read);
			}
			else
			{
				result = false;	// no network
			}
			// Now send the result to the calling task
			xTaskNotify(params.xClientTaskHandle,result,eSetValueWithOverwrite);
		}
	}
}


/******* HTTP_POST_Request *******
* Self-contained function, that puts together an HTTP POST request to a URL and
* processes the response.
* CREATED: 21/02/2020, by tom monkhouse
* returns true if bytes were read, false otherwise
*******/
bool HTTP_POST_Request(char* URL, char* resource, char* contents, char* response_buffer, \
						uint32_t response_size, bool wait, DERIVED_KEY_SOURCE tx_key, \
						DERIVED_KEY_SOURCE rx_key, int32_t *encrypted_data, uint32_t *bytes_read)
{
#define SIGNATURE_OFFSET 		13	// offset into x-signature reply of the actual signature.
#define MAX_HEADER_LINE_LEN		256	// for parsing http response header lines.
#define HTTP_POST_TIMEOUT		(65 * usTICK_SECONDS)
	HTTP_CONNECTION		connection = DEFAULT_HTTP_CONNECTION;
	TickType_t			wait_time;
	HTTP_RESPONSE_DATA	response_data = DEFAULT_HTTP_RESPONSE_DATA;

	response_data.buffer = response_buffer;
	response_data.buffer_length = response_size;
	if( NULL != bytes_read) *bytes_read = 0;
	
	if( true == wait )
	{
		wait_time = portMAX_DELAY;	// caller wants to wait until this function can continue
	} else
	{
		wait_time = 0;	// caller does not want to wait
	}

	// calculate signature if required
	if( false == CalculateSignature((uint8_t*)response_data.signature, tx_key, (uint8_t*)contents, strlen(contents)) )
	{
		zprintf(CRITICAL_IMPORTANCE, "Failed to calculate signature\r\n");
	}

	if( xSemaphoreTake(xHTTPPostSemaphoreHandle, wait_time) )
	{
		http_printf("\r\n\t+++++++ Starting HTTP POST +++++++\r\n");
		http_printf(HTTP_LINE "Setting up TLS.\r\n");

		do
		{
			if( false == HTTP_Open_Connection(&connection, URL) )
			{
				break;
			}
			http_printf(HTTP_LINE "Writing Request | \r\n");
			http_printf("%s\r\n", contents);

			if( false == HTTP_Transmit_Message(connection.session, URL, resource, contents, response_data.signature, tx_key) )
			{
				break;
			}
			http_printf(HTTP_LINE "Reading Response | ");

			wolfSSL_peek(connection.session, &connection.result, 1); // Load up the buffers!

			// The entire HTTP response is held within a WolfSSL internal buffer.
			// We can both peek and read bytes from it.
			// We need to parse the header, looking for x-signature and x-update fields
			// The header finishes with \r\n\r\n
			// The header may also be just the error message "HTTP/1.1 504"
			// So we are going to parse it a line at a time, terminating each line on either \r\n or no more data
			// A line of zero length indicates the end of the header

			if( false == HTTP_Process_Header(connection.session, &response_data) )
			{
				break;
			}
			// Header has now been processed, so we can copy the body over into
			// the response buffer provided by the calling function

			HTTP_Read_Content(&connection, &response_data);
		} while( false );

		HTTP_Kill_Connection(&connection);

		if( NULL != bytes_read) *bytes_read = connection.bytes_read;	// This is used by Hub Firmware Update.
		
		if( 0 < connection.bytes_read )
		{
			http_printf(HTTP_LINE "Response Received: %s\r\n", response_buffer);// tut tut, assumes a text reply, not true for f/w images!
		} else
		{
			http_printf(HTTP_LINE "POST Failure.\r\n");
		}

		HTTP_Calculate_Signature(&connection, &response_data, rx_key);

		http_printf("\t+++++++ HTTP POST Complete +++++++\r\n");
		DbgConsole_Flush();

		*encrypted_data = response_data.encrypted_data;	// return x-enc: parameter (-1 if not present)
		
		xSemaphoreGive(xHTTPPostSemaphoreHandle);
	
		if( true == response_data.reject_message)	// this is caused by a time mismatch
		{
			zprintf(LOW_IMPORTANCE,"HTTP response rejected - time is mismatched\r\n");
			return false;			
		}	
		if( false == response_data.got_signature)
		{
			zprintf(LOW_IMPORTANCE,"Signature missing from HTTP response\r\n");
			return false;
		}
		if( false == response_data.signature_matches)
		{
			zprintf(LOW_IMPORTANCE,"Signature mismatch on HTTP response\r\n");
			return false;			
		}
		if( true == response_data.server_error) // e.g. gateway timeout or service unavailable 
		{
			zprintf(LOW_IMPORTANCE,"Server error - no HTTP response\r\n");
			return false;			
		}			
	
		if( true == response_data.got_update)	// x-update: 1
		{	// Note that we don't actually know if the Server liked our key at this point
			StoreDerivedKey(); // We assume the Server did like our key, and we store it because
				// if the server did like it, it will start using it!
			zprintf(LOW_IMPORTANCE,"Firmware update triggered via X-Update: 1\r\n");
			process_system_event(STATUS_FIRMWARE_UPDATING);	// put on red steady LEDs at top priority
			HFU_trigger(true);	// start the update
			MQTT_notify_no_creds();	// Tell the MQTT subsystem that no credentials are coming.
									// This is essential to suppress the MQTT watchdog which would
									// otherwise reset the unit!				
			return false;			
		}			
		
		// no errors, no special flags, so we got a good response!
		return (0 < connection.bytes_read);
	} else
	{	// could not take semaphore in allotted time
		return false;
	}

}

static bool HTTP_Open_Connection(HTTP_CONNECTION* connection, char* URL)
{
	extern const char* 	starfield_fixed_ca_cert;
	uint32_t rx_timeout = 10000;

	if( NULL == (connection->context = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) )
	{
		return false;
	}

	wolfSSL_CTX_set_verify(connection->context, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
	wolfSSL_CTX_UseSNI(connection->context, WOLFSSL_SNI_HOST_NAME, URL, strlen(URL));

	wolfSSL_CTX_load_verify_buffer( connection->context, (unsigned char*)starfield_fixed_ca_cert,
									strlen(starfield_fixed_ca_cert), SSL_FILETYPE_PEM );

	wolfSSL_CTX_SetIORecv(connection->context, Wrapper_Receive);
	wolfSSL_CTX_SetIOSend(connection->context, Wrapper_Send);
	if( NULL == (connection->session = wolfSSL_new(connection->context)) )
	{
		return false;
	}
	wolfSSL_check_domain_name(connection->session, URL);

	http_printf(HTTP_LINE "Setting up Socket.\r\n");
	if( FREERTOS_INVALID_SOCKET == (connection->socket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP)) )
	{
		return false;
	}

	FreeRTOS_setsockopt( connection->socket, 0, FREERTOS_SO_RCVTIMEO, &rx_timeout, sizeof(rx_timeout) );
	FreeRTOS_setsockopt( connection->socket, 0, FREERTOS_SO_SNDTIMEO, &rx_timeout, sizeof(rx_timeout) );
	wolfSSL_set_fd(connection->session, (int)connection->socket);

	http_printf(HTTP_LINE "Resolving URL: %s | ", URL);
	struct freertos_sockaddr	address;
	address.sin_addr = FreeRTOS_gethostbyname(URL);
	address.sin_port = FreeRTOS_htons(HTTP_PORT);

	if( address.sin_addr == 0 )
	{
		http_printf("Failed!\r\n");
		return false;
	}
	http_printf("Found at %08x.\r\n", address.sin_addr);
	http_printf(HTTP_LINE "Connecting Socket | ");

	connection->result = FreeRTOS_connect(connection->socket, &address, sizeof(address));
	if( pdFREERTOS_ERRNO_NONE != connection->result )
	{
		http_printf("Failed! %d\r\n", connection->result);
		return false;
	}
	http_printf("Success!\r\n");
	return true;
}

static void HTTP_Kill_Connection(HTTP_CONNECTION* connection)
{
	wolfSSL_shutdown(connection->session);
	FreeRTOS_shutdown(connection->socket, FREERTOS_SHUT_RDWR);

	wolfSSL_free(connection->session);
	wolfSSL_CTX_free(connection->context);
	FreeRTOS_closesocket(connection->socket);
}

static bool HTTP_Read_Content(HTTP_CONNECTION* connection, HTTP_RESPONSE_DATA* response_data)
{
	uint32_t	local_bytes_read;

	if( false == response_data->reject_message )
	{
		if( false != response_data->chunked )
		{	// chunked. This means we have 1 or more chunks, each chunk being a length in ascii hex,
			// followed by \r\n, then that number of bytes followed by \r\n.
			// The final chunk has a length of 0, thus it becomes 0\r\n\r\n
			uint32_t	chunk_size;
			char		chunk_size_hex[10]; // ought to be enough digits!
			uint32_t	chunk_size_hex_index;
			uint32_t	chunk_bytes_read;
			bool		end_of_response = false;

			while( (false == end_of_response) &&
				   (connection->bytes_read < response_data->buffer_length) &&
				   ((get_microseconds_tick() - response_data->activity_timestamp) < HTTP_POST_TIMEOUT) )
			{ // we repeatedly read in chunks
				chunk_size = 0;
				chunk_size_hex_index = 0;
				memset(chunk_size_hex, 0, sizeof(chunk_size_hex));
				do
				{
					local_bytes_read = wolfSSL_read(connection->session, &chunk_size_hex[chunk_size_hex_index], 1);
					if( local_bytes_read > 0 ){ chunk_size_hex_index++; }
				} while( (chunk_size_hex[chunk_size_hex_index - 1] != '\n') &&
						 (chunk_size_hex_index < (sizeof(chunk_size_hex) - 1)) &&
						 ((get_microseconds_tick() - response_data->activity_timestamp) < HTTP_POST_TIMEOUT) );

				if( 0 == local_bytes_read ) // ran out of bytes before getting the hex size.
				{
					end_of_response = true;
					return false;
				}
				chunk_size_hex[chunk_size_hex_index] = '\0'; // terminate
				chunk_size = strtoul(chunk_size_hex, NULL, 16); // this handles the stray \r
				chunk_bytes_read = 0;

				if( 0 == chunk_size )
				{
					end_of_response = true;
				} else
				{
					while( (chunk_bytes_read < chunk_size) &&
						   (connection->bytes_read < response_data->buffer_length) &&
						   ((get_microseconds_tick() - response_data->activity_timestamp) < HTTP_POST_TIMEOUT) )
					{
						local_bytes_read = wolfSSL_read(connection->session, &response_data->buffer[connection->bytes_read], chunk_size - chunk_bytes_read);
						if( local_bytes_read > 0 )
						{
							chunk_bytes_read += local_bytes_read;
							connection->bytes_read += local_bytes_read;
						}
					}
//					for(uint32_t i=0; i<connection->bytes_read; i++)
//					{
//						if( response_data->buffer[i]>31 && response_data->buffer[i]<127)
//							zprintf(LOW_IMPORTANCE,"%c",response_data->buffer[i]);
//						else
//							zprintf(LOW_IMPORTANCE,".");
//						if( (i % 128) == 127) zprintf(LOW_IMPORTANCE,"\r\n");
//						if( (i % 16) == 15) DbgConsole_Flush();
//					}
//					zprintf(LOW_IMPORTANCE,"\r\n");
				}

				chunk_size_hex[0] = '\0';
				wolfSSL_read(connection->session, chunk_size_hex, 2);	// consume the trailing \r\n
				if( chunk_size_hex[0] != '\r' ) zprintf(LOW_IMPORTANCE,"Chunk termination incorrect\r\n");
			}
		} else
		{	// Not chunked, so must have Content Length.
			while(	(connection->bytes_read < response_data->content_length) &&
					(connection->bytes_read < response_data->buffer_length) &&
					((get_microseconds_tick() - response_data->activity_timestamp) < HTTP_POST_TIMEOUT) )
			{
				local_bytes_read = wolfSSL_read(connection->session, &response_data->buffer[connection->bytes_read], response_data->buffer_length - connection->bytes_read);
				if( local_bytes_read > 0 )
				{
					connection->bytes_read += local_bytes_read;
				}
			}
//			for(uint32_t i=0; i<connection->bytes_read; i++)
//			{
//				if( response_data->buffer[i]>31 && response_data->buffer[i]<127)
//					zprintf(LOW_IMPORTANCE,"%c",response_data->buffer[i]);
//				else
//					zprintf(LOW_IMPORTANCE,".");
//				if( (i % 128) == 127) zprintf(LOW_IMPORTANCE,"\r\n");
//				if( (i % 16) == 15) DbgConsole_Flush();
//			}			
//			zprintf(LOW_IMPORTANCE,"\r\n");
		}
		http_printf(response_data->buffer);
		http_printf("%d Bytes Read.\r\n", connection->bytes_read);
		zprintf(LOW_IMPORTANCE, "Bytes read = %d\r\n", connection->bytes_read);
		//for(int i=0; i<20; i++){zprintf(LOW_IMPORTANCE," %02X",response_data->buffer[i]);}zprintf(LOW_IMPORTANCE,"\r\n");		
	}

	return true;
}

static bool HTTP_Transmit_Message(WOLFSSL* ssl, char* URL, char* resource, char* contents, char* signature, DERIVED_KEY_SOURCE tx_key)
{
	const char*	postHeader1		= "POST ";
	const char* postHeader2		= " HTTP/1.1\r\nHost: ";
	const char*	postHeader3		= "\r\nAccept: */*\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
	const char* signatureHeader	= "\r\nX-Signature: ";

	char		postLength[8]; // Padding to allow extra length characters.

	snprintf(postLength, sizeof(postLength), "%d\r\n\r\n", strlen(contents)); // In-place replacement, hence the padding above.
	uint32_t total_length = strlen(postHeader1) + strlen(resource) +
							strlen(postHeader2) + strlen(URL) +
							strlen(postHeader3) + strlen(postLength) +
							strlen(contents);
	if( DERIVED_KEY_NONE != tx_key )
	{	// add in length of signature
		total_length += strlen(signatureHeader) + SIGNATURE_LENGTH_ASCII;
	}

	zprintf(LOW_IMPORTANCE, "HTTP Post:\r\n%s%s", postHeader1, resource);
	zprintf(LOW_IMPORTANCE, "%s%s", postHeader2, URL);
	zprintf(LOW_IMPORTANCE, "%s%s", signatureHeader, signature);
	zprintf(LOW_IMPORTANCE, "%s%s%s\r\n", postHeader3, postLength, contents);

	uint32_t transmitted_length = 0;

	transmitted_length += wolfSSL_write(ssl, postHeader1, strlen(postHeader1));
	transmitted_length += wolfSSL_write(ssl, resource, strlen(resource));
	transmitted_length += wolfSSL_write(ssl, postHeader2, strlen(postHeader2));
	transmitted_length += wolfSSL_write(ssl, URL, strlen(URL));
	if( DERIVED_KEY_NONE != tx_key )
	{	// add in length of signature
		transmitted_length += wolfSSL_write(ssl, signatureHeader, strlen(signatureHeader));
		transmitted_length += wolfSSL_write(ssl, signature, strlen(signature));
	}
	transmitted_length += wolfSSL_write(ssl, postHeader3, strlen(postHeader3));
	transmitted_length += wolfSSL_write(ssl, postLength, strlen(postLength));
	transmitted_length += wolfSSL_write(ssl, contents, strlen(contents));

	if( total_length != transmitted_length )
	{
		http_printf(HTTP_LINE "Transmission Error. %d/%d bytes sent.\r\n", transmitted_length, total_length);
		return false;
	}
	http_printf(HTTP_LINE "Wrote all %d bytes.\r\n", total_length);

	return true;
}

static bool HTTP_Process_Header(WOLFSSL* ssl, HTTP_RESPONSE_DATA* response_data)
{
	const char* transfer_encoding	= "transfer-encoding: chunked";
	const char* length_header		= "content-length:";
	const char* GatewayTimeout		= "HTTP/1.1 504";
	const char* ServiceUnavailable	= "HTTP/1.1 503";
	const char* xtime_r				= "x-time:";
	const char* update				= "x-update: 1"; // response from Server requires the space
	const char* xenc_r				= "x-enc: ";	// firmware page has additional encryption
	const char* receivedSignatureHeader = "x-signature: ";

	uint8_t*	line_buffer = pvPortMalloc(MAX_HEADER_LINE_LEN);
	uint8_t		c;
	uint32_t	line_ptr;
	uint8_t		eol_detect;
	uint32_t	timestamp	= get_microseconds_tick();
	bool		end_of_header_reached = false;
	uint64_t	time_now;

	if( NULL == line_buffer )
	{
		zprintf(CRITICAL_IMPORTANCE,"Could not allocate buffer to parse HTTP Response header\r\n");
		return false;
	}

	response_data->activity_timestamp = get_microseconds_tick();

	zprintf(LOW_IMPORTANCE,"HTTP response:\r\n");
	// Parse the header line by line.
	do
	{	// parse lines until we reach the end of the header
		line_ptr = 0;
		eol_detect = 0;
		while(	(eol_detect < 2) &&
			  	(line_ptr < MAX_HEADER_LINE_LEN - 1) &&
				((get_microseconds_tick() - response_data->activity_timestamp) < HTTP_POST_TIMEOUT) )
		{	// Timeout rather than relying on no data pending, in case of data interruption.
			if( 0 < wolfSSL_read(ssl, &c, 1) )
			{
				if( c == '\r' ){ eol_detect++; }
				else if( c == '\n' ){ eol_detect++; }
				else{ line_buffer[line_ptr++] = tolower(c); } // the tolower means we don't have to care about case
			}
		}

		line_buffer[line_ptr] = '\0';	// terminate line
		zprintf(LOW_IMPORTANCE, "%s\r\n", line_buffer);

		if( (2 == eol_detect) && (0 == line_ptr) )
		{
			end_of_header_reached = true;
			break;
		}

		// got a line, now see what it is...
		if( 0 == strncmp(length_header, (const char*)line_buffer, strlen(length_header)) )
		{	// Content Length received.
			sscanf((const char*)&line_buffer[strlen(length_header)], " %d", &response_data->content_length);
			http_printf("\tExpecting Content Length: %d\r\n", response_data->content_length);
		} else if(	(0 == strcmp(GatewayTimeout,(const char*)line_buffer)) ||
					(0 == strcmp(ServiceUnavailable,(const char*)line_buffer)) )
		{
			zprintf(LOW_IMPORTANCE,"Server Error: %s\r\n",line_buffer);
			response_data->server_error = true;
			end_of_header_reached = true;	// there will be nothing more from the Server.
		} else if( 0 == strncmp(receivedSignatureHeader,(const char*)line_buffer, strlen(receivedSignatureHeader)) )
		{	// got the x-signature field
			memcpy(response_data->received_signature, &line_buffer[SIGNATURE_OFFSET], SIGNATURE_LENGTH_ASCII);	// we use memcpy to defend against malformed headers. Definitely will not copy too many bytes
			response_data->received_signature[SIGNATURE_LENGTH_ASCII] = '\0';
			response_data->got_signature = true;
		} else if( 0 == strncmp(transfer_encoding,(const char*)line_buffer,strlen(transfer_encoding)) )
		{
			response_data->chunked = true;
		} else if( 0 == strncmp(xenc_r,(const char*)line_buffer,strlen(xenc_r)) )
		{	// got the x-enc field, now read what's in it.
			sscanf((const char*)&line_buffer[strlen(xenc_r)],"%d",&response_data->encrypted_data);
			//zprintf(CRITICAL_IMPORTANCE,"Received encrypted data parameter %d\r\n",response_data->encrypted_data);
		} else if( 0 == strncmp(update,(const char*)line_buffer,strlen(update)) )
		{	// got the x-update field
			response_data->got_update = true;	// need to know this for signature calculation later on
		} else if( 0 == strncmp(xtime_r, (const char*)line_buffer, strlen(xtime_r)) )
		{	// got the x-time field
			sscanf((const char*)&line_buffer[strlen(xtime_r)], " %lld", &response_data->message_time);
			get_UTC_ms(&time_now);
			if( time_now < response_data->message_time )
			{
				time_now = response_data->message_time  - time_now;
			} else
			{
				time_now = time_now - response_data->message_time;
			}
			if( time_now > MAX_TIME_DISCREPANCY_MS )
			{
				zprintf(CRITICAL_IMPORTANCE, "Discarding stale message %llums\r\n", time_now);
				end_of_header_reached = true;
				response_data->reject_message = true;
			}
		}
	} while( (false == end_of_header_reached) && ((get_microseconds_tick() - response_data->activity_timestamp) < HTTP_POST_TIMEOUT) );
	vPortFree(line_buffer);

	return end_of_header_reached;
}

/**************************************************************
 * Function Name   : HTTP_Calculate_Signature()
 * Description     : Checks the signature of an HTTP response.
 * Inputs          : Pointers to the relevant data to check, and a reference to the key to use
 * Returns         : Sets response_data->signature_matches, and returns that value.
 **************************************************************/
static bool HTTP_Calculate_Signature(HTTP_CONNECTION* connection, HTTP_RESPONSE_DATA* response_data, DERIVED_KEY_SOURCE rx_key)
{
	response_data->signature_matches = false;

	if( (true == response_data->got_signature) && (false == response_data->reject_message) )
	{	// need to calculate the signature of the response
		// Note that the signature is calculated over the X-Time field, the Content-Length field
		// and the actual response. So we need to generate that as a single data block! A bit of a faff, easy on the Server though!
		uint8_t 	*block_to_sign;
		uint8_t		*bptr;
		char		*xupdate_s	= "X-Update:1;";	// Note this is different to the format of the incoming response!
		char		*content	= "Content-Length:";
		char		*xtime_s	= "X-Time:";
		char		*xenc_s		= "X-Enc:";
		uint32_t	bytes_to_sign;
		
		// The Server may send extra bytes (to workaround an issue with Vodafone routers).
		// Under these circumstances, we must ignore bytes_read, and use content_length instead.
		if( -1 == response_data->content_length)
		{
			bytes_to_sign = connection->bytes_read;	// content_length was not specified, so data was chunked
		}	
		else if( 0 != response_data->content_length)
		{
			bytes_to_sign = response_data->content_length;	// content_length was specified and was non-zero
		}
		else
		{
			zprintf(LOW_IMPORTANCE,"content-length = 0, response being dropped\r\n");
			return false;
		}

		uint32_t	time_len = sprintf(NULL,"%llu", response_data->message_time);
		uint32_t	content_len	= sprintf(NULL,"%d", bytes_to_sign);
		uint32_t	length_to_sign = strlen(xtime_s) + time_len + 1 + strlen(content) + content_len + 1 + bytes_to_sign;

		if( true == response_data->got_update )
		{
			length_to_sign += strlen(xupdate_s);	// add on space for x-update if it was present
		}
		
		if( -1 != response_data->encrypted_data )
		{
			length_to_sign += ( strlen(xenc_s) + 2);	// add on space for x-enc and it's parameter and the semicolon if it was present
		}
		
		block_to_sign = pvPortMalloc( length_to_sign + 1 );
		if( NULL != block_to_sign )
		{
			bptr = block_to_sign;
			if( true == response_data->got_update ){ bptr += sprintf((char *)bptr,"%s", xupdate_s); }
			bptr += sprintf((char*)bptr, "%s%llu;", xtime_s, response_data->message_time);
			if( -1 != response_data->encrypted_data ){ bptr += sprintf((char *)bptr,"%s%d;", xenc_s,response_data->encrypted_data); }
			bptr += sprintf((char*)bptr, "%s%d;", content, bytes_to_sign);
			memcpy(bptr, response_data->buffer, bytes_to_sign);
			
//			key_dump();	
//			
//			zprintf(LOW_IMPORTANCE,"Checking DERIVED_KEY_CURRENT\r\n");
//			CalculateSignature((uint8_t*)response_data->signature, DERIVED_KEY_CURRENT, (uint8_t*)block_to_sign, length_to_sign);
//			if( 0 == strcmp((const char*)response_data->signature, (const char*)response_data->received_signature) )
//			{
//				zprintf(LOW_IMPORTANCE,"Match\r\n");
//			}			
//			zprintf(LOW_IMPORTANCE,"Checking DERIVED_KEY_PENDING\r\n");
//			CalculateSignature((uint8_t*)response_data->signature, DERIVED_KEY_PENDING, (uint8_t*)block_to_sign, length_to_sign);
//			if( 0 == strcmp((const char*)response_data->signature, (const char*)response_data->received_signature) )
//			{
//				zprintf(LOW_IMPORTANCE,"Match\r\n");
//			}				
//			zprintf(LOW_IMPORTANCE,"Checking DERIVED_KEY_FLASH\r\n");
//			CalculateSignature((uint8_t*)response_data->signature, DERIVED_KEY_FLASH, (uint8_t*)block_to_sign, length_to_sign);
//			if( 0 == strcmp((const char*)response_data->signature, (const char*)response_data->received_signature) )
//			{
//				zprintf(LOW_IMPORTANCE,"Match\r\n");
//			}	
//			
			CalculateSignature((uint8_t*)response_data->signature, rx_key, (uint8_t*)block_to_sign, length_to_sign);

			vPortFree(block_to_sign);
			if( 0 == strcmp((const char*)response_data->signature, (const char*)response_data->received_signature) )
			{
				response_data->signature_matches = true;
			} else
			{
				response_data->signature_matches = false;
			}
			return response_data->signature_matches;
		} else
		{	// assume signature failure
			zprintf(CRITICAL_IMPORTANCE,"Cannot allocate memory for HTTP Response signature check\r\n");
			return false;
		}
	}

	return false;
}