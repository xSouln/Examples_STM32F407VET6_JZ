//==============================================================================
#ifndef _RELAY_SERVICE_H_
#define _RELAY_SERVICE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xService.h"
#include "RelayService-AdapterBase.h"
//==============================================================================
//includes:

#define RELAY_SERVICE_UID 0x5C78800

#define RELAY_SERVICE_PINS_COUNT 3
//==============================================================================
/// @defgroup xServices Relay service types
/// @brief функции предостовляемые RelayService.c
/// @{

typedef enum
{
	RelayServiceRequestIdle = xServiceBaseRequestOffset,

	RelayServiceRequestGetPinsState,
	RelayServiceRequestSetPin

} RelayServiceRequests;
//------------------------------------------------------------------------------

typedef enum
{
	RelayServiceParameterThreshold1,
	RelayServiceParameterThreshold2,
	RelayServiceParameterThreshold3,

} RelayServiceParameters;
//------------------------------------------------------------------------------

typedef struct
{
	xServiceT Base;

	RelayServiceAdapterBaseT Adapter;

	uint32_t PinsState;

	uint16_t Thresholds[RELAY_SERVICE_PINS_COUNT];
	xServiceDependenceT Dependencies[RELAY_SERVICE_PINS_COUNT];

} RelayServiceT;
//------------------------------------------------------------------------------

typedef struct
{
	xServiceInitT Base;

} RelayServiceInitT;
/// @}
//==============================================================================
/// @defgroup xServices Relay service functions
/// @brief функции предостовляемые слоем RelayService.c
/// @{

xResult RelayServiceInit(RelayServiceT* service, RelayServiceInitT* init);

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_RELAY_SERVICE_H_
