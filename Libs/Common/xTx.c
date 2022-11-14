//==============================================================================
#include <string.h>
#include "xTx.h"
//==============================================================================
inline void xTxHandler(xTxT* tx)
{
	tx->Interface->Handler(tx);
}
//------------------------------------------------------------------------------
inline void xTxEventListener(xTxT* tx, xTxEventSelector event, uint32_t args, uint32_t count)
{
	tx->Interface->EventListener(tx, event, args, count);
}
//------------------------------------------------------------------------------
inline xResult xTxRequestListener(xTxT* tx, xTxRequestSelector selector, uint32_t args, uint32_t count)
{
	return tx->Interface->RequestListener(tx, selector, args, count);
}
//------------------------------------------------------------------------------
inline xResult xTxSetValue(xTxT* tx, xTxValueSelector selector, uint32_t value)
{
	return tx->Interface->SetValue(tx, selector, value);
}
//------------------------------------------------------------------------------
inline int xTxGetValue(xTxT* tx, xTxValueSelector selector)
{
	return tx->Interface->GetValue(tx, selector);
}
//------------------------------------------------------------------------------
inline int xTxTransmitData(xTxT* tx, void* data, uint32_t data_size)
{
	return tx->Interface->RequestListener(tx, xTxRequestTransmitData, (uint32_t)data, data_size);
}
//------------------------------------------------------------------------------
inline int xTxTransmitWord(xTxT* tx, uint32_t data)
{
	return tx->Interface->RequestListener(tx, xTxRequestTransmitData, (uint32_t)&data, sizeof(data));
}
//==============================================================================
inline int xTxTransmitByte(xTxT *tx, uint8_t byte)
{
	return tx->Interface->RequestListener(tx, xTxRequestTransmitData, (uint32_t)&byte, sizeof(byte));
}
//------------------------------------------------------------------------------
inline int xTxTransmitString(xTxT *tx, char* str)
{
	return tx->Interface->RequestListener(tx, xTxRequestTransmitData, (uint32_t)str, strlen(str));
}
//==============================================================================
xResult xTxInit(xTxT* tx,
								void* parent,
								xTxAdapterT* adapter,
								xTxInterfaceT* interface)
{
  if (tx && interface && adapter)
  {
		tx->Description = "xTxT";
		tx->Parent = parent;
		
		tx->Interface = interface;
		tx->Adapter = adapter;
		
		tx->Status.IsInit = true;
		
		return xResultAccept;
  }
  return xResultError;
}
//==============================================================================
