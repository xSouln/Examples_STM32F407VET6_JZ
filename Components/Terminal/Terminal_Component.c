//==============================================================================
#include "Terminal_Component.h"
//==============================================================================

void _TerminalComponentRequestListener(TerminalT* terminal, TerminalRequestSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------

void _TerminalComponentEventListener(TerminalT* terminal, TerminalEventSelector selector, void* arg, ...)
{
	switch((int)selector)
	{
		default: return;
	}
}
//------------------------------------------------------------------------------
/**
 * @brief main handler
 */
inline void _TerminalComponentHandler()
{
	TerminalHandler();
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void _TerminalComponentTimeSynchronization()
{
	TerminalTimeSynchronization();
}
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return xResult
 */
xResult _TerminalComponentInit(void* parent)
{
	TerminalInit(parent);

	return 0;
}
//==============================================================================
