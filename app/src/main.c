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

#include <unistd.h>
#include <fcntl.h>

#define TAG "main"

/* 调用播报API使用方式demo */
static void _response_broadcast_demo(const char *file_name) {
  static char raw_pcm[512]; //静态内存，不占用栈上空间；也可以换成malloc堆上空间
  int fd, len;

  /* step1: 打开pcm文件句柄 */
  fd = open(file_name, O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0664);
  if (fd <= 0) {
    LOGE(TAG, "open file failed.");
    return;
  }

  /* step2: 从文件中读出数据，流式传输给蜂鸟M进行播放，以此来减少内存开销 */
  while (1) {
    len = read(fd, raw_pcm, sizeof(raw_pcm));
    if (len > 0) {
      //读出的长度小于4K，说明文件已经读取完毕，不足4K的部分置零（追加静音）
      if (len != sizeof(raw_pcm)) {
        memset(raw_pcm + len, 0, sizeof(raw_pcm) - len);
      }

      //流式调用播放接口，len必须是512的整数倍，详情参照头文件注释
      ChnlIotDeviceFeedAudioData(raw_pcm, len);
    }

    //文件读取完毕，退出
    if (len < sizeof(raw_pcm)) {
      LOGT(TAG, "end of file. [%s]", file_name);
      break;
    }
  }

  /* step3: 关闭文件句柄 */
  close(fd);
}

#define MAX_PAYLOAD_LEN (16)
static void _hbm_command_callback(char *payload, uint32_t len) {
  /* payload 不带'\0', 追加字符串结尾符号 */
  char tmp[MAX_PAYLOAD_LEN] = {0};
  strncpy(tmp, payload, ((len < MAX_PAYLOAD_LEN) ? len : (MAX_PAYLOAD_LEN - 1)));
  LOGT(TAG, "param=%s", tmp);

  /* 根据离线识别结果，进行播报应答 */
  if (0 == strcmp(tmp, "wakeup_uni")) {
    _response_broadcast_demo("wozai.pcm");
  } else if (0 == strcmp(tmp, "exitUni")) {
    _response_broadcast_demo("youxuyaozaijiaowo.pcm");
  } else if (0 == strcmp(tmp, "ac_power_on")) {
    _response_broadcast_demo("yiweinidakaifengshan.pcm");
  } else {
    _response_broadcast_demo("ceshidefault.pcm");
  }
}

/* 离线识别结果回调函数 */
static void _hbm_command_cb(uint32_t cmd, char *payload, uint32_t len) {
  switch (cmd) {
    case CHNL_MSG_HBM_IOT_ASR_RESULT:
      _hbm_command_callback(payload, len);
      break;
    default:
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
