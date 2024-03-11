//==============================================================================
//header:

#ifndef _COMPONENTS_PRIVATE_CONFIG_H_
#define _COMPONENTS_PRIVATE_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:


//==============================================================================
//defines:

#define ASSEMBLY_EXECUTION1_ENABLE 1
//==============================================================================
//user configurations override:

#define xUSART_USER_CONFIG 0
#define xTIMER_USER_CONFIG 0
#define xCAN_USER_CONFIG 0

#define DEVICE1_COMPONENT_USER_CONFIG 1
#define HOST_DEVICE_COMPONENT_USER_CONFIG 1
//==============================================================================
//peripherals:

#define TIMER4_ENABLE 1

#define USART1_ENABLE 1
#define USART2_ENABLE 1
#define USART3_ENABLE 1
#define USART6_ENABLE 1

#define CAN1_ENABLE 1
#define CAN2_ENABLE 1
//==============================================================================
//components:

#define USART_PORTS_COMPONENT_ENABLE 1
#if USART_PORTS_COMPONENT_ENABLE == 1

//#define USART_PORTS_COMPONENT_USER_CONFIG 1
#define SERIAL3_ENABLE 1
#define SERIAL6_ENABLE 1

#define DEBUG_SERIAL_PORT_DEFAULT_NUMBER SERIAL6

#endif
//------------------------------------------------------------------------------
#define CAN_PORTS_COMPONENT_ENABLE 1
#if CAN_PORTS_COMPONENT_ENABLE == 1

//#define CAN_PORTS_COMPONENT_USER_CONFIG 1
#define CAN_PORT1_ENABLE 1
#define CAN_PORT2_ENABLE 1

#endif
//------------------------------------------------------------------------------
#define TERMINAL_COMPONENT_ENABLE 1
#if TERMINAL_COMPONENT_ENABLE == 1

#endif
//------------------------------------------------------------------------------
#define HOST_DEVICE_COMPONENT_ENABLE 0
#if HOST_DEVICE_COMPONENT_ENABLE == 1
//------------------------------------------------------------------------------
#define HOST_REQUEST_CONTROL_COMPONENT_ENABLE 1
#define HOST_TRANSFER_LAYER_COMPONENT_ENABLE 1

#define HOST_DEVICE_ID 2000

#define HOST_DEVICE_TEMPERATURE_SERVICE1_ENABLE 1
#define HOST_DEVICE_TEMPERATURE_SERVICE2_ENABLE 1
#define HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_ENABLE 1
#define HOST_DEVICE_DIGITAL_OUTPUTS_SERVICE1_ENABLE 1
//------------------------------------------------------------------------------
enum
{
#if HOST_DEVICE_TEMPERATURE_SERVICE1_ENABLE == 1
	HOST_DEVICE_TEMPERATURE_SERVICE1_ID = 22,
#endif

#if HOST_DEVICE_TEMPERATURE_SERVICE2_ENABLE
	HOST_DEVICE_TEMPERATURE_SERVICE2_ID,
#endif

#if HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_ENABLE == 1
	HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_ID,
#endif

#if HOST_DEVICE_DIGITAL_OUTPUTS_SERVICE1_ENABLE
	HOST_DEVICE_DIGITAL_OUTPUTS_SERVICE1_ID
#endif
};
//------------------------------------------------------------------------------
#if HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_ENABLE == 1

#define HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_PORT0 PortA
#define HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_LOGIC_PIN0 1
#define HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_HARDWARE_PIN0 4
/*
#define HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_PORT1 PortA
#define HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_LOGIC_PIN1 0
#define HOST_DEVICE_DIGITAL_INPUTS_SERVICE1_HARDWARE_PIN1 3*/

#endif
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
#define DEVICE1_COMPONENT_ENABLE 1
#if DEVICE1_COMPONENT_ENABLE == 1
//------------------------------------------------------------------------------
#define DEVICE1_DIGITAL_INPUTS_SERVICE1_ENABLE 1

#define DEVICE1_DIGITAL_INPUTS_SERVICE1_PORT0 GPIOD
#define DEVICE1_DIGITAL_INPUTS_SERVICE1_LOGIC_PIN0 0
#define DEVICE1_DIGITAL_INPUTS_SERVICE1_HARDWARE_PIN0 2

#define DEVICE1_DIGITAL_INPUTS_SERVICE1_PORT1 GPIOD
#define DEVICE1_DIGITAL_INPUTS_SERVICE1_LOGIC_PIN1 1
#define DEVICE1_DIGITAL_INPUTS_SERVICE1_HARDWARE_PIN1 3
//------------------------------------------------------------------------------
#define DEVICE1_DIGITAL_OUTPUTS_SERVICE1_ENABLE 1

#define DEVICE1_DIGITAL_OUTPUTS_SERVICE1_PORT1 GPIOA
#define DEVICE1_DIGITAL_OUTPUTS_SERVICE1_HARDWARE_PIN1 7
#define DEVICE1_DIGITAL_OUTPUTS_SERVICE1_LOGIC_PIN1 0

#define DEVICE1_DIGITAL_OUTPUTS_SERVICE1_PORT2 GPIOA
#define DEVICE1_DIGITAL_OUTPUTS_SERVICE1_HARDWARE_PIN2 5
#define DEVICE1_DIGITAL_OUTPUTS_SERVICE1_LOGIC_PIN2 1
//------------------------------------------------------------------------------
#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_ENABLE 1

#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_PINS 0x01
#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_FALL_EVENT_PINS_LOGIC_LEVEL 1
#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_RISE_EVENT_PINS_LOGIC_LEVEL 1
#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_FALL_EVENT_PINS_SET_MASK 0x01
#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_RISE_EVENT_PINS_SET_MASK 0x01
#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_FALL_EVENT_PARAMETER 23
#define DEVICE1_DIGITAL_OUTPUTS_ROUTER1_RISE_EVENT_PARAMETER 10
//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
#define LED1_Port PortE
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_COMPONENTS_PRIVATE_CONFIG_H_
