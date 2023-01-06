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
* Filename: debug.h    
* Author:   Chris Cowdery
* Purpose:  Debugging via LED toggling   
*             
*             
*
**************************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__

// setting the below to true means that the LEDs are diverted in
// hardware from the PWM pins to GPIOs so they can be accessed
// for debugging purposes. The PWM code remains intact and operational,
// but just talking to nothing.
#define USE_LEDS_FOR_DEBUG	false

// 4 bits for LEDs, bits 26-23.
// The two red LEDs are inverted, so a 0 turns them on
// The bits are [26..23] - RR,GR,GL,RL

// 				set bits,clear bits to turn on the LED.
#define LED_RED_LEFT	0x00,0x01
#define LED_GREEN_LEFT	0x02,0x00
#define LED_GREEN_RIGHT	0x04,0x00
#define LED_RED_RIGHT	0x00,0x08
#define LED_RED_BOTH	0x00,0x09
#define LED_GREEN_BOTH	0x06,0x00
#define LEDS_ON			0x06,0x09
#define LEDS_OFF		0x09,0x06

//#define LED_RED_LEFT	0x8 // 0b1000
//#define LED_GREEN_LEFT	0xb // 0b1011
//#define LED_GREEN_RIGHT	0xd // 0b1101
//#define LED_RED_RIGHT	0x1 // 0b0001
//#define	LED_RED_BOTH	0x0 // 0b0000
//#define	LED_GREEN_BOTH	0xf // 0b1111

//#define LEDS_OFF		0x9 // 0b1001
//#define LEDS_ON			0x6 // 0b0110

#define LED_DISP_RF			LED_RED_LEFT
#define LED_DISP_SPI		LED_GREEN_LEFT
#define LED_DISP_ETH		LED_RED_LEFT
#define LED_DISP_UART		LED_RED_RIGHT
#define LED_DISP_GPT		LED_GREEN_LEFT
#define LED_DISP_SYSTICK	LED_GREEN_RIGHT

//#define	LED_SET(param)	GPIO2->DR_CLEAR = 0x07800000; \
//						GPIO2->DR_SET = (param<<23);

#define LED_SET(param)	// called by the ISRs

//#define	LED_SET_2(param)	GPIO2->DR_CLEAR = 0x07800000; \
//							GPIO2->DR_SET = (param<<23);

#define	LED_SET_2(param)

static inline void LED_SET_3(uint32_t s, uint32_t c)	{GPIO2->DR_CLEAR = (c<<23); GPIO2->DR_SET = (s<<23);}

static inline void LED_CLEAR_3(uint32_t c, uint32_t s)	{GPIO2->DR_CLEAR = (c<<23); GPIO2->DR_SET = (s<<23);}

#endif
