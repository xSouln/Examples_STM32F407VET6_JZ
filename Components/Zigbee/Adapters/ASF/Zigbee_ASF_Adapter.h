//==============================================================================
//module enable:

#include "Zigbee/Adapters/Zigbee_AdapterConfig.h"
#ifdef ZIGBEE_ASF_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef ZIGBEE_ASF_ADAPTER_H
#define ZIGBEE_ASF_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Zigbee/Controls/Zigbee_Types.h"
#include "Common/xRxReceiver.h"
#include "mac_api.h"
#include "common_sw_timer.h"
//==============================================================================
//types:

typedef struct
{
	struct
	{
		uint32_t IRQ_Event : 1;
	};

} Zigbee_ASF_AdapterValuesT;
//------------------------------------------------------------------------------

typedef struct
{
	Zigbee_ASF_AdapterValuesT Values;

} Zigbee_ASF_AdapterT;
//==============================================================================
//functions:

xResult Zigbee_ASF_AdapterInit(ZigbeeT* network, Zigbee_ASF_AdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //ZIGBEE_ASF_ADAPTER_H
#endif //ZIGBEE_ASF_ADAPTER_ENABLE
