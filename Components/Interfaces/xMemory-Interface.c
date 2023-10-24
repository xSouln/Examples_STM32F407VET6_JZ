//==============================================================================
//includes:

#include "Common/xMemory.h"
//==============================================================================
//variables:

//SemaphoreHandle_t xMemoryMutex;
//==============================================================================
//functions:

void* xMemoryAllocate(int count, int type_size)
{
	uint32_t size = count * type_size;

	void* result = pvPortMalloc(size);

	while (!result)
	{

	}

	memset(result, 0, size);

	return result;
}
//------------------------------------------------------------------------------
void xMemoryFree(void* memory)
{
	vPortFree(memory);
}
//------------------------------------------------------------------------------
/*void xMemoryLock()
{
	xSemaphoreTake(xMemoryMutex, portMAX_DELAY);
}
//------------------------------------------------------------------------------
void xMemoryUnLock()
{
	xSemaphoreGive(xMemoryMutex);
}
//------------------------------------------------------------------------------
void xMemoryInit()
{
	xMemoryMutex = xSemaphoreCreateMutex();
}*/
//==============================================================================
