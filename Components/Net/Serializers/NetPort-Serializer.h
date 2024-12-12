//==============================================================================
//header:

#include "../Net-ComponentConfig.h"

#if !defined(_NET_PORT_SERIALIZERS_H_) && NET_COMPONENT_ENABLE == 1
#define _NET_PORT_SERIALIZERS_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Common/xDataBuffer.h"
#include "Common/xMemoryReader.h"
#include "Abstractions/xNet/xNet.h"
//==============================================================================
//functions:

xResult NetOpenObject(void* object, uint32_t signatureType);
xResult NetSaveObject(void* object, uint32_t signatureType);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_NET_PORT_SERIALIZERS_H_
