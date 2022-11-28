//==============================================================================
#ifndef X_CIRCLE_BUFFER_H
#define X_CIRCLE_BUFFER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "xTypes.h"
//==============================================================================
//defines:

#define X_CIRCLE_BUFFER_OBJECT_ID 0xC502FE33
//==============================================================================
//types:

typedef struct
{
	ObjectBaseT Object;
	
	uint8_t* Buffer;
	uint32_t TotalIndex;
	uint32_t HandlerIndex;
	uint32_t SizeMask;
	
} xCircleBufferT;
//------------------------------------------------------------------------------

uint32_t xCircleBufferAdd(xCircleBufferT* buffer, uint8_t* data, uint32_t size);
uint32_t xCircleBufferAddReverce(xCircleBufferT* buffer, uint8_t* data, uint32_t size);

uint8_t xCircleBufferGet(xCircleBufferT* buffer);
uint32_t xCircleBufferGetFreeSize(xCircleBufferT* buffer);
bool xCircleBufferIsEmpty(xCircleBufferT* buffer);

xResult xCircleBufferInit(xCircleBufferT* circle_buffer,
														void* parent,
														uint8_t* buffer,
														uint32_t buffer_size_mask);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //X_CIRCLE_BUFFER_H
