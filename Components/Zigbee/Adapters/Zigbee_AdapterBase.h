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

DEFINITION_HANDLER_TYPE(ZigbeeAdapter);
//------------------------------------------------------------------------------

typedef struct
{
	DECLARE_HANDLER(ZigbeeAdapter);

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
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_ADAPTER_BASE_H_
