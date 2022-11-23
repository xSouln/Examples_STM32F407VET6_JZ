//==============================================================================
#ifndef X_DATA_BUFFER_H
#define X_DATA_BUFFER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "xTypes.h"
//==============================================================================
//defines:

#define X_DATA_BUFFER_OBJECT_ID 0xC502FE33
//==============================================================================
//types:

typedef enum
{
	xDataBufferStateDisable,
	xDataBufferStateEnable
	
} xDataBufferState;
//------------------------------------------------------------------------------

typedef void (*xDataBufferActionSetLockState)(xDataBufferState state);
//------------------------------------------------------------------------------

typedef struct
{
	xDataBufferActionSetLockState SetLockState;
	
} xDataBufferControlT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;

	uint8_t* Data;
	uint32_t DataSize;
	uint32_t Size;
	
	xDataBufferControlT Control;
	
} xDataBufferT;
//==============================================================================
//functions:

xResult xDataBufferInit(xDataBufferT* buffer,
												void* parent,
												xDataBufferControlT* control,
												uint8_t* buf,
												uint16_t buf_size);
		
extern void xDataBufferAdd(xDataBufferT* buffer, xObject object, uint32_t object_size);
extern void xDataBufferClear(xDataBufferT* buffer);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //X_DATA_BUFFER_H
