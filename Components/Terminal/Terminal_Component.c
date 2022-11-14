//==============================================================================
#include "Terminal_Component.h"
//==============================================================================
/**
 * @brief main handler
 */
inline void TerminalComponentHandler()
{
	TerminalHandler();
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void TerminalComponentTimeSynchronization()
{
	TerminalTimeSynchronization();
}
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return xResult
 */
xResult TerminalComponentInit(void* parent)
{
	TerminalInit(parent);

	return 0;
}
//==============================================================================
