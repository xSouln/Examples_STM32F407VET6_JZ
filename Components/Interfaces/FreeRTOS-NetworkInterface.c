/*
 * FreeRTOS-NetworkInterface.c
 *
 *  Created on: 22 дек. 2023 г.
 *      Author: Souln
 */
//==============================================================================
//includes:

#include "rng.h"
#include "FreeRTOS_IP.h"
//==============================================================================
//functions:


uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
													uint16_t usSourcePort,
													uint32_t ulDestinationAddress,
													uint16_t usDestinationPort )
{
	uint32_t result;
	HAL_RNG_GenerateRandomNumber(&hrng, &result);

	return result;
}
//------------------------------------------------------------------------------

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    switch (eNetworkEvent)
    {
        case eNetworkDown:

            break;

        case eNetworkUp:

            break;

        default:

            break;
    }
}
//------------------------------------------------------------------------------

const char* pcApplicationHostnameHook(void)
{
	return NULL;
}
//------------------------------------------------------------------------------

BaseType_t xApplicationGetRandomNumber(uint32_t * pulNumber)
{
	uint32_t result;
	HAL_RNG_GenerateRandomNumber(&hrng, &result);

	return result;
}
//------------------------------------------------------------------------------

void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier)
{

}
//==============================================================================
