//==============================================================================
//header:

#include "MqttClient-ComponentConfig.h"
//------------------------------------------------------------------------------
#if !defined(_MQTT_CLIENT_COMPONENT_H_) && MQTT_CLIENT_COMPONENT_ENABLE == 1
#define _MQTT_CLIENT_COMPONENT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "MqttClient-ComponentConfig.h"
#include "Abstractions/xMQTT/xMQTT-Types.h"
//==============================================================================
//defines:

#if MQTT_CLIENT_COMPONENT_TASK_ENABLE == 1
#define MqttClientComponentHandler()
#endif

#if MQTT_CLIENT_COMPONENT_TIME_SYNCHRONIZATION_ENABLE == 0 && !defined(MqttClientComponentTimeSynchronization)
#define MqttClientComponentTimeSynchronization()
#endif
//==============================================================================
//functions:

xResult MqttClientComponentInit(void* parent);

#ifndef MqttClientComponentHandler
void MqttClientComponentHandler();
#endif

#ifndef MqttClientComponentTimeSynchronization
void MqttClientComponentTimeSynchronization();
#endif
//==============================================================================
//import:


//==============================================================================
//override:


//==============================================================================
//export:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_CLIENT_COMPONENT_H_
