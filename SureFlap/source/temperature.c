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
* Filename: temperature.c   
* Author:   Zach Cohen 05/11/2019
* Purpose:  Reads die temperature of processor.
*           
**************************************************************************/

#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_tempmon.h"

// Other includes

#include "fsl_tempmon.h"
#include "hermes-time.h"

/**************************************************************
 * Function Name   : get_temperature
 * Description     : Called from main application to print out the chip temperature
 * Inputs          :
 * Outputs         :
 * Returns         :
 **************************************************************/
uint32_t get_temperature(void)
{     
    float temperature = 0U;
    uint32_t intTemperature;
    tempmon_config_t config;
    
    TEMPMON_GetDefaultConfig(&config);
    config.frequency = 0x03U;
    TEMPMON_Init(TEMPMON, &config);
    TEMPMON_StartMeasure(TEMPMON);    
    /* Get temperature */
    temperature = TEMPMON_GetCurrentTemperature(TEMPMON);
    intTemperature = (uint32_t) temperature;
    return intTemperature;
    
}