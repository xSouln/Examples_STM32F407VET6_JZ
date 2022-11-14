//==============================================================================
#ifndef REGISTERS_STM32X4XX_SPI_H_
#define REGISTERS_STM32X4XX_SPI_H_
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
    /* 0x00000001 */ uint32_t ClockPhase: 1; //SPI_CR1_CPHA    
    /* 0x00000002 */ uint32_t ClockPolarity: 1; //SPI_CR1_CPOL
    /* 0x00000004 */ uint32_t MasterSelection: 1; //SPI_CR1_MSTR    
    /* 0x00000008 */ uint32_t BaudRate: 3; //SPI_CR1_BR
    
    /* 0x00000040 */ uint32_t Enable: 1; //SPI_CR1_SPE
    /* 0x00000080 */ uint32_t FrameFormat: 1; //SPI_CR1_LSBFIRST
    
    /* 0x00000100 */ uint32_t SlaveSelect: 1; //SPI_CR1_SSI    
    /* 0x00000200 */ uint32_t SoftwareSlave: 1; //SPI_CR1_SSM
    /* 0x00000400 */ uint32_t ReceiveOnly: 1; //SPI_CR1_RXONLY    
    /* 0x00000800 */ uint32_t DataFrameFormat: 1; //SPI_CR1_DFF
	
    /* 0x00001000 */ uint32_t TransmitCrcNext: 1; //SPI_CR1_CRCNEXT
    /* 0x00002000 */ uint32_t HardwareCrcCalculation: 1; //SPI_CR1_CRCEN
    /* 0x00004000 */ uint32_t BidirectionalOutput: 1; //SPI_CR1_BIDIOE
    /* 0x00008000 */ uint32_t BidirectionalData: 1; //SPI_CR1_BIDIMODE
  };
  uint32_t Value;
  
} REG_SPI_CR1_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t RxEnableDMA: 1; //SPI_CR2_RXDMAEN    
    /* 0x00000002 */ uint32_t TxEnableDMA: 1; //SPI_CR2_TXDMAEN
    /* 0x00000004 */ uint32_t SS_OutputEnable: 1; //SPI_CR2_SSOE
    /* 0x00000008 */ uint32_t Free_3: 1; //SPI_CR2_SSOE
    
    /* 0x00000010 */ uint32_t FrameFormat: 1; //SPI_CR2_FRF    
    /* 0x00000020 */ uint32_t ErrorInterruptEnable: 1; //SPI_CR2_ERRIE
    /* 0x00000040 */ uint32_t RxNotEmptyInterruptEnable: 1; //SPI_CR2_RXNEIE
    /* 0x00000080 */ uint32_t TxEmptyInterruptEnable: 1; //SPI_CR2_TXEIE
  };
  uint32_t Value;
  
} REG_SPI_CR2_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t RxNotEmpty: 1; //SPI_SR_RXNE    
    /* 0x00000002 */ uint32_t TxEmpty: 1; //SPI_SR_TXE
    /* 0x00000004 */ uint32_t ChannelSide: 1; //SPI_SR_CHSIDE
    /* 0x00000008 */ uint32_t Underrun: 1; //SPI_SR_UDR
    
    /* 0x00000010 */ uint32_t CrcError: 1; //SPI_SR_CRCERR    
    /* 0x00000020 */ uint32_t ModeFault: 1; //SPI_SR_MODF
    /* 0x00000040 */ uint32_t Overrun: 1; //SPI_SR_OVR
    /* 0x00000080 */ uint32_t Busy: 1; //SPI_SR_BSY
    
    /* 0x00000100 */ uint32_t FrameFormatRrror: 1; //SPI_SR_FRE
  };
  uint32_t Value;
  
} REG_SPI_SR_T;
//==============================================================================
typedef union
{
	volatile uint32_t Byte : 8;
	volatile uint32_t HalfWord : 16;
  volatile uint32_t Word;
  
} REG_SPI_DR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t CrcPolynomial: 16; //SPI_CRCPR_CRCPOLY 
  };
  uint32_t Value;
  
} REG_SPI_CRCPR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t RxCrc: 16; //SPI_RXCRCR_RXCRC 
  };
  uint32_t Value;
  
} REG_SPI_RXCRCR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t TxCrc: 16; //SPI_TXCRCR_TXCRC 
  };
  uint32_t Value;
  
} REG_SPI_TXCRCR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t ChannelLength: 1; //SPI_I2SCFGR_CHLEN    
    /* 0x00000002 */ uint32_t DataLength: 2; //SPI_I2SCFGR_DATLEN
    /* 0x00000008 */ uint32_t ClockPolarity: 1; //SPI_I2SCFGR_CKPOL
    
    /* 0x00000010 */ uint32_t StandardSelection: 3; //I2SSTD
    /* 0x00000080 */ uint32_t PCM_FrameSynchronization: 1; //SPI_I2SCFGR_PCMSYNC
    
    /* 0x00000100 */ uint32_t ConfigurationMode: 2; //SPI_I2SCFGR_I2SCFG
    /* 0x00000400 */ uint32_t Enable: 1; //SPI_I2SCFGR_I2SE
    /* 0x00000800 */ uint32_t ModeSelection : 1; //SPI_I2SCFGR_I2SMOD
	
    /* 0x00001000 */ uint32_t AsynchronousStartEnable: 1; //SPI_I2SCFGR_ASTRTEN
  };
  uint32_t Value;
  
} REG_SPI_I2SCFGR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t LinearPrescaler: 8; //SPI_I2SPR_I2SDIV
    
    /* 0x00000100 */ uint32_t OddFactor: 1; //SPI_I2SPR_ODD
    /* 0x00000200 */ uint32_t MasterClockOutputEnable: 1; //SPI_I2SPR_MCKOE
  };
  uint32_t Value;
  
} REG_SPI_I2SPR_T;
//==============================================================================
typedef struct
{
  volatile REG_SPI_CR1_T Control1; //SPI control register 1
  volatile REG_SPI_CR2_T Control2; //SPI control register 2
  volatile REG_SPI_SR_T Status; //SPI status register
  volatile REG_SPI_DR_T Data; //SPI data register
  volatile REG_SPI_CRCPR_T CRCPR; //SPI CRC polynomial register (not used in I2S mode)
  volatile REG_SPI_RXCRCR_T RXCRCR; //SPI RX CRC register (not used in I2S mode)
  volatile REG_SPI_TXCRCR_T TXCRCR; //SPI TX CRC register (not used in I2S mode)
  volatile REG_SPI_I2SCFGR_T I2SCFGR; //SPI_I2S configuration register
  volatile REG_SPI_I2SPR_T I2SPR; //SPI_I2S prescaler register
  
} REG_SPI_T;
//==============================================================================
#endif //REGISTERS_STM32F4XX_ENABLE
#endif //REGISTERS_STM32X4XX_SPI_H_
