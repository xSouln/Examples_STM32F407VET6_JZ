//==============================================================================
//header:

#ifndef _CAN_LOCAL_TYPES_H_
#define _CAN_LOCAL_TYPES_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Common/xTypes.h"
//==============================================================================
//types:

typedef struct PACKED_PREFIX
{
	uint32_t Identifier : 11;
	uint32_t ExtansionIsEnabled : 1;
	uint32_t Extansion : 18;

	uint8_t DataLength;

	union
	{
		uint8_t Bytes[8];
		uint16_t HalfWords[4];
		uint32_t Words[2];
		uint64_t DoubleWord;

	} Data;

} CAN_LocalSegmentT;
//------------------------------------------------------------------------------

typedef enum
{
	CAN_LocalPacketIdentifierNewDevice = 0x7FF,
	CAN_LocalPacketIdentifierDeviceApplyId = 0x3FF,

} CAN_LocalPacketIdentifier;
//------------------------------------------------------------------------------

typedef enum
{
	CAN_LocalMessageTypeError,
	CAN_LocalMessageTypeEvent,
	CAN_LocalMessageTypeBroadcast,
	CAN_LocalMessageTypeRequest,
	CAN_LocalMessageTypeResponse,
	CAN_LocalMessageTypeNotification,
	CAN_LocalMessageTypeAwake,

	CAN_LocalMessageTypesCount

} CAN_LocalMessageTypes;
//------------------------------------------------------------------------------

typedef enum
{
	CAN_LocalPacketTypeIdle,

} CAN_LocalPacketTypes;
//------------------------------------------------------------------------------

typedef union
{
	uint64_t MAC;

	uint64_t Value;

} CAN_LocalPacketNewDeviceT;
//------------------------------------------------------------------------------

typedef union
{
	uint64_t MAC;

	uint64_t Value;

} CAN_LocalPacketDeviceApplyIdT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint16_t Type;
		uint16_t Extansion;
	};

	uint32_t Value;

} CAN_LocalExtansionNewDeviceT;
//------------------------------------------------------------------------------

typedef union
{
	uint16_t Id;

	uint32_t Value;

} CAN_LocalExtansionDeviceApplyIdT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint16_t Type;
		uint16_t Extansion;
		uint8_t NumberOfServices;
		uint8_t NumberOfDevices;
	};

} CAN_LocalPacketDeviceAwakeT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_LOCAL_TYPES_H_
