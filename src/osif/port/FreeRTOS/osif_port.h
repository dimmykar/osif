/**
 * @file           : osif_port.h
 * @author         : Dmitry Karasev    <karasevsdmitry@yandex.ru>
 * @brief          : FreeRTOS specific types for Operation System Interface
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _OSIF_PORT_H_INCLUDED_
#define _OSIF_PORT_H_INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*============================================================================*
 *                                  Includes
 *============================================================================*/
#include <stdint.h>

/**
 * @addtogroup      OSIF
 * @{
 */

/* Include any OS specific features */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"


/*============================================================================*
 *                               Public defines
 *============================================================================*/
/**
 * @brief           OSIF thread ID type
 */
typedef TaskHandle_t                OSIF_THREAD;

/**
 * @brief           OSIF thread priority type
 *
 * It is used as priority type for OSIF interface function,
 * to start new threads by firmware.
 */
typedef UBaseType_t                 OSIF_THREAD_PRIO;

/**
 * @brief           OSIF message queue type
 *
 * It is used by firmware as base type of message queue.
 */
typedef QueueHandle_t               OSIF_MBOX;

/**
 * @brief           OSIF mutex type
 *
 * It is used by firmware as base type of mutex.
 */
typedef SemaphoreHandle_t           OSIF_MUTEX;

/**
 * @brief           OSIF semaphore type
 *
 * It is used by firmware as base type of semaphore.
 */
typedef SemaphoreHandle_t           OSIF_SEMAPHORE;

/**
 * @brief           OSIF software timer type
 *
 * It is used by firmware as base type of software timer.
 */
typedef TimerHandle_t               OSIF_TIMER;


/**
 * @brief           Message box invalid value
 *
 * Value assigned to @ref OSIF_MBOX type when it is not valid.
 */
#define OSIF_MBOX_NULL              ((OSIF_MBOX)0)

/**
 * @brief           Mutex invalid value
 *
 * Value assigned to @ref OSIF_MUTEX type when it is not valid.
 */
#define OSIF_MUTEX_NULL             ((OSIF_MUTEX)0)

/**
 * @brief           Semaphore invalid value
 *
 * Value assigned to @ref OSIF_SEMAPHORE type when it is not valid.
 */
#define OSIF_SEMAPHORE_NULL         ((OSIF_SEMAPHORE)0)

/**
 * @brief           Timer invalid value
 *
 * Value assigned to @ref OSIF_TIMER type when it is not valid.
 */
#define OSIF_TIMER_NULL             ((OSIF_TIMER)0)

/**
 * @brief           OSIF timeout value
 *
 * Value returned by operating system functions (mutex wait, sem wait, mbox wait)
 * when it returns timeout and does not give valid value to application
 */
#define OSIF_TIMEOUT                ((uint32_t)portMAX_DELAY)

/**
 * @brief           OSIF maximum delay value
 *
 *
 */
#define OSIF_MAX_DELAY              ((uint32_t)portMAX_DELAY)

/**
 * @brief           Default thread priority value, may be used by firmware
 *                      to start built-in threads
 *
 * Threads can well operate with normal (default) priority and do not require
 * any special feature in terms of priority for prior operation.
 */
#define OSIF_THREAD_DEFAULT_PRIO    (tskIDLE_PRIORITY + 1)

/**
 * @brief           Stack size in units of words for OSIF threads
 *
 * It is used as default stack size for all built-in threads.
 */
#define OSIF_THREAD_DEFAULT_SS      (sizeof(size_t) * 128U)


/*============================================================================*
 *                              Public variables
 *============================================================================*/



/*============================================================================*
 *                              Public functions
 *============================================================================*/



/*============================================================================*/

/**
 * @}
 */ /* End of addtogroup OSIF */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _OSIF_PORT_H_INCLUDED_ */