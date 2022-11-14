//==============================================================================
#include <string.h>
#include "OV2640_Driver.h"
#include "OV2640_InitializationTables.h"
#include "Common/xMemory.h"
//==============================================================================
xResult OV2640_DriverI2C_Write(OV2640_DriverT* driver, uint8_t reg, uint8_t data)
{
	if (driver && driver->Status.IsInit)
	{
		OV2640_RequestI2C_WriteT request =
		{
			.Address = OV2640_I2C_ADDRESS,
			.Register = reg,
			.Value = data
		};
		
		return driver->Interface->RequestListener(driver, OV2640_RequestI2C_Write, (uint32_t)&request, 0);
	}
	return xResultError;
}
//------------------------------------------------------------------------------
xResult OV2640_DriverI2C_Read(OV2640_DriverT* driver, uint8_t reg, uint8_t* data)
{
	if (driver && driver->Status.IsInit)
	{
		volatile OV2640_RequestI2C_ReadT request =
		{
			.Address = OV2640_I2C_ADDRESS | 0x01,
			.Register = reg,
			.Value = data
		};
		
		return driver->Interface->RequestListener(driver, OV2640_RequestI2C_Read, (uint32_t)&request, 0);
	}
	return xResultError;
}
//------------------------------------------------------------------------------
xResult OV2640_SetPowerDownState(OV2640_DriverT* driver, OV2640_DriverState state)
{
	if (driver && driver->Status.IsInit)
	{
		return driver->Interface->SetValue(driver, OV2640_ValuePowerDownState, state);
	}
	
	return xResultError;
}
//------------------------------------------------------------------------------
xResult OV2640_SetHardwareResetState(OV2640_DriverT* driver, OV2640_DriverState state)
{
	if (driver && driver->Status.IsInit)
	{
		return driver->Interface->SetValue(driver, OV2640_ValueHardwareResetState, state);
	}
	
	return xResultError;
}
//------------------------------------------------------------------------------
xResult OV2640_Delay(OV2640_DriverT* driver, uint32_t ms)
{
	if (driver && driver->Status.IsInit)
	{
		return driver->Interface->RequestListener(driver, OV2640_RequestDelay, ms, 0);
	}
	
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverReset(OV2640_DriverT* driver)
{
	if (driver && driver->Status.IsInit)
	{
		OV2640_SetPowerDownState(driver, OV2640_StateEnable);
		OV2640_SetHardwareResetState(driver, OV2640_StateEnable);
		OV2640_Delay(driver, 100);
		OV2640_SetPowerDownState(driver, OV2640_StateDisable);
		OV2640_Delay(driver, 5);
		OV2640_SetHardwareResetState(driver, OV2640_StateDisable);
		OV2640_Delay(driver, 100);
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverPowerReset(OV2640_DriverT* driver)
{
	if (driver && driver->Status.IsInit)
	{
		OV2640_SetPowerDownState(driver, OV2640_StateEnable);
		OV2640_Delay(driver, 100);
		OV2640_SetPowerDownState(driver, OV2640_StateDisable);
		OV2640_Delay(driver, 100);
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetConfiguration(OV2640_DriverT* driver, uint8_t* data, uint16_t data_size)
{
	//uint8_t reg_data;
	if (driver && data)
	{
		uint16_t i = 0;
		while (i < data_size)
		{
			xResult result_write = OV2640_DriverI2C_Write(driver, data[i], data[i + 1]);
			i += 2;
			
			if (result_write != xResultAccept)
			{
				driver->Debug.NumberOfInitializationErrors++;
			}
			//driver->Control->Delay(driver, 5);
		}
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetOutputFormat(OV2640_DriverT* driver, OV2640_OutputFormats output_format)
{
	if (driver && driver->Status.IsInit)
	{
		switch ((uint8_t)output_format)
		{
			case OV2640_OutputFormat_JPEG:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_JPEG_INIT, sizeof(OV2640_JPEG_INIT));
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_PIXEL_FORMAT_YUV422, sizeof(OV2640_PIXEL_FORMAT_YUV422));
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_JPEG_OUTPUT, sizeof(OV2640_JPEG_OUTPUT));
				break;
		}
		
		driver->Options.OutputFormat = output_format;
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetResolution(OV2640_DriverT* driver, OV2640_Resolutions resolution)
{
	if (driver && driver->Status.IsInit)
	{
		switch ((uint8_t)resolution)
		{
			case OV2640_Resolution_1280x960:
				OV2640_DriverSetConfiguration(driver,
																(uint8_t*)OV2640_RESOLUTION_1280x960,
																sizeof(OV2640_RESOLUTION_1280x960));
				break;
			
			case OV2640_Resolution_1024x768:
				OV2640_DriverSetConfiguration(driver,
																(uint8_t*)OV2640_RESOLUTION_1024x768,
																sizeof(OV2640_RESOLUTION_1024x768));
				break;
			
			case OV2640_Resolution_800x600:
				OV2640_DriverSetConfiguration(driver,
																(uint8_t*)OV2640_RESOLUTION_800x600,
																sizeof(OV2640_RESOLUTION_800x600));
				break;
			
			case OV2640_Resolution_640x480:
				OV2640_DriverSetConfiguration(driver,
																(uint8_t*)OV2640_RESOLUTION_640x480,
																sizeof(OV2640_RESOLUTION_640x480));
				break;
			
			case OV2640_Resolution_320x240:
				OV2640_DriverSetConfiguration(driver,
																(uint8_t*)OV2640_RESOLUTION_320x240,
																sizeof(OV2640_RESOLUTION_320x240));
				break;
			
			case OV2640_Resolution_160x120:
				OV2640_DriverSetConfiguration(driver,
																(uint8_t*)OV2640_RESOLUTION_160x120,
																sizeof(OV2640_RESOLUTION_160x120));
				break;
				
			default : return xResultError;
		}
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetContrast(OV2640_DriverT* driver, OV2640_Contrasts contrast)
{
	if (driver && driver->Status.IsInit)
	{
		switch ((uint8_t)contrast)
		{
			case OV2640_ContrastLevel_M2:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_CONTRAST_M2, sizeof(OV2640_CONTRAST_M2));
				break;
			
			case OV2640_ContrastLevel_M1:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_CONTRAST_M1, sizeof(OV2640_CONTRAST_M1));
				break;
			
			case OV2640_ContrastLevel_0:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_CONTRAST_0, sizeof(OV2640_CONTRAST_0));
				break;
			
			case OV2640_ContrastLevel_P1:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_CONTRAST_P1, sizeof(OV2640_CONTRAST_P1));
				break;
			
			case OV2640_ContrastLevel_P2:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_CONTRAST_P2, sizeof(OV2640_CONTRAST_P2));
				break;
			
			default : return xResultError;
		}
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetBrightness(OV2640_DriverT* driver, OV2640_Brightnesses brightness)
{
	if (driver && driver->Status.IsInit)
	{
		switch ((uint8_t)brightness)
		{
			case OV2640_BrightnessLevel_M2:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_BRIGHTNESS_M2, sizeof(OV2640_BRIGHTNESS_M2));
				break;
			
			case OV2640_BrightnessLevel_M1:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_BRIGHTNESS_M1, sizeof(OV2640_BRIGHTNESS_M1));
				break;
			
			case OV2640_BrightnessLevel_0:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_BRIGHTNESS_0, sizeof(OV2640_BRIGHTNESS_0));
				break;
			
			case OV2640_BrightnessLevel_P1:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_BRIGHTNESS_P1, sizeof(OV2640_BRIGHTNESS_P1));
				break;
			
			case OV2640_BrightnessLevel_P2:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_BRIGHTNESS_P2, sizeof(OV2640_BRIGHTNESS_P2));
				break;
			
			default : return xResultError;
		}
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetSaturation(OV2640_DriverT* driver, OV2640_Saturations saturation)
{
	if (driver && driver->Status.IsInit)
	{
		switch ((uint8_t)saturation)
		{
			case OV2640_SaturationLevel_M2:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SATURATION_M2, sizeof(OV2640_SATURATION_M2));
				break;
			
			case OV2640_SaturationLevel_M1:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SATURATION_M1, sizeof(OV2640_SATURATION_M1));
				break;
			
			case OV2640_SaturationLevel_0:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SATURATION_0, sizeof(OV2640_SATURATION_0));
				break;
			
			case OV2640_SaturationLevel_P1:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SATURATION_P1, sizeof(OV2640_SATURATION_P1));
				break;
			
			case OV2640_SaturationLevel_P2:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SATURATION_P2, sizeof(OV2640_SATURATION_P2));
				break;
			
			default : return xResultError;
		}
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetLightMode(OV2640_DriverT* driver, OV2640_LightModes light_mode)
{
	if (driver && driver->Status.IsInit)
	{
		switch ((uint8_t)light_mode)
		{
			case OV2640_LightMode_Auto:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_LIGHT_MODE_AUTO, sizeof(OV2640_LIGHT_MODE_AUTO));
				break;
			
			case OV2640_LightMode_Sunny:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_LIGHT_MODE_SUNNY, sizeof(OV2640_LIGHT_MODE_SUNNY));
				break;
			
			case OV2640_LightMode_Cloudy:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_LIGHT_MODE_CLOUDY, sizeof(OV2640_LIGHT_MODE_CLOUDY));
				break;
			
			case OV2640_LightMode_Office:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_LIGHT_MODE_OFFICE, sizeof(OV2640_LIGHT_MODE_OFFICE));
				break;
			
			case OV2640_LightMode_Home:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_LIGHT_MODE_HOME, sizeof(OV2640_LIGHT_MODE_HOME));
				break;
			
			default : return xResultError;
		}
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetSpecialEffect(OV2640_DriverT* driver, OV2640_SpecialEffects special_effect)
{
	if (driver && driver->Status.IsInit)
	{
		switch ((uint8_t)special_effect)
		{
			case OV2640_SpecialEffect_Normal:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_NORMAL, sizeof(OV2640_SPECIAL_EFFECTS_NORMAL));
				break;
			
			case OV2640_SpecialEffect_Black:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_BLACK, sizeof(OV2640_SPECIAL_EFFECTS_BLACK));
				break;
			
			case OV2640_SpecialEffect_Negative:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_NEGATIVE, sizeof(OV2640_SPECIAL_EFFECTS_NEGATIVE));
				break;
			
			case OV2640_SpecialEffect_BlackNegative:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_BLACK_NEGATIVE, sizeof(OV2640_SPECIAL_EFFECTS_BLACK_NEGATIVE));
				break;
			
			case OV2640_SpecialEffect_Bluish:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_BLUISH, sizeof(OV2640_SPECIAL_EFFECTS_BLUISH));
				break;
			
			case OV2640_SpecialEffect_Greenish:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_GREENISH, sizeof(OV2640_SPECIAL_EFFECTS_GREENISH));
				break;
			
			case OV2640_SpecialEffect_Reddish:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_REDDISH, sizeof(OV2640_SPECIAL_EFFECTS_REDDISH));
				break;
			
			case OV2640_SpecialEffect_Antique:
				OV2640_DriverSetConfiguration(driver, (uint8_t*)OV2640_SPECIAL_EFFECTS_ANTIQUE, sizeof(OV2640_SPECIAL_EFFECTS_ANTIQUE));
				break;
			
			default : return xResultError;
		}
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetQuantization(OV2640_DriverT* driver, OV2640_Quantizations quantization)
{
	if (driver && driver->Status.IsInit)
	{
		return OV2640_DriverI2C_Write(driver, OV2640_DSP_Qs, quantization & OV2640_Quantization_Mask);
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetGain(OV2640_DriverT* driver, uint8_t gain)
{
	if (driver && driver->Status.IsInit)
	{
		OV2640_DriverI2C_Write(driver, OV2640_DSP_RA_DLMT, 0x01);
		OV2640_DriverI2C_Write(driver, OV2640_SENSOR_GAIN, gain);
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverSetOptions(OV2640_DriverT* driver, OV2640_DriverOptionsT* options)
{
	if (driver && driver->Status.IsInit && options)
	{
		memcpy((uint8_t*)&driver->Options, (uint8_t*)options, sizeof(OV2640_DriverOptionsT));
		
		/* Prepare the camera to be configured */
		OV2640_DriverI2C_Write(driver, OV2640_DSP_RA_DLMT, 0x01);
		OV2640_DriverI2C_Write(driver, OV2640_SENSOR_COM7, 0x80);
		OV2640_Delay(driver, 100);
		
		OV2640_DriverSetOutputFormat(driver, options->OutputFormat);
		OV2640_DriverSetResolution(driver, options->Resolution);
		
		OV2640_DriverSetGain(driver, options->Gain);
		OV2640_DriverSetLightMode(driver, options->LightMode);
		OV2640_DriverSetSpecialEffect(driver, options->SpecialEffect);
		
		OV2640_DriverSetContrast(driver, options->Contrast);
		OV2640_DriverSetBrightness(driver, options->Brightness);
		OV2640_DriverSetSaturation(driver, options->Saturation);
		
		OV2640_DriverSetQuantization(driver, options->Quantization);
		
		return xResultAccept;
	}
	return xResultError;
}
//==============================================================================
xResult OV2640_DriverInit(OV2640_DriverT* driver,
																void* parent,
																void* adapter,
																OV2640_InterfaceT* interface,
																OV2640_DriverOptionsT* options)
{
	if (!driver)
	{
		return xResultLinkError;
	}
	
	if (!adapter || !interface)
	{
		driver->Status.InitResult = xResultLinkError;
		goto end;
	}
	
	if (xMemoryCheckLincs(interface, sizeof(OV2640_InterfaceT)) != 0)
	{
		driver->Status.InitResult = xResultLinkError;
		goto end;
	}
	
	driver->Description = "OV2640_T";
	driver->Parent = parent;
	driver->Adapter = adapter;
	
	if (options)
	{
		memcpy((uint8_t*)&driver->Options, (uint8_t*)options, sizeof(OV2640_DriverOptionsT));
	}
	
	driver->Interface = interface;
	
	driver->Status.IsInit = true;
	
	OV2640_DriverReset(driver);
	
	memset(&driver->Debug, 0, sizeof(driver->Debug));
	
	/* Prepare the sensor to read the Camera ID */
	OV2640_DriverI2C_Write(driver, OV2640_DSP_RA_DLMT, 0x01);
	OV2640_DriverI2C_Read(driver, OV2640_SENSOR_PIDH, &driver->ID);
	
	OV2640_Delay(driver, 100);
	
	if (driver->ID != OV2640_ID)
	{
		driver->Status.InitResult = xResulComponentInitializationError;
		goto end;
	}
	
	driver->Status.DeviceIsPresent = true;
	
	driver->Status.InitResult = OV2640_DriverSetOptions(driver, &driver->Options);
		
	end:;
	return driver->Status.InitResult;
}
//==============================================================================
