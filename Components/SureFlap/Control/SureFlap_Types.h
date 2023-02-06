//==============================================================================
#ifndef _SUREFLAP_TYPES_H
#define _SUREFLAP_TYPES_H
//------------------------------------------------------------------------------
#include "SureFlap_Config.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap/SureFlap_ComponentTypes.h"
#include "SureFlap_Info.h"
#include "SureFlap_Config.h"
#include "SureFlap_DeviceTypes.h"
#include "SureFlap_ZigbeeTypes.h"
#include "SureFlap_TcpTypes.h"
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
typedef void (*SureFlapReceiveDataFromZigbeeEventListenerT)(void* hub, SureFlapDeviceT* device);
typedef void (*SureFlapReceivePacketFromZigbeeEventListenerT)(void* hub, SureFlapDeviceT* device);
typedef void (*SureFlapGetTimeT)(void* hub);

typedef struct
{
	SureFlapReceivePacketFromZigbeeEventListenerT ReceivePacketFromZigbeeEventListener;
	SureFlapReceiveDataFromZigbeeEventListenerT ReceiveDataFromZigbeeEventListener;

	SureFlapGetTimeT GetTime;

} SureFlapInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	uint16_t UpdateTime;

} SureFlapOptionsT;
//------------------------------------------------------------------------------

typedef union
{
	uint32_t LastTime;
	uint8_t UptimeMinCount;

} SureFlapSynchronizationT;
//------------------------------------------------------------------------------

typedef struct _SureFlapT
{
	ObjectBaseT Object;

	SureFlapInterfaceT* Interface;

	SureFlapDeviceControlT DeviceControl;
	SureFlapZigbeeT Zigbee;
	SureFlapTcpT Tcp;

	SureFlapSynchronizationT Synchronization;

	struct
	{
		uint8_t UpdatingDevicesEveryMinute : 1;

	} DebugMode;

} SureFlapT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_TYPES_H
#endif //SUREFLAP_COMPONENT_ENABLE
