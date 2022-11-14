//==============================================================================
#include "xMemory.h"
//==============================================================================
int xMemoryCopy(void* destiny, void* data, uint32_t size)
{
	for (uint32_t i = 0; i < size; i++)
	{
		((uint8_t*)destiny)[i] = ((uint8_t*)data)[i];
	}
	
	return size;
}
//------------------------------------------------------------------------------
int xMemorySet(void* destiny, uint8_t value, uint32_t size)
{
	for (uint32_t i = 0; i < size; i++)
	{
		((uint8_t*)destiny)[i] = value;
	}
	
	return size;
}
//------------------------------------------------------------------------------
int xMemoryCompare(void* data1, void* data2, uint32_t size)
{
	for (uint32_t i = 0; i < size; i++)
	{
		if (((uint8_t*)data1)[i] != ((uint8_t*)data2)[i])
		{
			return -1;
		}
	}
	
	return 0;
}
//------------------------------------------------------------------------------
int xMemoryCheckLincs(void* links, uint32_t size)
{
	for (uint32_t i = 0; i < size /sizeof(uint32_t); i++)
	{
		if (((uint32_t*)links)[i] == 0)
		{
			return -1;
		}
	}
	
	return 0;
}
//==============================================================================
