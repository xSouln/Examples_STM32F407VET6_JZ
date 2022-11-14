/*
 * xType.h
 *
 * Created: 16.05.2019 14:29:38
 *  Author: souln
 */ 
//==============================================================================
#ifndef X_TYPES_H
#define X_TYPES_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
 extern "C" {
#endif 
//==============================================================================
#include <stdint.h>
#include <stdbool.h>
//==============================================================================
typedef enum
{
  xResultAccept = 0U,
	xResultError,
	xResultInvalidParameter,
	xResultBusy,
	xResultTimeOut,
	xResultNotSupported,
	xResultValueIsNotFound,
	xResultRequestIsNotFound,
	xResultLinkError,
	xResulComponentInitializationError,
	xResultOutOfRange
	
} xResult;
//==============================================================================
typedef void* xObject;
//==============================================================================
typedef void (*xActionHandler)(void* object);

typedef void (*xEventsReceiver)(void* object, uint32_t event, uint32_t arg);
typedef xResult (*xRequestReceiverT)(void* object, uint32_t selector, uint32_t value);

typedef int (*xActionGetValue)(void* object, uint32_t selector);
typedef xResult (*xActionSetValue)(void* object, uint32_t selector, uint32_t value);
//==============================================================================
#define SIZE_STRING(str)(sizeof(str) - 1)
#define SIZE_ARRAY(array)(sizeof(array) / sizeof(array[0]))
//------------------------------------------------------------------------------
#define sizeof_str(str)(sizeof(str) / sizeof(str[0]) - 1)
#define sizeof_array(array)(sizeof(array) / sizeof(array[0]))
//==============================================================================
#define OBJECT_HEADER\
  char* Description;\
	void* Parent
//==============================================================================
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif /* X_TYPES_H */
