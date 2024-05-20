//==============================================================================
//header:

#ifndef _X_TYPES_CONFIG_H_
#define _X_TYPES_CONFIG_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "semphr.h"

#include "stm32f4xx_hal.h"
#include "Registers/registers.h"
#include "Common/xDevice-CommonTypes.h"
//==============================================================================
//defines:

#define OS_TYPE_NON 0
#define OS_TYPE_FREERTOS 1

#ifdef FREERTOS_CONFIG_H
#define FREERTOS_USED 1
#define OS_TYPE OS_TYPE_FREERTOS
#endif

#ifndef OS_TYPE
#define OS_TYPE OS_TYPE_NON
#endif

#define PACKED_PREFIX __packed
//==============================================================================
//types:

typedef struct PACKED_PREFIX
{
	uint16_t Id_1;
	uint16_t Id_2;

	union
	{
		struct
		{
			uint32_t Id_3;
			uint32_t Id_4;
		};

		uint64_t MAC;
	};

} UniqueDeviceID_T;
//==============================================================================
//values:

extern volatile const UniqueDeviceID_T* UniqueDeviceID;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_X_TYPES_CONFIG_H_
