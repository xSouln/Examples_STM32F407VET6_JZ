//==============================================================================
#ifndef _ZIGBEE_ADAPTER_BASE_H_
#define _ZIGBEE_ADAPTER_BASE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
//==============================================================================
//types:

typedef enum
{
	ZigbeeAdapterEventIdle,

	ZigbeeAdapterEventEXTI

} ZigbeeAdapterEventSelector;
//------------------------------------------------------------------------------

DEFINITION_HANDLER_TYPE(ZigbeeAdapter);

typedef void (*ZigbeeAdapterEXTI_ListenerT)(void* network);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_HANDLER(ZigbeeAdapter);

	ZigbeeAdapterEXTI_ListenerT EXTI_Listener;

} ZigbeeAdapterInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;

	void* Child;

	void* Register;

	ZigbeeAdapterInterfaceT* Interface;

} ZigbeeAdapterBaseT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_ADAPTER_BASE_H_
