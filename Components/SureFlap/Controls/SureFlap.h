//==============================================================================
#ifndef _SUREFLAP_H_
#define _SUREFLAP_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Types.h"
#include "SureFlap_Types.h"
#include "SureFlap_Device.h"
#include "SureFlap_Zigbee.h"
//==============================================================================
//macros:


//==============================================================================
//functions:

xResult SureFlapInit(SureFlapT* hub, void* parent, SureFlapInterfaceT* interface);

void _SureFlapHandler(SureFlapT* hub);
void _SureFlapTimeSynchronization(SureFlapT* hub);
uint32_t _SureFlapGetTime(SureFlapT* hub);

void _SureFlapEventListener(SureFlapT* hub, SureFlapEventSelector selector, void* arg, ...);
xResult _SureFlapRequestListener(SureFlapT* hub, SureFlapRequestSelector selector, void* arg, ...);
//==============================================================================
//


//==============================================================================
//override:

#define SureFlapHandler(hub) _SureFlapHandler(hub)

#define SureFlapTimeSynchronization(hub) _SureFlapTimeSynchronization(hub)
#define SureFlapGetTime(hub) _SureFlapGetTime(hub)

#define SureFlapEventListener(hub, selector, arg, ...) ((SureFlapT*)hub)->Interface->EventListener(hub, selector, arg, ##__VA_ARGS__)
#define SureFlapRequestListener(hub, selector, arg, ...) ((SureFlapT*)hub)->Interface->RequestListener(hub, selector, arg, ##__VA_ARGS__)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_H_
