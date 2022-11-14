//==============================================================================
#ifndef REGISTERS_STM32F4XX_I2C_H
#define REGISTERS_STM32F4XX_I2C_H
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
    /* 0x00000001 */ uint32_t PeripheralEnable: 1; //I2C_CR1_PE    
    /* 0x00000002 */ uint32_t SMBusMode: 2; //I2C_CR1_SMBUS
    /* 0x00000008 */ uint32_t SMBusType: 1; //I2C_CR1_SMBTYPE
    
    /* 0x00000010 */ uint32_t ARPEnable: 1; //I2C_CR1_ENARP
    /* 0x00000020 */ uint32_t PECEnable: 1; //I2C_CR1_ENPEC
    /* 0x00000040 */ uint32_t GeneralCallEnable: 1; //I2C_CR1_ENGC
    /* 0x00000080 */ uint32_t SlaveMode: 1; //I2C_CR1_NOSTRETCH
    
    /* 0x00000100 */ uint32_t StartGeneration: 1; //I2C_CR1_START    
    /* 0x00000200 */ uint32_t StopGeneration: 1; //I2C_CR1_STOP
    /* 0x00000400 */ uint32_t AcknowledgeEnable: 1; //I2C_CR1_ACK    
    /* 0x00000800 */ uint32_t Acknowledge: 1; //I2C_CR1_POS
	
    /* 0x00001000 */ uint32_t PacketErrorChecking: 1; //I2C_CR1_PEC
    /* 0x00002000 */ uint32_t SMBusAlert: 2; //I2C_CR1_ALERT
    /* 0x00004000 */ //uint32_t SoftwareReset: 1; //I2C_CR1_SWRST
    /* 0x00008000 */ uint32_t SoftwareReset: 1; //I2C_CR1_SWRST
  };
  uint32_t Value;
  
} REG_I2C_CR1_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t PeripheralClockFrequency: 6; //I2C_CR2_FREQ    

    /* 0x00000040 */ uint32_t Free0x40: 1; //I2C_CR1_ENGC
    /* 0x00000080 */ uint32_t Free0x80: 1; //I2C_CR1_NOSTRETCH    
    
    /* 0x00000100 */ uint32_t ErrorInterruptEnable: 1; //I2C_CR2_ITERREN    
    /* 0x00000200 */ uint32_t EventInterruptEnable: 1; //I2C_CR2_ITEVTEN
    /* 0x00000400 */ uint32_t BufferInterruptEnable: 1; //I2C_CR2_ITBUFEN    
    /* 0x00000800 */ uint32_t DMARequestsEnable: 1; //I2C_CR2_DMAEN
	
    /* 0x00001000 */ uint32_t DMALastTransfer: 1; //I2C_CR2_LAST
  };
  uint32_t Value;
  
} REG_I2C_CR2_T;
//==============================================================================
typedef union{
  struct{
    /* 0x00000001 */ uint32_t InterfaceAddress0x01: 1; //I2C_OAR1_ADD0    
    /* 0x00000002 */ uint32_t InterfaceAddress: 9; //I2C_OAR1_ADD1_7
    
    /* 0x00004000 */ uint32_t Free0x400_0x4000: 5; //Free0x400_0x4000
    /* 0x00008000 */ uint32_t SlaveMode: 1; //I2C_OAR1_ADDMODE
  };
  uint32_t Value;
} REG_I2C_OAR1_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t DualAddressingModeEnable: 1; //I2C_OAR2_ENDUAL    
    /* 0x00000002 */ uint32_t InterfaceAddress: 7; //I2C_OAR1_ADD1_7
  };
  uint32_t Value;
  
} REG_I2C_OAR2_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t StartBit: 1; //I2C_SR1_SB    
    /* 0x00000002 */ uint32_t AddressSent: 1; //I2C_SR1_ADDR
    /* 0x00000004 */ uint32_t ByteTransferFinished: 1; //I2C_SR1_BTF
    /* 0x00000008 */ uint32_t HeaderSent10bit: 1; //I2C_SR1_ADD10

    /* 0x00000010 */ uint32_t StopDetection: 1; //I2C_SR1_STOPF
    /* 0x00000020 */ uint32_t Free0x20: 1; //Free0x20
    /* 0x00000040 */ uint32_t RxNotEmpty: 1; //I2C_SR1_RXNE
    /* 0x00000080 */ uint32_t TxEmpty: 1; //I2C_SR1_TXE
    
    /* 0x00000100 */ uint32_t BusError: 1; //I2C_SR1_BERR    
    /* 0x00000200 */ uint32_t ArbitrationLost: 1; //I2C_SR1_ARLO
    /* 0x00000400 */ uint32_t AcknowledgeFailure: 1; //I2C_SR1_AF    
    /* 0x00000800 */ uint32_t OverrunOrUnderrun: 1; //I2C_SR1_OVR
	
    /* 0x00001000 */ uint32_t PECErrorInReception: 1; //I2C_SR1_PECERR
    /* 0x00002000 */ uint32_t Free0x2000: 1; //Free0x2000
    /* 0x00004000 */ uint32_t TimeoutOrTlowError: 1; //I2C_SR1_TIMEOUT
    /* 0x00008000 */ uint32_t SMBusAlert: 1; //I2C_SR1_SMBALERT
  };
  uint32_t Value;
  
} REG_I2C_SR1_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t MasterOrSlave: 1; //I2C_SR2_MSL    
    /* 0x00000002 */ uint32_t BusBusy: 1; //I2C_SR2_BUSY
    /* 0x00000004 */ uint32_t TransmitterOrReceiver: 1; //I2C_SR1_BTF
    /* 0x00000008 */ uint32_t Free0x8: 1; //Free0x8

    /* 0x00000010 */ uint32_t GeneralCallAddress: 1; //I2C_SR2_GENCALL
    /* 0x00000020 */ uint32_t SMBusDeviceDefaultAddress: 1; //I2C_SR2_SMBDEFAULT
    /* 0x00000040 */ uint32_t SMBusHostHeader: 1; //I2C_SR2_SMBHOST
    /* 0x00000080 */ uint32_t DualFlag: 1; //I2C_SR2_DUALF
    
    /* 0x00000100 */ uint32_t PacketErrorCheckingRegister: 8; //I2C_SR2_PEC
  };
  uint32_t Value;
  
} REG_I2C_SR2_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t ClockControlRegister : 12; //I2C_CCR_CCR

    /* 0x00001000 */ uint32_t Free0x1000_0x2000: 2; //Free0x1000_0x2000

    /* 0x00004000 */ uint32_t FastModeDutyCycle: 1; //I2C_CCR_DUTY
    /* 0x00008000 */ uint32_t I2CMasterModeSelection: 1; //I2C_CCR_FS
  };
  uint32_t Value;
  
} REG_I2C_CCR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t MaximumRiseTime : 6; //I2C_CCR_CCR
  };
  uint32_t Value;
  
} REG_I2C_TRISE_T;
//==============================================================================
typedef struct
{
  volatile REG_I2C_CR1_T Control1;
  volatile REG_I2C_CR2_T Control2;
  volatile REG_I2C_OAR1_T OAR1;
  volatile REG_I2C_OAR2_T OAR2;
  volatile uint32_t Data;
  volatile REG_I2C_SR1_T Status1;
  volatile REG_I2C_SR2_T Status2;
  volatile REG_I2C_CCR_T CCR;
  volatile REG_I2C_TRISE_T TRISE;
  
} REG_I2C_T;
//==============================================================================
#endif //REGISTERS_STM32F4XX_ENABLE
#endif //REGISTERS_STM32F4XX_I2C_H
