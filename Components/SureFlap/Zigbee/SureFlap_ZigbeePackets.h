//==============================================================================
#ifndef _SUREFLAP_ZIGBEE_PACKETS_H
#define _SUREFLAP_ZIGBEE_PACKETS_H
//------------------------------------------------------------------------------
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap_Types.h"
#include "SureFlap_ZigbeeTypes.h"
//==============================================================================
//defines:

// This is 2 bytes for command, 2 for length and 1 for parity
#define SUREFLAP_ZIGBEE_MESSAGE_OVERHEAD	5
//==============================================================================
//types:

// parameters used as part of DEVICE_RCVD_SEGS message
typedef struct
{
    uint8_t FetchChunkUpper;
    uint8_t FetchChunkLower;
    uint8_t FetchChunkBlocks; // 14
    uint8_t ReceivedSegments[9];

} SureFlapDevice_RCVD_SEGS_ParametersT;
//------------------------------------------------------------------------------
// This would be more logically located in SureNet.h, however, it needs DEVICE_DATA_STATUS
// which is used in two orthogonal ways, as a parameter in PACKET_DEVICE_AWAKE, and
// to manage the HUB_CONVERSATION. Perhaps they should be separated.
typedef struct
{
	SureFlapDeviceDataStatus DeviceDataStatus;

	//multiply by 32 to get device battery voltage in millivolts
	uint8_t BatteryVoltage;
	uint8_t DeviceHours;
	uint8_t DeviceMinutes;

	//only used on Pet Door
	uint8_t LockStatus;
	uint8_t DeviceRSSI;
	uint8_t AwakeCount;
	//no idea what this is
	uint8_t Sum;
	//TBC
	uint8_t TxStats[SUREFLAP_ZIGBEE_TX_STAT_BACKSTOP];
	//used for firmware download, if device_data_status == DEVICE_RCVD_SEGS
	union
	{
		SureFlapDeviceEncryptionTypes EncryptionType;
		SureFlapDevice_RCVD_SEGS_ParametersT RCVD_SEGS_Parameters;
	};

	SureFlapDeviceEncryptionTypes EncryptionTypeExtended;

} SureFlapZigbeeAwakePacketT;
//------------------------------------------------------------------------------

typedef struct
{
	// SureFlapZigbeeCommands
	int16_t Command;

	 // total length, i.e. sizeof(command) + sizeof(length) + payload_length
	int16_t Length;

	uint8_t Payload[SUREFLAP_ZIGBEE_MESSAGE_PAYLOAD_SIZE + SUREFLAP_ZIGBEE_MESSAGE_PARITY_SIZE];

} SureFlapZigbeeMessageT;
//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_ZIGBEE_PACKETS_H
#endif //SUREFLAP_COMPONENT_ENABLE
