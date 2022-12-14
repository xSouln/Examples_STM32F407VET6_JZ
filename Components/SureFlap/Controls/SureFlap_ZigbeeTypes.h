//==============================================================================
#ifndef _SUREFLAP_ZIGBEE_TYPES_H
#define _SUREFLAP_ZIGBEE_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Components_Types.h"
#include "SureFlap_Config.h"
#include "SureFlap/Adapters/SureFlap_ZigbeeAdapterBase.h"
//==============================================================================
//defines:

//==============================================================================
//types:

// used to indicate where a pairing request came from
typedef enum
{
	PAIRING_REQUEST_SOURCE_UNKNOWN,
	PAIRING_REQUEST_SOURCE_SERVER,
	PAIRING_REQUEST_SOURCE_BUTTON,
	PAIRING_REQUEST_SOURCE_CLI,
	PAIRING_REQUEST_SOURCE_TIMEOUT,
	PAIRING_REQUEST_SOURCE_BEACON_REQUEST,

} SureFlapZigbeeParingRequestSources;
//------------------------------------------------------------------------------

typedef enum
{
	SUREFLAP_ZIGBEE_PACKET_UNKNOWN,
	SUREFLAP_ZIGBEE_PACKET_DATA,
	SUREFLAP_ZIGBEE_PACKET_DATA_ACK,
	SUREFLAP_ZIGBEE_PACKET_ACK,
	SUREFLAP_ZIGBEE_PACKET_BEACON,
	SUREFLAP_ZIGBEE_PACKET_PAIRING_REQUEST,
	SUREFLAP_ZIGBEE_PACKET_PAIRING_CONFIRM,
	SUREFLAP_ZIGBEE_PACKET_CHANNEL_HOP,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_AWAKE,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_TX, // 9
	SUREFLAP_ZIGBEE_PACKET_END_SESSION,
	SUREFLAP_ZIGBEE_PACKET_DETACH,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_SLEEP,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_P2P,
	SUREFLAP_ZIGBEE_PACKET_ENCRYPTION_KEY,
	SUREFLAP_ZIGBEE_PACKET_REPEATER_PING,
	SUREFLAP_ZIGBEE_PACKET_PING,
	SUREFLAP_ZIGBEE_PACKET_PING_R,
	SUREFLAP_ZIGBEE_PACKET_REFUSE_AWAKE,
	SUREFLAP_ZIGBEE_PACKET_DEVICE_STATUS, // 0x13 / 19
	SUREFLAP_ZIGBEE_PACKET_DEVICE_CONFIRM,  //20
	SUREFLAP_ZIGBEE_PACKET_DATA_SEGMENT,
	SUREFLAP_ZIGBEE_PACKET_BLOCKING_TEST,
	SUREFLAP_ZIGBEE_PACKET_DATA_ALT_ENCRYPTED,

} SureFlapZigbeePacketTypes;
//------------------------------------------------------------------------------

typedef struct
{
	//including header and parity after payload
	uint16_t PacketLength;
	uint8_t SequenceNumber;

	//should be PACKET_TYPE?
	uint8_t PacketType;
	uint64_t SourceAddress;
	uint64_t DestinationAddress;
	uint16_t Crc;

	//received signal strength.
	uint8_t Rss;

	//sizeof = 24
	uint8_t Spare;

} SureFlapZigbeePacketHeaderT;
//------------------------------------------------------------------------------

typedef struct
{
	SureFlapZigbeePacketHeaderT Header;
	uint8_t Payload[SUREFLAP_ZIGBEE_MAX_PAYLOAD_SIZE];

} SureFlapZigbeePacketT;
//------------------------------------------------------------------------------

typedef union
{
	SureFlapZigbeePacketT Packet;
	uint8_t	Buffer[sizeof(SureFlapZigbeePacketT)];

} SureFlapZigbeeRxPacketT;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t Timeout;
	SureFlapZigbeeParingRequestSources Source;

	struct
	{
		uint8_t IsEnable : 1;
	};

} SureFlapZigbeeParingRequestT;
//------------------------------------------------------------------------------
// used to transfer received message between SureNet and SureNetDriver
typedef struct
{
	// Source address
    uint64_t uiSrcAddr;

    uint64_t uiDstAddr;
    uint8_t ucBufferLength;
    uint8_t ucRSSI;

    // Receive buffer
    uint8_t ucRxBuffer[127];

} SureFlapZigbeeRxBufferT;
//------------------------------------------------------------------------------
//used for the mailbox indicating a successful association
typedef struct
{
    uint64_t DeviceAddress;
    uint8_t DeviceType;
    uint8_t DeviceRSSI;

    // who put us in association mode in the first place
    SureFlapZigbeeParingRequestT Source;

} SureFlapZigbeeAssociationInfoT;
//------------------------------------------------------------------------------

typedef struct
{
	uint32_t LastTime;

} SureFlapZigbeeSynchronizationT;
//------------------------------------------------------------------------------

typedef struct
{
	ObjectBaseT Object;
	
	SureFlapZigbeeAdapterBaseT Adapter;

	SureFlapZigbeeAssociationInfoT AssociationInfo;
	SureFlapZigbeeParingRequestT ParingRequest;
	
	struct
	{
		uint32_t TransmissionComplite : 1;
		uint32_t IsStarted : 1;
		uint32_t ParingEnable : 1;
	};

	SureFlapZigbeeSynchronizationT Synchronization;

	uint16_t PanId;
	uint16_t CurrentChannel;
	uint16_t CurrentChannelPage;

	uint8_t BeaconPayload[SUREFLAP_ZIGBEE_BEACON_PAILOAD_SIZE];
	int8_t TxPowerPerChannel[SUREFLAP_ZIGBEE_CHANNELS_COUNT];

	SureFlapZigbeeRxPacketT RxPacket;
	SureFlapZigbeeRxBufferT RxBuffer;

	uint8_t TxSequenceNumber;
	  
} SureFlapZigbeeT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_ZIGBEE_TYPES_H
