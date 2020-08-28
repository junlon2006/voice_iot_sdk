/**************************************************************************
 * Copyright (C) 2017-2017  Unisound
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **************************************************************************
 *
 * Description : porting.h
 * Author      : junlon2006@163.com
 * Date        : 2020.07.21
 *
 **************************************************************************/
#ifndef HAL_INC_PORTING_H_
#define HAL_INC_PORTING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define IOT_DEVICE_VERSION  "v4.1.0"

#define UNI_WAIT_FOREVER   (-1)
#define NULL_PTR_CHECK(p)  (NULL == p)
#define MZERO(p)            memset(p, 0, sizeof(*p))
#define OUT_MEM_STRING     "alloc memory failed"

#define uni_max(x, y)      ((x) > (y) ? (x) : (y))
#define uni_min(x, y)      ((x) < (y) ? (x) : (y))

typedef struct {
    void *hdl;
} uni_hdl_t;

typedef uni_hdl_t uni_mutex_t;
typedef uni_hdl_t uni_sem_t;
typedef uni_hdl_t uni_thread_t;

/**
 * Alloc a mutex.
 *
 * @param[in]  mutex  pointer of mutex object, mutex object must be alloced,
 *                    hdl pointer in uni_mutex_t will refer a kernel obj internally.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_mutex_new(uni_mutex_t *mutex);

/**
 * Free a mutex.
 *
 * @param[in]  mutex  mutex object, mem refered by hdl pointer in
 *                    uni_mutex_t will be freed internally.
 *
 * @return  none.
 */
int uni_mutex_free(uni_mutex_t *mutex);

/**
 * Lock a mutex.
 *
 * @param[in]  mutex    mutex object, it contains kernel obj pointer.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_mutex_lock(uni_mutex_t *mutex);

/**
 * Unlock a mutex.
 *
 * @param[in]  mutex  mutex object, it contains kernel obj pointer.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_mutex_unlock(uni_mutex_t *mutex);

/**
 * Alloc a semaphore.
 *
 * @param[out]  sem    pointer of semaphore object, semaphore object must be
 *                     alloced, hdl pointer in uni_sem_t will refer a kernel obj internally.
 * @param[in]   count  initial semaphore counter.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_sem_new(uni_sem_t *sem, unsigned int count);

/**
 * Destroy a semaphore.
 *
 * @param[in]  sem  pointer of semaphore object, mem refered by hdl pointer
 *                  in uni_sem_t will be freed internally.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_sem_free(uni_sem_t *sem);

/**
 * Acquire a semaphore.
 *
 * @param[in]  sem      semaphore object, it contains kernel obj pointer
 *                      which uni_sem_new alloced.
 * @param[in]  timeout  waiting until timeout in milliseconds.
 *                      value:  UNI_WAIT_FOREVER: wait mutex for ever.
 *                              0: return immediately if not get mutex.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_sem_wait(uni_sem_t *sem, unsigned int timeout);

/**
 * Release a semaphore.
 *
 * @param[in]  sem  semaphore object, it contains kernel obj pointer which uni_sem_new alloced.
 *                  It will wakeup one task which is waiting for the sem according to task priority.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_sem_signal(uni_sem_t *sem);

/**
 * Create a task.
 *
 * @param[in]  name       task name.
 * @param[in]  fn         function to run.
 * @param[in]  arg        argument of the function.
 * @param[in]  stacksize  stack-size in bytes.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_thread_new(const char *name, void* (*fn)(void *), void *arg, int stack_size);

/**
 * Get current thread id.
 *
 * @param[in] void
 *
 * @return thread id
 *
 */
unsigned int uni_thread_self(void);

/**
 * Sleep.
 *
 * @param[in]  msec       sleep msecond.
 *
 * @return  0: success, otherwise: fail.
 */
int uni_msleep(unsigned int msec);

/**
 * Get clock time in msecond.
 *
 * @param[in] void
 *
 * @return msecond
 */
long uni_get_clock_time_ms(void);

/**
 * Get UTC second.
 *
 * @param[in] void
 *
 * @return utc second
 */
long uni_get_utc_time_sec(void);

/**
 * Reboot system.
 *
 * @param[in] void
 *
 * @return void
 *
 */
void uni_reboot(void);

/**
 * memory APIs.
 */
#define uni_malloc  malloc
#define uni_free    free
#define uni_realloc realloc
#define uni_memset  memset
#define uni_assert  assert
#define uni_calloc  calloc

/**
 * socket APIs. all posix APIs.
 */

#ifdef __cplusplus
}
#endif
#endif // HAL_INC_PORTING_H_