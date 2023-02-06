//==============================================================================
#ifndef _SUREFLAP_ZIGBEE_H
#define _SUREFLAP_ZIGBEE_H
//------------------------------------------------------------------------------
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap_ZigbeeTypes.h"
#include "SureFlap/Control/SureFlap_Types.h"
//==============================================================================
//defines:


//==============================================================================
//functions:

xResult _SureFlapZigbeeInit(SureFlapT* hub);
void _SureFlapZigbeeHandler(SureFlapZigbeeT* network);
void _SureFlapZigbeeTimeSynchronization(SureFlapZigbeeT* network);

xResult _SureFlapZigbeeStartNetwork(SureFlapZigbeeT* network);

void SureFlapZigbeeDecodeFrom_IEEE(SureFlapZigbeeT *network, SF_ZigbeeRxBufferT *rx_buffer);
void SureFlapZigbeePacketReceiver(SureFlapZigbeeT* network, SF_ZigbeeRxBufferT *rx_buffer);

xResult SureFlapZigbeeAssociate(SureFlapZigbeeT* network, uint64_t address, uint8_t type, uint8_t rssi, uint16_t* out_short_address);
xResult SureFlapZigbeeResetAssociations(SureFlapZigbeeT* network);
xResult SureFlapZigbeeAcceptPairing(SureFlapZigbeeT* network, uint64_t address);
xResult SureFlapZigbeeRequestAccept(SureFlapZigbeeT* network);
SureFlapZigbeeAssociationT* SureFlapZigbeeGetAssociationFrom_MAC(SureFlapZigbeeT* network, uint64_t mac);

int16_t _SureFlapZigbeeTransmit(SureFlapZigbeeT* network,
		SureFlapZigbeePacketTypes type,
		SureFlapDeviceT* dest,
		uint8_t *payload_ptr,
		uint16_t length,
		int16_t seq,
		bool request_ack);
//==============================================================================
//override:

#define SureFlapZigbeeInit(hub) _SureFlapZigbeeInit(hub)
#define SureFlapZigbeeHandler(network) _SureFlapZigbeeHandler(network)
#define SureFlapZigbeeTimeSynchronization(network) _SureFlapZigbeeTimeSynchronization(network)

#define SureFlapZigbeeStartNetwork(network) _SureFlapZigbeeStartNetwork(network);

#define SureFlapZigbeeTransmit(network, type, dest, payload_ptr, length, seq, request_ack)\
	_SureFlapZigbeeTransmit(network, type, dest, payload_ptr, length, seq, request_ack);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_SUREFLAP_ZIGBEE_H
#endif //SUREFLAP_COMPONENT_ENABLE
