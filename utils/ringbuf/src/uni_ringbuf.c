/**************************************************************************
 * Copyright (C) 2018-2019 Junlon2006
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
 * Description : uni_ringbuf.c
 * Author      : junlon2006@163.com
 * Date        : 2019.03.17
 *
 **************************************************************************/
#include "uni_ringbuf.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RINGBBUF_FILL_BYTE_SIZE  (1)
#define RINGBBUF_ATTR_READ       (1 << 0)
#define RINGBBUF_ATTR_SYNC       (1 << 1)
#define RINGBBUF_ATTR_WRITE      (1 << 2)

typedef int RingBufferAttr;

typedef struct {
  int  size;
  char *buf;
  int  head, tail;
} RingBuf;

static RingBuf *_create_ring_buf() {
  RingBuf *ring_buf = (RingBuf *)malloc(sizeof(RingBuf));
  ring_buf->head = ring_buf->tail = 0;
  return ring_buf;
}

static int _get_data_size(RingBuf *ring_buf) {
  return (ring_buf->head + ring_buf->size - ring_buf->tail) % ring_buf->size;
}

static int _get_free_size(RingBuf *ring_buf) {
  return ring_buf->size - _get_data_size(ring_buf) - RINGBBUF_FILL_BYTE_SIZE;
}

static void _init_ring_buf(void *ring_buf, int size) {
  RingBuf *buf = (RingBuf *)ring_buf;
  buf->size = size + RINGBBUF_FILL_BYTE_SIZE;
  buf->buf = (char *)malloc(size + RINGBBUF_FILL_BYTE_SIZE);
  buf->head = buf->tail = 0;
}

static inline int _read_attr_setted(RingBufferAttr attr) {
  return ((attr & RINGBBUF_ATTR_READ) == 0 ? 0 : 1);
}

static inline int _write_attr_setted(RingBufferAttr attr) {
  return ((attr & RINGBBUF_ATTR_WRITE) == 0 ? 0 : 1);
}

static inline int _sync_attr_setted(RingBufferAttr attr) {
  return ((attr & RINGBBUF_ATTR_SYNC) == 0 ? 0 : 1);
}

static inline int _read_length_overflow(RingBuf *ring_buf, int readlen) {
  return _get_data_size(ring_buf) < readlen;
}

static inline int _write_length_overflow(RingBuf *ring_buf, int writelen) {
  return _get_free_size(ring_buf) < writelen;
}

static int _read_internal(RingBufferHandle handle, char *buf, int size,
                          RingBufferAttr attr) {
  RingBuf *ring_buf = (RingBuf *)handle;
  int pos, remain;
  pos = (ring_buf->tail + size) % ring_buf->size;
  if (_read_attr_setted(attr)) {
    if (pos >= ring_buf->tail) {
      memcpy(buf, ring_buf->buf + ring_buf->tail, size);
    } else {
      remain = (ring_buf->size - ring_buf->tail);
      memcpy(buf, ring_buf->buf + ring_buf->tail, remain);
      memcpy(buf + remain, ring_buf->buf, size - remain);
    }
  }
  if (_sync_attr_setted(attr)) {
    ring_buf->tail = pos;
  }
  return size;
}

static int _write_internal(RingBufferHandle handle, char *buf, int size,
                           RingBufferAttr attr) {
  RingBuf *ring_buf = (RingBuf *)handle;
  int pos, remain;
  pos = (ring_buf->head + size) % ring_buf->size;
  if (_write_attr_setted(attr)) {
    if (pos >= ring_buf->head) {
      memcpy(ring_buf->buf + ring_buf->head, buf, size);
    } else {
      remain = (ring_buf->size - ring_buf->head);
      memcpy(ring_buf->buf + ring_buf->head, buf, remain);
      memcpy(ring_buf->buf, buf + remain, size - remain);
    }
  }
  if (_sync_attr_setted(attr)) {
    ring_buf->head = pos;
  }
  return size;
}

RingBufferHandle RingBufferCreate(int size) {
  RingBuf *ring_buf;
  if (size <= 0) {
    return (RingBufferHandle)NULL;
  }
  ring_buf = _create_ring_buf();
  _init_ring_buf(ring_buf, size);
  return (RingBufferHandle)ring_buf;
}

int RingBufferDestroy(RingBufferHandle handle) {
  RingBuf *ring_buf = (RingBuf *)handle;
  free(ring_buf->buf);
  free(ring_buf);
  return 0;
}

static int _ring_buffer_read(char *dst, int readlen, RingBufferHandle handle,
                             RingBufferAttr attr) {
  if (_read_length_overflow((RingBuf *)handle, readlen)) {
    return -1;
  }
  return _read_internal(handle, dst, readlen, attr);
}

static int _ring_buffer_write(RingBufferHandle handle, char *src, int writelen,
                              RingBufferAttr attr) {
  if (_write_length_overflow((RingBuf *)handle, writelen)) {
    return -1;
  }
  return _write_internal(handle, src, writelen, attr);
}

int RingBufferClear(RingBufferHandle handle) {
  RingBuf *ring_buf = (RingBuf *)handle;
  ring_buf->head = ring_buf->tail = 0;
  return 0;
}

int RingBufferGetFreeSize(RingBufferHandle handle) {
  RingBuf *ring_buf = (RingBuf *)handle;
  return _get_free_size(ring_buf);
}

int RingBufferGetDataSize(RingBufferHandle handle) {
  RingBuf *ring_buf = (RingBuf *)handle;
  return _get_data_size(ring_buf);
}

int RingBufferPeek(char *dst, int readlen, RingBufferHandle handle) {
  return _ring_buffer_read(dst, readlen, handle, RINGBBUF_ATTR_READ);
}

int RingBufferWrite(RingBufferHandle handle, char *src, int writelen) {
  return _ring_buffer_write(handle, src, writelen,
                            RINGBBUF_ATTR_WRITE | RINGBBUF_ATTR_SYNC);
}

int RingBufferRead(char *dst, int readlen, RingBufferHandle handle) {
  return _ring_buffer_read(dst, readlen, handle,
                           RINGBBUF_ATTR_READ | RINGBBUF_ATTR_SYNC);
}