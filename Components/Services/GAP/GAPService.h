//==============================================================================
#ifndef _GAP_SERVICE_H_
#define _GAP_SERVICE_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Abstractions/xDevice/xService.h"
#include "GAPService-AdapterBase.h"
//==============================================================================
//includes:

#define GAP_SERVICE_UID 0x5C78700
//==============================================================================
/// @defgroup xServices temperature service types
/// @brief функции предостовляемые GAPService.c
/// @{

typedef enum
{
	GAPServiceRequestIdle = xServiceBaseRequestOffset,

	GAPServiceRequestGetNumberOfServices,
	GAPServiceRequestGetService

} GAPServiceRequests;
//------------------------------------------------------------------------------

typedef struct
{
	xServiceT Base;

	//GAPServiceAdapterBaseT Adapter;

} GAPServiceT;
//------------------------------------------------------------------------------

typedef struct
{
	xServiceInitT Base;

} GAPServiceInitT;
/// @}
//==============================================================================
/// @defgroup xServices temperature service functions
/// @brief функции предостовляемые слоем GAPService.c
/// @{

xResult GAPServiceInit(GAPServiceT* service, GAPServiceInitT* init);

/// @}
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_GAP_SERVICE_H_
