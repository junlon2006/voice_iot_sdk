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
 * Description : uni_channel.c
 * Author      : junlon2006@163.com
 * Date        : 2020.07.21
 *
 **************************************************************************/
#include "uni_channel.h"
#include "uni_log.h"
#include "uni_event_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "porting.h"

#define TAG "channel"

typedef struct {
  EventListHandle event_handle;
  EventListHandle event_non_block_handle;
  hbm_command_cb  cmd_callback;
  uint32_t        inited;
  uint8_t         adpcm_packet_cnt;
} Channel;

static Channel g_channel = {0};

static bool _is_non_block_cmd(int cmd) {
  return (cmd == CHNL_MSG_IOT_INIT ||
          cmd == CHNL_MSG_ASR_CHALLENGE_PACK ||
          cmd == CHNL_MSG_DAEMON_REBOOT_REQUEST);
}

/**
 * 保护内存使用，对于大内存块，限制个数，目前监控ADPCM数据包128Byte每包，最大不得缓存超过2KB即16个
 */
static bool _is_memory_cost_packet_limited(int cmd) {
  if (cmd != CHNL_MSG_IOT_RASR_DATA_FEED) return false;
  if (g_channel.adpcm_packet_cnt++ >= 16) return true;
  return false;
}

/* interrupt callback, cannot block */
void ChnlReceiveCommProtocolPacket(CommPacket *packet) {
  if (_is_memory_cost_packet_limited(packet->cmd)) {
    return;
  }

  CommPacket *event = (CommPacket *)uni_malloc(packet->payload_len + sizeof(CommPacket));
  if (NULL == event) {
    LOGT(TAG, "alloc memory failed.");
    return;
  }

  event->cmd         = packet->cmd;
  event->payload     = (char *)event + sizeof(CommPacket);
  event->payload_len = packet->payload_len;
  if (event->payload_len > 0) {
    memcpy(event->payload, packet->payload, packet->payload_len);
  }

  if (_is_non_block_cmd(event->cmd)) {
    EventListAdd(g_channel.event_non_block_handle, event, EVENT_LIST_PRIORITY_HIGHEST);
  } else {
    EventListAdd(g_channel.event_handle, event, EVENT_LIST_PRIORITY_HIGHEST);
  }
}

void _do_iot_device_init(char *packet, int len) {
  ChIoTInitParam *param = (ChIoTInitParam *)packet;
  LOGT(TAG, "recv iot init event");
}

static void _do_iot_device_rasr_start(char *packet, int len) {
  ChnIoTRasrStartParam *param = (ChnIoTRasrStartParam *)packet;
  LOGT(TAG, "recv iot rasr start. vui_session_id=%u", param->vui_session_id);
}

static void _do_iot_device_rasr_stop(char *packet, int len) {
  LOGT(TAG, "recv iot rasr stop");
}

static void _do_iot_rasr_adpcm_data(char *packet, int len) {
  ChnIoTRasrFeedDataParam *param = (ChnIoTRasrFeedDataParam *)packet;
  g_channel.adpcm_packet_cnt--;
  //param->adpcm数组中即为ADPCM数据
  LOGT(TAG, "recv adpcm data. cache[%d]", g_channel.adpcm_packet_cnt);
}

static void _do_reboot_request(char *packet, int len) {
  LOGE(TAG, "recv daemon reboot cmd");
  uni_reboot();
}

static int _do_challenge_pack(char *packet, int len) {
  static uint32_t current_sequence = 1;
  ChnIoTChallengePackParam *param = (ChnIoTChallengePackParam *)packet;

  ChnIoTChallengePackAck ack;
  ack.sequence      = current_sequence;
  ack.net_connected = 1; //TODO, 网络状态，如果设置为0蜂鸟M将停止推送ADPCM数据
  snprintf(ack.version, sizeof(ack.version), "%s", IOT_DEVICE_VERSION);

  LOGE(TAG, "receive challenge pack, cur_seq=%u, net=%d", current_sequence, ack.net_connected);

  CommAttribute attr;
  attr.reliable = true;
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_ASR_CHALLENGE_PACK_ACK,
                                              (char *)&ack,
                                              sizeof(ack),
                                              &attr);
  if (ret != 0) {
    LOGW(TAG, "transmit failed");
    return -1;
  }

  current_sequence++;
  return 0;
}

static void _do_netconfigure(char *packet, int len) {
  LOGT(TAG, "recv netconfig");
  //TODO 开始配网
}

static void _do_netconfigure_stop(char *packet, int len) {
  LOGT(TAG, "netconfig stop");
  //TODO 停止配网
}

static void _cmd_to_iot_device_filter(CommPacket *packet) {
  //回调给IoT设备，filter事件，根据消息范围收敛在HBM master中，slave不感知
  if (packet->cmd > CHNL_MSG_HBM_IOT_DEVICE_BASE) {
    if (g_channel.cmd_callback) {
      g_channel.cmd_callback(packet->cmd, packet->payload, packet->payload_len);
    }
  }
}

static void _event_handle(void *event) {
  CommPacket *packet = (CommPacket *)event;
  _cmd_to_iot_device_filter(packet);

  switch (packet->cmd) {
    case CHNL_MSG_IOT_INIT:
      _do_iot_device_init(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_IOT_RASR_START:
      _do_iot_device_rasr_start(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_IOT_RASR_STOP:
      _do_iot_device_rasr_stop(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_IOT_RASR_DATA_FEED:
      _do_iot_rasr_adpcm_data(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_IOT_NET_CONFIGURE:
      _do_netconfigure(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_IOT_NET_CONFIGURE_STOP:
      _do_netconfigure_stop(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_DAEMON_REBOOT_REQUEST:
      _do_reboot_request(packet->payload, packet->payload_len);
      break;
    case CHNL_MSG_ASR_CHALLENGE_PACK:
      _do_challenge_pack(packet->payload, packet->payload_len);
      break;
    default:
      LOGT(TAG, "unhandled event. cmd=%d", packet->cmd);
      break;
  }

  uni_free(event);
}

static void _create_event_list() {
  g_channel.event_handle           = EventListCreate(_event_handle, 2048);
  g_channel.event_non_block_handle = EventListCreate(_event_handle, 2048);
}

static void _register_cmd_callback(hbm_command_cb cmd_callback) {
  g_channel.cmd_callback = cmd_callback;
}

static void _set_channel_inited() {
  g_channel.inited = 1;
}

static int _is_channel_inited() {
  return g_channel.inited;
}

int ChnlInit(hbm_command_cb cmd_callback) {
  _create_event_list();
  _register_cmd_callback(cmd_callback);
  _set_channel_inited();
  LOGT(TAG, "channel init success");
  return 0;
}

int ChnlIotDeviceRasrResult(ChnIoTRasrResult *result) {
  CommAttribute attr = {1};
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_IOT_RASR_RESULT,
                                              (char *)result,
                                              sizeof(ChnIoTRasrResult) + strlen(result->cmd_hash_string) + 1,
                                              &attr);
  if (ret != 0) {
    LOGT(TAG, "transmit failed. err=%d", ret);
    return -1;
  }
  return 0;
}

int ChnlIotDeviceSetParam(ChnIoTSetParam *param) {
  return 0;
}

int ChnlIotDeviceNetConfigureStatus(ChnIoTNetConfigureStatus *status) {
  CommAttribute attr = {1};
  int ret = CommProtocolPacketAssembleAndSend(CHNL_MSG_IOT_NET_CONFIGURE_STATUS,
                                              (char *)status,
                                              sizeof(ChnIoTNetConfigureStatus),
                                              &attr);
  if (ret != 0) {
    LOGT(TAG, "transmit failed. err=%d", ret);
    return -1;
  }
  return 0;
}

int ChnlIotDevicePushCmd(uint32_t cmd, char *payload, uint32_t payload_len) {
  CommAttribute attr = {1};
  if (!_is_channel_inited()) {
    LOGE(TAG, "module not init");
    return -1;
  }

  int ret = CommProtocolPacketAssembleAndSend(cmd,
                                              payload,
                                              payload_len,
                                              &attr);
  if (ret != 0) {
    LOGT(TAG, "transmit failed. err=%d", ret);
    return -1;
  }
  return 0;
}
