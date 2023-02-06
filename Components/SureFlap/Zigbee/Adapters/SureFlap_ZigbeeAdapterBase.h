//==============================================================================
#ifndef _SUREFLAP_ZIGBEE_ADAPTER_BASE_H_
#define _SUREFLAP_ZIGBEE_ADAPTER_BASE_H_
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
//basic representation of the adapter
typedef struct
{
	ObjectBaseT Object;

	//additional data of the heir
	void* Child;

	//interface for working with the adapter
	SureFlapZigbeeInterfaceT* Interface;

} SureFlapZigbeeAdapterBaseT;
//==============================================================================
//macros:

//receiving an external interrupt event from a transceiver
#define SureFlapZigbeeAdapterEXTI_Interapt(network) (network)->Adapter.Interface->EXTI_Listener(network)

//adapter handler
#define SureFlapZigbeeAdapterHandler(network) (network)->Adapter.Interface->Handler(network)

//enabling the network environment via the adapter
#define SureFlapZigbeeAdapterStartNetwork(network) (network)->Adapter.Interface->StartNetwork(network)

//data transfer via adapter
#define SureFlapZigbeeAdapterTransmit(network, request) (network)->Adapter.Interface->Transmit(network, request)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_ZIGBEE_ADAPTER_BASE_H_
#endif //SUREFLAP_COMPONENT_ENABLE
