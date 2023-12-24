//==============================================================================
//header:

#ifndef _MQTT_CLIENT_COMPONENT_CONFIG_H_
#define _MQTT_CLIENT_COMPONENT_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components-Types.h"
//==============================================================================
//defines:

#define MQTT_CLIENT_COMPONENT_MAIN_TASK_STACK_SECTION __attribute__((section("._user_heap_stack")))

#define MQTT_TASK_STACK_SIZE 0x100

#define MQTT_BROKER_IP_ADDR0 90
#define MQTT_BROKER_IP_ADDR1 156
#define MQTT_BROKER_IP_ADDR2 229
#define MQTT_BROKER_IP_ADDR3 205

#define MQTT_BROKER_ADDRESS	"90.156.229.205"
#define MQTT_BROKER_PORT    1883
#define MQTT_CLIENT_ID      "bro-123456"
#define MQTT_TOPIC_RX		"bro-rx"
#define MQTT_TOPIC_TX		"bro-tx"

#define MQTT_TX_BUFFER_SIZE	250

#define MQTT_UNDEFINED_LAYOUT 0
#define MQTT_LWIP_LAYOUT 1
#define MQTT_FREERTOS_LAYOUT 2

#ifndef MQTT_TARGET_LAYOUT
#define MQTT_TARGET_LAYOUT NET_FREERTOS_LAYOUT
#endif
//==============================================================================
//import:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_CLIENT_COMPONENT_CONFIG_H_
