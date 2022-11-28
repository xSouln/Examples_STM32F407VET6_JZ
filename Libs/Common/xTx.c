//==============================================================================
//includes:

#include <string.h>
#include "xTx.h"
//==============================================================================
//variables:

static const ObjectDescriptionT xTxObjectDescription =
{
	.Key = OBJECT_DESCRIPTION_KEY,
	.ObjectId = X_TX_OBJECT_ID,
	.Type = nameof(xTxT)
};
//==============================================================================
//functions:
/*
inline int xTxTransmitData(xTxT* tx, uint8_t* data, uint32_t data_size)
{
	return tx->Interface->RequestListener(tx, xTxRequestTransmitData, (uint32_t)data, data_size);
}
*/
//------------------------------------------------------------------------------
xResult _xTxTransmitData()
{
	return xResultAccept;
}
//------------------------------------------------------------------------------

xResult _xTxTransmitWord(xTxT* tx, uint32_t data)
{
	return xTxTransmitData(tx, (uint8_t*)&data, sizeof(data));
}
//------------------------------------------------------------------------------

xResult _xTxTransmitByte(xTxT *tx, uint8_t byte)
{
	return xTxTransmitData(tx, &byte, sizeof(byte));
}
//------------------------------------------------------------------------------

xResult _xTxTransmitString(xTxT *tx, char* str)
{
	return xTxTransmitData(tx, (uint8_t*)str, strlen(str));
}
//==============================================================================
//initialization:

xResult xTxInit(xTxT* tx,
								void* parent,
								xTxAdapterT* adapter,
								xTxInterfaceT* interface)
{
  if (tx && interface && adapter)
  {
		tx->Object.Description = &xTxObjectDescription;
		tx->Object.Parent = parent;
		
		tx->Interface = interface;
		tx->Adapter = adapter;
		
		tx->Status.IsInit = true;
		
		return xResultAccept;
  }
  return xResultError;
}
//==============================================================================
