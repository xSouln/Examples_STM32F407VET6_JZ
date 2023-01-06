//==============================================================================
#ifndef _SUREFLAP_TCP_ADAPTER_BASE_H_
#define _SUREFLAP_TCP_ADAPTER_BASE_H_
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
