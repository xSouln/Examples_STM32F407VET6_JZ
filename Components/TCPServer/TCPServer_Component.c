//==============================================================================
//module enable:

#include "TCPServer_ComponentConfig.h"
#ifdef TCP_SERVER_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "TCPServer_Component.h"
//==============================================================================
/**
 * @brief main handler
 */
inline void TCPServerComponentHandler()
{
	#ifdef TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
	SerialPortUARTComponentHandler();
	#endif
}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
inline void TCPServerComponentTimeSynchronization()
{
	#ifdef TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
	SerialPortUARTComponentTimeSynchronization();
	#endif
}
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return int
 */
int TCPServerComponentInit(void* parent)
{
	#ifdef TCP_SERVER_WIZ_SPI_COMPONENT_ENABLE
	SerialPortUARTComponentInit(parent);
	#endif

	return 0;
}
//==============================================================================
#endif //TCP_SERVER_COMPONENT_ENABLE
