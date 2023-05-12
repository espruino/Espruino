/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * JavaScript methods for Espruino utility functions
 * ----------------------------------------------------------------------------
 */

#ifndef JSWRAP_ESPRUINO_H_
#define JSWRAP_ESPRUINO_H_

#include "jsvar.h"
#include "jshardware.h"
#include "jsflags.h" // for E.get/setFlags

JsVarFloat jswrap_espruino_getTemperature();
JsVar *jswrap_espruino_nativeCall(JsVarInt addr, JsVar *signature, JsVar *data);

JsVarFloat jswrap_espruino_clip(JsVarFloat x, JsVarFloat min, JsVarFloat max);
JsVarFloat jswrap_espruino_sum(JsVar *arr);
JsVarFloat jswrap_espruino_variance(JsVar *arr, JsVarFloat mean);
JsVarFloat jswrap_espruino_convolve(JsVar *a, JsVar *b, int offset);
void jswrap_espruino_FFT(JsVar *arrReal, JsVar *arrImag, bool inverse);

void jswrap_espruino_enableWatchdog(JsVarFloat time, JsVar *isAuto);
void jswrap_espruino_kickWatchdog();
/// Return an array of errors based on the current flags
JsVar *jswrap_espruino_getErrorFlagArray(JsErrorFlags flags);
JsVar *jswrap_espruino_getErrorFlags();
JsVar *jswrap_espruino_toArrayBuffer(JsVar *str);
JsVar *jswrap_espruino_toUint8Array(JsVar *args);
JsVar *jswrap_espruino_toString(JsVar *args);
JsVar *jswrap_espruino_toJS(JsVar *v);
JsVar *jswrap_espruino_memoryArea(int addr, int len);
void jswrap_espruino_setBootCode(JsVar *code, bool alwaysExec);
int jswrap_espruino_setClock(JsVar *options);
void jswrap_espruino_setConsole(JsVar *device, JsVar *options);
JsVar *jswrap_espruino_getConsole();

int jswrap_espruino_reverseByte(int v);
void jswrap_espruino_dumpTimers();
void jswrap_espruino_dumpLockedVars();
void jswrap_espruino_dumpFreeList();
void jswrap_e_dumpFragmentation();
void jswrap_e_dumpVariables();
JsVar *jswrap_espruino_getSizeOf(JsVar *v, int depth);
JsVarInt jswrap_espruino_getAddressOf(JsVar *v, bool flatAddress);
void jswrap_espruino_mapInPlace(JsVar *from, JsVar *to, JsVar *map, JsVarInt bits);
JsVar *jswrap_espruino_lookupNoCase(JsVar *haystack, JsVar *needle, bool returnKey);
JsVar *jswrap_e_dumpStr();
JsVar *jswrap_espruino_CRC32(JsVar *data);
JsVar *jswrap_espruino_HSBtoRGB(JsVarFloat hue, JsVarFloat sat, JsVarFloat bri, int format);
void jswrap_espruino_setPassword(JsVar *pwd);
void jswrap_espruino_lockConsole();
void jswrap_espruino_setTimeZone(JsVarFloat zone);
#ifndef ESPR_NO_DAYLIGHT_SAVING
void jswrap_espruino_setDST(JsVar *params);
#endif
JsVar *jswrap_espruino_memoryMap(JsVar *baseAddress, JsVar *registers);
void jswrap_espruino_asm(JsVar *callspec, JsVar *args);
void jswrap_espruino_compiledC(JsVar *code);
void jswrap_espruino_reboot();

void jswrap_espruino_setUSBHID(JsVar *arr);
bool jswrap_espruino_sendUSBHID(JsVar *arr);

JsVarInt jswrap_espruino_getBattery();
void jswrap_espruino_setRTCPrescaler(int prescale);
int jswrap_espruino_getRTCPrescaler(bool calibrate);
JsVar *jswrap_espruino_decodeUTF8(JsVar *str, JsVar *lookup, JsVar *replaceFn);
void jswrap_espruino_stopEventPropagation();

#endif // JSWRAP_ESPRUINO_H_
