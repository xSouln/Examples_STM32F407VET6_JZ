//==============================================================================
//header:

#include "MqttClient-Serializer.h"

#ifdef _MQTT_CLIENT_SERIALIZERS_H_
//==============================================================================
//includes:

#include "Components/MemoryControl/Flash/FlashMemoryControl-Component.h"
#include "Common/xMemory.h"
#include "MqttClient/Adapters/Ports/FreeRTOS-MQTT/MqttPort-Adapter.h"
//==============================================================================
//defines:


//==============================================================================
//types:

typedef struct
{
	uint16_t SerializerType;
	uint16_t Flags;

	uint16_t SizeOfParameters;

} HeaderT;
//==============================================================================
//functions:

int SerializeMqttClientPortOptions(xPortT* port, xDataBufferT* buffer, uint32_t* reservedMemerySize)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;
	HeaderT* header = xDataBufferGetTotalPointer(buffer);
	header->SerializerType = 0;
	header->Flags = 0;

	xDataBufferOffset(buffer, sizeof(HeaderT));

	uint16_t offset = buffer->Length;

	xDataBufferAdd(buffer, &adapter->NetAddress, sizeof(adapter->NetAddress));
	xDataBufferAdd(buffer, &adapter->NetPort, sizeof(adapter->NetPort));

	xDataBufferAdd(buffer, adapter->ClientId, sizeof(adapter->ClientId));
	xDataBufferAdd(buffer, adapter->RxTopic, sizeof(adapter->RxTopic));
	xDataBufferAdd(buffer, adapter->TxTopic, sizeof(adapter->TxTopic));

	header->SizeOfParameters = buffer->Length - offset;

	return 0;
}
//------------------------------------------------------------------------------

int DeserializeMqttClientPortOptions(xPortT* port, xMemoryReaderT* reader)
{
	MqttPortAdapterT* adapter = (MqttPortAdapterT*)port->Adapter.Content;
	HeaderT* header = xMemoryReaderGetTotalPointer(reader);

	xMemoryReaderOffset(reader, sizeof(HeaderT));

	if (xMemoryReaderGetRemainLength(reader) < header->SizeOfParameters)
	{
		return -1;
	}

	if (header->SerializerType == 0)
	{
		xMemoryReaderRead(reader, &adapter->NetAddress, sizeof(adapter->NetAddress));
		xMemoryReaderRead(reader, &adapter->NetPort, sizeof(adapter->NetPort));

		xMemoryReaderRead(reader, adapter->ClientId, sizeof(adapter->ClientId));
		xMemoryReaderRead(reader, adapter->RxTopic, sizeof(adapter->RxTopic));
		xMemoryReaderRead(reader, adapter->TxTopic, sizeof(adapter->TxTopic));
	}

	return 0;
}
//------------------------------------------------------------------------------

xResult MqttSaveObject(void* object, uint32_t signatureType)
{
#if FLASH_MEMORY_CONTROL_COMPONENT_ENABLE == 1
	int16_t result = 0;
	uint32_t reservedMemory = 0;

	xDataBufferT buffer = { 0 };
	buffer.MaxLength = 512;
	buffer.Memory = xMemoryAllocate(1, buffer.MaxLength);

	CHECK_ALLOCATION(buffer.Memory);

	xUnitFileHeaderT fileHeader;
	fileHeader.UID = xMQTT_UID;
	fileHeader.ControlId = 0;
	fileHeader.Signature = signatureType;

	switch (signatureType)
	{
		case 0:
		{
			result = SerializeMqttClientPortOptions(object, &buffer, &reservedMemory);
			break;
		}

		default:
			result = -1;
			break;
	}

	if (result >= 0)
	{
		xMemoryRequestWriteFileT requestWrite;
		requestWrite.Name = &fileHeader;
		requestWrite.NameLength = sizeof(fileHeader);
		requestWrite.ReservedMemory = reservedMemory;
		requestWrite.Content = buffer.Memory;
		requestWrite.ContentSize = buffer.Length;

		xMemoryWriteFile(&CommonFileSpace, &requestWrite);
	}

	xMemoryFree(buffer.Memory);

	return xResultAccept;
#else
	return xResultNotSupported;
#endif
}
//------------------------------------------------------------------------------

xResult MqttOpenObject(void* object, uint32_t signatureType)
{
#if FLASH_MEMORY_CONTROL_COMPONENT_ENABLE == 1
	xUnitFileHeaderT fileHeader;
	fileHeader.UID = xMQTT_UID;
	fileHeader.ControlId = 0;
	fileHeader.Signature = signatureType;

	xMemoryFileHeaderT* out = null;
	xMemoryFindFileInSpace(&CommonFileSpace,
			&fileHeader,
			sizeof(xUnitFileHeaderT),
			&out);

	if (out == null)
	{
		return xResultError;
	}

	xMemoryReaderT memoryReader = { 0 };
	memoryReader.Content = out->Content;
	memoryReader.Length = out->ContentSize;

	switch (signatureType)
	{
		case 0:
			DeserializeMqttClientPortOptions(object, &memoryReader);
			break;

		default: return xResultNotSupported;
	}

	return xResultAccept;
#else
	return xResultNotSupported;
#endif
}
//==============================================================================
#endif //_MQTT_CLIENT_SERIALIZERS_H_
