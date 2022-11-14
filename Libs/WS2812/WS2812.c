//==============================================================================
#include <string.h>
#include <stdbool.h>
#include "WS2812.h"
//==============================================================================
uint16_t WS2812_PutColorToArray(WS2812_BufT* buffer, uint16_t period, uint8_t color)
{
	uint16_t offset = 0;
	
	for (uint8_t bit_number = 0; bit_number < sizeof(color) * 8; bit_number++)
	{
		if (color & 0x80)
		{
			buffer[offset] = (WS2812_BufT)((float)(period + 1) * 0.85 / 1.25);
		}
		else
		{
			buffer[offset] = (WS2812_BufT)((float)(period + 1) * 0.4 / 1.25);
		}
		
		color <<= 1;
		offset++;
	}
	
	return offset;
}
//------------------------------------------------------------------------------
uint16_t WS2812_PutPixelsToArray(WS2812_BufT* buffer, uint16_t period, WS2812_ColorT* pixels, int pixels_count, WS2812_PixelAddMode mode)
{
	uint16_t size = 0;
	
	for (uint16_t i = 0; i < pixels_count; i++)
	{
		size += WS2812_PutColorToArray(buffer + size, period, pixels[i & mode].Green);
		size += WS2812_PutColorToArray(buffer + size, period, pixels[i & mode].Red);
		size += WS2812_PutColorToArray(buffer + size, period, pixels[i & mode].Blue);
	}
	
	return size;
}
//------------------------------------------------------------------------------
uint16_t WS2812_FillPixels(WS2812_T* driver, WS2812_ColorT pixel, uint16_t start_position, uint16_t pixels_count)
{
	if (driver && driver->Status.DriverInit)
	{
		uint16_t buffer_position = start_position * WS2812_BITS_IN_PIXEL;
		
		if ((buffer_position + (pixels_count * WS2812_BITS_IN_PIXEL)) <= driver->BufferSize)
		{
			return WS2812_PutPixelsToArray(driver->Buffer + buffer_position,
																			driver->Interface->GetValue(driver, WS2812_ValuePeriod),
																			&pixel,
																			pixels_count,
																			WS2812_PixelAddModeFill);
		}
	}
	return 0;
}
//------------------------------------------------------------------------------
uint16_t WS2812_SetPixels(WS2812_T* driver, WS2812_ColorT* color, uint16_t start_position, uint16_t pixels_count)
{
	if (driver && driver->Status.DriverInit)
	{
		uint16_t buffer_position = start_position * WS2812_BITS_IN_PIXEL;
		
		if ((buffer_position + (WS2812_BITS_IN_PIXEL * pixels_count)) <= driver->BufferSize)
		{
			return WS2812_PutPixelsToArray(driver->Buffer + buffer_position,
																			driver->Interface->GetValue(driver, WS2812_ValuePeriod),
																			color,
																			pixels_count,
																			WS2812_PixelAddModePut);
		}
	}
	
	return 0;
}
//------------------------------------------------------------------------------
uint16_t WS2812_SetPixel(WS2812_T* driver, WS2812_ColorT color, uint16_t position)
{
	return WS2812_SetPixels(driver, &color, position, 1);
}
//------------------------------------------------------------------------------
WS2812_TransmitterStatus WS2812_GetTransmitterStatus(WS2812_T* driver)
{
	driver->Status.Transmitter = driver->Interface->GetValue(driver, WS2812_ValueTransmitterStatus);
	return driver->Status.Transmitter;
}
//------------------------------------------------------------------------------
WS2812_Result WS2812_UpdateLayout(WS2812_T* driver)
{
	xResult result = xResultBusy;

	driver->Status.Transmitter = driver->Interface->GetValue(driver, WS2812_ValueTransmitterStatus);

	if (driver->Status.Transmitter == WS2812_TransmitterStopped)
	{
		result = driver->Interface->RequestListener(driver, WS2812_RequestTransmitterEnable, 0, 0);
	}
	
	return result;
}
//------------------------------------------------------------------------------
void WS2812_TimeSynchronization(WS2812_T* driver)
{
	if (driver->DrawManager.Status.DrawingIsEnable && driver->DrawManager.UpdateTime)
	{
		driver->DrawManager.UpdateTime--;
	}
}
//==============================================================================
void WS2812_DrawManagerHandler(WS2812_T* driver)
{
	if (driver->DrawManager.Status.DrawingIsEnable)
	{
		//driver->DrawManager.UpdateTime = driver->DrawManager.Options.UpdateTime;
		
		driver->DrawManager.Handler(&driver->DrawManager);
		driver->DrawManager.Status.IsUpdate = true;

		WS2812_UpdateLayout(driver);
	}
}
//------------------------------------------------------------------------------
xResult WS2812_DrawingStart(WS2812_T* driver, void* template_request)
{
	xResult result = driver->DrawManager.RequestListener(&driver->DrawManager, WS2812_DrawManagerRequestDrawingStart, (uint32_t)template_request, 0);

	driver->DrawManager.Status.DrawingIsEnable = result == xResultAccept;
	return result;
}
//------------------------------------------------------------------------------
void WS2812_DrawingStop(WS2812_T* driver)
{
	driver->DrawManager.Status.DrawingIsEnable = false;
	driver->DrawManager.RequestListener(&driver->DrawManager, WS2812_DrawManagerRequestDrawingStop, 0, 0);
}
//------------------------------------------------------------------------------
xResult WS2812_DrawManagerSetTemplate(WS2812_T* driver, void* value)
{
	return driver->DrawManager.RequestListener(&driver->DrawManager, WS2812_DrawManagerRequestSetTemplate, (uint32_t)value, 0);
}
//==============================================================================
WS2812_Result WS2812_Init(WS2812_T* driver,
													void* parent,
													WS2812_AdapterT* adapter,
													WS2812_InterfaceT* interface,
													WS2812_BufT* buffer,
													uint16_t buffer_size)
{
	if (driver)
	{
		driver->Description = "WS2812_T";
		driver->Parent = parent;
		
		driver->Adapter = adapter;
		driver->Interface = interface;
		
		driver->Buffer = buffer;
		driver->BufferSize = buffer_size;
		
		driver->Status.DriverInit = true;

		WS2812_DrawManagerBaseInit(&driver->DrawManager, driver);
	}
	
	return WS2812_ResultError;
}
//==============================================================================
