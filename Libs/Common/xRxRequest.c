//==============================================================================
#include <string.h>
#include "xRxRequest.h"
//==============================================================================
xRxRequestT* xRxRequestIdentify(xRxT* rx, void* device, xRxRequestT* requests, uint8_t data[], uint16_t data_size)
{
	xRxRequestManagerT manager;
  uint8_t i = 0;
	
  while(requests[i].Header)
  {
    if(data_size >= requests[i].HeaderLength)
    {
      if(memcmp(data, requests[i].Header, requests[i].HeaderLength) == 0)
      {
        if (requests[i].Action)
        {
					manager.Device = device;
					manager.Requests = requests;
					manager.FoundRequest = &requests[i];
					manager.RxLine = rx;
					
          xResult result = requests[i].Action(&manager,
																							data + requests[i].HeaderLength,
																							data_size - requests[i].HeaderLength);
					
					if (result == xResultAccept)
					{
						return &requests[i];
					}
        }
      }
    }
    i++;
  }
  return 0;
}
//==============================================================================
xResult xRxRequestManagerInit(xRxRequestManagerT* manager, void* device, xRxRequestT* requests)
{
	if (manager && device && requests)
	{
		manager->Device = device;
		manager->Requests = requests;
		
		return xResultAccept;
	}
	
	return xResultError;
}
//==============================================================================
