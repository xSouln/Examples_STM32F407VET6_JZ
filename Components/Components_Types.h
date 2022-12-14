 //==============================================================================
#ifndef _COMPONENTS_TYPES_H
#define _COMPONENTS_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include <string.h>
#include "main.h"
#include "Common/xTypes.h"
#include "Registers/registers.h"
//==============================================================================
//defines:

//==============================================================================
//types:

typedef enum
{
	ComponentsSystemRequestIdle,

	ComponentsSystemRequestDelay,
	ComponentsSystemRequestDisableIRQ,
	ComponentsSystemRequestInableIRQ

} ComponentsSystemRequests;
//------------------------------------------------------------------------------

#define COMPONENT_EVENT_BASE\
	ObjectBaseT Object;\
	uint16_t Selector
//------------------------------------------------------------------------------

typedef struct
{
	uint16_t Selector;
	void* Content;

} ComponentEventBaseT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectDescriptionT* Description;
	void* Parent;

} ComponentObjectBaseT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
