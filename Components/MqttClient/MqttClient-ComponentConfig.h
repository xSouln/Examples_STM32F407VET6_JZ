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

#include "Components-Config.h"
#include "Components-Types.h"
//==============================================================================
//defines:

#define MQTT_UNDEFINED_LAYOUT 0
#define MQTT_LWIP_LAYOUT 1
#define MQTT_FREERTOS_LAYOUT 2
//------------------------------------------------------------------------------
#if !defined(MQTT_CLIENT_COMPONENT_TASK_ENABLE)
#define MQTT_CLIENT_COMPONENT_TASK_ENABLE 1
#endif
//------------------------------------------------------------------------------
#if MQTT_CLIENT_COMPONENT_TASK_ENABLE == 1

#ifndef MQTT_CLIENT_COMPONENT_TASK_STACK_SECTION
#define MQTT_CLIENT_COMPONENT_TASK_STACK_SECTION __attribute__((section("._user_heap_stack")))
#endif

#ifndef MQTT_TASK_STACK_SIZE
#define MQTT_TASK_STACK_SIZE 0x100
#endif

#ifndef MQTT_TASK_NAME
#define MQTT_TASK_NAME "mqtt task"
#endif

#ifndef MQTT_TASK_PRIORITY
#define MQTT_TASK_PRIORITY osPriorityNormal
#endif

#endif //MQTT_CLIENT_COMPONENT_TASK_ENABLE
//------------------------------------------------------------------------------
#ifndef MQTT_BROKER_IP_ADDR0
#define MQTT_BROKER_IP_ADDR0 90
#endif

#ifndef MQTT_BROKER_IP_ADDR1
#define MQTT_BROKER_IP_ADDR1 156
#endif

#ifndef MQTT_BROKER_IP_ADDR2
#define MQTT_BROKER_IP_ADDR2 229
#endif

#ifndef MQTT_BROKER_IP_ADDR3
#define MQTT_BROKER_IP_ADDR3 205
#endif

/*#ifndef MQTT_BROKER_ADDRESS
#define MQTT_BROKER_ADDRESS	"90.156.229.205"
#endif*/

#ifndef MQTT_BROKER_PORT
#define MQTT_BROKER_PORT 1883
#endif

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "bro-123456"
#endif

#ifndef MQTT_CLIENT_RX_TOPIC
#define MQTT_CLIENT_RX_TOPIC "bro-rx"
#endif

#ifndef MQTT_CLIENT_TX_TOPIC
#define MQTT_CLIENT_TX_TOPIC "bro-tx"
#endif

#ifndef MQTT_CLIENT_RX_BUFFER_SIZE
#define MQTT_CLIENT_RX_BUFFER_SIZE	256
#endif

#ifndef MQTT_CLIENT_COMPONENT_TIME_SYNCHRONIZATION_ENABLE
#define MQTT_CLIENT_COMPONENT_TIME_SYNCHRONIZATION_ENABLE 0
#endif

#ifndef MQTT_TARGET_LAYOUT
#define MQTT_TARGET_LAYOUT NET_FREERTOS_LAYOUT
#endif

#ifndef MQTT_CLIENT_PORT_TX_BUFFER_SIZE
#define MQTT_CLIENT_PORT_TX_BUFFER_SIZE 512
#endif

#ifndef MQTT_CLIENT_PORT_START_ID_OFFSET
#define MQTT_CLIENT_PORT_START_ID_OFFSET 60
#endif
//==============================================================================
//import:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_MQTT_CLIENT_COMPONENT_CONFIG_H_
