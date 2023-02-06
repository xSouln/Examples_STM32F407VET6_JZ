//==============================================================================
#include "SureFlap_Config.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

/*
 * Function Name   : GetParity
 * Description     : Calculate parity over a message
 * Inputs          :
 * Outputs         :
 * Returns         :
*/
uint8_t SureFlapUtilitiesGetParity(int8_t *ParityMessage, int16_t size)
{
	uint8_t parity = 0xAA;
	uint8_t byte;

	if(size == 0x0000)
	{
		size = strlen((char*)ParityMessage);
	}
	size--;
	//  parity check
	do
	{
		byte = ParityMessage[size];
		parity ^= byte;
	}
	while(size--);

	return parity;
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE
