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
 * Description : uni_event_list.c
 * Author      : junlon2006@163.com
 * Date        : 2018.06.19
 *
 **************************************************************************/
#include "uni_event_list.h"
#include "list_head.h"
#include "porting.h"
#include "errcode.h"
#include "uni_log.h"
#include <stdio.h>

#define _consume_highest_list              _consume_one_list
#define _consume_medium_list               _consume_one_list
#define _consume_lowest_list               _consume_one_list

#define TAG "event_list"

typedef int (*_interruptable_handler)(void *args);

typedef struct {
  list_head link;
  int       priority;
  void      *event;
} EventListItem;

typedef struct {
  list_head              highest_list;
  list_head              medium_list;
  list_head              lowest_list;
  int                    highest_cnt;
  int                    medium_cnt;
  int                    lowest_cnt;
  EventHandler           event_handler;
  _interruptable_handler highest_interrupt_handler;
  _interruptable_handler medium_interrupt_handler;
  _interruptable_handler lowest_interrupt_handler;
  uni_mutex_t            mutex;
  uni_sem_t              sem_new_event;
  uni_sem_t              sem_thread_exit_sync;
  int                    is_running;
} EventList;

static inline void _consume_one_list(list_head *header,
                                     EventHandler event_handler,
                                     _interruptable_handler interrupt,
                                     void *interrupt_args,
                                     uni_mutex_t *mutex,
                                     int *total_cnt,
                                     int *is_running) {
  list_head *p;
  EventListItem *item;
  do {
    uni_mutex_lock(mutex);
    if (NULL != interrupt && interrupt(interrupt_args)) {
      uni_mutex_unlock(mutex);
      break;
    }
    if (NULL == (p = list_get_head(header))) {
      uni_mutex_unlock(mutex);
      break;
    }
    item = list_entry(p, EventListItem, link);
    list_del(&item->link);
    *total_cnt = *total_cnt - 1;
    uni_mutex_unlock(mutex);
    if (NULL != event_handler && *is_running) {
      event_handler(item->event);
    }
    uni_free(item);
  } while (p != NULL);
}

static int _highest_interruptable_handler(void *args) {
  return 0;
}

static int _medium_interruptable_handler(void *args) {
  EventList *event_list = (EventList *)args;
  return (0 < event_list->highest_cnt);
}

static int _lowest_interruptable_handler(void *args) {
  EventList *event_list = (EventList *)args;
  return (0 < event_list->highest_cnt || 0 < event_list->medium_cnt);
}

static void _consume_event_list(EventList *event_list) {
  _consume_highest_list(&event_list->highest_list, event_list->event_handler,
                        event_list->highest_interrupt_handler, event_list,
                        &event_list->mutex,
                        &event_list->highest_cnt, &event_list->is_running);
  _consume_medium_list(&event_list->medium_list, event_list->event_handler,
                       event_list->medium_interrupt_handler, event_list,
                       &event_list->mutex,
                       &event_list->medium_cnt, &event_list->is_running);
  _consume_lowest_list(&event_list->lowest_list, event_list->event_handler,
                       event_list->lowest_interrupt_handler, event_list,
                       &event_list->mutex,
                       &event_list->lowest_cnt, &event_list->is_running);
}

static void _reset_event_handler(EventList *event_list) {
  event_list->event_handler = NULL;
}

static void _reset_interrupt_handler(EventList *event_list) {
  event_list->highest_interrupt_handler = NULL;
  event_list->medium_interrupt_handler = NULL;
  event_list->lowest_interrupt_handler = NULL;
}

static void _destroy_all(EventList *event_list) {
  _reset_event_handler(event_list);
  _reset_interrupt_handler(event_list);
  _consume_event_list(event_list);
  uni_mutex_free(&event_list->mutex);
  uni_sem_free(&event_list->sem_new_event);
  uni_sem_free(&event_list->sem_thread_exit_sync);
  uni_free(event_list);
}

static int _all_list_empty(EventList *event_list) {
  return (0 == event_list->highest_cnt &&
          0 == event_list->medium_cnt &&
          0 == event_list->lowest_cnt);
}

static void _try_sem_wait(EventList *event_list) {
  int need_wait = 0;
  if (_all_list_empty(event_list)) {
    uni_mutex_lock(&event_list->mutex);
    if (_all_list_empty(event_list)) {
      need_wait = 1;
    }
    uni_mutex_unlock(&event_list->mutex);
    if (need_wait) {
      uni_sem_wait(&event_list->sem_new_event, UNI_WAIT_FOREVER);
    }
  }
}

static void* _event_handler(void *args) {
  EventList *event_list = (EventList *)args;
  while (event_list->is_running) {
    _consume_event_list(event_list);
    _try_sem_wait(event_list);
  }

  uni_sem_signal(&event_list->sem_thread_exit_sync);
  return NULL;
}

static void _mzero_event_list(EventList *event_list) {
  uni_memset(event_list, 0, sizeof(EventList));
}

static void _list_init(EventList *event_list) {
  list_init(&event_list->highest_list);
  list_init(&event_list->medium_list);
  list_init(&event_list->lowest_list);
}

static void _register_event_handler(EventList *event_list,
                                    EventHandler event_handler) {
  event_list->event_handler = event_handler;
}

static void _register_interuppt_handler(EventList *event_list) {
  event_list->highest_interrupt_handler = _highest_interruptable_handler;
  event_list->medium_interrupt_handler  = _medium_interruptable_handler;
  event_list->lowest_interrupt_handler  = _lowest_interruptable_handler;
}

static void _worker_thread_create(EventList *event_list, unsigned int stack_size) {
  uni_sem_new(&event_list->sem_new_event, 0);
  uni_sem_new(&event_list->sem_thread_exit_sync, 0);
  uni_mutex_new(&event_list->mutex);
  event_list->is_running = 1;
  int ret = uni_thread_new(TAG, _event_handler, event_list, stack_size);
  uni_assert(ret == OK);
}

EventListHandle EventListCreate(EventHandler event_handler, unsigned int stack_size) {
  EventList *event_list;
  if (NULL == (event_list = (EventList *)uni_malloc(sizeof(EventList)))) {
    LOGE(TAG, "malloc failed !");
    return NULL;
  }

  _mzero_event_list(event_list);
  _list_init(event_list);
  _register_event_handler(event_list, event_handler);
  _register_interuppt_handler(event_list);
  _worker_thread_create(event_list, stack_size);
  return event_list;
}

static void _worker_thread_exit(EventList *event_list) {
  event_list->is_running = 0;
  uni_sem_signal(&event_list->sem_new_event);
  uni_sem_wait(&event_list->sem_thread_exit_sync, UNI_WAIT_FOREVER);
}

int EventListDestroy(EventListHandle handle) {
  EventList *event_list = (EventList *)handle;
  _worker_thread_exit(event_list);
  _destroy_all(event_list);
  return 0;
}

int EventListAdd(EventListHandle handle, void *event, int priority) {
  EventListItem *item = NULL;
  EventList *event_list = (EventList *)handle;
  if (NULL == (item = (EventListItem *)uni_malloc(sizeof(EventListItem)))) {
    LOGE(TAG, "alloc memory failed");
    return -1;
  }

  item->priority = priority;
  item->event    = event;
  uni_mutex_lock(&event_list->mutex);
  if (EVENT_LIST_PRIORITY_HIGHEST == priority) {
    list_add_tail(&item->link, &event_list->highest_list);
    event_list->highest_cnt++;
  } else if (EVENT_LIST_PRIORITY_MEDIUM == priority) {
    list_add_tail(&item->link, &event_list->medium_list);
    event_list->medium_cnt++;
  } else if (EVENT_LIST_PRIORITY_LOWEST == priority) {
    list_add_tail(&item->link, &event_list->lowest_list);
    event_list->lowest_cnt++;
  }
  uni_mutex_unlock(&event_list->mutex);
  uni_sem_signal(&event_list->sem_new_event);
  return 0;
}

int EventListClear(EventListHandle handle) {
  EventList *event_list = (EventList *)handle;
  _consume_highest_list(&event_list->highest_list, NULL, NULL, NULL,
                        &event_list->mutex, &event_list->highest_cnt,
                        &event_list->is_running);
  _consume_medium_list(&event_list->medium_list, NULL, NULL, NULL,
                       &event_list->mutex, &event_list->medium_cnt,
                       &event_list->is_running);
  _consume_lowest_list(&event_list->lowest_list, NULL, NULL, NULL,
                       &event_list->mutex, &event_list->lowest_cnt,
                       &event_list->is_running);
  return 0;
}
