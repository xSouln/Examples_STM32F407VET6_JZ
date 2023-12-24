/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(MQTTFreeRTOS_H)
#define MQTTFreeRTOS_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "FreeRTOS_IP.h"

typedef struct MQTTTimer
{
	TickType_t xTicksToWait;
	TimeOut_t xTimeOut;
} MQTTTimerT;

typedef struct MQTTNetwork MQTTNetworkT;

struct MQTTNetwork
{
	xSocket_t my_socket;
	int (*mqttread) (MQTTNetworkT*, unsigned char*, int, int);
	int (*mqttwrite) (MQTTNetworkT*, unsigned char*, int, int);
	void (*disconnect) (MQTTNetworkT*);
};

void TimerInit(MQTTTimerT*);
char TimerIsExpired(MQTTTimerT*);
void TimerCountdownMS(MQTTTimerT*, unsigned int);
void TimerCountdown(MQTTTimerT*, unsigned int);
int TimerLeftMS(MQTTTimerT*);

typedef struct MQTTMutex
{
	SemaphoreHandle_t sem;
} MQTTMutexT;

void MutexInit(MQTTMutexT*);
int MutexLock(MQTTMutexT*);
int MutexUnlock(MQTTMutexT*);

typedef struct MQTTThread
{
	TaskHandle_t task;
} MQTTThreadT;

int ThreadStart(MQTTThreadT*, void (*fn)(void*), void* arg);

int FreeRTOS_read(MQTTNetworkT*, unsigned char*, int, int);
int FreeRTOS_write(MQTTNetworkT*, unsigned char*, int, int);
void FreeRTOS_disconnect(MQTTNetworkT*);

void NetworkInit(MQTTNetworkT*);
int NetworkConnect(MQTTNetworkT*, char*, int);
/*int NetworkConnectTLS(Network*, char*, int, SlSockSecureFiles_t*, unsigned char, unsigned int, char);*/

#endif
