//==============================================================================
#ifndef _SUREFLAP_ZIGBEE_ADAPTER_BASE_H_
#define _SUREFLAP_ZIGBEE_ADAPTER_BASE_H_
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
	SureFlapZigbeeAdapterEventIdle,

	SureFlapZigbeeAdapterEventEXTI

} SureFlapZigbeeAdapterEventSelector;
//------------------------------------------------------------------------------

typedef struct
{
	uint64_t MAC_Address;

	//add parameters
	void* Context;

	void* Data;
	uint8_t Size;

	struct
	{
		uint8_t AckEnable : 1;
	};

} SureFlapZigbeeAdapterRequestTransmitT;
//------------------------------------------------------------------------------

typedef void (*SureFlapZigbeeHandlerT)(void* network);
typedef void (*SureFlapZigbeeEXTI_ListenerT)(void* network);
typedef void (*SureFlapZigbeeTimeSynchronizationT)(void* network);
typedef xResult (*SureFlapZigbeeStartNetworkT)(void* network);
typedef xResult (*SureFlapZigbeeTransmitT)(void* network, SureFlapZigbeeAdapterRequestTransmitT* request);
//------------------------------------------------------------------------------

typedef struct
{
	SureFlapZigbeeHandlerT Handler;

	SureFlapZigbeeEXTI_ListenerT EXTI_Listener;

	SureFlapZigbeeTimeSynchronizationT TimeSynchronization;
	SureFlapZigbeeStartNetworkT StartNetwork;

	SureFlapZigbeeTransmitT Transmit;

} SureFlapZigbeeInterfaceT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;

	void* Child;

	SureFlapZigbeeInterfaceT* Interface;

} SureFlapZigbeeAdapterBaseT;
//==============================================================================
//macros:

#define SureFlapZigbeeAdapterEXTI_Interapt(network) (network)->Adapter.Interface->EXTI_Listener(network)
#define SureFlapZigbeeAdapterHandler(network) (network)->Adapter.Interface->Handler(network)
#define SureFlapZigbeeAdapterStartNetwork(network) (network)->Adapter.Interface->StartNetwork(network)
#define SureFlapZigbeeAdapterTransmit(network, request) (network)->Adapter.Interface->Transmit(network, request)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_ZIGBEE_ADAPTER_BASE_H_
