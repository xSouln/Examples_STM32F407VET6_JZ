//==============================================================================
//header:

#include "NetPort-Serializer.h"

#ifdef _NET_PORT_SERIALIZERS_H_
//==============================================================================
//includes:

#include "Components/MemoryControl/Flash/FlashMemoryControl-Component.h"
#include "Common/xMemory.h"
#include "Net/Serializers/NetPort-Serializer.h"
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

int SerializePortOptions(xNetT* net, xDataBufferT* buffer, uint32_t* reservedMemerySize)
{
#if FLASH_MEMORY_CONTROL_COMPONENT_ENABLE == 1
	/*
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

	header->SizeOfParameters = buffer->Length - offset;*/

	return 0;
#else
	return xResultNotSupported;
#endif
}
//------------------------------------------------------------------------------

int DeserializeNetOptions(xPortT* port, xMemoryReaderT* reader)
{
#if FLASH_MEMORY_CONTROL_COMPONENT_ENABLE == 1
	/*
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
	}*/

	return 0;
#else
	return xResultNotSupported;
#endif
}
//------------------------------------------------------------------------------

xResult NetSaveObject(void* object, uint32_t signatureType)
{
#if FLASH_MEMORY_CONTROL_COMPONENT_ENABLE == 1
	int16_t result = 0;
	uint32_t reservedMemory = 0;

	xDataBufferT buffer = { 0 };
	buffer.MaxLength = 512;
	buffer.Memory = xMemoryAllocate(1, buffer.MaxLength);

	CHECK_ALLOCATION(buffer.Memory);

	xUnitFileHeaderT fileHeader;
	fileHeader.UID = xNET_UID;
	fileHeader.ControlId = 0;
	fileHeader.Signature = signatureType;

	switch (signatureType)
	{
		case 0:
		{
			result = SerializePortOptions(object, &buffer, &reservedMemory);
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

xResult NetOpenObject(void* object, uint32_t signatureType)
{
#if FLASH_MEMORY_CONTROL_COMPONENT_ENABLE == 1
	xUnitFileHeaderT fileHeader;
	fileHeader.UID = xNET_UID;
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
			DeserializeNetOptions(object, &memoryReader);
			break;

		default: return xResultNotSupported;
	}

	return xResultAccept;
#else
	return xResultNotSupported;
#endif
}
//==============================================================================
#endif //_NET_PORT_SERIALIZERS_H_
