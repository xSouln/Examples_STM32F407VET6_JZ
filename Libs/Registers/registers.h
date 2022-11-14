//==============================================================================
#ifndef _REGISTERS_H_
#define _REGISTERS_H_
//==============================================================================
#include <stdint.h>
#include <stdbool.h>
//==============================================================================
#ifdef STM32H743xx

#include "registers_stm32h7xx/registers_stm32h7xx.h"

#endif

#ifdef STM32F103xB

#include "registers_stm32f1xx/registers_stm32f1xx_spi.h"
#include "registers_stm32f1xx/registers_stm32f1xx_timer.h"
#include "registers_stm32f1xx/registers_stm32f1xx_uart.h"
#include "registers_stm32f1xx/registers_stm32f1xx_i2c.h"

#endif

#ifdef STM32F407xx

#include "registers_stm32f4xx/registers_stm32f4xx.h"

#endif
//==============================================================================
#endif
