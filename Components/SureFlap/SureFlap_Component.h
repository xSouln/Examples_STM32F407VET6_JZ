//==============================================================================
//module enable:

#include "SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//header:

#ifndef _SUREFLAPE_COMPONENT_H
#define _SUREFLAPE_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "SureFlap_ComponentTypes.h"
#include "SureFlap/SureFlap_ComponentConfig.h"
//==============================================================================
//configurations:

#include "SureFlap/Control/SureFlap.h"
//==============================================================================
//defines:


//==============================================================================
//macros:


//==============================================================================
//functions:

xResult _SureFlapComponentInit(void* parent);

void _SureFlapComponentHandler();
void _SureFlapComponentTimeSynchronization();
//==============================================================================
//export:

extern SureFlapT SureFlap;
//==============================================================================
//override:

//#define SureFlapComponentInit(parent) SureFlapInit(&SureFlap, parent)
#define SureFlapComponentHandler() _SureFlapComponentHandler()//SureFlapHandler(&SureFlap)
#define SureFlapComponentTimeSynchronization() _SureFlapComponentTimeSynchronization()//SureFlapTimeSynchronization(&SureFlap)
//------------------------------------------------------------------------------
#ifndef SureFlapComponentInit
#define SureFlapComponentInit(parent) _SureFlapComponentInit(parent)
#endif
//------------------------------------------------------------------------------
#ifndef SureFlapComponentHandler
#define SureFlapComponentHandler() _SureFlapComponentHandler()
#endif
//------------------------------------------------------------------------------
#ifndef SureFlapComponentTimeSynchronization
#define SureFlapComponentTimeSynchronization() _SureFlapComponentTimeSynchronization()
#endif
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAPE_COMPONENT_H
#endif //SUREFLAP_COMPONENT_ENABLE
