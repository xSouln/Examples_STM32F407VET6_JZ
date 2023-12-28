//==============================================================================
//header:

#ifndef _REQUEST_CONTROL_COMPONENT_CONFIG_H_
#define _REQUEST_CONTROL_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components-Types.h"
#include "Abstractions/xDevice/xService-Types.h"
#include "CAN_Local-Types.h"
//==============================================================================
//defines:

#define REQUEST_CONTROL_BUFFER_MEM_SECTION __attribute__((section("._user_heap_stack")))
#define REQUEST_CONTROL_BUFFER_SIZE 10
#define REQUEST_CONTROL_PORT CAN_Port2
//==============================================================================
//types:

typedef struct
{
	CAN_LocalSegmentT Segment;

	void* Content;
	uint8_t ContentSize;

} CAN_LocalRequestAddEx2T;
//==============================================================================
//import:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_REQUEST_CONTROL_COMPONENT_CONFIG_H_
