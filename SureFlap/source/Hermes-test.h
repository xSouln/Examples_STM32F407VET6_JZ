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
* Filename: Hermes-test.h
* Author:   Tony Thurgood 27/09/2019
* Purpose:
*
* Test cases are invoked by the CLI shell commands
*
**************************************************************************/

#ifndef __HERMES_TEST_H__
#define __HERMES_TEST_H__

#define PRINT_HERMES_TEST	false

#if PRINT_HERMES_TEST
#define hermes_test_printf(...)		zprintf(CRITICAL_IMPORTANCE, __VA_ARGS__)
#else
#define hermes_test_printf(...)
#endif

typedef enum
{
	GET_SERIAL_NUMBER,
	SET_SERIAL_NUMBER,
	GET_ETHERNET_MAC,
	SET_ETHERNET_MAC,
    GET_RF_MAC,
    SET_RF_MAC,
    GET_RF_PANID,
    SET_RF_PANID,
	GET_SECRET_SERIAL,
	SET_SECRET_SERIAL,
    ERASE_SECTOR,
    GET_AES_HIGH,
    SET_AES_HIGH,
    GET_AES_LOW,
    SET_AES_LOW,
// Factory test results
    SET_FT_REV_NUM,
    GET_FT_REV_NUM,
    SET_FT_STDBY_MA,
    GET_FT_STDBY_MA,
    SET_FT_GREEN_MA,
    GET_FT_GREEN_MA,
    SET_FT_RED_MA,
    GET_FT_RED_MA,
    SET_FT_PASS_RESULTS,
    GET_FT_PASS_RESULTS,
    SET_PRODUCT_STATE,
} TEST_ACTION;


void hermesTestTask(void *pvParameters);
void hermesTestRunCase(uint8_t testCase, int8_t *pcParameterString);
void getProductConfig();
void hermesTestProcessResponse();
void getAesConfig();
void getFTdata();



#endif


