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

xSystemTimeT xSystemGetTime(void)
{
	return HAL_GetTick();
}
//------------------------------------------------------------------------------
void xSystemDelay(xSystemTimeT time)
{
	HAL_Delay(time);
}
//------------------------------------------------------------------------------
void xSystemEnableIRQ(void* context)
{
	__enable_irq();
}
//------------------------------------------------------------------------------
void xSystemDisableIRQ(void* context)
{
	__disable_irq();
}
//------------------------------------------------------------------------------
void xSystemReset(void* context)
{

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
