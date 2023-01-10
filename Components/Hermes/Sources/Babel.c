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
* Filename: Babel.c    
* Author:   Tom Monkhouse
* Purpose:     
*             
*             
*
**************************************************************************/

#include "Babel.h"
#include "hermes.h"

uint32_t 			*srcbuffer;
uint32_t 			*dstbuffer;
uint32_t 			payload[8];
const uint8_t 		key_count = 16;

static uint8_t		encryption_key[16];

bool BABEL_aes_encrypt_init(void)
{
	
	return false;
}




void BABEL_set_aes_key(uint8_t serial_number_in[])
{
	memset(encryption_key,0,sizeof(encryption_key));
	strncpy((char *)&encryption_key[2],(char const *)serial_number_in,12);	
	
	encryption_key[0] = 0x10;
	encryption_key[1] = 0xCA;
	encryption_key[14] = 0xDE;
	encryption_key[15] = 0x17;		
}
