
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
 * Description : uni_log.c
 * Author      : junlon2006@163.com
 * Date        : 2020.06.11
 *
 **************************************************************************/
#include "uni_log.h"
#include "porting.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define LOG_BUFFER_LEN       (1024 * 2)
#define uni_min(x,y)         (x > y ? y : x)
#define uni_max(x,y)         (x > y ? x : y)

static LogConfig   g_log_config = {1, 1, 1, 1, N_LOG_ALL};
static uni_mutex_t g_mutex = {0};
static int         g_inited = 0;

static const char* _level_tostring(LogLevel level) {
  switch (level) {
    case N_LOG_ERROR: return g_log_config.enable_color ?
                             "\033[0m\033[41;33m[E]\033[0m" : "[E]";
    case N_LOG_DEBUG: return g_log_config.enable_color ?
                             "\033[0m\033[47;33m[D]\033[0m" : "[D]";
    case N_LOG_TRACK: return g_log_config.enable_color ?
                             "\033[0m\033[42;33m[T]\033[0m" : "[T]";
    case N_LOG_WARN:  return g_log_config.enable_color ?
                             "\033[0m\033[41;33m[W]\033[0m" : "[W]";
    default:          return "[N/A]";
  }
}

static void _get_now_str(char *buf, int len) {
  long now = uni_get_clock_time_ms();
  snprintf(buf, len, "%ld ", now);
}

static void _get_thread_id_str(char *buf, int len) {
  unsigned int thread_id = uni_thread_self();
  snprintf(buf, len, "%x", (unsigned int)thread_id);
}

static int _fill_log_level(LogLevel level, char *buf, int len) {
  int write_len = 0;
  switch (level) {
    case N_LOG_DEBUG:
      write_len = snprintf(buf, len, "%s ", _level_tostring(N_LOG_DEBUG));
      break;
    case N_LOG_TRACK:
      write_len = snprintf(buf, len, "%s ", _level_tostring(N_LOG_TRACK));
      break;
    case N_LOG_WARN:
      write_len = snprintf(buf, len, "%s ", _level_tostring(N_LOG_WARN));
      break;
    case N_LOG_ERROR:
      write_len = snprintf(buf, len, "%s ", _level_tostring(N_LOG_ERROR));
      break;
    default:
      break;
  }
  return uni_max(0, write_len);
}

static int _fill_tag(char *buf, int len, const char *tag) {
  return uni_max(0, snprintf(buf, len, "<%s>", tag));
}

static int _fill_time(char *buf, int len) {
  char now[64];
  if (!g_log_config.enable_time) {
    return 0;
  }
  _get_now_str(now, sizeof(now));
  return uni_max(0, snprintf(buf, len, "%s", now));
}

static int _fill_function_line(char *buf, int len, const char *function, int line) {
  return (g_log_config.enable_function_line ?
          uni_max(0, snprintf(buf, len, "%s:%d->", function, line)) : 0);
}

static int _fill_thread_id(char *buf, int len) {
  char thread_id[32];
  if (!g_log_config.enable_thread_id) {
    return 0;
  }
  _get_thread_id_str(thread_id, sizeof(thread_id));
  return uni_max(0, snprintf(buf, len, "%s", thread_id));
}

static void _fill_customer_info(char *buf, int len, char *fmt, va_list args,
                                int append_feed_line) {
  int length, remain_len;
  length = vsnprintf(buf, len, fmt, args);
  length = uni_max(length, 0);
  length = uni_min(length, len);
  remain_len = len - length;
  if (0 == remain_len) {
    if (append_feed_line) {
      buf[len - 2] = '\n';
    }
    buf[len - 1] = '\0';
    return;
  }
  if (1 == remain_len) {
    if (append_feed_line) {
      buf[len - 2] = '\n';
    }
    return;
  }
  if (append_feed_line) {
    strncat(buf, "\n", remain_len);
  }
  return;
}

int LogLevelValid(LogLevel level) {
  return level <= g_log_config.set_level ? 1 : 0;
}

#define _log_assemble(buf, level, tags, function, line, fmt, args) do { \
  int len = 0; \
  if (level != N_LOG_RAW) { \
    len += _fill_log_level(level, buf + len, LOG_BUFFER_LEN - len); \
    len += _fill_time(buf + len, LOG_BUFFER_LEN - len); \
    len += _fill_thread_id(buf + len, LOG_BUFFER_LEN - len); \
    len += _fill_tag(buf + len, LOG_BUFFER_LEN - len, tags); \
    len += _fill_function_line(buf + len, LOG_BUFFER_LEN - len, \
                               function, line); \
  } \
  _fill_customer_info(buf + len, LOG_BUFFER_LEN - len, fmt, args, \
                      level != N_LOG_RAW); \
} while (0)

static int _sync_write_process(LogLevel level, const char *tags,
                               const char *function, int line,
                               char *fmt, va_list args) {
  static char buf[LOG_BUFFER_LEN];
  uni_mutex_lock(&g_mutex);
  _log_assemble(buf, level, tags, function, line, fmt, args);
  printf("%s", buf);
  uni_mutex_unlock(&g_mutex);
  return 0;
}

static void _lazy_init() {
  if (!g_inited) {
    uni_mutex_new(&g_mutex);
    g_inited = 1;
  }
}

int LogWrite(LogLevel level, const char *tags, const char *function, int line,
             char *fmt, ...) {
  va_list args;
  _lazy_init();
  va_start(args, fmt);
  _sync_write_process(level, tags, function, line, fmt, args);
  va_end(args);
  return 0;
}

int LogLevelSet(LogLevel level) {
  g_log_config.set_level = level;
  return 0;
}

int LogInitialize(LogConfig logConfig) {
  g_log_config.enable_time          = logConfig.enable_time;
  g_log_config.enable_thread_id     = logConfig.enable_thread_id;
  g_log_config.enable_function_line = logConfig.enable_function_line;
  g_log_config.enable_color         = logConfig.enable_color;
  g_log_config.set_level            = logConfig.set_level;

  uni_mutex_new(&g_mutex);
  g_inited = 1;
  return 0;
}

int LogFinalize(void) {
  uni_mutex_free(&g_mutex);
  return 0;
}
