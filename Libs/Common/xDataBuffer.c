//==============================================================================
//includes:

#include <string.h>
#include "xDataBuffer.h"
//==============================================================================
//variables:

static const ObjectDescriptionT xDataBufferObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = X_DATA_BUFFER_OBJECT_ID,
	.Type = "xDataBufferT"
};
//==============================================================================
//functions:

void xDataBufferAdd(xDataBufferT* buffer, xObject object, uint32_t object_size)
{
	if (buffer && object)
	{
		uint8_t* ptr = (uint8_t*)object;
		
		while (object_size && buffer->DataSize < buffer->Size)
		{
			buffer->Data[buffer->DataSize++] = *ptr++;
			object_size--;
		}
	}
}
//------------------------------------------------------------------------------

void xDataBufferClear(xDataBufferT* buffer)
{
	if (buffer)
	{
		buffer->DataSize = 0;
	}
}
//==============================================================================
//initialization:

xResult xDataBufferInit(xDataBufferT* buffer,
												void* parent,
												xDataBufferControlT* control,
												uint8_t* buf,
												uint16_t buf_size)
{
	if (buffer && buf && buf_size)
	{
		buffer->Object.Description = &xDataBufferObjectDescription;
		buffer->Object.Parent = parent;
		
		buffer->Data = buf;
		buffer->Size = buf_size;
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
