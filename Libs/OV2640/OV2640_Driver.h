//==============================================================================
#ifndef _OV2640_DRIVER_H
#define _OV2640_DRIVER_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
 extern "C" {
#endif 
//==============================================================================
#include "Common/xTypes.h"
#include "OV2640_Registers.h"
#include "OV2640_InitializationTables.h"
//==============================================================================
#define OV2640_I2C_ADDRESS				0x60
#define OV2640_I2C_ACTION_WRITE		OV2640_I2C_ADDRESS
#define OV2640_I2C_ACTION_READ		(OV2640_I2C_ADDRESS | 0x01)
#define OV2640_ID									0x26
//==============================================================================
/*
typedef enum
{
	OV2640_ResultAccept = 0U,
	OV2640_ResultError,
	OV2640_ResultParametersError,
	OV2640_ResultNotInit,
	OV2640_ResultBusy,
	OV2640_ResultNotSupported,
	OV2640_ResultValueIsNotFound,
	OV2640_ResultRequestIsNotFound,
	OV2640_ResultTimeOut
	
} OV2640_Result;
*/
//==============================================================================
typedef enum
{
	OV2640_SaturationLevel_0,
	
	OV2640_SaturationLevel_P1,
	OV2640_SaturationLevel_P2,
	
	OV2640_SaturationLevel_M1,
	OV2640_SaturationLevel_M2,
	
	OV2640_Saturation_Mask,
	
} OV2640_Saturations;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_ContrastLevel_0,
	
	OV2640_ContrastLevel_P1,
	OV2640_ContrastLevel_P2,
	
	OV2640_ContrastLevel_M1,
	OV2640_ContrastLevel_M2,
	
	OV2640_Contrast_Mask,
	
} OV2640_Contrasts;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_BrightnessLevel_0,
	
	OV2640_BrightnessLevel_P1,
	OV2640_BrightnessLevel_P2,
	
	OV2640_BrightnessLevel_M1,
	OV2640_BrightnessLevel_M2,
	
	OV2640_Brightness_Mask,
	
} OV2640_Brightnesses;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_LightMode_Auto,
	OV2640_LightMode_Sunny,
	OV2640_LightMode_Cloudy,
	OV2640_LightMode_Office,
	OV2640_LightMode_Home,
	
} OV2640_LightModes;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_SpecialEffect_Normal,
	
	OV2640_SpecialEffect_Black,
	OV2640_SpecialEffect_Negative,
	OV2640_SpecialEffect_BlackNegative,
	
	OV2640_SpecialEffect_Bluish,
	OV2640_SpecialEffect_Greenish,
	OV2640_SpecialEffect_Reddish,
	
	OV2640_SpecialEffect_Antique,
	
} OV2640_SpecialEffects;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_Quantization_Default = 0x0C,
	
	OV2640_Quantization_Mask = 0x3F,
	
} OV2640_Quantizations;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_Resolution_160x120,
	OV2640_Resolution_320x240,
	OV2640_Resolution_640x480,
	OV2640_Resolution_800x600,
	OV2640_Resolution_1024x768,
	OV2640_Resolution_1280x960,
	
} OV2640_Resolutions;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_OutputFormat_JPEG,
	OV2640_OutputFormat_YUV422,
	OV2640_OutputFormat_RGB565,
	OV2640_OutputFormat_RAW,
	
} OV2640_OutputFormats;
//==============================================================================
typedef enum
{
	OV2640_StateDisable,
	OV2640_StateEnable
	
} OV2640_DriverState;
//==============================================================================
typedef enum
{
	OV2640_EventX = 1U,
	
} OV2640_EventSelector;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_RequestI2C_Write = 1U,
	OV2640_RequestI2C_Read,
	
	OV2640_RequestDelay,
	
	OV2640_RequestDCMI_Start,
	OV2640_RequestDCMI_Stop,
	OV2640_RequestDCMI_Suspend,
	
} OV2640_RequestSelector;
//------------------------------------------------------------------------------
typedef enum
{
	OV2640_ValueHardwareResetState = 1U,
	OV2640_ValuePowerDownState
	
} OV2640_ValueSelector;
//------------------------------------------------------------------------------
typedef void (*OV2640_HandlerT)(void* camera);

typedef xResult (*OV2640_RequestListenerT)(void* camera, OV2640_RequestSelector selector, uint32_t args, uint32_t count);

typedef int (*OV2640_ActionGetValueT)(void* camera, OV2640_ValueSelector selector);
typedef xResult (*OV2640_ActionSetValueT)(void* camera, OV2640_ValueSelector selector, uint32_t value);
//------------------------------------------------------------------------------
typedef struct
{
	uint8_t Address;
	uint8_t Register;
	uint8_t Value;
	
} OV2640_RequestI2C_WriteT;
//------------------------------------------------------------------------------
typedef struct
{
	uint8_t Address;
	uint8_t Register;
	uint8_t* Value;
	
} OV2640_RequestI2C_ReadT;
//------------------------------------------------------------------------------
typedef struct
{
	OV2640_HandlerT Handler;
	
	OV2640_RequestListenerT RequestListener;
	
	OV2640_ActionGetValueT GetValue;
	OV2640_ActionSetValueT SetValue;
	
} OV2640_InterfaceT;
//------------------------------------------------------------------------------
typedef struct
{
	OV2640_OutputFormats OutputFormat;
	OV2640_Resolutions Resolution;
	
	OV2640_Contrasts Contrast;
	OV2640_Saturations Saturation;
	OV2640_Brightnesses Brightness;
	
	OV2640_LightModes LightMode;
	OV2640_SpecialEffects SpecialEffect;
	OV2640_Quantizations Quantization;
	
	uint8_t Gain;
	
} OV2640_DriverOptionsT;
//------------------------------------------------------------------------------
typedef union
{
	struct
	{
		uint32_t IsInit : 1;
		uint32_t DeviceIsPresent : 1;
		uint32_t IsCapture : 1;
		uint32_t Busy : 1;
		
		uint32_t InitResult : 4;
	};
	uint32_t Value;
	
} OV2640_DriverStatusT;
//------------------------------------------------------------------------------
typedef struct
{
	uint32_t NumberOfInitializationErrors;
	
} OV2640_DriverDebugT;
//------------------------------------------------------------------------------
typedef struct
{
	char* Description;
	void* Parent;
	
	void* Adapter;
	OV2640_InterfaceT* Interface;
	
	OV2640_DriverStatusT Status;
	OV2640_DriverOptionsT Options;
	
	uint8_t ID;
	
	OV2640_DriverDebugT Debug;
	
} OV2640_DriverT;
//==============================================================================
xResult OV2640_DriverI2C_Write(OV2640_DriverT* driver, uint8_t reg, uint8_t data);
xResult OV2640_DriverI2C_Read(OV2640_DriverT* driver, uint8_t reg, uint8_t* data);
xResult OV2640_DriverReset(OV2640_DriverT* driver);

xResult OV2640_SetPowerDownState(OV2640_DriverT* driver, OV2640_DriverState state);
xResult OV2640_SetHardwareResetState(OV2640_DriverT* driver, OV2640_DriverState state);
xResult OV2640_Delay(OV2640_DriverT* driver, uint32_t ms);

xResult OV2640_DriverSetConfiguration(OV2640_DriverT* driver, uint8_t* data, uint16_t data_size);

xResult OV2640_DriverSetOutputFormat(OV2640_DriverT* driver, OV2640_OutputFormats output_format);
xResult OV2640_DriverSetResolution(OV2640_DriverT* driver, OV2640_Resolutions resolution);

xResult OV2640_DriverSetSaturation(OV2640_DriverT* driver, OV2640_Saturations saturation);
xResult OV2640_DriverSetContrast(OV2640_DriverT* driver, OV2640_Contrasts contrast);
xResult OV2640_DriverSetBrightness(OV2640_DriverT* driver, OV2640_Brightnesses brightness);

xResult OV2640_DriverSetLightMode(OV2640_DriverT* driver, OV2640_LightModes light_mode);
xResult OV2640_DriverSetSpecialEffect(OV2640_DriverT* driver, OV2640_SpecialEffects special_effect);
xResult OV2640_DriverSetQuantization(OV2640_DriverT* driver, OV2640_Quantizations quantization);

xResult OV2640_DriverSetGain(OV2640_DriverT* driver, uint8_t gain);

xResult OV2640_DriverSetOptions(OV2640_DriverT* driver, OV2640_DriverOptionsT* options);

xResult OV2640_DriverInit(OV2640_DriverT* driver,
													void* parent,
													void* adapter,
													OV2640_InterfaceT* interface,
													OV2640_DriverOptionsT* options);
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* _OV2640_DRIVER_H */

