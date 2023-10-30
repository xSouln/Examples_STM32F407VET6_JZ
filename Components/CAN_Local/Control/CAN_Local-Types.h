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
//defines:

#define CAN_LOCAL_MESSAGE_TYPE_SIZE 3
#define CAN_LOCAL_SERVICE_TYPE_SIZE 8
#define CAN_LOCAL_PACKET_TYPE_SIZE 5
#define CAN_LOCAL_SERVICE_ID_SIZE 13
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

	CAN_LocalMessageTypeTransfer,

	CAN_LocalMessageTypesCount

} CAN_LocalMessageTypes;
//------------------------------------------------------------------------------
typedef enum PACKED_PREFIX
{
	CAN_LocalTransferPacketTypeOpenTransfer,
	CAN_LocalTransferPacketTypeApproveTransfer,
	CAN_LocalTransferPacketTypeCloseTransfer,

	CAN_LocalTransferPacketTypeRequestExchange,
	CAN_LocalTransferPacketTypeResponseExchange,
	CAN_LocalTransferPacketTypeExchange,

	CAN_LocalTransferPacketTypesCount

} CAN_LocalTransferPacketTypes;
//------------------------------------------------------------------------------
typedef enum PACKED_PREFIX
{

	CAN_LocalRequestPacketTypesCount

} CAN_LocalRequestPacketTypes;
//------------------------------------------------------------------------------
typedef enum PACKED_PREFIX
{

	CAN_LocalResponsePacketTypesCount

} CAN_LocalResponsePacketTypes;
//------------------------------------------------------------------------------
typedef enum PACKED_PREFIX
{
	///extension types
	CAN_LocalPacketTypeAwake,

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

			uint64_t Value;

		} Data;
	};

	struct
	{
		uint32_t MessageType : CAN_LOCAL_MESSAGE_TYPE_SIZE;
		uint32_t ServiceType : CAN_LOCAL_SERVICE_TYPE_SIZE;
		uint32_t PacketType : CAN_LOCAL_PACKET_TYPE_SIZE;
		uint32_t ServiceId : CAN_LOCAL_SERVICE_ID_SIZE;

		uint32_t IsEnabled : 1;

	} ExtensionHeader;

	struct
	{
		uint32_t MessageType : CAN_LOCAL_MESSAGE_TYPE_SIZE;
		uint32_t PacketType : CAN_LOCAL_PACKET_TYPE_SIZE;
		uint32_t ServiceId : CAN_LOCAL_SERVICE_ID_SIZE;
		uint32_t Characteristic : 8;

		uint32_t IsEnabled : 1;

	} TransferHeader;

	struct
	{
		uint32_t MessageType : CAN_LOCAL_MESSAGE_TYPE_SIZE;
		uint32_t ServiceType : CAN_LOCAL_SERVICE_TYPE_SIZE;

	} Header;

	struct
	{
		uint32_t MessageType : CAN_LOCAL_MESSAGE_TYPE_SIZE;
		uint32_t ServiceType : CAN_LOCAL_SERVICE_TYPE_SIZE;
	};

} CAN_LocalSegmentT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint16_t EventType;
		uint16_t ContentType : 4;
		uint16_t Id : 12;

		uint32_t Content;
	};

	uint64_t Value;

} CAN_LocalBaseEventPacketT;
//------------------------------------------------------------------------------

typedef union
{
	float Temperature;

	uint32_t Value;

} CAN_LocalTemperatureSensoreEventContentT;
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
		uint16_t ServiceId;
		uint16_t Action;

		uint16_t Id : 8;
		uint16_t Type : 2;
		uint16_t ValidationIsEnabled : 1;

		uint8_t Token;
	};

	uint64_t Value;

} CAN_LocalPacketOpenTransferRequestT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint16_t ServiceId;
		uint16_t Action;

		uint16_t Reserved;

		uint8_t Token;
		int8_t Result;
	};

	uint64_t Value;

} CAN_LocalPacketOpenTransferResponseT;
//------------------------------------------------------------------------------

typedef union
{
	uint8_t Data[8];
	uint64_t Value;

} CAN_LocalPacketTransferRequestT;
//------------------------------------------------------------------------------

typedef union
{
	uint8_t Data[8];
	uint64_t Value;

} CAN_LocalPacketTransferResponseT;
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

} CAN_LocalPacketCloseTransferRequestT;
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

} CAN_LocalPacketCloseTransferResponseT;
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

} CAN_LocalTransferT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_LOCAL_TYPES_H_
