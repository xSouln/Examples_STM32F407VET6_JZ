//==============================================================================
#include "xRxReceiver.h"
//==============================================================================
void xRxReceiverReceive(xRxReceiverT* receiver, uint8_t *data, uint32_t data_size)
{
	for(uint16_t i = 0; i < data_size; i++)
  {
    receiver->Buffer[receiver->BytesReceived] = data[i];
    receiver->BytesReceived++;
    
    if(receiver->BytesReceived >= receiver->BufferSize)
    {
      receiver->Interface->EventListener(receiver,
																				xRxReceiverEventBufferIsFull,
																				(uint32_t)receiver->Buffer,
																				receiver->BytesReceived);
			receiver->BytesReceived = 0;
    }    
    else if(data[i] == '\r')
    {
			receiver->Interface->EventListener(receiver,
																				xRxReceiverEventEndLine,
																				(uint32_t)receiver->Buffer,
																				receiver->BytesReceived - 1);
    }
  }
}
//==============================================================================
void xRxReceiverRead(xRxReceiverT* receiver, xCircleBufferT* circle_buffer)
{
	while (circle_buffer->HandlerIndex != circle_buffer->TotalIndex)
	{
		uint8_t byte = circle_buffer->Buffer[circle_buffer-> HandlerIndex];
		circle_buffer->HandlerIndex++;
		circle_buffer->HandlerIndex &= circle_buffer->SizeMask;
		
		receiver->Buffer[receiver->BytesReceived] = byte;
		receiver->BytesReceived++;
		
		if(receiver->BytesReceived >= receiver->BufferSize)
    {
      receiver->Interface->EventListener(receiver,
																				xRxReceiverEventBufferIsFull,
																				(uint32_t)receiver->Buffer,
																				receiver->BytesReceived);
			receiver->BytesReceived = 0;
    }    
    else if (byte == '\r')
    {
			receiver->Interface->EventListener(receiver,
																				xRxReceiverEventEndLine,
																				(uint32_t)receiver->Buffer,
																				receiver->BytesReceived - 1);
    }
	}
}
//------------------------------------------------------------------------------
int xRxReceiverInit(xRxReceiverT* receiver,
										xRxT* parent,
										xRxReceiverInterfaceT* interface,
										uint8_t* buffer,
										uint32_t buffer_size)
{
	if (receiver)
	{
		receiver->Description = "xRxReceiverT";
		receiver->Parent = parent;
		receiver->Interface = interface;
		receiver->Buffer = buffer;
		receiver->BufferSize = buffer_size;
		
		receiver->Status.IsInit = true;
		
		return 0;
	}
	
	return -1;
}
//==============================================================================
