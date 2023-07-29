//==============================================================================
//includes:

#include "Abstractions/xSystem/xSystem.h"
#include "Components.h"
//==============================================================================
//types:


//==============================================================================
//functions:

xSystemTimeT xSystemGetTime(void* context)
{
	return HAL_GetTick();
}
//------------------------------------------------------------------------------
void xSystemDelay(void* context, xSystemTimeT time)
{
	HAL_Delay(time);
}
//------------------------------------------------------------------------------
xResult xSystemEnableIRQ(void* context)
{
	return xResultNotSupported;
}
//------------------------------------------------------------------------------
xResult xSystemDisableIRQ(void* context)
{
	return xResultNotSupported;
}
//------------------------------------------------------------------------------
xResult xSystemReset(void* context)
{
	return xResultNotSupported;
}
//==============================================================================
