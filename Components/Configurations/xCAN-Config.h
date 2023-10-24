//==============================================================================
#ifndef _X_CAN_CONFIG_H_
#define _X_CAN_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "xTypes-Config.h"
#include "Registers/registers.h"
//==============================================================================
//defines:

#define CAN1_ENABLE 1
#define CAN2_ENABLE 1
#define CAN3_ENABLE 1
//------------------------------------------------------------------------------

typedef enum PACKED_PREFIX
{
#if rCAN1_IsDefined && CAN1_ENABLE
	xCAN1,
#endif

#if rCAN2_IsDefined && CAN2_ENABLE
	xCAN2,
#endif

#if rCAN3 && CAN3_ENABLE
	xCAN3,
#endif

#if rCAN4 && CAN4_ENABLE
	xCAN4,
#endif

	xCANsCounts

} xCAN_Numbers;
//------------------------------------------------------------------------------
#define xCAN_HandleT CAN_RegT
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_X_CAN_CONFIG_H_
