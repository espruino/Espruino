/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * TV output capability on STM32 devices
 * ----------------------------------------------------------------------------
 */

typedef struct {
  Pin pinVideo;
  Pin pinSync;
  int width;
  int height;
} tv_info_pal;

typedef struct {
  Pin pinVideo;
  Pin pinSync;
  Pin pinSyncV;
  int width;
  int height;
  int lineRepeat;
} tv_info_vga;

void tv_info_pal_init(tv_info_pal *inf);
void tv_info_vga_init(tv_info_vga *inf);

JsVar *tv_setup_pal(tv_info_pal *inf);
JsVar *tv_setup_vga(tv_info_vga *inf);
