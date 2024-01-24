//==============================================================================
//includes:

#include "Common/xMemory.h"
//==============================================================================
//variables:

//SemaphoreHandle_t xMemoryMutex;
//==============================================================================
//functions:

void* xMemoryAllocate(int count, int typeSize)
{
	void* result = NULL;
	uint32_t size = count * typeSize;

	if (!size)
	{
		return result;
	}

	result = pvPortMalloc(size);

	while (!result)
	{

	}

	memset(result, 0, size);

	return result;
}
//------------------------------------------------------------------------------
void xMemoryFree(void* memory)
{
	extern uint8_t ucHeap[configTOTAL_HEAP_SIZE];

	if ((uint32_t)memory >= (uint32_t)ucHeap
		&& (uint32_t)memory <= (uint32_t)(ucHeap + configTOTAL_HEAP_SIZE))
	{
		vPortFree(memory);
	}
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
