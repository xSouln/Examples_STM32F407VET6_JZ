//==============================================================================
#ifndef _SUREFLAP_TYPES_H
#define _SUREFLAP_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "SureFlap_Info.h"
#include "SureFlap_Config.h"
#include "SureFlap_DeviceTypes.h"
#include "SureFlap_ZigbeeTypes.h"
#include "SureFlap_EthernetTypes.h"
//==============================================================================
//defines:

//==============================================================================
//types:

typedef enum
{
	SureFlapEventIdle,

	SureFlapEventPairingModeHasChanged,
	SureFlapEventAssociationSuccessful

} SureFlapEventSelector;
//------------------------------------------------------------------------------

typedef enum
{
	SureFlapRequestIdle,


} SureFlapRequestSelector;
//------------------------------------------------------------------------------
DEFINITION_EVENT_LISTENER_TYPE(SureFlap, SureFlapEventSelector);
DEFINITION_REQUEST_LISTENER_TYPE(SureFlap, SureFlapRequestSelector);

typedef struct
{
	DECLARE_EVENT_LISTENER(SureFlap);
	DECLARE_REQUEST_LISTENER(SureFlap);

} SureFlapInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	uint16_t UpdateTime;

} SureFlapOptionsT;
//------------------------------------------------------------------------------

typedef union
{
	uint64_t Value;

	uint8_t Address[sizeof(uint64_t)];

} SureFlapMAC_T;
//------------------------------------------------------------------------------

typedef union
{
	uint32_t LastTime;

} SureFlapSynchronizationT;
//------------------------------------------------------------------------------

typedef struct _SureFlapT
{
	ObjectBaseT Object;

	SureFlapInterfaceT* Interface;

	SureFlapDeviceControlT DeviceControl;
	SureFlapZigbeeT Zigbee;
	SureFlapEthernetT Ethernet;

	SureFlapSynchronizationT Synchronization;
	  
} SureFlapT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_TYPES_H
