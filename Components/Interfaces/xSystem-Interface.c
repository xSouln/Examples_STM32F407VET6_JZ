//==============================================================================
//includes:

#include "Abstractions/xSystem/xSystem.h"
#include "Components.h"
#include "rng.h"
//==============================================================================
//types:


//==============================================================================
//variables:

#if OS_TYPE == OS_TYPE_FREERTOS
static SemaphoreHandle_t privateRNGMutex;
#endif
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
//------------------------------------------------------------------------------
uint32_t xSystemGetRandom()
{
	uint32_t result;

	xSemaphoreTake(privateRNGMutex, portMAX_DELAY);

	result = HAL_RNG_GetRandomNumber(&hrng);

	xSemaphoreGive(privateRNGMutex);

	return result;
}
//------------------------------------------------------------------------------
xResult xSystemInterfaceInit()
{
#if OS_TYPE == OS_TYPE_FREERTOS
	privateRNGMutex = xSemaphoreCreateMutex();
#endif

	return xResultAccept;
}
//==============================================================================
