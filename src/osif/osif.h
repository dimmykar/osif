/**
 * @file           : osif.h
 * @author         : Dmitry Karasev    <karasevsdmitry@yandex.ru>
 * @brief          : Operation system interface (OSIF) for embedded devices
 * @date           : 2021-07-05
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 Dmitry Karasev
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is part of OSIF - Operating system interface for embedded devices.
 *
 * Author:          Dmitry KARASEV <karasevsdmitry@yandex.ru>
 * Version:         $_version_$
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _OSIF_H_INCLUDED_
#define _OSIF_H_INCLUDED_


/*============================================================================*
 *                                  Includes
 *============================================================================*/
#include <stdint.h>

/* Include OSIF interface port file from port folder */
#include "osif_port.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*============================================================================*
 *                           Public defines & types
 *============================================================================*/
/**
 * @defgroup        OSIF OS interface functions
 * @brief           OS based function for OS management, timings, etc
 * @{
 */

/**
 * @brief           Status code values returned by OSIF functions
 */
typedef enum
{
    osifOK                  =  0,              /*!< Operation completed successfully */
    osifERR                 = -1,              /*!< Unspecified RTOS error: run-time error but no other error message fits */
    osifERR_PARAM           = -2,              /*!< Parameter error */
    osifERR_MEM             = -3,              /*!< System is out of memory: it was imposifsible to allocate or reserve memory for the operation */
    osifERR_TIMEOUT         = -4,              /*!< Operation not completed within the timeout period */
    osifERR_RESOURCE        = -5,              /*!< Resource not available */
    osifERR_ISR             = -6,              /*!< Not allowed in ISR context: the function cannot be called from interrupt service routines */
} OSIF_RESULT;

/**
 * @brief           Thread function prototype
 */
typedef void (*OSIF_THREAD_FN)(void *);

/**
 * @brief           RTOS software timer callback function prototype
 */
typedef void (*OSIF_TIMER_FN)(const void *);


/*============================================================================*
 *                                Public data
 *============================================================================*/



/*============================================================================*
 *                              Public functions
 *============================================================================*/
/**
 * @anchor          OSIF_KERNEL_MNG
 * @name            OSIF Kernel Management
 */

OSIF_RESULT    OSIF_KernelStart(void);
OSIF_RESULT    OSIF_KernelSuspend(void);
OSIF_RESULT    OSIF_KernelResume(void);

uint32_t       OSIF_KernelLock(void);
void           OSIF_KernelUnlock(uint32_t *flags);

uint32_t       OSIF_GetSysTicks(void);
void           OSIF_Delay(uint32_t ms);
void           OSIF_DelayUntil(uint32_t *prev_wake_time, uint32_t ms);

/**
 * @}
 */

/**
 * @anchor          OSIF_THREAD
 * @name            Threads
 */

OSIF_RESULT    OSIF_ThreadCreate(OSIF_THREAD *t, const char *name, OSIF_THREAD_FN thread_fn, void * const arg, size_t stack_sz, OSIF_THREAD_PRIO prio);
OSIF_RESULT    OSIF_ThreadSuspend(OSIF_THREAD *t);
OSIF_RESULT    OSIF_ThreadResume(OSIF_THREAD *t);

OSIF_RESULT    OSIF_ThreadGetId(OSIF_THREAD *t);

OSIF_RESULT    OSIF_ThreadTerminate(OSIF_THREAD *t);
OSIF_RESULT    OSIF_ThreadYield(void);

OSIF_RESULT    OSIF_ThreadGetPriority(OSIF_THREAD *t, OSIF_THREAD_PRIO *prio);
OSIF_RESULT    OSIF_ThreadSetPriority(OSIF_THREAD *t, OSIF_THREAD_PRIO prio);

OSIF_RESULT    OSIF_ThreadSendSignal(OSIF_THREAD *t, uint32_t signal);
OSIF_RESULT    OSIF_ThreadGetSignal(uint32_t *signal);
OSIF_RESULT    OSIF_ThreadRecvSignal(uint32_t *signal, uint32_t timeout_ms);
OSIF_RESULT    OSIF_ThreadClearSignal(uint32_t signal);

size_t         OSIF_ThreadPeekFreeStackSize(OSIF_THREAD *t);

/**
 * @}
 */

/**
 * @anchor          OSIF_MBOX
 * @name            Message queues
 */

OSIF_RESULT    OSIF_MboxCreate(OSIF_MBOX *b, const char *name, size_t msg_len, size_t item_sz);
OSIF_RESULT    OSIF_MboxDelete(OSIF_MBOX *b);

int32_t        OSIF_MboxPut(OSIF_MBOX *b, void *m, uint32_t timeout_ms);
int32_t        OSIF_MboxGet(OSIF_MBOX *b, void *m, uint32_t timeout_ms);
int32_t        OSIF_MboxPeek(OSIF_MBOX *b, void *m, uint32_t timeout_ms);
uint32_t       OSIF_MboxMessagesWaiting(OSIF_MBOX *b);
int32_t        OSIF_MboxSpacesAvailable(OSIF_MBOX *b);
OSIF_RESULT    OSIF_MboxReset(OSIF_MBOX *b);

OSIF_RESULT    OSIF_MboxIsValid(OSIF_MBOX *b);
OSIF_RESULT    OSIF_MboxInvalid(OSIF_MBOX *b);

/**
 * @}
 */

/**
 * @anchor          OSIF_MUTEX
 * @name            Mutex
 */

OSIF_RESULT    OSIF_MutexCreate(OSIF_MUTEX *p, const char *name);
OSIF_RESULT    OSIF_MutexDelete(OSIF_MUTEX *p);

OSIF_RESULT    OSIF_MutexLock(OSIF_MUTEX *p);
OSIF_RESULT    OSIF_MutexUnlock(OSIF_MUTEX *p);

OSIF_RESULT    OSIF_MutexIsValid(OSIF_MUTEX *p);
OSIF_RESULT    OSIF_MutexInvalid(OSIF_MUTEX *p);

/**
 * @}
 */

/**
 * @anchor          OSIF_SEMAPHORE
 * @name            Semaphores
 */

OSIF_RESULT    OSIF_SemaphoreCreate(OSIF_SEMAPHORE *p, const char *name, uint8_t cnt);
OSIF_RESULT    OSIF_SemaphoreDelete(OSIF_SEMAPHORE *p);

int32_t        OSIF_SemaphoreWait(OSIF_SEMAPHORE *p, uint32_t timeout_ms);
OSIF_RESULT    OSIF_SemaphoreRelease(OSIF_SEMAPHORE *p);

OSIF_RESULT    OSIF_SemaphoreIsValid(OSIF_SEMAPHORE *p);
OSIF_RESULT    OSIF_SemaphoreInvalid(OSIF_SEMAPHORE *p);

/**
 * @}
 */

/**
 * @anchor          OSIF_TIMER
 * @name            OS Software times
 */

OSIF_RESULT    OSIF_TimerCreate(OSIF_TIMER *p, const char *name, OSIF_TIMER_FN tim_fn, void * const arg, uint8_t reload);
OSIF_RESULT    OSIF_TimerDelete(OSIF_TIMER *p);

OSIF_RESULT    OSIF_TimerStart(OSIF_TIMER *p, uint32_t period_ms);
OSIF_RESULT    OSIF_TimerStop(OSIF_TIMER *p);
OSIF_RESULT    OSIF_TimerRestart(OSIF_TIMER *p, uint32_t period_ms);
OSIF_RESULT    OSIF_TimerGetId(OSIF_TIMER *p, uint32_t *p_timer_id);

OSIF_RESULT    OSIF_TimerIsValid(OSIF_TIMER *p);
OSIF_RESULT    OSIF_TimerInvalid(OSIF_TIMER *p);

/**
 * @}
 */

/**
 * @anchor          OSIF_MEM_MNG
 * @name            OS Memory Managment
 */

void *         OSIF_MemAlloc(size_t size);
void *         OSIF_MemAllocAligned(size_t size, uint8_t alignment);
OSIF_RESULT    OSIF_MemFree(void *p_block);
OSIF_RESULT    OSIF_MemFreeAligned(void *p_block);

size_t         OSIF_MemPeekFreeSize(void);
size_t         OSIF_MemPeekMinimumEverFreeSize(void);

/**
 * @}
 */

/**
 * @}
 */


/*============================================================================*/


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _OSIF_H_INCLUDED_ */
