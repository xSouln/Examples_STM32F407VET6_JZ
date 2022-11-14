//==============================================================================
#ifndef X_DATA_BUFFER_H
#define X_DATA_BUFFER_H
//==============================================================================
#include "xTypes.h"
//==============================================================================
typedef enum
{
	xDataBufferStateDisable,
	xDataBufferStateEnable
	
} xDataBufferState;
//==============================================================================
typedef void (*xDataBufferActionSetLockState)(xDataBufferState state);
//==============================================================================
typedef struct
{
	xDataBufferActionSetLockState SetLockState;
	
} xDataBufferControlT;
//==============================================================================
typedef struct
{
  OBJECT_HEADER;
  
  uint8_t* Data;
	uint32_t DataSize;
  uint32_t Size;
	
	xDataBufferControlT Control;
	
} xDataBufferT;
//==============================================================================
xResult xDataBufferInit(xDataBufferT* buffer,
												void* parent,
												xDataBufferControlT* control,
												uint8_t* buf,
												uint16_t buf_size);
		
extern void xDataBufferAdd(xDataBufferT* buffer, xObject object, uint32_t object_size);
extern void xDataBufferClear(xDataBufferT* buffer);
//==============================================================================
#endif
