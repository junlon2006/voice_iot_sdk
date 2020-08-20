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
#include "app.h"
#include "uni_uart.h"
#include "uni_log.h"
#include "stdio.h"

#define TAG "main"

#define MAX_PAYLOAD_LEN (16)
static void _hbm_command_callback(char *payload, uint32_t len) {
  /* payload 不带'\0', 追加字符串结尾符号 */
  char tmp[MAX_PAYLOAD_LEN] = {0};
  strncpy(tmp, payload, ((len < MAX_PAYLOAD_LEN) ? len : (MAX_PAYLOAD_LEN - 1)));
  LOGT(TAG, "param=%s", tmp);
  ChnlIotDevicePushCmd(CHNL_MSG_IOT_HBM_ACTION_PLAY, payload, len);
}

static void _hbm_command_cb(uint32_t cmd, char *payload, uint32_t len) {
  switch (cmd) {
    case CHNL_MSG_HBM_IOT_ASR_RESULT:
    _hbm_command_callback(payload, len);
    break;
  }
}

static void _uart_init(int argc, char *argv[]) {
  UartConfig uart_config;
  snprintf(uart_config.device, sizeof(uart_config.device), "%s", argv[1]);
  uart_config.speed = B921600;
  if (0 != UartInitialize(&uart_config)) {
    LOGE(TAG, "uart init failed.");
  }
}

int main(int argc, char *argv[]) {
  LogLevelSet(N_LOG_TRACK);
  _uart_init(argc, argv);
  unisound_app_start(_hbm_command_cb);
  while (1) usleep(1000 * 1000);
  return 0;
}
