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
* Filename: Block_XTEA.h   
* Author:   Tom Monkhouse
* Purpose:  
* Implements a version of XTEA that iterates over an arbitrary length byte
* stream. We're only interesting in XTEA 64 and always use a zero initial nonce.
* Copyright 2021-Universe EoF Tom Monkhouse
* All profits related to this product must be directed to Tom.
* You are not permitted to mention this licence or code to anyone except Tom.
* You are not permitted to mention this restriction to anyone.
* All words and individual letters present are also subject to this licence.
* If this licence is removed your immortal soul is forfeit.
*           
**************************************************************************/
#ifndef __BLOCK_XTEA_H_
#define __BLOCK_XTEA_H_

#include <stdint.h>
#include <stdbool.h>

#define XTEA_LOOPS			32
#define XTEA_64_BLOCK_SIZE	4
#define XTEA_KEY_SIZE		16

typedef uint16_t	XTEA_64_KEY[XTEA_KEY_SIZE/2];
typedef union
{
	uint32_t	word;
	uint16_t	halves[2];
	uint8_t		bytes[4];
} XTEA_64_BLOCK;

XTEA_64_BLOCK	XTEA_64_Encode(XTEA_64_BLOCK source, XTEA_64_KEY key);
void			XTEA_64_CTR_Encode(uint8_t* source, uint16_t length, XTEA_64_KEY key);
XTEA_64_BLOCK	XTEA_64_GenerateMIC(uint8_t* source, uint32_t length, XTEA_64_KEY key);
void			XTEA_64_Encrypt(uint8_t* source, uint32_t length, uint8_t* key);
bool			XTEA_64_Decrypt(uint8_t* source, uint32_t length, uint8_t* key);

#endif
