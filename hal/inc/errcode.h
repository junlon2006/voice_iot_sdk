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
 * Description : errcode.h
 * Author      : junlon2006@163.com
 * Date        : 2020.07.21
 *
 **************************************************************************/
#ifndef HAL_INC_ERRCODE_H_
#define HAL_INC_ERRCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
    FAIL = -1,
    OK,
    OUT_OF_MEMORY,
    INVALID_IN_PARAM,
    CREATE_TASK_FAIL,
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* HAL_INC_ERRCODE_H_ */