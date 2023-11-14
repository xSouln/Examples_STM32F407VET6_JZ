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
#include "Abstractions/xRequestControl/xRequestControl-Types.h"
//==============================================================================
//defines:

#define CAN_LOCAL_ID_SIZE 11
#define CAN_LOCAL_EXTENSION_ID_SIZE 18

#define CAN_LOCAL_MESSAGE_TYPE_SIZE 3
#define CAN_LOCAL_SERVICE_TYPE_SIZE 8
#define CAN_LOCAL_PACKET_TYPE_SIZE 5
#define CAN_LOCAL_SERVICE_ID_SIZE 13

#define CAN_LOCAL_DHCP_PACKET_TYPE_SIZE 2
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

	CAN_LocalMessageTypeCustom,

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
	CAN_LocalRequestPacketTypeCommon,

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
typedef enum PACKED_PREFIX
{
	CAN_LocalTemperatureNotificationUpdateTemperature

} CAN_LocalTemperatureNotifications;
//------------------------------------------------------------------------------
typedef union PACKED_PREFIX
{
	struct
	{
		uint32_t Identifier : CAN_LOCAL_ID_SIZE;
		uint32_t Extension : CAN_LOCAL_EXTENSION_ID_SIZE;
		uint32_t ExtensionIsEnabled : 1;

		uint8_t DataLength;

		union
		{
			uint8_t Bytes[8];
			uint16_t HalfWords[4];
			uint32_t Words[2];
			uint64_t DoubleWord;

			uint64_t Value;
			uint64_t Content;

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
		uint32_t ServiceType : CAN_LOCAL_SERVICE_TYPE_SIZE;
		uint32_t PacketType : CAN_LOCAL_DHCP_PACKET_TYPE_SIZE;

		uint32_t Characteristic : CAN_LOCAL_EXTENSION_ID_SIZE - CAN_LOCAL_DHCP_PACKET_TYPE_SIZE;

		uint32_t IsEnabled : 1;

	} DHCP_Header;

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

} CAN_LocalContentTemperatureSensoreEventT;
//------------------------------------------------------------------------------

typedef enum
{
	CAN_LocalBroadcastPacketTypeDHCPRequestGetId,
	CAN_LocalBroadcastPacketTypeDHCPResponseGetId,

} CAN_LocalBroadcastPacketTypes;
//------------------------------------------------------------------------------

typedef union
{
	uint64_t MAC;

	uint64_t Value;

} CAN_LocalRequestContentDHCPGetIdT;
//------------------------------------------------------------------------------

typedef union
{
	uint64_t MAC;

	uint64_t Value;

} CAN_LocalResponseContentDHCPGetIdT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint8_t Type;
		uint8_t Extansion;
	};

	uint16_t Value;

} CAN_LocalCharacteristicDHCPGetIdT;
//------------------------------------------------------------------------------

typedef union
{
	uint16_t Id;

	uint16_t Value;

} CAN_LocalCharacteristicDHCPApplyIdT;
//------------------------------------------------------------------------------

typedef struct PACKED_PREFIX
{
	uint16_t Recipient : 13;
	uint16_t Sequence : 3;
	uint16_t Action;

} CAN_LocalRequestDescriptionT;
//------------------------------------------------------------------------------

typedef struct PACKED_PREFIX
{
	uint16_t Sender : 13;
	uint16_t Sequence : 3;
	uint16_t Action;

} CAN_LocalResponseDescriptionT;
//------------------------------------------------------------------------------

typedef union PACKED_PREFIX
{
	struct
	{
		CAN_LocalRequestDescriptionT Description;

		union
		{
			uint8_t Bytes[4];
			uint16_t HalfWords[2];
			uint32_t Word;

			uint32_t Content;

		} Data;
	};

	uint64_t Value;

} CAN_LocalRequestContentT;
//------------------------------------------------------------------------------
typedef union PACKED_PREFIX
{
	struct
	{
		CAN_LocalResponseDescriptionT Description;

		union
		{
			uint8_t Bytes[4];
			uint16_t HalfWords[2];
			uint32_t Word;

			uint32_t Content;

		} Data;
	};

	uint64_t Value;

} CAN_LocalResponseContentT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint16_t ServiceId;
		uint16_t NewServiceId;
	};

	uint32_t Value;

} CAN_LocalRequestContentServiceSetIdT;
//------------------------------------------------------------------------------
typedef union
{
	uint8_t Value;

} CAN_LocalResponseContentServiceSetIdT;
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
	};

	uint64_t Value;

} CAN_LocalRequestContentOpenTransferT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint16_t ServiceId;
		uint16_t Action;

		uint16_t ContantSize;

		uint8_t Token;
		int8_t Result;
	};

	uint64_t Value;

} CAN_LocalResponseContentOpenTransferT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint8_t Segment;

		uint8_t Data[7];
	};

	uint64_t Value;

} CAN_LocalRequestContentTransferT;
//------------------------------------------------------------------------------

typedef union
{
	struct
	{
		uint8_t Segment;

		uint8_t Data[7];
	};

	uint64_t Value;

} CAN_LocalResponseContentTransferT;
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

} CAN_LocalRequestContentCloseTransferT;
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

} CAN_LocalResponseContentCloseTransferT;
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

} CAN_LocalPacketAwakeT;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

typedef struct
{
	xTransferT Base;

	uint16_t Action;

} CAN_LocalTransferT;
//------------------------------------------------------------------------------

typedef struct
{
	xRequestT Base;

	uint32_t StartTime;
	uint16_t TransmitionTime;

	uint16_t Action;
	struct xServiceT* Recipient;

	union
	{
		uint8_t Bytes[4];
		uint16_t HalfWords[2];
		uint32_t Word;

		uint32_t Value;

	} Data;

	void* Content;

} CAN_LocalRequestT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint8_t Id;
	};

	uint8_t Value;

} CAN_LocalRequestGATGetServiceT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint16_t Id;
		uint8_t Type;
		uint8_t Extension;
	};

	uint32_t Value;

} CAN_LocalResponseGATGetServiceT;
//------------------------------------------------------------------------------
typedef union
{
	struct PACKED_PREFIX
	{
		uint16_t Temperature;
		uint32_t TimeStamp;
	};

	uint8_t Data[6];

} CAN_LocalTemperatureNotificationUpdateTemperatureT;
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_CAN_LOCAL_TYPES_H_
