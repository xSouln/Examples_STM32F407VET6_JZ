//==============================================================================
//header:

#ifndef _TRANSACTION_MANAGER_COMPONENT_H_
#define _TRANSACTION_MANAGER_COMPONENT_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components-Types.h"
#include "Abstractions/xTransferLayer/xTransferLayer.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult TransferLayerComponentInit(void* parent);

void TransferLayerComponentHandler();

#define TransferLayerComponentTimeSynchronization()
//==============================================================================
//import:


//==============================================================================
//override:


//==============================================================================
//export:

extern xTransferLayerT LocalTransferLayer;
extern xTransferLayerT ExternalTransferLayer;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_TRANSACTION_MANAGER_COMPONENT_H_
