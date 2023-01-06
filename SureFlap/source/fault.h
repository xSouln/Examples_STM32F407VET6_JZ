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
* Filename: Fault.h   
* Author:   Chris Cowdery
* Purpose:  20/08/2019   
*   
*             
**************************************************************************/
#ifndef __FAULT_H__
#define __FAULT_H__

void uart_putc(char c);
void uart_puts(char *str);
void uart_putbyte(uint8_t b);
void uart_putdec(uint32_t calc_bin, bool leading_zeroes);
void uart_newl(void);
void uart_putuint16(uint16_t w);
void uart_putuint32(uint32_t l);
void mem_dump_fault(uint8_t *addr, uint8_t lines);

#endif


