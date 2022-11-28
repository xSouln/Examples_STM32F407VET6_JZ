//==============================================================================
//header:

#ifndef TCP_CLIENT_CONFIG_H
#define TCP_CLIENT_CONFIG_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//includes:

#include "Components_Config.h"
//==============================================================================
//defines:

static const uint8_t TCP_CLIENT_DEFAULT_IP[4] = { 192, 168, 0, 40 };
static const uint8_t TCP_CLIENT_DEFAULT_GETAWAY[4] = { 192, 168, 0, 1 };
static const uint8_t TCP_CLIENT_DEFAULT_NETMASK[4] = { 255, 255, 255, 0 };
static const uint8_t TCP_CLIENT_DEFAULT_MAC_ADDRESS[6] = { 0xdc, 0x31, 0x82, 0xc5, 0xfe, 0x11 };

#define TCP_CLIENT_DEFAULT_PORT 5000
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //TCP_CLIENT_CONFIG_H
