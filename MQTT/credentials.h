#ifndef __CREDENTIALS_H_
#define __CREDENTIALS_H_

#include "Components.h"

#include <stdint.h>
#include <stdbool.h>

#include "hermes.h"

#define AWS_CLIENT_ID_MINIMUM_LENGTH	5
#define VERSION_MAX_SIZE		8
#define ID_MAX_SIZE				16
#define CLIENT_ID_MAX_SIZE		48
#define USER_MAX_SIZE			8
#define PASSWORD_MAX_SIZE		8
#define	BASE_TOPIC_MAX_SIZE		96
#define HOST_MAX_SIZE			64
#define CERTIFICATE_MAX_SIZE	3492 // 3584 // Pared down - let's hope Ash always gives the same size certificate...
#define AWS_HASH_MAX_LENGTH		48

#define CREDS_ABSOLUTE_MAX_LENGTH	4096

#define MAC_STRING_MAX_SIZE		16

#define CRED_MALLOC(x)			malloc(x)
#define CRED_FREE(x)			free(x)

#define PRINT_CREDENTIALS		false

#if PRINT_CREDENTIALS
#define cred_printf(...)		zprintf(TERMINAL_IMPORTANCE, __VA_ARGS__)
#else
#define cred_printf(...)
#endif

typedef enum
{
	SUREFLAP_NETWORK_TYPE_XIVELY	= 0,
	SUREFLAP_NETWORK_TYPE_AWS		= 1
} SUREFLAP_NETWORK_TYPE;

typedef struct
{
    char					version[VERSION_MAX_SIZE];
    char					id[ID_MAX_SIZE];
    char					client_id[CLIENT_ID_MAX_SIZE];
    char					username[USER_MAX_SIZE];
    char					password[PASSWORD_MAX_SIZE];
	char					base_topic[BASE_TOPIC_MAX_SIZE];
	SUREFLAP_NETWORK_TYPE	network_type;
	char					host[HOST_MAX_SIZE];
	char*					certificate;
	uint32_t				unpacked_cert_length;
	void*					decoded_cert;
	void*					decoded_key;
	uint32_t				decoded_cert_size;
	uint32_t				decoded_key_size;
	int32_t					decode_result;
	char					cert_hash[AWS_HASH_MAX_LENGTH];
	char					key_hash[AWS_HASH_MAX_LENGTH];
	uint32_t				combined_hash;
} SUREFLAP_CREDENTIALS;

typedef struct
{
	uint32_t	size;
	uint8_t		data[1024 - sizeof(uint32_t)];
} STORED_CREDENTIAL;

bool Credentials_Parse(char* buffer, SUREFLAP_CREDENTIALS* credentials);

#endif
