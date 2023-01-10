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
* Filename: mymac.h   
* Author:   Chris Cowdery 
* Purpose:  Default MAC addresses which should NEVER actually be used.
*           The real ones should be programmed in which overwrites these.
*             
**************************************************************************/

#ifndef _MYMAC_H_
#define _MYMAC_H_

// These are default placeholder values. They will be written into Flash if the firmware
// decides that the Flash has not been initialised.

#define		myRFMAC 		0x70b3d5f9c0000801
#define		myEthMAC 		{0x00,0x11,0x22,0xCC,0x19,0x73}
#define 	mySerial		"H017-9999998\0"
#define		mySecretSerial	"15B1F087B2A1EC3E56D759B07538B3A9\0"

#endif

