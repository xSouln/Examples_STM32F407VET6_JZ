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
* Filename: Fault.c   
* Author:   Chris Cowdery
* Purpose:  20/08/2019   
*   
*             
**************************************************************************/

#include "FreeRTOS.h"
#include "board.h"
#include "hermes-time.h"
#include "fault.h"
#include "task.h"

#pragma default_function_attributes = @ "STARTUP"

// Note we assume that the UART is set up, but we crudely change
// the settings to suit a more basic output.

void uart_puts(char *str)
{
    uint8_t c;
    
    while( c = *str++ )
        uart_putc(c);
}

void uart_putc(char c)
{
    LPUART1->DATA = (uint16_t)c;
    while((LPUART1->STAT & (1<<22))==0);    // wait for transmitter to become idle
}

void uart_newl(void)
{
  uart_puts("\r\n");
}

void uart_putbit(uint8_t b)
{
  if (b==0) uart_putc('0'); else uart_putc('1');
}

void uart_putnibble(uint8_t b)
{
  uart_putc((b>9)?(b+'7'):(b+'0'));
}

void uart_putbyte(uint8_t b)
{
  uart_putnibble((b>>4)&0xf);
  uart_putnibble(b&0xf);
}

void uart_putuint16(uint16_t w)
{
  uart_putbyte((w>>8)&0xff);
  uart_putbyte(w&0xff);
}

void uart_putuint32(uint32_t l)
{
  uart_putuint16((l>>16)&0xffff);
  uart_putuint16(l&0xffff);
}

void debug_byte(char*s,uint8_t b)
{
  uart_puts(s);
  uart_putbyte(b);
  uart_newl();
}

void debug_uint16(char*s,uint16_t i)
{
  uart_puts(s);
  uart_putuint16(i);
  uart_newl();
}

void debug_uint32(char*s,uint32_t l)
{
  uart_puts(s);
  uart_putuint32(l);
  uart_newl();
}

void debug_dec(char*s,uint32_t l)
{
  uart_puts(s);
  uart_putdec(l,false);
  uart_newl();
}

/**************************************************************
 * Function Name   : uart_putdec
 * Description     : Print 24-bit binary number UART BCD using 'double-dabble' algorithm
 * Inputs          : 24bit number, leading zeroes flag. True prints leading zeroes, false omits them
 * Outputs         :
 * Returns         :
 **************************************************************/
void uart_putdec(uint32_t calc_bin, bool leading_zeroes)
{
  uint32_t calc_bcd; // did try to do this with UINT64, but the compiler seems buggy
  uint32_t mask=0;
  uint32_t add;
  uint8_t nibble;
  uint8_t i,j;

  if (calc_bin>0x00ffffff)
  {
    uart_puts("Number too big\r\n");
    return;
  }
  calc_bin = calc_bin << 8;
  calc_bcd = 0;
  for (j=0; j<24; j++)
  {
    // first thing is to scan all the BCD digits, and if they are >4, increment by 3
    mask = 0xf;
    add = 3;
    for (i=0; i<8; i++)
    {
      if (((calc_bcd & mask)>>(i*4))>4) calc_bcd = calc_bcd + add;
      mask = mask << 4;
      add = add << 4; // note that using syntax like add = 3<<shift doesn't work for shifts > 16......!
    }
    calc_bcd = calc_bcd << 1;
    if ((calc_bin & 0x80000000) != 0) calc_bcd++;   // carry between our 32bit numbers
    calc_bin = calc_bin << 1;
  }
  if (leading_zeroes==false)
    uart_putuint32(calc_bcd);
  else  // leading_zeroes = FALSE
  {
    mask = 0xf0000000;
    for (i=0; i<8; i++)
    {
      if (i==7) leading_zeroes=true; // need to display last zero
      nibble = (calc_bcd & mask) >> ((7-i)*4);
      if ((nibble!=0) || (leading_zeroes==true))
      {
        uart_putnibble(nibble);
        leading_zeroes=true;
      }
      mask = mask >> 4;
    }
  }
}

extern uint32_t pxCurrentTCB;	// would be nice to import the typedef struct
// but it's defined inside a .c file so we can't access it. Even doing this
// abuses the linkers lack of type checking.
BaseType_t sanity_check_heap(char tag);

void hardfault_handler(uint32_t *pulFaultStackAddress)
{
/* When a fault occurs, the processor acts as if an interrupt has occurred.
* This means that the processor stacks PSR,PC,LR,R12,R3,R2,R1,R0 in that
* order which means (because the stack is descending), we get the following
* in memory:
* R0,R1,R2,R3,R12,LR,PC,PSR where R0 is at the lowest address and PSR the highest
* and the stack pointer points to R0.
* The Hardfault handler starts in Assembler (in startup_MIMXRT1021.s) and
* sorts out which stack pointer is in use. We get that as our first parameter.
* We can extract the stacked information, which is the registers listed
* above as they were at the time of the fault (so we know where the program
* counter was (pc) and the function it was in before the most recent branch
* (lr)). 
* The stack pointer at the time of exception was therefore 8 entries (32 bytes)
* higher in memory than it is now, so we can correct for that and
* give a memory dump centred on that region.
* The contents of the stack is entirely dependent on the call tree that
* got to this point, so the compiler output will have to be inspected to determine
* what the call tree is.
* Note that function addresses usually start with 0x60 (which is our flash)
* so they can be identified. Of course, those addresses will also occur in
* registers which may get stacked so it can be confusing establishing the actual
* call tree.
* Note that GCC includes the Frame Pointer in the stack frame, so it is easy
* to unwind the stack. IAR doesn't support this feature (yet!!), but it has
* been requested (SR-722).
*/    
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr; /* Link register. */
    volatile uint32_t pc; /* Program counter. */
    volatile uint32_t psr;/* Program status register. */
    volatile uint32_t task_sp;

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ]; 
    task_sp = (uint32_t)&pulFaultStackAddress[8];
    
    uart_puts("\r\nHARDFAULT\r\n");
//#ifdef DEBUG
	uart_puts("---------\r\nProcessor registers at time of fault:\r\n");
    debug_uint32("r0  = 0x",r0);
    debug_uint32("r1  = 0x",r1);
    debug_uint32("r2  = 0x",r2);
    debug_uint32("r3  = 0x",r3);
    debug_uint32("r12 = 0x",r12);
    debug_uint32("lr  = 0x",lr);
    debug_uint32("pc  = 0x",pc);
    debug_uint32("psr = 0x",psr);
    debug_uint32("task sp  = 0x",task_sp);
	
	sanity_check_heap('F');
	
	uart_puts("Current task : ");
	uart_puts((char *)(pxCurrentTCB+0x34));
//	debug_uint32("\r\nStart of stack = 0x",*(uint32_t *)(pxCurrentTCB+0x30));
    uart_puts("\r\nStack: (note the stack grows downwards in memory, i.e. towards 0x00000000)\r\nSo the last register stacked is at the address pointed to by the task sp\r\n");
    mem_dump_fault((uint8_t *)(task_sp),128);
//#endif
    while(1);
}

/**************************************************************
 * Function Name   : vApplicationStackOverflowHook
 * Description     : Called if a task stack overflows when configCHECK_FOR_STACK_OVERFLOW is 1
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	uart_puts("STACK OVERFLOW - faulty task = ");
	uart_puts(pcTaskName);
	uart_puts("\r\nHANGING UP...");
	while(1);
}

/**************************************************************
 * Function Name   : mem_dump_fault
 * Description     : Dump contents of mem in a nice readable format
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
void mem_dump_fault(uint8_t *addr, uint8_t lines)
{
#define LINELEN 16
  uint8_t i,j,c;
  uart_newl();
  for (i=0; i<lines; i++) // should be i<16 for entire eeprom contents
  {
    uart_putuint32((uint32_t)addr+(i*LINELEN));
    uart_putc(':');
    uart_putc(' ');
    for (j=0; j<LINELEN; j++)
    {
      uart_putbyte(*(addr+(i*LINELEN)+j));
      uart_putc(' ');
    }
    for (j=0; j<LINELEN; j++)
    {
        c = *(addr+(i*LINELEN)+j);
        if ((c>31) && (c<127))
            uart_putc(c);
        else
            uart_putc('.');
    }
    uart_newl();
  }
}
