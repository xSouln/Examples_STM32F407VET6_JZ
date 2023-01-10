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

// These utilities are project wide, and can be called by any process.
// They have to be re-entrant (i.e. not use static variables)

#include "hermes.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/**************************************************************
 * Function Name   : GetParity
 * Description     : Calculate parity over a message
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
unsigned char GetParity(int8_t *ParityMessage, int16_t size)
{
  uint8_t parity=0xAA;
  uint8_t byte;

  if(size == 0x0000)
    size = strlen((char*)ParityMessage);
  size--;
  //  parity check
  do
  {
    byte = ParityMessage[size];
    parity^= byte;
  }while(size--);
  return parity;
}

/**************************************************************
 * Function Name   : CRC16
 * Description     : Calculates 16bit CRC
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint16_t CRC16(uint8_t *ptr, uint32_t count, uint16_t initCRC)
{
    uint16_t crc;
    uint8_t i;
    uint32_t j;
	
    crc = initCRC;
    for (j=0; j<count; j++)
    {
      crc = crc ^ ((uint16_t) ptr[j] << 8);
      for(i = 0; i < 8; i++)
      {
        if( crc & 0x8000 )
        {
          crc = (crc << 1) ^ 0x1021;
        }
        else
        {
          crc = crc << 1;
        }
      }
    }
    return crc;
}

/**************************************************************
 * Function Name   : CRC32
 * Description     : Calculates 32bit CRC
 * Inputs          : ptr = pointer to data, count = number of bytes, initCRC is initial value of CRC
 * Outputs         :
 * Returns         :
 **************************************************************/
uint32_t CRC32(uint8_t *ptr, uint32_t count, uint32_t initCRC)
{
    uint32_t crc;
    uint8_t i;
    uint32_t j;
	
//	zprintf(CRITICAL_IMPORTANCE,"CRC'ing block: - size %d\r\n",count);
//	for (j=0; j<count; j++)
//	{
//		zprintf(CRITICAL_IMPORTANCE,"0x%02x,",ptr[j]);
//		DbgConsole_Flush();
//	}
//	zprintf(CRITICAL_IMPORTANCE,"\r\n");
//	
    crc = initCRC;
    for (j=0; j<count; j++)
    {
      crc = crc ^ ((uint32_t) ptr[j] << 24);
      for(i = 0; i < 8; i++)
      {
        if( crc & 0x80000000 )
        {
          crc = (crc << 1) ^ 0x04C11DB7;
        }
        else
        {
          crc = crc << 1;
        }
      }
    }
    return crc;
}

