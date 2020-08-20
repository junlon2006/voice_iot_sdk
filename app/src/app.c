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
 * Description : app.c
 * Author      : junlon2006@163.com
 * Date        : 2020.07.21
 *
 **************************************************************************/
#include "uni_log.h"
#include "uni_uart.h"
#include "uni_channel.h"
#include "uni_communication.h"
#include "porting.h"

#define TAG "app"

/* 串口通信协议栈跨平台APIs hook注册函数集合 */
static void* _comm_protocol_sem_alloc_fn() {
  return uni_malloc(sizeof(uni_sem_t));
}

static int _comm_protocol_sem_init(void *sem, unsigned int value) {
  uni_sem_t *s = (uni_sem_t *)sem;
  return uni_sem_new(s, value);
}

static void _comm_protocol_sem_destroy_fn(void *sem) {
  uni_sem_t *s = (uni_sem_t *)sem;
  uni_sem_free(s);
  uni_free(s);
}

static int _comm_protocol_sem_post_fn(void *sem) {
  uni_sem_t *s = (uni_sem_t *)sem;
  return uni_sem_signal(s);
}

static int _comm_protocol_sem_wait_fn(void *sem) {
  uni_sem_t *s = (uni_sem_t *)sem;
  return uni_sem_wait(s, UNI_WAIT_FOREVER);
}

static int _comm_protocol_sem_timedwait_fn(void *sem, unsigned int timeout_msecond) {
  uni_sem_t *s = (uni_sem_t *)sem;
  return uni_sem_wait(s, timeout_msecond);
}

int unisound_app_start(hbm_command_cb cmd_callback) {
  CommProtocolHooks hooks = {0};
  hooks.free_fn    = uni_free;
  hooks.malloc_fn  = uni_malloc;
  hooks.msleep_fn  = uni_msleep;
  hooks.realloc_fn = uni_realloc;

  hooks.sem_alloc_fn     = _comm_protocol_sem_alloc_fn;
  hooks.sem_destroy_fn   = _comm_protocol_sem_destroy_fn;
  hooks.sem_init_fn      = _comm_protocol_sem_init;
  hooks.sem_post_fn      = _comm_protocol_sem_post_fn;
  hooks.sem_wait_fn      = _comm_protocol_sem_wait_fn;
  hooks.sem_timedwait_fn = _comm_protocol_sem_timedwait_fn;

  CommProtocolRegisterHooks(&hooks);
  CommProtocolInit(UartWrite, ChnlReceiveCommProtocolPacket);
  ChnlInit(cmd_callback);
  return 0;
}

int iot_device_send_command_to_hbm(uint32_t cmd, char *payload, uint32_t payload_len) {
  return ChnlIotDevicePushCmd(cmd, payload, payload_len);
}