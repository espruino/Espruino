/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2024 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Contains JavaScript interface for Pipboy
 * ----------------------------------------------------------------------------
 */

#include "jsvar.h"

void jswrap_pb_videoStart(JsVar *fn, JsVar *options);
void jswrap_pb_videoStop();
void jswrap_pb_audioStart(JsVar *fn, JsVar *options);
JsVar *jswrap_pb_audioRead(JsVar *fn);
JsVar *jswrap_pb_audioBuiltin(JsVar *id);
int jswrap_pb_audioGetFree();
void jswrap_pb_audioStartVar(JsVar *wav, JsVar *options);

void jswrap_pb_setVol(int volume);
void jswrap_pb_initDAC();
void jswrap_pb_setDACPower(bool isOn);
void jswrap_pb_setDACMode(JsVar *mode);
void jswrap_pb_writeDACReg(int reg, int value);
int jswrap_pb_readDACReg(int reg);
void jswrap_pb_setLCDPower(bool isOn);
void jswrap_pb_off();
void jswrap_pb_sleep();
void jswrap_pb_wake();
void jswrap_pb_blitImage(JsVar *image, int x, int y, JsVar *options);
void jswrap_pb_getAudioWaveform(JsVar *dst, int y1, int y2);
bool jswrap_pb_audioIsPlaying();
JsVar *jswrap_pb_streamPlaying();
void jswrap_pb_setPalette(JsVar *pal);

void jswrap_pb_init();
void jswrap_pb_kill();
bool jswrap_pb_idle();
