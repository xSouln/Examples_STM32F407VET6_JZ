//==============================================================================
#ifndef _COMPONENTS_CONFIG_H
#define _COMPONENTS_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

//==============================================================================
//components:

#define TERMINAL_COMPONENT_ENABLE 1
//#define SERIAL_PORT_COMPONENT_ENABLE 1
//==============================================================================
//defines:

#define BREW_GROUP_MOTOR_TIMER_PWM1 TIM9
#define BREW_GROUP_MOTOR_TIMER_PWM1_CHANNEL 0

#define BREW_GROUP_MOTOR_TIMER_PWM2 TIM9
#define BREW_GROUP_MOTOR_TIMER_PWM2_CHANNEL 1

#define BREW_GROUP_FLOW_METER_HTIMER htim8
//------------------------------------------------------------------------------

#define SLIDER_MOTOR_TIMER_PWM1 TIM4
#define SLIDER_MOTOR_TIMER_PWM1_CHANNEL 2

#define SLIDER_MOTOR_TIMER_PWM2 TIM4
#define SLIDER_MOTOR_TIMER_PWM2_CHANNEL 3
//------------------------------------------------------------------------------

#define FLOW_DIR_MOTOR_TIMER_PWM1 TIM5
#define FLOW_DIR_MOTOR_TIMER_PWM1_CHANNEL 2

#define FLOW_DIR_MOTOR_TIMER_PWM2 TIM5
#define FLOW_DIR_MOTOR_TIMER_PWM2_CHANNEL 3
//------------------------------------------------------------------------------

#define CAROUSEL_MOTOR_TIMER_PWM1 TIM4
#define CAROUSEL_MOTOR_TIMER_PWM1_CHANNEL 0

#define CAROUSEL_MOTOR_TIMER_PWM2 TIM4
#define CAROUSEL_MOTOR_TIMER_PWM2_CHANNEL 1
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif

