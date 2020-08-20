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
 * @brief IoT设备向蜂鸟M设置参数，可扩展
 * @param param
 * @return 0 成功，-1 失败
 */
int ChnlIotDeviceSetParam(ChnIoTSetParam *param);

/**
 * @brief IoT设备向蜂鸟M发送控制命令
 * @param cmd 控制命令
 * @param payload 控制命令参数列表
 * @param payload_len 控制命令参数列表长度
 * @return 0 成功，-1 失败
 */
int ChnlIotDevicePushCmd(unsigned int cmd, char *payload, unsigned int payload_len);

#ifdef __cplusplus
}
#endif
#endif  // SDK_CHANNEL_INC_UNI_CHANNEL_H_