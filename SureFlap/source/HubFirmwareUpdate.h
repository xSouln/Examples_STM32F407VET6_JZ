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
* Filename: HubFirmwareUpdate.h   
* Author:   Chris Cowdery 01/09/2020
* Purpose:   
*   
* This file handles updating the Hub firmware with the firmware
* coming down from the Server.
*            
**************************************************************************/

#ifndef __HUB_FIRMWARE_UPDATE_H__
#define __HUB_FIRMWARE_UPDATE_H__

#define FIRMWARE_ENCRYPTION	"&enc=1"	// "" requests unencrypted firmware
										// "&enc=1" requests AES encrypted firmware

#define REPROG_CRYPT_NOT_SPECIFIED 	-1
#define REPROG_CRYPT_XOR			0
#define REPROG_CRYPT_XOR_AES		1

void HFU_trigger(bool retry);
void HFU_init(void);
void HFU_task(void *pvParameters);

#endif




