
//==============================================================================
#ifndef REGISTERS_STM32F4XX_UART_H
#define REGISTERS_STM32F4XX_UART_H
//==============================================================================
#include "Registers/registers_config.h"
//------------------------------------------------------------------------------
#ifdef REGISTERS_STM32F4XX_ENABLE
//==============================================================================
#include <stdint.h>
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t SendBreak: 1; //USART_CR1_SBK    
    /* 0x00000002 */ uint32_t ReceiverWakeup: 1; //USART_CR1_RWU
    /* 0x00000004 */ uint32_t ReceiverEnable: 1; //USART_CR1_RE    
    /* 0x00000008 */ uint32_t TransmitterEnable: 1; //USART_CR1_TE
	
    /* 0x00000010 */ uint32_t IDLE_InterruptEnable: 1; //USART_CR1_IDLEIE
    /* 0x00000020 */ uint32_t RxNotEmptyInterruptEnable: 1; //USART_CR1_RXNEIE    
    /* 0x00000040 */ uint32_t TxCompleteInterruptEnable: 1; //USART_CR1_TCIE
    /* 0x00000080 */ uint32_t TxEmptyInterruptEnable: 1; //USART_CR1_TXEIE
    
    /* 0x00000100 */ uint32_t PE_InterruptEnable: 1; //USART_CR1_PEIE    
    /* 0x00000200 */ uint32_t ParitySelection: 1; //USART_CR1_PS
    /* 0x00000400 */ uint32_t ParityControlEnable: 1; //USART_CR1_PCE    
    /* 0x00000800 */ uint32_t WakeupMethod: 1; //USART_CR1_WAKE
	
    /* 0x00001000 */ uint32_t WordLength: 1; //USART_CR1_M
    /* 0x00002000 */ uint32_t USART_Enable: 2; //USART_CR1_UE
    /* 0x00008000 */ uint32_t OversamplingBy8: 1; //USART_CR1_OVER8
  };
  uint32_t Value;
  
} REG_UART_CR1_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t AddressNode: 5; //USART_CR2_ADD
    
    /* 0x00000020 */ uint32_t LIN_Length: 1; //USART_CR2_LBDL
    /* 0x00000040 */ uint32_t LIN_Interrupt_Enable: 2; //USART_CR2_LBDIE
	
    /* 0x00000100 */ uint32_t LastBitClockPulse: 1; //USART_CR2_LBCL	
    /* 0x00000200 */ uint32_t ClockPhase: 1; //USART_CR2_CPHA
    /* 0x00000400 */ uint32_t ClockPolarity: 1; //USART_CR2_CPOL    
    /* 0x00000800 */ uint32_t ClockEnable: 1; //USART_CR2_CLKEN
	
    /* 0x00001000 */ uint32_t StopBit_0: 1; //USART_CR2_STOP_0    
    /* 0x00002000 */ uint32_t StopBit_1: 1; //USART_CR2_STOP_1    
    /* 0x00004000 */ uint32_t LIN_enable: 1; //USART_CR1_PS
  };
  uint32_t Value;
  
} REG_UART_CR2_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t ErrorInterruptEnable: 1; //USART_CR3_EIE    
    /* 0x00000002 */ uint32_t IrDA_Enable: 1; //USART_CR3_IREN
    /* 0x00000004 */ uint32_t IrDA_LowPower : 1; //USART_CR3_IRLP
    /* 0x00000008 */ uint32_t HalfDuplexSelection: 1; //USART_CR3_HDSEL
	
    /* 0x00000010 */ uint32_t SmartcardNACK_Enable: 1; //USART_CR3_NACK	
    /* 0x00000020 */ uint32_t SmartcardEnable: 1; //USART_CR3_SCEN
    /* 0x00000040 */ uint32_t DMA_EnableReceiver: 1; //USART_CR3_DMAR    
    /* 0x00000080 */ uint32_t DMA_EnableTransmitter: 1; //USART_CR3_DMAT
	
    /* 0x00000100 */ uint32_t RTS_Enable: 1; //USART_CR3_RTSE    
    /* 0x00000200 */ uint32_t CTS_Enable: 1; //USART_CR3_CTSE    
    /* 0x00000400 */ uint32_t CTS_InterruptEnable: 1; //USART_CR3_CTSIE
    /* 0x00000800 */ uint32_t OneBitMethodEnable: 1; //USART_CR3_ONEBIT
  };
  uint32_t Value;
  
} REG_UART_CR3_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t Prescaler: 8; //USART_GTPR_PSC
	
    /* 0x00000100 */ uint32_t GuardTime: 8; //USART_GTPR_GT
  };
  uint32_t Value;
  
} REG_UART_GTPR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t FractionUSARTDIV: 4; //USART_BRR_DIV_Fraction
	
    /* 0x00000010 */ uint32_t MantissaUSARTDIV: 12; //USART_BRR_DIV_Mantissa
  };
  uint32_t Value;
  
} REG_UART_BRR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t ErrorParity: 1; //USART_SR_PE
    /* 0x00000002 */ uint32_t ErrorFraming: 1;  //USART_SR_FE
    /* 0x00000004 */ uint32_t ErrorNoise: 1;  //USART_SR_NE
    /* 0x00000008 */ uint32_t ErrorOverRun: 1;  //USART_SR_ORE
    
    /* 0x00000010 */ uint32_t IDLE_Detected: 1; //USART_SR_IDLE    
    /* 0x00000020 */ uint32_t RxNotEmpty: 1; //USART_SR_RXNE    
    /* 0x00000040 */ uint32_t TxComplete: 1; //USART_SR_TC
    /* 0x00000080 */ uint32_t TxEmpty: 1; //USART_SR_TXE
    
    /* 0x00000100 */ uint32_t LIN_BreakDetection: 1; //USART_SR_LBD		
    /* 0x00000200 */ uint32_t CTS_Flag: 1; //USART_SR_CTS
  };
  uint32_t Value;
  
} REG_UART_SR_T;
//==============================================================================
typedef struct
{
  volatile REG_UART_SR_T Status; //USART Status register
  volatile uint32_t Data; //USART Data register
  volatile REG_UART_BRR_T BaudRate; //USART Baud rate register
  volatile REG_UART_CR1_T Control1; //USART Control register 1
  volatile REG_UART_CR2_T Control2; //USART Control register 2
  volatile REG_UART_CR3_T Control3; //USART Control register 3
  volatile REG_UART_GTPR_T GTPR; //SART Guard time and prescaler register
  
} REG_UART_T; // For REG
//==============================================================================
#endif //REGISTERS_STM32F4XX_ENABLE
#endif //REGISTERS_STM32F4XX_UART_H
