//==============================================================================
#ifndef _COMPONENTS_SELECTOR_H_
#define _COMPONENTS_SELECTOR_H_
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif 
//==============================================================================
//includes:

#include "Peripherals/xTimer/xTimer.h"
#include "Peripherals/xUSART/xUSART.h"
#include "Peripherals/CAN/xCAN.h"
//==============================================================================
/// @defgroup SelectedComponents SelectedComponents
/// @brief подключенные компоненты
/// @{

#include "Components/Terminal/Terminal-Component.h"
#include "Components/USART-Ports/USART-Ports-Component.h"
#include "CAN/CAN_Example-Component.h"
#include "TcpServer/LWIP/LWIP-NetTcpServer-Component.h"
#include "Devices/LoaclDevice/LocalDevice-Component.h"

/// @}
//==============================================================================
//defines:


//==============================================================================
//macros:


//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif //_COMPONENTS_SELECTOR_H_

