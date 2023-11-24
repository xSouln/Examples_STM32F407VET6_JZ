//==============================================================================
//header:

#ifndef _MQTT_CLIENT_COMPONENT_H_
#define _MQTT_CLIENT_COMPONENT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xNet/xNet.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult MqttClientComponentInit(void* parent);
void MqttClientComponentHandler();
//==============================================================================
//import:


//==============================================================================
//override:

//#define MqttClientComponentHandler() //TCPServerHandler(&TCPServerWIZspi)

#define MqttClientComponentTimeSynchronization() //TCPServerTimeSynchronization(&TCPServerWIZspi)
//==============================================================================
//export:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_CLIENT_COMPONENT_H_
