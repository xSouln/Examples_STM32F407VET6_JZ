//==============================================================================
//module enable:

#include "SureFlap/Adapters/SureFlap_AdapterConfig.h"
#ifdef SUREFLAP_ZIGBEE_ASF_ADAPTER_ENABLE
//==============================================================================
//header:

#ifndef _SUREFLAP_ZIGBEE_ASF_ADAPTER_H
#define _SUREFLAP_ZIGBEE_ASF_ADAPTER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap/Control/SureFlap.h"
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

	uint32_t UpdateTimeStamp;

} SureFlapZigbee_ASF_AdapterValuesT;
//------------------------------------------------------------------------------

typedef struct
{
	SureFlapZigbee_ASF_AdapterValuesT Values;

} SureFlapZigbee_ASF_AdapterT;
//==============================================================================
//functions:

xResult SureFlapZigbee_ASF_AdapterInit(SureFlapZigbeeT* network, SureFlapZigbee_ASF_AdapterT* adapter);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //ZIGBEE_ASF_ADAPTER_H
#endif //_SUREFLAP_ZIGBEE_ASF_ADAPTER_H
