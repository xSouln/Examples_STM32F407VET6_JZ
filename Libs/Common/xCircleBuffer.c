//==============================================================================
//includes:

#include "xCircleBuffer.h"
//==============================================================================
//variables:

static const ObjectDescriptionT xCircleBufferObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = X_CIRCLE_BUFFER_OBJECT_ID,
	.Type = "xCircleBufferT"
};
//==============================================================================
//functions:

uint32_t xCircleBufferAdd(xCircleBufferT* buffer, uint8_t* data, uint32_t size)
{
	for (uint16_t i = 0; i < size; i++)
	{
		buffer->Buffer[buffer->TotalIndex] = data[i];
		buffer->TotalIndex++;
		buffer->TotalIndex &= buffer->SizeMask;
	}

	return size;
}
//------------------------------------------------------------------------------

uint32_t xCircleBufferAddReverce(xCircleBufferT* buffer, uint8_t* data, uint32_t size)
{
	while(size)
	{
		size--;
		
		buffer->Buffer[buffer->TotalIndex] = data[size];
		buffer->TotalIndex++;
		buffer->TotalIndex &= buffer->SizeMask;
	}
	
	return 0;
}
//------------------------------------------------------------------------------

uint8_t xCircleBufferGet(xCircleBufferT* buffer)
{
	uint8_t value = buffer->Buffer[buffer->HandlerIndex];
	buffer->HandlerIndex++;
	buffer->HandlerIndex &= buffer->SizeMask;
	
	return value;
}
//------------------------------------------------------------------------------

bool xCircleBufferIsEmpty(xCircleBufferT* buffer)
{
	return buffer->HandlerIndex == buffer->TotalIndex;
}
//------------------------------------------------------------------------------

uint32_t xCircleBufferGetFreeSize(xCircleBufferT* buffer)
{
	return (buffer->SizeMask + 1) - ((buffer->TotalIndex - buffer->HandlerIndex) & buffer->SizeMask);
}
//------------------------------------------------------------------------------

xResult xCircleBufferInit(xCircleBufferT* circle_buffer,
														void* parent,
														uint8_t* buffer,
														uint32_t buffer_size_mask)
{
  if (circle_buffer && buffer && buffer_size_mask)
  {
		circle_buffer->Object.Description = &xCircleBufferObjectDescription;
		circle_buffer->Object.Parent = parent;
		
		circle_buffer->Buffer = buffer;
		circle_buffer->SizeMask = buffer_size_mask;
		
		return xResultError;
  }
  return xResultError;
}
//==============================================================================
