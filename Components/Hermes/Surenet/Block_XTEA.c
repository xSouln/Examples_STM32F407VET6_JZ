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
* Filename: Block_XTEA.c   
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

#include "Components.h"
#include "Block_XTEA.h"

//	Fundamtental encode function. Operates on a single word (BLOCK).
//	Note that we only use the first 4 16-bit words of key, but SureNet provides
//	16 bytes - the latter half of those keys are wasted.
XTEA_64_BLOCK XTEA_64_Encode(XTEA_64_BLOCK source, XTEA_64_KEY key)
{
	uint32_t	i;
	uint16_t	sum = 0, delta = 0x9E37;

	for( i = 0; i < XTEA_LOOPS; i++ )
	{
		source.halves[0] += (((source.halves[1] << 4) ^ (source.halves[1] >> 5)) + source.halves[1]) ^ (sum + key[sum & 3]);
		sum += delta;
		source.halves[1] += (((source.halves[0] << 4) ^ (source.halves[0] >> 5)) + source.halves[0]) ^ (sum + key[(sum >> 11) & 3]);
	}

	return source;
}

// Counter-mode encoding. Applies the fundamental encode function, which operates
// on a single word, over a byte array of arbitrary length. This function is
// symmetric - if you apply it twice with the same key, you get your original
// data back.
void XTEA_64_CTR_Encode(uint8_t* source, uint16_t length, XTEA_64_KEY key)
{
	uint32_t		i, j, position, block = 1 + (length / XTEA_64_BLOCK_SIZE);
	XTEA_64_BLOCK	nonce	= {0};
	XTEA_64_BLOCK	crypted	= {0};

	for( i = 0; i < block; i++ )
	{
		nonce.word = crypted.word;	// This is a left-over from a previous implementation. Probably it's supposed to be zero, but we're stuck with it.
		nonce.bytes[XTEA_64_BLOCK_SIZE - 1] = i;	// Counter-mode increment ensures we don't apply the same XOR to identical words.
		crypted = XTEA_64_Encode(nonce, key);	// Actual encode occurs on the nonce rather than the data.

		for( j = 0; j < XTEA_64_BLOCK_SIZE; j++ )
		{
			position = (i * XTEA_64_BLOCK_SIZE) + j;
			if( position >= length ){ break; }
			source[position] ^= crypted.bytes[j];	// Then the encoded nonce scrambles the text. This allows symmetry.
		}
	}
}

// Generates a Message Integrity Code (fancy checksum) for a given message and key.
// This iteratively calls the fundamental encode function, so that any small change
// in data causes a big change in the MIC.
XTEA_64_BLOCK XTEA_64_GenerateMIC(uint8_t* source, uint32_t length, XTEA_64_KEY key)
{
	XTEA_64_BLOCK	MIC = {0};
	uint32_t		i, j, position, block = 1 + (length / XTEA_64_BLOCK_SIZE);

	for( i = 0; i < block; i++ )
	{
		for( j = 0; j < XTEA_64_BLOCK_SIZE; j++ )
		{
			position = (i * XTEA_64_BLOCK_SIZE) + j;
			if( position >= length ){ break; }
			MIC.bytes[j] ^= source[position];
		}
		MIC = XTEA_64_Encode(MIC, key);
	}

	return MIC;
}

// Main external encrypt function. First, appends an MIC of the text onto the array,
// then encrypts the whole thing. Modifies the source directly.
// N.B. Source must have space for 4 bytes on the back of the message to allow
// for the MIC.
void XTEA_64_Encrypt(uint8_t* source, uint32_t length, uint8_t* key)
{
	uint32_t		i;
	XTEA_64_BLOCK	MIC = XTEA_64_GenerateMIC(source, length, (uint16_t*)key);

	for( i = 0; i < XTEA_64_BLOCK_SIZE; i++ )
	{
		source[length + i] = MIC.bytes[i];
	}

	XTEA_64_CTR_Encode(source, length + XTEA_64_BLOCK_SIZE, (uint16_t*)key);
}

// Main external decrypt function. Decrypts the whole thing, which reveals the
// 4-byte MIC on the tail end. The message itself is processed to check the MIC -
// return value indicates the integrity of the message. Modifies the source directly.
// Received message is 4 bytes shorter than the provided array.
bool XTEA_64_Decrypt(uint8_t* source, uint32_t length, uint8_t* key)
{
	uint32_t		i;
	XTEA_64_BLOCK	received_MIC, calculated_MIC;

	XTEA_64_CTR_Encode(source, length, (uint16_t*)key);

	memcpy(received_MIC.bytes, &source[length - XTEA_64_BLOCK_SIZE], XTEA_64_BLOCK_SIZE);
	calculated_MIC = XTEA_64_GenerateMIC(source, length - XTEA_64_BLOCK_SIZE, (uint16_t*)key);

	if( received_MIC.word != calculated_MIC.word ){ return false; }
	return true;
}
