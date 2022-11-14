//==============================================================================
#ifndef X_CIRCLE_BUFFER_H
#define X_CIRCLE_BUFFER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
 extern "C" {
#endif 
//==============================================================================
#include "xTypes.h"
//==============================================================================
typedef struct
{
	OBJECT_HEADER;
	
	uint8_t* Buffer;
	uint32_t TotalIndex;
	uint32_t HandlerIndex;
	uint32_t SizeMask;
	
} xCircleBufferT;
//------------------------------------------------------------------------------
uint32_t xCircleBufferAdd(xCircleBufferT* buffer, uint8_t* data, uint32_t size);
uint8_t xCircleBufferGet(xCircleBufferT* buffer);

uint32_t xCircleBufferGetFreeSize(xCircleBufferT* buffer);

xResult xCircleBufferInit(xCircleBufferT* circle_buffer,
														void* parent,
														uint8_t* buffer,
														uint32_t buffer_size_mask);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* X_CIRCLE_BUFFER_H */
