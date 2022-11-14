//==============================================================================
#ifndef REGISTERS_STM32F4XX_TIMER_H
#define REGISTERS_STM32F4XX_TIMER_H
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
	  /* 0x00000001 */ uint32_t CounterEnable: 1; //TIM_CR1_CEN
	  /* 0x00000002 */ uint32_t UpdateDisable: 1; //TIM_CR1_UDIS
	  /* 0x00000004 */ uint32_t UpdateRequestSource: 1; //TIM_CR1_URS
	  /* 0x00000008 */ uint32_t OnePulseMode: 1; //TIM_CR1_OPM
	  
	  /* 0x00000010 */ uint32_t Direction: 1; //TIM_CR1_DIR
	  /* 0x00000020 */ uint32_t AlignedModeSelection: 2; //TIM_CR1_CMS
	  /* 0x00000080 */ uint32_t PreloadEnable: 1; //TIM_CR1_ARPE
	  
	  /* 0x00000100 */ uint32_t ClockDivision: 1; //TIM_CR1_CKD
	};
	uint32_t Value;
	
} REG_TIM_CR1_T;
//==============================================================================
typedef union
{
    struct{
      /* 0x00000001 */ uint32_t ComparePreloadedControl: 1; //TIM_CR1_CKD
      /* 0x00000002 */ uint32_t Free_1: 1;
      /* 0x00000004 */ uint32_t CompareControlUpdateSelection: 1; //TIM_CR2_CCUS
      /* 0x00000008 */ uint32_t CompareDMA_Selection: 1; //TIM_CR2_CCDS
	  
      /* 0x00000010 */ uint32_t MasterModeSelection: 3; //TIM_CR2_MMS      
      /* 0x00000080 */ uint32_t TI1_Selection: 1; //TIM_CR2_TI1S
      
      /* 0x00000100 */ uint32_t OutputIdleState1: 1; //TIM_CR2_OIS1
      /* 0x00000200 */ uint32_t OutputIdleStateN1: 1; //TIM_CR2_OIS1N      
      /* 0x00000400 */ uint32_t OutputIdleState2: 1; //TIM_CR2_OIS2
      /* 0x00000800 */ uint32_t OutputIdleStateN2: 1; //TIM_CR2_OIS2N
      
      /* 0x00001000 */ uint32_t OutputIdleState3: 1; //TIM_CR2_OIS3
      /* 0x00002000 */ uint32_t OutputIdleStateN3: 1; //TIM_CR2_OIS3N      
      /* 0x00004000 */ uint32_t OutputIdleState4: 1; //TIM_CR2_OIS4
      /* 0x00008000 */ uint32_t OutputIdleStateN4: 1; //TIM_CR2_OIS4N
    };
	
    uint32_t Value;

} REG_TIM_CR2_T;
//==============================================================================
typedef union
{
    struct
	{
      /* 0x00000001 */ uint32_t SlaveMode: 3; //TIM_SMCR_SMS
      /* 0x00000008 */ uint32_t Free_4: 1; //Free_4
	  
      /* 0x00000010 */ uint32_t TriggerSelection: 3; //TIM_SMCR_TS      
      /* 0x00000080 */ uint32_t MasterSlaveMode: 1; //TIM_SMCR_MSM
	  
      /* 0x00000100 */ uint32_t ExternalTriggerFilter: 4; //TIM_SMCR_ETF
      
      /* 0x00001000 */ uint32_t ExternalTriggerPrescaler: 2; //TIM_SMCR_ETPS
      /* 0x00004000 */ uint32_t ExternalClockEnable : 1; //TIM_SMCR_ECE
      /* 0x00008000 */ uint32_t ExternalTriggerPolarity: 1; //TIM_SMCR_ETP
    };
	
    uint32_t Value;
	
} REG_TIM_SMCR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t UpdateInterruptEnable: 1; //TIM_DIER_UIE
    /* 0x00000002 */ uint32_t Compare1InterruptEnable: 1; //TIM_DIER_CC1IE
    /* 0x00000004 */ uint32_t Compare2InterruptEnable: 1; //TIM_DIER_CC2IE
    /* 0x00000008 */ uint32_t Compare3InterruptEnable: 1; //TIM_DIER_CC3IE
	
    /* 0x00000010 */ uint32_t Compare4InterruptEnable: 1; //TIM_DIER_CC4IE    
    /* 0x00000020 */ uint32_t COM_InterruptEnable: 1; //TIM_DIER_COMIE    
    /* 0x00000040 */ uint32_t TriggerInterruptEnable: 1; //TIM_DIER_TIE
    /* 0x00000080 */ uint32_t BreakInterruptEnable: 1; //TIM_DIER_BIE
    
    /* 0x00000100 */ uint32_t DMA_RequestEnable: 1; //TIM_DIER_UDE
    /* 0x00000200 */ uint32_t DMA_RequestCompare1Enable: 1; //TIM_DIER_CC1DE
    /* 0x00000400 */ uint32_t DMA_RequestCompare2Enable: 1; //TIM_DIER_CC2DE
    /* 0x00000800 */ uint32_t DMA_RequestCompare3Enable: 1; //TIM_DIER_CC3DE
	
    /* 0x00001000 */ uint32_t DMA_RequestCompare4Enable: 1; //TIM_DIER_CC4DE
    /* 0x00000001 */ uint32_t COM_DMA_RequestEnable: 1; //TIM_DIER_COMDE
    /* 0x00000001 */ uint32_t Trigger_DMA_RequestEnable: 1; //TIM_DIER_TDE
  };
  
  uint32_t Value;
  
} REG_TIM_DIER_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t UpdateInterrupt: 1; //TIM_SR_UIF
    /* 0x00000002 */ uint32_t Compare1Interrupt: 1; //TIM_SR_CC1IF
    /* 0x00000004 */ uint32_t Compare2Interrupt: 1; //TIM_SR_CC2IF
    /* 0x00000008 */ uint32_t Compare3Interrupt: 1; //TIM_SR_CC3IF
	
    /* 0x00000010 */ uint32_t Compare4Interrupt: 1; //TIM_SR_CC4IF    
    /* 0x00000020 */ uint32_t COM_Interrupt: 1; //TIM_SR_COMIF
    /* 0x00000040 */ uint32_t TriggerInterrupt: 1; //TIM_SR_TIF
    /* 0x00000080 */ uint32_t BreakInterrupt: 1; //TIM_SR_BIF
	
    /* 0x00000100 */ uint32_t Free_8: 1; //Free_8
    /* 0x00000200 */ uint32_t Compare1Overcapture: 1; //TIM_SR_CC1OF	
    /* 0x00000400 */ uint32_t Compare2Overcapture: 1; //TIM_SR_CC2OF
    /* 0x00000800 */ uint32_t Compare3Overcapture: 1; //TIM_SR_CC3OF
	
    /* 0x00000001 */ uint32_t Compare4Overcapture: 1; //TIM_SR_CC4OF
  };
  
  uint32_t Value;
  
} REG_TIM_SR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t UpdateGeneration: 1; //TIM_EGR_UG    
    /* 0x00000002 */ uint32_t Compare1Generation : 1; //TIM_EGR_CC1G
    /* 0x00000004 */ uint32_t Compare2Generation : 1; //TIM_EGR_CC2G
    /* 0x00000008 */ uint32_t Compare3Generation: 1; //TIM_EGR_CC3G
	
    /* 0x00000010 */ uint32_t Compare4Generation: 1; //TIM_EGR_CC4G    
    /* 0x00000020 */ uint32_t ControlUpdateGeneration : 1; //TIM_EGR_COMG
    /* 0x00000040 */ uint32_t TriggerGeneration : 1; //TIM_EGR_TG
    /* 0x00000080 */ uint32_t BreakGeneration: 1; //TIM_EGR_BG
  };
  
  uint32_t Value;
  
} REG_TIM_EGR_T;
//==============================================================================
typedef union
{
    struct
	{
      /* 0x00000001 */ uint32_t Compare1Selection: 2; //TIM_CCMR1_CC1S
      /* 0x00000004 */ uint32_t Compare1FastEnable: 1; //TIM_CCMR1_OC1FE
      /* 0x00000008 */ uint32_t Compare1PreloadEnable: 1; //TIM_CCMR1_OC1PE
      
      /* 0x00000010 */ uint32_t Compare1OutputMode: 3; //TIM_CCMR1_OC1M
      /* 0x00000080 */ uint32_t Compare1ClearEnable: 1; //TIM_CCMR1_OC1CE
      
      /* 0x00000100 */ uint32_t Compare2Selection: 2; //TIM_CCMR1_CC2S
      /* 0x00000400 */ uint32_t Compare2FastEnable: 1; //TIM_CCMR1_OC2FE
      /* 0x00000800 */ uint32_t Compare2PreloadEnable: 1; //TIM_CCMR1_OC2PE
      
      /* 0x00001000 */ uint32_t Compare2OutputMode: 3; //TIM_CCMR1_OC2M
      /* 0x00008000 */ uint32_t Compare2ClearEnable: 1; //TIM_CCMR1_OC2CE
    } Out;
	
    struct
	{
      /* 0x00000001 */ uint32_t Free_0_1: 2; //Free_0_1
      /* 0x00000004 */ uint32_t Capture1Prescaler: 2; //TIM_CCMR1_IC1PSC
      
      /* 0x00000010 */ uint32_t Capture1Filter: 4; //TIM_CCMR1_IC1F
      
      /* 0x00000100 */ uint32_t Free_8_9: 2; //Free_8_9
      /* 0x00000400 */ uint32_t Capture2Prescaler: 2; //TIM_CCMR1_IC2PSC
      
      /* 0x00001000 */ uint32_t Capture2Filter: 4; //TIM_CCMR1_IC2F
    } In;
	
    uint32_t Value;
  
} REG_TIM_CCMR1_T;
//==============================================================================
typedef union
{
    struct
	{
      /* 0x00000001 */ uint32_t Compare3Selection: 2; //TIM_CCMR2_CC3S
      /* 0x00000004 */ uint32_t Compare3FastEnable: 1; //TIM_CCMR2_OC3FE
      /* 0x00000008 */ uint32_t Compare3PreloadEnable: 1; //TIM_CCMR2_OC3PE
      
      /* 0x00000010 */ uint32_t Compare3OutputMode: 3; //TIM_CCMR2_OC3M
      /* 0x00000080 */ uint32_t Compare3ClearEnable: 1; //TIM_CCMR2_OC3CE
      
      /* 0x00000100 */ uint32_t Compare4Selection: 2; //TIM_CCMR2_CC4S
      /* 0x00000400 */ uint32_t Compare4FastEnable: 1; //TIM_CCMR2_OC4FE
      /* 0x00000800 */ uint32_t Compare4PreloadEnable: 1; //TIM_CCMR2_OC4PE
      
      /* 0x00001000 */ uint32_t Compare4OutputMode: 3; //TIM_CCMR2_OC4M
      /* 0x00008000 */ uint32_t Compare4ClearEnable: 1; //TIM_CCMR2_OC4CE
    } Out;
	
    struct
	{
      /* 0x00000001 */ uint32_t Free_0_1: 2; //Free_0_1
      /* 0x00000004 */ uint32_t Capture3Prescaler: 2; //TIM_CCMR2_IC3PSC
      
      /* 0x00000010 */ uint32_t Capture3Filter: 4; //TIM_CCMR2_IC3F
      
      /* 0x00000100 */ uint32_t Free_8_9: 2; //Free_8_9
      /* 0x00000400 */ uint32_t Capture4Prescaler: 2; //TIM_CCMR2_IC4PSC
      
      /* 0x00001000 */ uint32_t Capture4Filter: 4; //TIM_CCMR2_IC4F
    } In;
	
    uint32_t Value;
	
} REG_TIM_CCMR2_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t Compare1OutputEnable: 1; //TIM_CCER_CC1E
    /* 0x00000002 */ uint32_t Compare1Polarity : 1; //TIM_CCER_CC1P
    /* 0x00000004 */ uint32_t Compare1NOutputEnable : 1; //TIM_CCER_CC1NE
    /* 0x00000008 */ uint32_t Compare1NPolarity: 1; //TIM_CCER_CC1NP
    
    /* 0x00000010 */ uint32_t Compare2OutputEnable: 1; //TIM_CCER_CC2E
    /* 0x00000020 */ uint32_t Compare2Polarity : 1; //TIM_CCER_CC2P
    /* 0x00000040 */ uint32_t Compare2NOutputEnable : 1; //TIM_CCER_CC2NE
    /* 0x00000080 */ uint32_t Compare2NPolarity: 1; //TIM_CCER_CC2NP
    
    /* 0x00000100 */ uint32_t Compare3OutputEnable: 1; //TIM_CCER_CC3E
    /* 0x00000200 */ uint32_t Compare3Polarity : 1; //TIM_CCER_CC3P
    /* 0x00000400 */ uint32_t Compare3NOutputEnable : 1; //TIM_CCER_CC3NE
    /* 0x00000800 */ uint32_t Compare3NPolarity: 1; //TIM_CCER_CC3NP
    
    /* 0x00001000 */ uint32_t Compare4OutputEnable: 1; //TIM_CCER_CC4E
    /* 0x00002000 */ uint32_t Compare4Polarity : 1; //TIM_CCER_CC4P
    /* 0x00004000 */ uint32_t Free_14 : 1; //Free_14
    /* 0x00008000 */ uint32_t Compare4NPolarity: 1; //TIM_CCER_CC4NP
  };
  
  uint32_t Value;
  
} REG_TIM_CCER_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t Counter: 32; //TIM_CNT_CNT
  };
  
  uint32_t Value;
  
} REG_TIM_CNT_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t Prescaler: 16; //TIM_PSC_PSC
  };
  
  uint32_t Value;
  
} REG_TIM_PSC_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t Period: 16; //TIM_ARR_ARR
  };
  
  uint32_t Value;
  
} REG_TIM_ARR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t DeadTimeGenerator: 8; //TIM_BDTR_DTG
    
    /* 0x00000100 */ uint32_t LockConfiguration: 2; //TIM_BDTR_LOCK
    /* 0x00000400 */ uint32_t OffStateSelectionIdle : 1; //TIM_BDTR_OSSI
    /* 0x00000800 */ uint32_t OffStateSelectionRun: 1; //TIM_BDTR_OSSR
    
    /* 0x00001000 */ uint32_t BreakEnable: 1; //TIM_BDTR_BKE
    /* 0x00002000 */ uint32_t BreakPolarity : 1; //TIM_BDTR_BKP
    /* 0x00004000 */ uint32_t AutomaticOutputEnable : 1; //TIM_BDTR_AOE
    /* 0x00008000 */ uint32_t MainOutputEnable: 1; //TIM_BDTR_MOE
  };
  
  uint32_t Value;
  
} REG_TIM_BDTR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t DMA_BaseAddress: 4; //TIM_DCR_DBA
    
    /* 0x00000010 */ uint32_t DMA_BurstLength: 4; //TIM_DCR_DBL
  };
  
  uint32_t Value;
  
} REG_TIM_DCR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t DMA_BurstAccesses: 8; //TIM_DMAR_DMAB
  };
  
  uint32_t Value;
  
} REG_TIM_DMAR_T;
//==============================================================================
typedef union
{
  struct
  {
    /* 0x00000001 */ uint32_t TIM11_InputCapture1Remap: 2; //TIM_OR_TI1_RMP
	
    /* 0x00000004 */ uint32_t Free_2_5 : 4; //Free_1_5
	
    /* 0x00000040 */ uint32_t TIM5_Input4Remap : 2; //TIM_OR_TI4_RMP
    
    /* 0x00000100 */ uint32_t Free_8_10: 2; //Free_8_10
    /* 0x00000400 */ uint32_t TIM2_InternalTrigger1Remap : 2; //TIM_OR_ITR1_RMP
  };
  
  uint32_t Value;
  
} REG_TIM_OR_T;
//==============================================================================
typedef struct
{
  volatile REG_TIM_CR1_T Control1; //control register 1
  volatile REG_TIM_CR2_T Control2; //control register 2
  volatile REG_TIM_SMCR_T SlaveModeControl; //slave mode control register
  volatile REG_TIM_DIER_T DMAOrInterrupts; //DMA/interrupt enable register
  
  volatile REG_TIM_SR_T Status; //status register
  volatile REG_TIM_EGR_T EventGeneration; //event generation register
  
  volatile REG_TIM_CCMR1_T CaptureCompareMode1; //capture/compare mode register 1
  volatile REG_TIM_CCMR2_T CaptureCompareMode2; //capture/compare mode register 2
  volatile REG_TIM_CCER_T CaptureCompare; //capture/compare enable register
  
  volatile uint32_t Counter; //CNT counter register
  volatile uint32_t Prescaler; //PSC prescaler
  volatile uint32_t Period; //ARR auto-reload register
  volatile uint32_t RepetitionCounter; //RCR repetition counter register
  
  volatile uint32_t CaptureCompare1Value; //CCR1 capture/compare register 1
  volatile uint32_t CaptureCompare2Value; //CCR2 capture/compare register 2
  volatile uint32_t CaptureCompare3Value; //CCR3 capture/compare register 3
  volatile uint32_t CaptureCompare4Value; //CCR4 capture/compare register 4
  
  volatile REG_TIM_BDTR_T BreakAndDeadTime; //break and dead-time register
  volatile REG_TIM_DCR_T DMA_Control; //DMA control register
  volatile uint32_t DMA_Address; //DMA address for full transfer
  
  volatile REG_TIM_OR_T Option; //option register
  
} REG_TIM_T;
//==============================================================================
#endif //REGISTERS_STM32F4XX_ENABLE
#endif //REGISTERS_STM32F4XX_TIMER_H
