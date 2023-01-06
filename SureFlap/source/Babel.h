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
* Filename: Babel.h    
* Author:   Tom Monkhouse
* Purpose:     
*             
*             
*
**************************************************************************/
#include <stdint.h>
#include <stdbool.h>

bool BABEL_aes_encrypt_init();
void BABEL_set_aes_key(uint8_t serial_number_in[]);
bool BABEL_aes_encrypt_isit();

#define ENCRYPT_PAYLOAD_KEY = 		
#define ENCRYPT_CIPHER_ENCRYPT = 	
#define ENCRYPT_CIPHER_INIT =		
#define ENCRYPT_ENABLE_CIPHER =		
#define ENCRYPT_DECR_SEMAPHORE = 		
#define ENCRYPT_INTERRUPT = 		
#define ENCRYPT_CIPHER_MODE = 		






