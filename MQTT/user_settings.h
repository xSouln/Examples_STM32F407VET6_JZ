/*
 * File:   user_settings.h
 * Author: tom monkhouse
 *
 * Created on 16 August 2019, 09:53
 */

#ifndef USER_SETTINGS_H
#define	USER_SETTINGS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <errno.h>
#include "hermes-time.h"
#define XTIME(t)	get_UTC()
//#define WOLF_PRINTS
//#define WOLFSSL_DEBUG_PKCS12

//#define WOLFSSL_TLS13
//#define WC_RSA_PSS
//#define HAVE_TLS_EXTENSIONS		1

#define FREERTOS
#define FREERTOS_TCP
#define FREESCALE_KSDK_FREERTOS

#define NO_FILESYSTEM
//#define SINGLE_THREADED
#define WOLFSSL_USER_IO
#define HAVE_ERRNO_H
#define FREESCALE_KSDK_1_3
#define FREESCALE_COMMON

#define WOLFSSL_SMALL_STACK
//#define	WOLFSSL_SHA3_SMALL
//#define WOLFSSL_MAX_SUITE_SZ	8
//#define WOLFSSL_MAX_SIGALGO		4
//#define OPENSSL_EXTRA
#define WOLFSSL_STATIC_RSA
#define WC_RSA_BLINDING
#define USE_SLOW_SHA2
#define USE_SLOW_SHA256
#define WC_RSA_NO_PADDING
#define WOLFSSL_TRUST_PEER_CERT
#define WOLFSSL_BASE64_ENCODE

#define WOLFSSL_HAVE_SP_RSA
#define WOLFSSL_SP
#define WOLFSSL_SP_SMALL
#define WOLFSSL_SP_MATH
#define WOLFSSL_SP_NO_3072
#define TFM_MIPS

#define WOLFSSL_AES_NO_UNROLL

// Ommissions:
//#define NO_RSA
#define NO_WOLFSSL_SERVER
#define NO_ERROR_STRINGS
#define NO_DES3
#define NO_DSA
#define NO_MD4
#define NO_DH
#define NO_OLD_TLS
#define NO_SHA
#define NO_RC4
#define NO_RABBIT
#define NO_HC128
#define NO_SESSION_CACHE
#define NO_AES_128
#define NO_AES_192
//#define NO_AES_256
//#define NO_64BIT
#define NO_MD5
#define NO_PSK
#define WC_NO_CACHE_RESISTANT

// Inclusions:
#define HAVE_HKDF
//#define HAVE_AESGCM
//#define HAVE_AEAD
#define HAVE_TLS_EXTENSIONS
#define HAVE_SNI				1
#define HAVE_CHACHA
#define HAVE_PBKDF2

//#define HAVE_ECC
//#define HAVE_CURVE25519
//#define NO_ECC_SECP
//#define HAVE_ALL_CURVES
//#define HAVE_SUPPORTED_CURVES
//#define WOLFSSL_NO_SIGALG

#define USER_TIME	// to allow us to redirect gmtime() to a thread-safe form. Changes made to wc_port.h
#define HAVE_TM_TYPE
#define HAVE_TIME_T_TYPE
#define XGMTIME(c, t)   hermes_gmtime((c))

#define TRNG0	TRNG // Bodge for slightly different expectations re: peripheral naming.

#ifdef	__cplusplus
}
#endif

#endif	/* USER_SETTINGS_H */

