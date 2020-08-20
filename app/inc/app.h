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
 * Description : app.h
 * Author      : junlon2006@163.com
 * Date        : 2020.07.21
 *
 **************************************************************************/
#ifndef APP_INC_APP_H_
#define APP_INC_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "uni_channel.h"

/**
 * @brief unisound app初始化函数
 * @param cmd_callback 蜂鸟控制命令回调IoT设备hook
 * @return 0 成功，-1 失败
 */
int unisound_app_start(hbm_command_cb cmd_callback);

/**
 * @brief IoT设备向蜂鸟M发送控制命令
 * @param cmd 控制命令
 * @param payload 控制命令参数列表
 * @param payload_len 控制命令参数列表长度
 * @return 0 成功，-1 失败
 */
int iot_device_send_command_to_hbm(uint32_t cmd, char *payload, uint32_t payload_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* APP_INC_APP_H_ */