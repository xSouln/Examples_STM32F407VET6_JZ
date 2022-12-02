//==============================================================================
#ifndef _TERMINAL_H
#define _TERMINAL_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Terminal_Types.h"
#include "Terminal/Adapters/Terminal_Adapters.h"
//==============================================================================
//types:

//==============================================================================
//functions:

xResult TerminalInit(void* parent);
void TerminalHandler();
void TerminalTimeSynchronization();

void _TerminalReceiveData(xRxT* rx, uint8_t* data, uint32_t size);
void _TerminalRequestReceiver(TerminalT* terminal, TerminalRequestSelector selector, void* arg, ...);
//==============================================================================
//macros:

#define TerminalTxBind(tx) Terminal.Tx = tx
#define TerminalTransmit(data, size) xTxTransmitData(Terminal.Tx, data, size)
//==============================================================================
//override:

#define TerminalReceiveData(rx, data, size) _TerminalReceiveData(rx, data, size)
//==============================================================================
//export:

extern TerminalT Terminal;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
