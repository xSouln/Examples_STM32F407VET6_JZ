//==============================================================================
#ifndef _SERIAL_PORT_ADAPTER_BASE_H_
#define _SERIAL_PORT_ADAPTER_BASE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
//==============================================================================
//types:

typedef struct
{
	ObjectBaseT Object;

	void* Child;

	void* Register;

} SerialPortAdapterBaseT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SERIAL_PORT_ADAPTER_BASE_H_
