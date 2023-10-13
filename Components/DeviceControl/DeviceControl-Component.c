//==============================================================================
//includes:

#include "DeviceControl-Component.h"
#include "Components.h"
//==============================================================================
//defines:

#define TASK_STACK_SIZE 0x200

//==============================================================================
//import:


//==============================================================================
//variables:

static TaskHandle_t taskHandle;
static StaticTask_t taskBuffer;
static StackType_t taskStack[TASK_STACK_SIZE];

static uint32_t timeStamp;
//==============================================================================
//functions:

static void privateEventListener(ObjectBaseT* object, int selector, void* arg)
{

}
//------------------------------------------------------------------------------
static void privateTask(void* arg)
{
	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
//------------------------------------------------------------------------------
void DeviceControlComponentHandler()
{

}
//------------------------------------------------------------------------------
void DeviceControlComponentTimeSynchronization()
{

}
//==============================================================================
//initializations:


//==============================================================================
//initialization:

xResult DeviceControlComponentInit(void* parent)
{
	taskHandle =
			xTaskCreateStatic(privateTask, // Function that implements the task.
								"device control task", // Text name for the task.
								TASK_STACK_SIZE, // Number of indexes in the xStack array.
								NULL, // Parameter passed into the task.
								osPriorityNormal, // Priority at which the task is created.
								taskStack, // Array to use as the task's stack.
								&taskBuffer);

	return xResultAccept;
}
//==============================================================================
