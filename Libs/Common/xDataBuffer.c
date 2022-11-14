//==============================================================================
#include <string.h>
#include "xDataBuffer.h"
//==============================================================================
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
//==============================================================================
void xDataBufferClear(xDataBufferT* buffer)
{
	if (buffer)
	{
		buffer->DataSize = 0;
	}
}
//==============================================================================
xResult xDataBufferInit(xDataBufferT* buffer,
												void* parent,
												xDataBufferControlT* control,
												uint8_t* buf,
												uint16_t buf_size)
{
	if (buffer && buf && buf_size)
	{
		buffer->Description = "xDataBufferT";
		buffer->Parent = parent;
		
		buffer->Data = buf;
		buffer->Size = buf_size;
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
