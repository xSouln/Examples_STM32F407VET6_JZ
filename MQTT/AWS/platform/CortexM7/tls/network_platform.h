/*
 * File:   network_platform.h
 * Author: tom monkhouse
 *
 * Created on 6th February 2020, 10:38
 */

#ifndef NETWORK_PLATFORM_H
#define	NETWORK_PLATFORM_H

#include "Components.h"
#include "FreeRTOS_Sockets.h"

#include "wolfssl\ssl.h"
#include "wolfssl\internal.h"

#include "credentials.h"

#define PRINT_TLS_WRAPPER	false

#if PRINT_TLS_WRAPPER
#define wrapper_printf(...)		zprintf(MEDIUM_IMPORTANCE, __VA_ARGS__)
#else
#define wrapper_printf(...)
#endif


typedef struct
{
	WOLFSSL_CTX*			ssl_ctx;
	WOLFSSL*				ssl_obj;
	SUREFLAP_CREDENTIALS*	credentials;
	bool					connected;
	Socket_t				socket;
} TLSDataParams;

int Wrapper_Receive(WOLFSSL* ssl, char* buf, int sz, void* ctx);
int Wrapper_Send(WOLFSSL* ssl, char* buf, int sz, void* ctx);

#endif	/* NETWORK_PLATFORM_H */

