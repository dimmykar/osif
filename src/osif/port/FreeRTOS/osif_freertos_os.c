/**
 * @file           : osif_freertos_os.c
 * @author         : Dmitry Karasev    <karasevsdmitry@yandex.ru>
 * @brief          : FreeRTOS specific routines for Operation System Interface
 * @date           : 2021-07-20
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

/*============================================================================*
 *                                  Includes
 *============================================================================*/
#include <stdint.h>
#include <string.h>

#include "osif.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"


/*============================================================================*
 *                          Private defines & types
 *============================================================================*/
/**
 * @brief           Determine whether we are in thread mode or in handler mode
 * @return          `1` if in handler mode, `0` if in thread mode
 * @hideinitializer
 */
#define IS_IN_HANDLER_MODE()        (__get_IPSR() != 0U)


/*============================================================================*
 *                                Private data
 *============================================================================*/



/*============================================================================*
 *                             Private functions
 *============================================================================*/



/*============================================================================*/



/**
 * @brief           Start OS kernel scheduler
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_KernelStart(void)
{
    vTaskStartScheduler();

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Suspend OS kernel scheduler
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_KernelSuspend(void)
{
    vTaskSuspendAll();

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Resume execution of suspended OS kernel scheduler
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_KernelResume(void)
{
    return (xTaskResumeAll() == pdTRUE ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Enter the critical region. Disable preemptive context switch and interrupts
 * @return          Interrupt mask flag
 */
uint32_t OSIF_KernelLock(void)
{
    uint32_t flags = 0U;

    if (IS_IN_HANDLER_MODE()) {
        flags = taskENTER_CRITICAL_FROM_ISR();
    } else {
        vTaskSuspendAll();
    }

    return (flags);
}
/*============================================================================*/



/**
 * @brief           Exit the critical region. Enable preemptive context switch and interrupts
 * @param[in]       flags: Pointer to interrupt mask flag to be restored. Must be 'NULL' if not called from ISR
 */
void OSIF_KernelUnlock(uint32_t *flags)
{
    if (IS_IN_HANDLER_MODE()) {
        if (flags != NULL) {
            taskEXIT_CRITICAL_FROM_ISR(*flags);
        }
    } else {
        xTaskResumeAll();
    }
}
/*============================================================================*/



/**
 * @brief           Get current time in units of milliseconds
 * @return          Current time in units of ticks
 */
uint32_t OSIF_GetSysTicks(void)
{
    if (IS_IN_HANDLER_MODE()) {
        return ((uint32_t)xTaskGetTickCountFromISR());
    } else {
        return ((uint32_t)xTaskGetTickCount());
    }
}
/*============================================================================*/



/**
 * @brief           Delay current task in a given milliseconds
 * @param[in]       ms: Time delay value in milliseconds
 */
void OSIF_Delay(uint32_t ms)
{
    TickType_t ticks = pdMS_TO_TICKS(ms);
    vTaskDelay(ticks != 0 ? ticks : 1);          /* Minimum delay = 1 tick */
}
/*============================================================================*/



/**
 * @brief           Delay current task in a given milliseconds
 * @param[in]       prev_wake_time: Pointer to a variable that holds the time at which the
 *                      task was last unblocked. This variable must be initialized with the
 *                      pointer to the current time (prev_wake_time = OSIF_GetSysTicks())
 * @param[in]       ms: Time delay value in milliseconds
 */
void OSIF_DelayUntil(uint32_t *prev_wake_time, uint32_t ms)
{
    TickType_t ticks = pdMS_TO_TICKS(ms);
    vTaskDelayUntil((TickType_t *)prev_wake_time, ticks != 0 ? ticks : 1);
}
/*============================================================================*/



/**
 * @brief           Create a new thread
 * @param[out]      t: Pointer to thread identifier if create was successful.
 *                     It may be set to `NULL`
 * @param[in]       name: Name of a new thread
 * @param[in]       thread_fn: Thread function to use as thread body
 * @param[in]       arg: Thread function argument
 * @param[in]       stack_sz: Size of thread stack in uints of bytes. If set to 0, reserve default stack size
 * @param[in]       prio: Thread priority
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadCreate(OSIF_THREAD *t, const char *name, OSIF_THREAD_FN thread_fn, void * const arg, size_t stack_sz, OSIF_THREAD_PRIO prio)
{
    if (t == NULL || thread_fn == NULL) {
        return (osifERR_PARAM);
    }

    OSIF_THREAD id = NULL;

    if (xTaskCreate(thread_fn, (char *)name, stack_sz, arg, prio, &id) == pdPASS) {
        *t = id;
    }

    return (id != NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Suspend thread routine execution
 * @param[in]       t: Pointer to thread handle to suspend
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadSuspend(OSIF_THREAD *t)
{
    if (*t == NULL || t == NULL) {
        return (osifERR_PARAM);
    }

    vTaskSuspend(*t);

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Resume thread routine execution
 * @param[in]       t: Pointer to thread handle to resume
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadResume(OSIF_THREAD *t)
{
    if (*t == NULL || t == NULL) {
        return (osifERR_PARAM);
    }

    if (IS_IN_HANDLER_MODE()) {
        if (xTaskResumeFromISR(*t) == pdTRUE) {
            portYIELD_FROM_ISR(pdTRUE);
        }
    } else {
        vTaskResume(*t);
    }

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Get thread ID of the currently running thread
 * @param[out]      t: Pointer to thread handle that will store the thread ID
 *                      of the currently running thread
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadGetId(OSIF_THREAD *t)
{
    if (*t == NULL || t == NULL) {
        return (osifERR_PARAM);
    }

    *t = xTaskGetCurrentTaskHandle();

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Terminate thread (shut it down and remove)
 * @param[in]       t: Pointer to thread handle to terminate.
 *                      If set to `NULL`, terminate current thread (thread from where function is called)
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadTerminate(OSIF_THREAD *t)
{
    vTaskDelete(t != NULL ? *t : NULL);

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Yield current thread
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadYield(void)
{
//    vTaskDelay(0);
    taskYIELD();

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Get priority of an active thread
 * @param[in]       t: Pointer to thread identifier
 * @param[out]      prio: Pointer to variable to save priority value
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadGetPriority(OSIF_THREAD *t, OSIF_THREAD_PRIO *prio)
{
    if (*t == NULL || t == NULL) {
        return (osifERR_PARAM);
    }

    *prio = uxTaskPriorityGet(*t);

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Change priority of an active thread
 * @param[in]       t: Pointer to thread identifier
 * @param[out]      prio: New priority value
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadSetPriority(OSIF_THREAD *t, OSIF_THREAD_PRIO prio)
{
    if (*t == NULL || t == NULL) {
        return (osifERR_PARAM);
    }

    vTaskPrioritySet(*t, prio);

    return (osifOK);
}
/*============================================================================*/


//TODO
/**
 * @brief           Send the specified Thread Signal to the target thread
 * @param[in]       t: Pointer to thread identifier
 * @param[in]       signal: Signal of the target thread that shall be send
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadSendSignal(OSIF_THREAD *t, uint32_t signal)
{
    //TODO

    return (osifERR);
}
/*============================================================================*/



/**
 * @brief           Get the current Thread Signal of current running thread
 * @param[out]      signal: Pointer to current Thread Signal
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadGetSignal(uint32_t *signal)
{
    //TODO

    return (osifERR);
}
/*============================================================================*/



/**
 * @brief           Receive with timeout one or more Thread Signal of the current
 *                      running thread to become signaled
 * @param[out]      signal: Pointer to thread signal to receive
 * @param[in]       timeout_ms: Maximum timeout to wait for Thread Signal.
 *                      When `OSIF_MAX_DELAY` is passed, wait for unlimited time
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadRecvSignal(uint32_t *signal, uint32_t timeout_ms)
{
    //TODO

    return (osifERR);
}
/*============================================================================*/



/**
 * @brief           Clear the specified Thread Signal of current running thread
 * @param[in]       signal: Signal of the thread that shall be cleared
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_ThreadClearSignal(uint32_t signal)
{
    //TODO

    return (osifERR);
}
/*============================================================================*/



/**
 * @brief           Peek the minimum amount of remaining stack space that was available
 *                      to the thread since the thread started executing
 * @param[in]       t: Pointer to thread identifier
 * @return          Minimum amount of remaining stack space in bytes
 */
size_t OSIF_ThreadPeekFreeStackSize(OSIF_THREAD *t)
{
    if (t == NULL || *t == NULL) {
        return (0UL);
    }

    return (uxTaskGetStackHighWaterMark(*t) / sizeof(size_t));
}
/*============================================================================*/



/**
 * @brief           Create a new message queue
 * @param[out]      b: Pointer to message queue structure
 * @param[in]       name: Name of a new message queue
 * @param[in]       msg_len: Number of entries for message queue to hold
 * @param[in]       item_sz: Number of bytes each item in the message queue
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MboxCreate(OSIF_MBOX *b, const char *name, size_t msg_len, size_t item_sz)
{
    if (b == NULL) {
        return (osifERR_PARAM);
    }

    *b = xQueueCreate(msg_len, item_sz);

    if (*b != NULL) {
#if OSIF_CFG_OS_DEBUG
        vQueueAddToRegistry(*b, (const char *)name);
#endif /* OSIF_CFG_OS_DEBUG */

        return (osifOK);
    } else {
        return (osifERR);
    }
}
/*============================================================================*/



/**
 * @brief           Delete message queue
 * @param[in]       b: Pointer to message queue structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MboxDelete(OSIF_MBOX *b)
{
    if (OSIF_MboxMessagesWaiting(b) == osifOK) {
        return (osifERR);
    }

    vQueueDelete(*b);

    return (b == NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Put a new entry to message queue and wait until memory available
 * @param[in]       b: Pointer to message queue structure
 * @param[in]       m: Pointer to entry to insert to message queue
 * @param[in]       timeout_ms: Maximum timeout to wait for space to become available on the queue.
 *                      When `OSIF_MAX_DELAY` is passed, wait for unlimited time
 * @return          Time in units of milliseconds needed to put a message to queue
 */
int32_t OSIF_MboxPut(OSIF_MBOX *b, void *m, uint32_t timeout_ms)
{
    if (b == NULL || *b == NULL) {
        return (0);
    }

    BaseType_t res = pdFALSE;
    uint32_t tick = OSIF_GetSysTicks();

    if (IS_IN_HANDLER_MODE()) {
        BaseType_t task_woken = pdFALSE;

        res = xQueueSendToBackFromISR(*b, m, &task_woken);
        portEND_SWITCHING_ISR(task_woken);
    } else {
        res = xQueueSend(*b, m, timeout_ms == OSIF_MAX_DELAY ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));
    }

    if (res == pdTRUE) {
        return ((int32_t)((OSIF_GetSysTicks() - tick) / portTICK_PERIOD_MS));
    } else {
        return (osifERR_TIMEOUT);
    }
}
/*============================================================================*/



/**
 * @brief           Get a new entry from message queue with timeout
 * @param[in]       b: Pointer to message queue structure
 * @param[in]       m: Pointer to result to save value from message queue to
 * @param[in]       timeout_ms: Maximum timeout to wait for new message. When `OSIF_MAX_DELAY` is passed,
 *                      wait for unlimited time
 * @return          Time in units of milliseconds needed to get a message to queue
 *                      or @ref osifERR_TIMEOUT if it was not successful
 */
int32_t OSIF_MboxGet(OSIF_MBOX *b, void *m, uint32_t timeout_ms)
{
    if (b == NULL || *b == NULL) {
        return (0);
    }

    BaseType_t res = pdFALSE;
    uint32_t tick = OSIF_GetSysTicks();

    if (IS_IN_HANDLER_MODE()) {
        BaseType_t task_woken = pdFALSE;

        res = xQueueReceiveFromISR(*b, m, &task_woken);
        portEND_SWITCHING_ISR(task_woken);
    } else {
        res = xQueueReceive(*b, m, timeout_ms == OSIF_MAX_DELAY ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));
    }

    if (res == pdTRUE) {
        return ((int32_t)((OSIF_GetSysTicks() - tick) / portTICK_PERIOD_MS));
    } else {
        return (osifERR_TIMEOUT);
    }
}
/*============================================================================*/



/**
 * @brief           Get a new entry from message queue without removing it from message queue with timeout
 * @param[in]       b: Pointer to message queue structure
 * @param[in]       m: Pointer to result to save value from message queue to
 * @param[in]       timeout_ms: Maximum timeout to wait for new message. When `OSIF_MAX_DELAY` is passed,
 *                      wait for unlimited time
 * @return          Time in units of milliseconds needed to peek a message from queue
 *                      or @ref osifERR_TIMEOUT if it was not successful
 */
int32_t OSIF_MboxPeek(OSIF_MBOX *b, void *m, uint32_t timeout_ms)
{
    if (b == NULL || *b == NULL) {
        return (0);
    }

    BaseType_t res = pdFALSE;
    uint32_t tick = OSIF_GetSysTicks();

    if (IS_IN_HANDLER_MODE()) {
        res = xQueuePeekFromISR(*b, m);
    } else {
    res = xQueuePeek(*b, m, timeout_ms == OSIF_MAX_DELAY ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));
    }

    if (res == pdTRUE) {
        return ((int32_t)((OSIF_GetSysTicks() - tick) / portTICK_PERIOD_MS));
    } else {
        return (osifERR_TIMEOUT);
    }
}
/*============================================================================*/



/**
 * @brief           Get the number of available messages stored in the message queue
 * @param[in]       b: Pointer to message queue structure
 * @return          The number of items available in the message queue
 */
uint32_t OSIF_MboxMessagesWaiting(OSIF_MBOX *b)
{
    if (b == NULL || *b == NULL) {
        return (0);
    }

    uint32_t num = 0;

    if (IS_IN_HANDLER_MODE()) {
        num = (uint32_t)uxQueueMessagesWaitingFromISR(*b);
    } else {
        num = (uint32_t)uxQueueMessagesWaiting(*b);
    }

    return (num);
}
/*============================================================================*/



/**
 * @brief           Get the number of free spaces available in the message queue
 * @param[in]       b: Pointer to message queue structure
 * @return          The number of spaces available in the message queue
 */
int32_t OSIF_MboxSpacesAvailable(OSIF_MBOX *b)
{
    if (b == NULL || *b == NULL) {
        return (0);
    }

    uint32_t num = 0;

    if (IS_IN_HANDLER_MODE()) {
        return (osifERR_ISR);  // TODO FreeRTOS hasn't uxQueueSpacesAvailableFromISR function
    } else {
        num = (uint32_t)uxQueueSpacesAvailable(*b);
    }

    return (num);
}
/*============================================================================*/



/**
 * @brief           Reset a message queue to its empty state
 * @param[in]       b: Pointer to message queue structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MboxReset(OSIF_MBOX *b)
{
    if (b == NULL || *b == NULL) {
        return (osifERR_PARAM);
    }

    BaseType_t res = pdFALSE;

    res = xQueueReset(*b);

    return (res == pdTRUE ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Check if message queue is valid
 * @param[in]       b: Pointer to message queue structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MboxIsValid(OSIF_MBOX *b)
{
    return (b != NULL && *b != NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Set message queue structure as invalid
 * @param[in]       b: Pointer to message queue structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MboxInvalid(OSIF_MBOX *b)
{
    *b = OSIF_MBOX_NULL;

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Create new recursive mutex
 * @note            Recursive mutex has to be created as it may be locked multiple times before unlocked
 * @param[in]       name: Name of a new recursive mutex (can be anything in this port)
 * @param[out]      p: Pointer to mutex structure to allocate
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MutexCreate(OSIF_MUTEX *p, const char *name)
{
    if (p == NULL) {
        return (osifERR_PARAM);
    }

    *p = xSemaphoreCreateRecursiveMutex();
    if (*p != NULL) {
        OSIF_SemaphoreRelease(p);

#if OSIF_CFG_OS_DEBUG
        vQueueAddToRegistry(*p, (const char *)name);
#endif /* OSIF_CFG_OS_DEBUG */

        return (osifOK);
    } else {
        return (osifERR);
    }
}
/*============================================================================*/



/**
 * @brief           Delete recursive mutex from system
 * @param[in]       p: Pointer to mutex structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MutexDelete(OSIF_MUTEX *p)
{
    vSemaphoreDelete(*p);

    return (*p == NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Lock recursive mutex, wait forever to lock
 * @param[in]       p: Pointer to mutex structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MutexLock(OSIF_MUTEX *p)
{
    if (p == NULL || *p == NULL) {
        return (osifERR_PARAM);
    } else {
        return (xSemaphoreTakeRecursive(*p, portMAX_DELAY) == pdTRUE ? osifOK : osifERR);
    }
}
/*============================================================================*/



/**
 * @brief           Unlock recursive mutex
 * @param[in]       p: Pointer to mutex structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MutexUnlock(OSIF_MUTEX *p)
{
    if (p == NULL || *p == NULL) {
        return (osifERR_PARAM);
    } else {
        return (xSemaphoreGiveRecursive(*p) == pdTRUE ? osifOK : osifERR);
    }
}
/*============================================================================*/



/**
 * @brief           Check if mutex structure is valid
 * @param[in]       p: Pointer to mutex structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MutexIsValid(OSIF_MUTEX *p)
{
    return (p != NULL && *p != NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Set recursive mutex structure as invalid
 * @param[in]       p: Pointer to mutex structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_MutexInvalid(OSIF_MUTEX *p)
{
    *p = OSIF_MUTEX_NULL;

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Create a new binary semaphore and set initial state
 * @note            Semaphore may only have `1` token available
 * @param[in]       name: Name of a new binary semaphore (can be anything in this port)
 * @param[out]      p: Pointer to semaphore structure to fill with result
 * @param[in]       cnt: Count indicating default semaphore state:
 *                     `0`: Take semaphore token immediately
 *                     `1`: Keep token available
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_SemaphoreCreate(OSIF_SEMAPHORE *p, const char *name, uint8_t cnt)
{
    if (p == NULL) {
        return (osifERR_PARAM);
    }

    *p = xSemaphoreCreateBinary();
    if (*p != NULL) {
        if (cnt != 0) {
            if (IS_IN_HANDLER_MODE()) {
                BaseType_t task_woken = pdFALSE;
                xSemaphoreTakeFromISR(*p, &task_woken);
                portEND_SWITCHING_ISR(task_woken);
            } else {
                xSemaphoreTake(*p, portMAX_DELAY);
            }
        }

#if OSIF_CFG_OS_DEBUG
        vQueueAddToRegistry(*p, (const char *)name);
#endif /* OSIF_CFG_OS_DEBUG */

        return (osifOK);
    } else {
        return (osifERR);
    }
}
/*============================================================================*/



/**
 * @brief           Delete binary semaphore
 * @param[in]       p: Pointer to semaphore structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_SemaphoreDelete(OSIF_SEMAPHORE *p)
{
    vSemaphoreDelete(*p);

    return (*p == NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Wait for semaphore to be available
 * @param[in]       p: Pointer to semaphore structure
 * @param[in]       timeout: Timeout to wait in milliseconds. When `0` is passed, wait forever
 * @return          Number of milliseconds waited for semaphore to become available or
 *                      @ref osifERR_TIMEOUT if not available within given time
 */
int32_t OSIF_SemaphoreWait(OSIF_SEMAPHORE *p, uint32_t timeout_ms)
{
    if (p == NULL || *p == NULL) {
        return (osifERR_PARAM);
    }

    BaseType_t res = pdFALSE;
    uint32_t tick = OSIF_GetSysTicks();

    if (IS_IN_HANDLER_MODE()) {
        BaseType_t task_woken = pdFALSE;
        res = xSemaphoreTakeFromISR(*p, &task_woken);
        portEND_SWITCHING_ISR(task_woken);
    } else {
        res = xSemaphoreTake(*p, timeout_ms == OSIF_MAX_DELAY ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms));
    }

    if (res == pdTRUE) {
        return ((uint32_t)((OSIF_GetSysTicks() - tick) / portTICK_PERIOD_MS));
    } else {
        return (osifERR_TIMEOUT);
    }
}
/*============================================================================*/



/**
 * @brief           Release semaphore
 * @param[in]       p: Pointer to semaphore structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_SemaphoreRelease(OSIF_SEMAPHORE *p)
{
    if (p == NULL || *p == NULL) {
        return (osifERR_PARAM);
    }

    BaseType_t res = pdFALSE;

    if (IS_IN_HANDLER_MODE()) {
        BaseType_t task_woken = pdFALSE;
        res = xSemaphoreGiveFromISR(*p, &task_woken);
        portEND_SWITCHING_ISR(task_woken);
    } else {
        res = xSemaphoreGive(*p);
    }

    return (res == pdTRUE ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Check if semaphore is valid
 * @param[in]       p: Pointer to semaphore structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_SemaphoreIsValid(OSIF_SEMAPHORE *p)
{
    return (p != NULL && *p != NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Set semaphore structure as invalid
 * @param[in]       p: Pointer to semaphore structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_SemaphoreInvalid(OSIF_SEMAPHORE *p)
{
    *p = OSIF_SEMAPHORE_NULL;

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Create a new RTOS software timer
 * @param[in]       name: Name of a new software timer (can be anything in this port)
                    p: Pointer to timer structure if create was successful.
 * @param[in]       tim_fn: Timer callback function invoked when timer underflow
 * @param[in]       arg: Timer function argument
 * @param[in]       reload: `1` if periodic timer, `0` if one-shot timer
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerCreate(OSIF_TIMER *p, const char *name, OSIF_TIMER_FN tim_fn, void * const arg, uint8_t reload)
{
    if (p == NULL || tim_fn == NULL) {
        return (osifERR_PARAM);
    }

    *p = xTimerCreate((const char *)name, 1, (BaseType_t)reload, arg, (TimerCallbackFunction_t)tim_fn);

    return (*p != NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Delete RTOS software timer
 * @param[in]       p: Pointer to timer structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerDelete(OSIF_TIMER *p)
{
    xTimerDelete(*p, OSIF_MAX_DELAY);

    return (*p == NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Start RTOS software timer
 * @param[in]       p: Pointer to timer structure
 * @param[in]       period_ms: time interval value of the timer in milliseconds
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerStart(OSIF_TIMER *p, uint32_t period_ms)
{
    if (p == NULL || *p == NULL) {
        return (osifERR_PARAM);
    }

    BaseType_t res = pdFALSE;
    TickType_t ticks = pdMS_TO_TICKS(period_ms);

    if (ticks == 0) {
        ticks = 1;
    }

    if (IS_IN_HANDLER_MODE()) {
        BaseType_t task_woken = pdFALSE;
        res = xTimerChangePeriodFromISR(*p, ticks, &task_woken);
        portEND_SWITCHING_ISR(task_woken);
    } else {
        res = xTimerChangePeriod(*p, ticks, 0);
    }

    return (res == pdTRUE ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Stop RTOS software timer
 * @param[in]       p: Pointer to timer structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerStop(OSIF_TIMER *p)
{
    if (p == NULL || *p == NULL) {
        return (osifERR_PARAM);
    }

    BaseType_t res = pdFALSE;

    if (IS_IN_HANDLER_MODE()) {
        BaseType_t task_woken = pdFALSE;
        res = xTimerStopFromISR(*p, &task_woken);
        portEND_SWITCHING_ISR(task_woken);
    } else {
        res = xTimerStop(*p, 0);
    }

    return (res == pdTRUE ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Restart RTOS software timer with new period
 * @param[in]       p: Pointer to timer structure
 * @param[in]       period_ms: New time interval value in milliseconds
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerRestart(OSIF_TIMER *p, uint32_t period_ms)
{
    return (OSIF_TimerStart(p, period_ms));
}
/*============================================================================*/



/**
 * @brief           Get RTOS software timer ID
 * @param[in]       p: Pointer to timer structure
 * @param[out]      p_timer_id: Pointer to ID value
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerGetId(OSIF_TIMER *p, uint32_t *p_timer_id)
{
    if (p == NULL || *p == NULL || *p_timer_id == NULL) {
        return (osifERR_PARAM);
    }

    *p_timer_id = (uint32_t)pvTimerGetTimerID((TimerHandle_t)p);

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Check if software timer is valid
 * @param[in]       p: Pointer to timer structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerIsValid(OSIF_TIMER *p)
{
    return (p != NULL && *p != NULL ? osifOK : osifERR);
}
/*============================================================================*/



/**
 * @brief           Set timer structure as invalid
 * @param[in]       p: Pointer to timer structure
 * @return          `osifOK` on success, member of @ref OSIF_RESULT otherwise
 */
OSIF_RESULT OSIF_TimerInvalid(OSIF_TIMER *p)
{
    *p = OSIF_TIMER_NULL;

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Allocate memory
 * @param[in]       size: Allocated memory size
 * @return          Pointer to allocated memory block
 */
void * OSIF_MemAlloc(size_t size)
{
    return (pvPortMalloc(size));
}
/*============================================================================*/



/**
 * @brief           Allocate aligned memory
 * @param[in]       size: Allocated memory size
 * @param[in]       alignment: 
 * @return          Pointer to allocated memory block
 */
void * OSIF_MemAllocAligned(size_t size, uint8_t alignment)
{
    void *p;
    void *p_aligned;

    if (alignment == 0) {
        alignment = portBYTE_ALIGNMENT;
    }

    p = pvPortMalloc(size + sizeof(void *) + alignment);
    if (p == NULL) {
        return (p);
    }

    p_aligned = (void *)(((size_t)p + sizeof(void *) + alignment) & ~(alignment - 1));

    memcpy((uint8_t *)p_aligned - sizeof(void *), &p, sizeof(void *));

    return (p_aligned);
}
/*============================================================================*/



/**
 * @brief           Free allocated memory
 * @param[in]       p_block: Pointer to allocated memory block
 */
OSIF_RESULT OSIF_MemFree(void *p_block)
{
    if (p_block == NULL) {
        return (osifERR_PARAM);
    }

    vPortFree(p_block);

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Free allocated aligned memory
 * @param[in]       p_block: Pointer to allocated aligned memory block
 */
OSIF_RESULT OSIF_MemFreeAligned(void *p_block)
{
    void *p;
    if (p_block == NULL) {
        return (osifERR_PARAM);
    }

    memcpy(&p, (uint8_t *)p_block - sizeof(void *), sizeof(void *));

    vPortFree(p);

    return (osifOK);
}
/*============================================================================*/



/**
 * @brief           Peek unused (available) heap memory size
 * @return          Unused heap size in bytes
 */
size_t OSIF_MemPeekFreeSize(void)
{
    return (xPortGetFreeHeapSize() / sizeof(size_t));
}
/*============================================================================*/



/**
 * @brief           Peek the lowest amount of free heap space that has existed
 *                      on the system since the application was booted
 * @return          Minimum free heap size ever in bytes
 */
size_t OSIF_MemPeekMinimumEverFreeSize(void)
{
    return (xPortGetMinimumEverFreeHeapSize() / sizeof(size_t));
}
/*============================================================================*/
