/********* Credentials - Copyright 2019 Sure Petcare - TAM 04/11/2019 *********/
#include "credentials.h"

#include "wolfssl/ssl.h"
#include "wolfssl/wolfcrypt/error-crypt.h"
#include "wolfssl/wolfcrypt/pkcs12.h"
#include "wolfssl/wolfcrypt/coding.h"

static void Credentials_Unpack(SUREFLAP_CREDENTIALS* credentials);
static bool Credentials_Check(SUREFLAP_CREDENTIALS* credentials);
static bool Credentials_LoadField(char* buffer, uint32_t* index, char* field, uint32_t max_size);

// TODO: Replace stand-in serials.
const char 	mySerial[13] 		= "H004-0088306\0";
const char	myLongSerial[33] 	= "4EE9AC68B11D226CB6E57C8103205392\0";

bool Credentials_Parse(char* buffer, SUREFLAP_CREDENTIALS* credentials)
{
	bool 		response_valid = true;
	uint32_t	progress = 0;

	response_valid = Credentials_LoadField(buffer, &progress, credentials->version, VERSION_MAX_SIZE);
	if( false != response_valid ) response_valid = Credentials_LoadField(buffer, &progress, credentials->id, ID_MAX_SIZE);
	if( false != response_valid ) response_valid = Credentials_LoadField(buffer, &progress, credentials->client_id, CLIENT_ID_MAX_SIZE);
	if( false != response_valid ) response_valid = Credentials_LoadField(buffer, &progress, credentials->username, USER_MAX_SIZE);
	if( false != response_valid ) response_valid = Credentials_LoadField(buffer, &progress, credentials->password, PASSWORD_MAX_SIZE);
	if( false != response_valid )
	{
		credentials->network_type = (SUREFLAP_NETWORK_TYPE)(buffer[progress++] - '0');
		while( buffer[progress] && buffer[progress] != ':' ){ progress++; }
		if( buffer[progress] != ':' )
		{
			response_valid = false;
		} else
		{
			progress++;
		}
	}
	if( false != response_valid ) response_valid = Credentials_LoadField(buffer, &progress, credentials->base_topic, BASE_TOPIC_MAX_SIZE);
	if( false != response_valid ) response_valid = Credentials_LoadField(buffer, &progress, credentials->host, HOST_MAX_SIZE);
	credentials->certificate = CRED_MALLOC(CERTIFICATE_MAX_SIZE);
	if( NULL == credentials->certificate ){ response_valid = false; }
	if( false != response_valid ) Credentials_LoadField(buffer, &progress, credentials->certificate, CERTIFICATE_MAX_SIZE);

	if( false != response_valid ) response_valid = Credentials_Check(credentials);

	if( false != response_valid )
	{
		cred_printf("\n\r-------- Credentials Decoded --------\n\r");
		cred_printf("\tHost:\t\t%s\n\r\tClient ID:\t%s\n\r\tBase Topic:\t%s\n\r\tVersion:\t%s\n\r\tID:\t\t%s\n\r\tUsername:\t%s\n\r\tPassword:\t%s\n\r\tNetwork Type:\t%d\n\r\tCertificate:\t%s\n\r", credentials->host, credentials->client_id, credentials->base_topic, credentials->version, credentials->id, credentials->username, credentials->password, credentials->network_type, credentials->certificate);
		Credentials_Unpack(credentials);
		cred_printf("\tLength:\t\t%d\n\r", credentials->unpacked_cert_length);
		cred_printf("\n\r-------- End Credentials --------\n\r");
	} else
	{
		cred_printf("\n\r~~~~~~~ Credentials Retrieval Failed ~~~~~~~\n\r");
	}

	return response_valid;
}

static bool Credentials_Check(SUREFLAP_CREDENTIALS* credentials)
{
	if( strlen(credentials->client_id) < AWS_CLIENT_ID_MINIMUM_LENGTH )
	{
		return false;
	}

	return true;
}

static void Credentials_Unpack(SUREFLAP_CREDENTIALS* credentials)
{
	WC_PKCS12*			pkcs = wc_PKCS12_new();
	uint32_t			length = strlen(credentials->certificate);

	credentials->unpacked_cert_length = length;
	Base64_Decode((byte*)credentials->certificate, credentials->unpacked_cert_length, (uint8_t*)credentials->certificate, (word32*)&credentials->unpacked_cert_length);

	credentials->decode_result = wc_d2i_PKCS12((const uint8_t*)credentials->certificate, credentials->unpacked_cert_length, pkcs);
	credentials->decode_result = wolfSSL_PKCS12_parse(pkcs, myLongSerial, (WOLFSSL_EVP_PKEY**)&credentials->decoded_key, (WOLFSSL_X509**)&credentials->decoded_cert, NULL);

	wc_PKCS12_free(pkcs);
	CRED_FREE(credentials->certificate);
}

static bool Credentials_LoadField(char* buffer, uint32_t* index, char* field, uint32_t max_size)
{
	uint32_t	ptr = 0;
	bool		ret = true;

	while(	buffer[*index] &&
			(buffer[*index] != ':') &&
			(ptr < max_size) )
	{
		field[ptr++] = buffer[(*index)++];
	}
	if( buffer[*index] != ':' ) ret = false;
	field[ptr] = '\0';
	(*index)++;

	return ret;
}