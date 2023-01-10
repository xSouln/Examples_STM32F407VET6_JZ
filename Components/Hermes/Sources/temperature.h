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
* Filename: temperature.h   
* Author:   Zach Cohen 05/11/2019
* Purpose:  Reads die temperature of processor.
*           
**************************************************************************/

#ifndef __TEMP_H__
#define __TEMP_H__

#define DEMO_TEMPMON TEMPMON
#define DEMO_TEMP_PANIC_IRQn TEMP_PANIC_IRQn
#define DEMO_TEMP_LOW_HIGH_IRQHandler TEMP_LOW_HIGH_IRQHandler
#define DEMO_TEMP_PANIC_IRQHandler TEMP_PANIC_IRQHandler
#define DEMO_HIGHALARMTEMP 42U
#define DEMO_LOWALARMTEMP 40U


uint32_t get_temperature(void);

#endif
