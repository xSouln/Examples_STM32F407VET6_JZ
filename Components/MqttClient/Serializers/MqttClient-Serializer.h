//==============================================================================
//header:

#include "../MqttClient-ComponentConfig.h"

#if !defined(_MQTT_CLIENT_SERIALIZERS_H_) && MQTT_CLIENT_COMPONENT_ENABLE == 1
#define _MQTT_CLIENT_SERIALIZERS_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Common/xDataBuffer.h"
#include "Common/xMemoryReader.h"
#include "Abstractions/xMQTT/xMQTT.h"
//==============================================================================
//functions:

xResult MqttOpenObject(xPortT* port, uint32_t signatureType);
xResult MqttSaveObject(xPortT* port, uint32_t signatureType);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_CLIENT_SERIALIZERS_H_
