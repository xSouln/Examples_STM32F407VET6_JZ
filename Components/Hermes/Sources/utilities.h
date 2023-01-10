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
* Filename: utilities.c   
* Author:   Chris Cowdery 29/10/2019
* Purpose:  Miscellaneous Parity and CRC functions
*           
**************************************************************************/

#ifndef __UTILITIES_H__
#define __UTILITIES_H__


// These utilities are project wide, and can be called by any process.
// They have to be re-entrant (i.e. not use static variables)


unsigned char GetParity(int8_t *ParityMessage, int16_t size);
uint16_t CRC16(uint8_t *ptr, uint32_t count, uint16_t initCRC);
uint32_t CRC32(uint8_t *ptr, uint32_t count, uint32_t initCRC);
#endif
