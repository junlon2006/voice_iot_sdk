/**************************************************************************
 * Copyright (C) 2020-2020  Junlon2006
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
 * Description : porting.c
 * Author      : junlon2006@163.com
 * Date        : 2020.07.21
 *
 **************************************************************************/
#include "porting.h"
#include "errcode.h"

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

int uni_mutex_new(uni_mutex_t *mutex) {
    if (NULL_PTR_CHECK(mutex)) {
        return -INVALID_IN_PARAM;
    }

    mutex->hdl = malloc(sizeof(pthread_mutex_t));
    if (NULL_PTR_CHECK(mutex->hdl)) {
        return -OUT_OF_MEMORY;
    }

    pthread_mutex_init((pthread_mutex_t *)mutex->hdl, NULL);
    return OK;
}

int uni_mutex_free(uni_mutex_t *mutex) {
    if (NULL_PTR_CHECK(mutex) || NULL_PTR_CHECK(mutex->hdl)) {
        return -INVALID_IN_PARAM;
    }

    free(mutex->hdl);
    mutex->hdl = NULL;

    return OK;
}

int uni_mutex_lock(uni_mutex_t *mutex) {
    if (NULL_PTR_CHECK(mutex) || NULL_PTR_CHECK(mutex->hdl)) {
        return -INVALID_IN_PARAM;
    }

    pthread_mutex_lock((pthread_mutex_t *)mutex->hdl);
    return OK;
}

int uni_mutex_unlock(uni_mutex_t *mutex) {
    if (NULL_PTR_CHECK(mutex) || NULL_PTR_CHECK(mutex->hdl)) {
        return -INVALID_IN_PARAM;
    }

    pthread_mutex_unlock((pthread_mutex_t *)mutex->hdl);
    return OK;
}

int uni_sem_new(uni_sem_t *sem, unsigned int count) {
    if (NULL_PTR_CHECK(sem)) {
        return -INVALID_IN_PARAM;
    }

    sem->hdl = malloc(sizeof(sem_t));
    if (NULL_PTR_CHECK(sem->hdl)) {
        return -OUT_OF_MEMORY;
    }

    sem_init((sem_t *)sem->hdl, 0, count);
    return OK;
}

int uni_sem_free(uni_sem_t *sem) {
    if (NULL_PTR_CHECK(sem) || NULL_PTR_CHECK(sem->hdl)) {
        return -INVALID_IN_PARAM;
    }

    free(sem->hdl);
    sem->hdl = NULL;
    return OK;
}

static int __sem_timedwait(void *sem, unsigned int msec) {
    struct timespec ts;
    sem_t *s = (sem_t *)sem;
    long long nsec;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec = ts.tv_sec + (msec / 1000);
    nsec = ts.tv_nsec + ((msec % 1000) * 1000000UL);
    if (nsec >= 1000000000UL) {
        ts.tv_sec += 1;
        ts.tv_nsec = nsec - 1000000000;
    } else {
        ts.tv_nsec = nsec;
    }

    return sem_timedwait(s, &ts);
}

int uni_sem_wait(uni_sem_t *sem, unsigned int timeout) {
    if (NULL_PTR_CHECK(sem) || NULL_PTR_CHECK(sem->hdl)) {
        return -INVALID_IN_PARAM;
    }

    if (timeout == UNI_WAIT_FOREVER) {
        sem_wait((sem_t *)sem->hdl);
        return OK;
    }

    __sem_timedwait((sem_t *)sem->hdl, timeout);
    return OK;
}

int uni_sem_signal(uni_sem_t *sem) {
    if (NULL_PTR_CHECK(sem) || NULL_PTR_CHECK(sem->hdl)) {
        return -INVALID_IN_PARAM;
    }

    sem_post((sem_t *)sem->hdl);
    return OK;
}

int uni_thread_new(const char *name, void* (*fn)(void *), void *arg, int stack_size) {
    pthread_t pid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, stack_size);
    if (0 != pthread_create(&pid, &attr, fn, arg)) {
        pthread_attr_destroy(&attr);
        return -CREATE_TASK_FAIL;
    }

    pthread_detach(pid);
    pthread_attr_destroy(&attr);
    return OK;
}

unsigned int uni_thread_self(void) {
    return pthread_self();
}

int uni_msleep(unsigned int msec) {
    usleep(msec * 1000);
    return OK;
}

long uni_get_clock_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

long uni_get_utc_time_sec(void) {
    return time(NULL);
}

void uni_reboot(void) {
    //TODO系统重启
}