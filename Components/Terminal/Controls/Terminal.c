//==============================================================================
#include "Terminal.h"
#include "Terminal/Communication/Terminal_RxRequests.h"
#include "Components.h"
//==============================================================================
#define RESPONSE_DATA_BUFFER_SIZE 1024
//==============================================================================
static uint8_t response_data_buffer_memory[RESPONSE_DATA_BUFFER_SIZE];
xDataBufferT ResponseBuffer =
{
	.Data = response_data_buffer_memory,
	.Size = RESPONSE_DATA_BUFFER_SIZE
};
//------------------------------------------------------------------------------
TerminalTransferAdapterT TransferAdapter =
{
	.HeaderTransferStart = "HeaderTransferStart",
	.HeaderTransfer = "data: ",
	.HeaderTransferEnd = "HeaderTransferEnd"
};
//------------------------------------------------------------------------------
//const char TransferData[] = "111111111111111111\r111111111111111111\r111111111111111111\r111111111111111111\r111111111111111111\r111111111111111111\r111111111111111111\r111111111111111111\r";
TerminalT Terminal;

uint64_t WorkTime;
xTxTransferT Transfer;
//==============================================================================
/**
 * @brief main handler
 */
void TerminalHandler()
{

}
//------------------------------------------------------------------------------
/**
 * @brief time synchronization of time-dependent processes
 */
void TerminalTimeSynchronization()
{
	WorkTime++;
}
//------------------------------------------------------------------------------
/**
 * @brief initializing the component
 * @param parent binding to the parent object
 * @return xResult
 */
xResult TerminalInit(void* parent)
{
	Terminal.Description = "TerminalT";
	Terminal.Parent = parent;

	return xResultAccept;
}
//==============================================================================
