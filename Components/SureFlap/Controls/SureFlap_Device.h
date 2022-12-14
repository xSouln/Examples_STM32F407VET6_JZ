//==============================================================================
#ifndef _SUREFLAP_DEVICE_H
#define _SUREFLAP_DEVICE_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "SureFlap_DeviceTypes.h"
#include "SureFlap_Types.h"
//==============================================================================
//macros:


//==============================================================================
//functions:

xResult SureFlapDeviceInit(SureFlapT* control);
void _SureFlapDeviceHandler(SureFlapDeviceControlT* control);
void _SureFlapDeviceTimeSynchronization(SureFlapDeviceControlT* control);

xResult SureFlapDeviceAssociate(SureFlapDeviceControlT* control, uint64_t address, uint8_t type, uint8_t rssi, uint16_t* out_short_address);
xResult SureFlapDeviceResetAssociations(SureFlapDeviceControlT* control);
SureFlapDeviceAssociationT* SureFlapDeviceGetAssociationFrom_MAC(SureFlapDeviceControlT* control, uint64_t mac);
xResult SureFlapDeviceAcceptPairing(SureFlapDeviceControlT* control, uint64_t address);

int8_t SureFlapDeviceGetIndexFrom_MAC(SureFlapDeviceControlT* control, uint64_t mac);
SureFlapDeviceT* SureFlapDeviceGetFrom_MAC(SureFlapDeviceControlT* control, uint64_t mac);
//------------------------------------------------------------------------------
//SureFlap_DeviceSecurity.c:

SureFlapZigbeePacketTypes SureFlapDeviceEncrypt(SureFlapDeviceT* device,
		uint8_t* data,
		uint32_t length,
		uint32_t position);

void SureFlapDeviceDecrypt(SureFlapDeviceT* control,
		uint8_t* data,
		uint32_t length,
		uint32_t position);


void SureFlapDeviceSetSecretKey(SureFlapDeviceT* device);
void SureFlapDeviceSetSecurityKey(SureFlapDeviceT* device);
//==============================================================================
//override:

#define SureFlapDeviceHandler(hub) _SureFlapDeviceHandler(hub)
#define SureFlapDeviceTimeSynchronization(hub) _SureFlapDeviceTimeSynchronization(hub)
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_ZIGBEE_TYPES_H
