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
* Filename: Signing.h   
* Author:   Chris Cowdery 02/06/2021
* Purpose:  Signing of communications between Server and Hub
*             
**************************************************************************/

#ifndef __SIGNING_H__
#define __SIGNING_H__

#include "Components.h"

#define SHARED_SECRET_LENGTH	16
#define DERIVED_KEY_LENGTH		32	// 256bits
#define	SIGNATURE_LENGTH		32	// 256bits, because of SHA256
#define SIGNATURE_LENGTH_ASCII	64	// signature in text form
#define PBKDF2_ITERATIONS		1000	
#define	MAX_TIME_DISCREPANCY_MS	15000	// Maximum amount of time between now and a server message
										// being sent before it's rejected (stops replay attacks)
typedef enum
{
	DERIVED_KEY_NONE,		// Don't use a key
	DERIVED_KEY_CURRENT,	// use the current Derived Key
	DERIVED_KEY_PENDING,	// Use the pending Derived Key
	DERIVED_KEY_FLASH,		// Use the Derived Key stored in Flash (which is the key in use before a power cycle)
} DERIVED_KEY_SOURCE;

typedef enum
{
	SHARED_SECRET_NONE,	// not used
	SHARED_SECRET_CURRENT,	// use the current shared secret
	SHARED_SECRET_PENDING,	// use the pending shared secret
} SHARED_SECRET_SOURCE;

uint8_t *GenerateSharedSecret(SHARED_SECRET_SOURCE src);	// Generates a new Shared Secret. Returns a pointer to it
uint8_t *GenerateDerivedKey(DERIVED_KEY_SOURCE src);		// Generates a new Derived Key. Returns a pointer to it.
uint8_t *GetSharedSecret(void);			// returns a pointer to the Shared Secret
uint8_t *GetPendingSharedSecret(void);	// returns a pointer to the Pending Shared Secret
uint8_t *GetDerivedKey(void);			// returns a pointer to the Derived Key
char *GetDerivedKey_text(void);			// returns a pointer to a hex string form of the Derived Key
void StoreDerivedKey(void);				// Store DerivedKey in Flash (in product_configuration)
bool checkFlashDerivedKey(void);		// see if the DerivedKey stored in Flash is present
bool CalculateSignature(uint8_t *sig, DERIVED_KEY_SOURCE src, uint8_t *buf, uint32_t buflen);	// Calculate digital signature
void key_dump(void);					// Dump the keys to the terminal
void key_copy_pending_to_current(void);	// Copy the pending secret and derived key to current and Flash
void send_shared_secret(void);					// Share the shared secret with the Server
bool check_recd_mqtt_signature( char *message,char *subtopic);	// Check an incoming MQTT message is correctly signed

#endif



