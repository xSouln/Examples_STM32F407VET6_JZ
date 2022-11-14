//==============================================================================
#ifndef _COMPONENTS_H
#define _COMPONENTS_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "Components_Config.h"
//==============================================================================
//configurations:

#ifdef TERMINAL_COMPONENT_ENABLE
#include "Terminal/Terminal_Component.h"
#endif

#ifdef SERIAL_PORT_COMPONENT_ENABLE
#include "SerialPort/SerialPort_Component.h"
#endif
//==============================================================================
//functions:

int ComponentsInit(void* parent);
void ComponentsTimeSynchronization();
void ComponentsHandler();
//==============================================================================
//export:

//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif

