//==============================================================================
//header:

#ifndef _USART_PORT_COMPONENT_H_
#define _USART_PORT_COMPONENT_H_
//==============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Abstractions/xPort/xPort.h"
#include "UsartPort-ComponentConfig.h"
//==============================================================================
//defines:


//==============================================================================
//macros:


//==============================================================================
//functions:

xResult UsartPortComponentInit(void* parent);

void UsartPortComponentHandler();
void UsartPortComponentTimeSynchronization();
void UsartPortComponentIRQ();
//==============================================================================
//import:


//==============================================================================
//export:

extern xPortT SerialPorts[SERIAL_PORTS_COUNT];

#define SerialPort SerialPorts[SERIAL1]
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_USART_PORT_COMPONENT_H_
