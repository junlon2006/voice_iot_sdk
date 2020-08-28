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
#ifndef SDK_CHANNEL_INC_UNI_CHANNEL_H_
#define SDK_CHANNEL_INC_UNI_CHANNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel_common.h"
#include "uni_communication.h"
#include "porting.h"

typedef void (* hbm_command_cb)(uint32_t cmd, char *payload, uint32_t len);

/**
 * @brief channel全局初始化
 * @param cmd_callback
 * @return 0 成功, -1 失败
 */
int ChnlInit(hbm_command_cb cmd_callback);

/**
 * @brief channel packet解析函数
 * @param packet
 * @return void
 */
void ChnlReceiveCommProtocolPacket(CommPacket *packet);

/**
 * @brief IoT设备上报配网状态
 * @param status
 * @return 0 成功，-1 失败
 */
int ChnlIotDeviceNetConfigureStatus(ChnIoTNetConfigureStatus *status);

/**
 * @brief IoT设备发送RASR识别结果到蜂鸟M
 * @param result
 * @return 0 成功，-1 失败
 */
int ChnlIotDeviceRasrResult(ChnIoTRasrResult *result);

/**
 * @brief IoT设备向蜂鸟M发送控制命令
 * @param cmd 控制命令
 * @param payload 控制命令参数列表
 * @param payload_len 控制命令参数列表长度
 * @return 0 成功，-1 失败
 */
int ChnlIotDevicePushCmd(unsigned int cmd, char *payload, unsigned int payload_len);

/**
 * @brief IoT设备向蜂鸟M发送音频播报raw PCM数据
 * @param pcm pcm数据buffer首指针
 * @param len pcm数据长度字节数 [注意：len必须是512的倍数，除了最后一次可以不是512倍数]
 * Tips: 流式推送，务必满足len的条件，比如音频1026字节数据，推送长度可以是[512，512，2]
 * 如果不采用流式推送，也可以一次推送完，比如音频200K字节，可以一次推送完，len = 200K
 */
int ChnlIotDeviceFeedAudioData(char *pcm, int len);

#ifdef __cplusplus
}
#endif
#endif  // SDK_CHANNEL_INC_UNI_CHANNEL_H_