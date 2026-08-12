/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2026 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Super compact ARM RTT debug output
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct {
  const char* name;
  char* buf;
  unsigned SizeOfBuffer;
  unsigned wrOffs;
  volatile  unsigned rdOffs;
  unsigned flags;
} SEGGER_RTT_BUFFER;

#define RTT_UP_SIZE 128
#define RTT_DN_SIZE 16

typedef struct {
  char                    acID[16];                                 // Initialized to "SEGGER RTT"
  int                     upBuffers;                          // Initialized to SEGGER_RTT_MAX_NUM_UP_BUFFERS (type. 2)
  int                     downBuffers;                        // Initialized to SEGGER_RTT_MAX_NUM_DOWN_BUFFERS (type. 2)
  SEGGER_RTT_BUFFER       up;       // Up buffers, transferring information up from target via debug probe to host
#if RTT_DN_SIZE
  SEGGER_RTT_BUFFER       dn;     // Down buffers, transferring information down from host via debug probe to target
#endif
} SEGGER_RTT_CB;

char RTT_UP[RTT_UP_SIZE];
char RTT_DN[RTT_DN_SIZE];

SEGGER_RTT_CB RTT = {
  .acID = "SEGGER RTT\0\0\0\0\0",
  .upBuffers = 1,
  .downBuffers = RTT_DN_SIZE ? 1 : 0,
  .up = {
    .name = "Terminal",
    .buf = RTT_UP,
    .SizeOfBuffer = RTT_UP_SIZE,
    .wrOffs = 0,
    .rdOffs = 0,
    .flags = 0 // SEGGER_RTT_MODE_NO_BLOCK_SKIP
  },
#if RTT_DN_SIZE
  .dn = {
    .name = "Terminal",
    .buf = RTT_DN,
    .SizeOfBuffer = RTT_DN_SIZE,
    .wrOffs = 0,
    .rdOffs = 0,
    .flags = 0 // SEGGER_RTT_MODE_NO_BLOCK_SKIP
  }
#endif
};

void rtt_write_ch(char ch) {
  RTT.up.buf[RTT.up.wrOffs] = ch;
  RTT.up.wrOffs = (RTT.up.wrOffs+1) & (RTT_UP_SIZE-1);
}

void rtt_write(const char *buf) {
  while (*buf) rtt_write_ch(*(buf++));
}

void rtt_printf(const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  char buf[16];
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      char fmtChar = *fmt++;
      if (fmtChar=='d') {
        itoa(va_arg(argp, int), buf, 10);
        rtt_write(buf);
      } else {
        rtt_write_ch('%');
        rtt_write_ch(fmtChar);
      }
    } else {
      rtt_write_ch(*(fmt++));
    }
  }
  va_end(argp);
}