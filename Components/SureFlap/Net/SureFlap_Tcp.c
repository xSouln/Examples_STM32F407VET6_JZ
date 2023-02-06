//==============================================================================
#include "SureFlap/SureFlap_ComponentConfig.h"
#ifdef SUREFLAP_COMPONENT_ENABLE
//==============================================================================
//includes:

#include "SureFlap_Tcp.h"
//==============================================================================
//variables:

//==============================================================================
//functions:

void _SureFlapTcpHandler(SureFlapTcpT* tcp)
{

}
//------------------------------------------------------------------------------

void _SureFlapTcpTimeSynchronization(SureFlapTcpT* tcp)
{

}
//------------------------------------------------------------------------------

xResult _SureFlapTcpAddToServerBuffer(SureFlapTcpT* tcp, uint64_t mac, void* data, uint16_t size)
{
	SureFlapTcpMessageT* message = 0;

	for(int i = 0; i < SUREFLAP_SERVER_BUFFER_ENTRIES; i++)
	{
		if(!tcp->MessagesFromDevices[i].InProcessing)
		{
			message = &tcp->MessagesFromDevices[i];
		}
	}

	if(message)
	{
		message->EntryTimestamp = SureFlapGetTime();
		message->MAC = mac;
		memcpy(message->Data, data, size);

		return xResultAccept;
	}

	return xResultBusy;
}
//==============================================================================
//initialization:

xResult _SureFlapTcpInit(SureFlapT* hub)
{
	if (hub)
	{
		hub->Tcp.Object.Description = nameof(SureFlapTcpT);
		hub->Tcp.Object.Parent = hub;

		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
#endif //SUREFLAP_COMPONENT_ENABLE

