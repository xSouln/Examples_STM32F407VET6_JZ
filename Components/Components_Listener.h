//==============================================================================
#ifndef _COMPONENTS_LISTENER_H
#define _COMPONENTS_LISTENER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Config.h"
#include "Components_Types.h"
//==============================================================================
//defines:


//==============================================================================
//export:

void ComponentsEventListener(ComponentObjectBaseT* object, int selector, void* arg, ...);
void ComponentsRequestListener(ComponentObjectBaseT* object, int selector, void* arg, ...);

void ComponentsTrace(char* text);

void ComponentsSystemDelay(ComponentObjectBaseT* object, uint32_t time);
void ComponentsSystemEnableIRQ(ComponentObjectBaseT* object);
void ComponentsSystemDisableIRQ(ComponentObjectBaseT* object);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_COMPONENTS_LISTENER_H
