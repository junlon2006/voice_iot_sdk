/**************************************************************************
 * Copyright (C) 2020-2020  Unisound
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
 * Description : uni_channel.h
 * Author      : junlon2006@163.com
 * Date        : 2020.07.21
 *
 **************************************************************************/
#ifndef SDK_CHANNEL_INC_UNI_CHANNEL_COMMON_H_
#define SDK_CHANNEL_INC_UNI_CHANNEL_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_communication.h"

#define CHANNEL_BASE  100

enum {
  CHNL_MSG_IOT_INIT = CHANNEL_BASE,
  CHNL_MSG_IOT_RASR_START,
  CHNL_MSG_IOT_RASR_STOP,
  CHNL_MSG_IOT_RASR_DATA_FEED,
  CHNL_MSG_IOT_RASR_RESULT,
  CHNL_MSG_IOT_SET_PARAM,
  CHNL_MSG_IOT_NET_CONFIGURE_STATUS,
  CHNL_MSG_DAEMON_REBOOT_REQUEST,
  CHNL_MSG_ASR_CHALLENGE_PACK,
  CHNL_MSG_ASR_CHALLENGE_PACK_ACK,
  CHNL_MSG_IOT_HBM_STATE_CTRL,
  CHNL_MSG_IOT_HBM_ACTION_PLAY,
  CHNL_MSG_IOT_HBM_PLAY_AUDIO,
  CHNL_MSG_IOT_HBM_PLAY_BUZZER,
  CHNL_MSG_IOT_HBM_REBOOT,

  CHNL_MSG_HBM_IOT_DEVICE_BASE = 1000, //1001开始的所有msg为IoT端需要感知处理的消息
  CHNL_MSG_HBM_IOT_ASR_RESULT,
  CHNL_MSG_IOT_NET_CONFIGURE,
  CHNL_MSG_IOT_NET_CONFIGURE_STOP,
};

typedef struct {
  char appkey[48];
  char appsecret[36];
} UNI_PACKED ChIoTInitParam;

typedef struct {
  unsigned int vui_session_id;
} UNI_PACKED ChnIoTRasrStartParam;

typedef struct {
} UNI_PACKED ChnIoTRasrStartStopParam;

typedef struct {
  char adpcm[128];
} UNI_PACKED ChnIoTRasrFeedDataParam;

typedef struct {
  unsigned int vui_session_id;
  unsigned int cmd_hash_code;
  char         cmd_hash_string[0];
} UNI_PACKED ChnIoTRasrResult;

typedef struct {
} UNI_PACKED ChnIoTSetParam;

typedef struct {
  unsigned int sequence;
  unsigned int net_connected;
  char         version[28];
} UNI_PACKED ChnIoTChallengePackAck;

typedef struct {
  unsigned int status; //0成功，1失败
} UNI_PACKED ChnIoTNetConfigureStatus;

typedef struct {
  ChIoTInitParam init;
} UNI_PACKED ChnIoTChallengePackParam;

#ifdef __cplusplus
}
#endif
#endif // SDK_CHANNEL_INC_UNI_CHANNEL_COMMON_H_