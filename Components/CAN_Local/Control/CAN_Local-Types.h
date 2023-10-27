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
#include "Abstractions/xTransferLayer/xTransferLayer-Types.h"
//==============================================================================
//types:

typedef enum PACKED_PREFIX
{
	///types
	CAN_LocalMessageTypeError,
	CAN_LocalMessageTypeEvent,
	CAN_LocalMessageTypeBroadcast,
	CAN_LocalMessageTypeRequest,
	CAN_LocalMessageTypeResponse,
	CAN_LocalMessageTypeNotification,

} CAN_LocalMessageTypes;
//------------------------------------------------------------------------------
typedef enum PACKED_PREFIX
{
	///extension types
	CAN_LocalPacketTypeAwake,

	CAN_LocalPacketTypeOpenTransaction,
	CAN_LocalPacketTypeApproveTransaction,
	CAN_LocalPacketTypeTransaction,

	CAN_LocalPacketTypesCount

} CAN_LocalPacketTypes;
//------------------------------------------------------------------------------
typedef union PACKED_PREFIX
{
	struct
	{
		uint32_t Identifier : 11;
		uint32_t Extension : 18;
		uint32_t ExtensionIsEnabled : 1;

		uint8_t DataLength;

		union
		{
			uint8_t Bytes[8];
			uint16_t HalfWords[4];
			uint32_t Words[2];
			uint64_t DoubleWord;

		} Data;
	};

	struct
	{
		uint32_t MessageType : 3;
		uint32_t DeviceType : 8;
		uint32_t PacketType : 5;
		uint32_t Address : 12;

	} ExtensionHeader;

	struct
	{
		uint32_t MessageType : 3;
		uint32_t Type : 8;

	} Header;

	uint8_t MessageType : 3;
	//uint8_t PacketType : 4;

} CAN_LocalSegmentT;
//------------------------------------------------------------------------------

typedef enum
{
	CAN_LocalPacketIdentifierNewDevice = 0x7FF,
	CAN_LocalPacketIdentifierDeviceApplyId = 0x3FF,

} CAN_LocalPacketIdentifier;
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

typedef enum PACKED_PREFIX
{
	CAN_LocalIdAffiliationDevice,
	CAN_LocalIdAffiliationService

} CAN_LocalIdAffiliation;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint16_t DeviceId;
		uint16_t ServiceId;

		uint16_t Action;
		uint8_t Token;
	};

	uint64_t Value;

} CAN_LocalPacketOpenTransactionRequestT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint16_t DeviceId;
		uint16_t ServiceId;
		uint16_t Action;

		uint8_t Token;
		int8_t Result;
	};

	uint64_t Value;

} CAN_LocalPacketOpenTransactionResponseT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint8_t Token;
		uint8_t Segment;

		uint8_t Data[6];
	};

	uint64_t Value;

} CAN_LocalPacketTransactionRequestT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint8_t Token;
		uint8_t Segment;

		uint8_t Data[6];
	};

	uint64_t Value;

} CAN_LocalPacketTransactionResponseT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint16_t DeviceId;
		uint8_t Token;
		uint8_t Reason;
	};

	uint32_t Value;

} CAN_LocalPacketCloseTransactionRequestT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint16_t DeviceId;
		uint8_t Token;
		uint8_t Reason;
	};

	uint32_t Value;

} CAN_LocalPacketCloseTransactionResponseT;
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
//------------------------------------------------------------------------------
struct xFieldT;

typedef struct
{
	xTransferT Base;

	struct xFieldT* Field;

} CAN_LocalTransferT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_LOCAL_TYPES_H_
