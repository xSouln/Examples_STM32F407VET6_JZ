//==============================================================================
#ifndef _SUREFLAP_TCP_ADAPTER_BASE_H_
#define _SUREFLAP_TCP_ADAPTER_BASE_H_
//------------------------------------------------------------------------------
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap/SureFlap_ComponentTypes.h"
//==============================================================================
//types:

typedef struct
{
	ObjectBaseT Object;

	void* Child;

	SureFlapZigbeeInterfaceT* Interface;

} SureFlapZigbeeAdapterBaseT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_TCP_ADAPTER_BASE_H_
#endif //SUREFLAP_COMPONENT_ENABLE
