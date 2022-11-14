//==============================================================================
#ifndef X_MEMORY_H
#define X_MEMORY_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
 extern "C" {
#endif 
//==============================================================================
#include "xTypes.h"
//==============================================================================
int xMemoryCopy(void* destiny, void* data, uint32_t size);
int xMemoryCompare(void* data1, void* data2, uint32_t size);
int xMemoryCheckLincs(void* links, uint32_t size);
int xMemorySet(void* destiny, uint8_t value, uint32_t size);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* X_MEMORY_H */
