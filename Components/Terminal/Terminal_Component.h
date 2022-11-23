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
#include "Components_Listener.h"
//==============================================================================
//functions:

xResult _TerminalComponentInit(void* parent);

void _TerminalComponentHandler();
void _TerminalComponentTimeSynchronization();

void _TerminalComponentRequestListener(TerminalT* terminal, TerminalRequestSelector selector, void* arg, ...);
void _TerminalComponentEventListener(TerminalT* terminal, TerminalEventSelector selector, void* arg, ...);
//==============================================================================
//override:

/**
 * @brief override main handler
 */
#define TerminalComponentInit(parent) TerminalInit(parent)
//------------------------------------------------------------------------------
/**
 * @brief override time synchronization of time-dependent processes
 */
#define TerminalComponentTimeSynchronization() TerminalTimeSynchronization()
//------------------------------------------------------------------------------
/**
 * @brief override main handler
 */
#define TerminalComponentHandler() TerminalHandler()

#define TerminalComponentEventListener(terminal, selector, arg, ...) ComponentsEventListener(terminal, selector, arg, ##__VA_ARGS__)
#define TerminalComponentRequestListener(terminal, selector, arg, ...) ComponentsRequestListener(terminal, selector, arg, ##__VA_ARGS__)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif

