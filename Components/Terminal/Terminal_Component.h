//==============================================================================
#ifndef _TERMINAL_COMPONENT_H
#define _TERMINAL_COMPONENT_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Terminal/Controls/Terminal.h"
//==============================================================================
//functions:

xResult TerminalComponentInit(void* parent);
void TerminalComponentHandler();
void TerminalComponentTimeSynchronization();
/**
 * @brief time synchronization of time-dependent processes
 */
//#define TerminalComponentTimeSynchronization() TerminalTimeSynchronization()

/**
 * @brief main handler
 */
//#define TerminalComponentHandler() TerminalHandler()
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif

