/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2016 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Software Serial
 * ----------------------------------------------------------------------------
 */
#include "jsserial.h"
#include "jsinteractive.h"
#include "jstimer.h"
#include "jswrap_espruino.h"
#include "jswrap_stream.h"

void jsserialHardwareFunc(unsigned char data, serial_sender_data *info) {
  IOEventFlags device = *(IOEventFlags*)info;
  jshTransmit(device, data);
}

/**
 * Send a single byte through Serial.
 */
void jsserialSoftwareFunc(
    unsigned char data,
    serial_sender_data *info
  ) {
  // jsiConsolePrintf("jsserialSoftwareFunc: data=%x\n", data);
  JshUSARTInfo *inf = (JshUSARTInfo*)info;
  if (!jshIsPinValid(inf->pinTX)) return;

  // Work out what to send
  // stop bits
  int bitData = (1<<inf->stopbits)-1;
  int bitCnt = inf->stopbits;
  // TODO: parity
  // data
  bitData = (bitData << inf->bytesize) | (data & ((1<<inf->bytesize)-1));
  bitCnt += inf->bytesize;
  // start bit
  bitData = (bitData << 1);
  bitCnt += 1;

  // Get ready to send
  JsSysTime bitTime = jshGetTimeFromMilliseconds(1000.0 / inf->baudRate);
  JsSysTime time;
  UtilTimerTask task;
  if (jstGetLastPinTimerTask(inf->pinTX, &task)) {
    time = task.time + bitTime; // leave one bit of time for a stop bit
  } else {
    // no timer - just start in a little while!
    time = jshGetSystemTime()+jshGetTimeFromMilliseconds(1);
  }
  //bool outState = 1;
  int outCount = 0;
  // Now send...
  while (bitCnt) {
    // get bit
    bool bit = bitData&1;
    bitData>>=1;
    bitCnt--;
    // figure out what to do
    /*if (bit == outState) {
      outCount++;
    } else {
      // state changed!
      time += bitTime*outCount;
      jstPinOutputAtTime(time, &inf->pinTX, 1, bit);
      outState = bit;
      outCount = 1;
    }*/
    // hacky - but seems like we may have some timing problems otherwise
    jstPinOutputAtTime(time, &inf->pinTX, 1, bit);
    //jsiConsolePrintf("-> %d\n",bit);
    time += bitTime;
  }
  // And finish off by raising...
  time += bitTime*outCount;
  //jsiConsolePrintf("-> 1 (final)\n");
  jstPinOutputAtTime(time, &inf->pinTX, 1, 1);
  // we do this even if we are high, because we want to ensure that the next char is properly spaced
  // Ideally we'd be able to store the last bit time when sending so we could just go straight on from it
}

bool jsserialPopulateUSARTInfo(
    JshUSARTInfo *inf,
    JsVar      *baud,
    JsVar      *options
  ) {
  jshUSARTInitInfo(inf);

  JsVar *parity = 0;
  JsVar *flow = 0;
  jsvConfigObject configs[] = {
      {"rx", JSV_PIN, &inf->pinRX},
      {"tx", JSV_PIN, &inf->pinTX},
      {"ck", JSV_PIN, &inf->pinCK},
      {"cts", JSV_PIN, &inf->pinCTS},
      {"bytesize", JSV_INTEGER, &inf->bytesize},
      {"stopbits", JSV_INTEGER, &inf->stopbits},
      {"parity", JSV_OBJECT /* a variable */, &parity},
      {"flow", JSV_OBJECT /* a variable */, &flow},
      {"errors", JSV_BOOLEAN, &inf->errorHandling},
  };

  if (!jsvIsUndefined(baud)) {
    int b = (int)jsvGetInteger(baud);
    if (b<=100 || b > 10000000)
      jsExceptionHere(JSET_ERROR, "Invalid baud rate specified");
    else
      inf->baudRate = b;
  }

  bool ok = true;
  if (jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))) {
    // sort out parity
    inf->parity = 0;
    if(jsvIsString(parity)) {
      if (jsvIsStringEqual(parity, "o") || jsvIsStringEqual(parity, "odd"))
        inf->parity = 1;
      else if (jsvIsStringEqual(parity, "e") || jsvIsStringEqual(parity, "even"))
        inf->parity = 2;
    } else if (jsvIsInt(parity)) {
      inf->parity = (unsigned char)jsvGetInteger(parity);
    }
    if (inf->parity>2) {
      jsExceptionHere(JSET_ERROR, "Invalid parity %d", inf->parity);
      ok = false;
    }

    if (ok) {
      if (jsvIsUndefined(flow) || jsvIsNull(flow) || jsvIsStringEqual(flow, "none"))
        inf->xOnXOff = false;
      else if (jsvIsStringEqual(flow, "xon"))
        inf->xOnXOff = true;
      else {
        jsExceptionHere(JSET_ERROR, "Invalid flow control: %q", flow);
        ok = false;
      }
    }
  }
  jsvUnLock(parity);
  jsvUnLock(flow);

  return ok;
}

// Get the correct Serial send function (and the data to send to it).
bool jsserialGetSendFunction(JsVar *serialDevice, serial_sender *serialSend, serial_sender_data *serialSendData) {
  IOEventFlags device = jsiGetDeviceFromClass(serialDevice);
  JshUSARTInfo inf;

  // See if the device is hardware or software.
  if (DEVICE_IS_USART(device)) {
    // Hardware
    if (!jshIsDeviceInitialised(device)) {
      jshUSARTInitInfo(&inf);
      jshUSARTSetup(device, &inf);
    }
    *serialSend = jsserialHardwareFunc;
    *(IOEventFlags*)serialSendData = device;
    return true;
  } else if (device == EV_NONE) {
    // Software Serial
    JsVar *baud = jsvObjectGetChild(serialDevice, USART_BAUDRATE_NAME, 0);
    JsVar *options = jsvObjectGetChild(serialDevice, DEVICE_OPTIONS_NAME, 0);
    jsserialPopulateUSARTInfo(&inf, baud, options);
    jsvUnLock(options);
    jsvUnLock(baud);

    *serialSend = jsserialSoftwareFunc;
    *serialSendData = inf;
    return true;
  }
  return false;
}


typedef struct {
  char buf[7]; ///< received data
  unsigned char bufLen; ///< amount of received characters
  JsSysTime lastTime;
  int baudRate;
  /** We shift left for each bit, and always reset this with a '1' in it,
   * so that we can use the variable to mark how many bit we've received
   * as well as the data itself.
   */
  int bits;
  int bitMax;
} SerialEventCallbackData;


JsVar *jsserialGetSerialList(bool create) {
  return jsvObjectGetChild(execInfo.hiddenRoot, "swserial", create?JSV_ARRAY:0);
}

bool jsserialEventCallbackInit(JsVar *parent, JshUSARTInfo *inf) {
  JsVar *dataVar = jsvNewFlatStringOfLength(sizeof(SerialEventCallbackData));
  if (!dataVar) {
    jsExceptionHere(JSET_ERROR, "Unable to allocate data for Serial RX");
    return false;
  }
  jsvObjectSetChildAndUnLock(parent, "irqData", dataVar);
  SerialEventCallbackData *data = (SerialEventCallbackData *)jsvGetFlatStringPointer(dataVar);
  data->bufLen = 0;
  data->lastTime = jshGetSystemTime();
  data->baudRate = inf->baudRate;
  data->bits = 1;
  data->bitMax = 1 << (1+inf->bytesize+inf->stopbits);
  if (inf->parity) data->bitMax<<=1;

  IOEventFlags exti = jshPinWatch(inf->pinRX, true);
  if (exti) {
    jsvObjectSetChildAndUnLock(parent, "exti", jsvNewFromInteger(exti));
    JsVar *list = jsserialGetSerialList(true);
    if (!list) return false;
    jsvSetArrayItem(list, exti, parent);
    jsvUnLock(list);
    jshSetEventCallback(exti, jsserialEventCallback);
  } else {
    jsExceptionHere(JSET_ERROR, "Unable to watch pin %p, no Software Serial RX\n", inf->pinRX);
    return false;
  }

  return true;
}

void jsserialEventCallbackKill(JsVar *parent, JshUSARTInfo *inf) {
  JsVar *v = jsvObjectGetChild(parent, "exti", 0);
  if (v) {
    IOEventFlags exti = (IOEventFlags)jsvGetIntegerAndUnLock(v);
    jshPinWatch(exti, false);
    JsVar *list = jsserialGetSerialList(false);
    if (list) {
      JsVar *parentName = jsvGetArrayIndex(list, (JsVarInt)exti);
      if (parentName) jsvRemoveChild(list, parentName);
      if (!jsvGetChildren(list))
        jsvObjectRemoveChild(execInfo.hiddenRoot, "swserial");
      jsvUnLock2(parentName, list);
    }
  }
}

bool jsserialEventCallbackIdle() {
  bool busy = false;
  JsVar *list = jsserialGetSerialList(false);
  JsvObjectIterator it;
  jsvObjectIteratorNew(&it, list);
  while (jsvObjectIteratorHasValue(&it)) {
    JsVar *parent = jsvObjectIteratorGetValue(&it);
    JsVar *dataVar = jsvObjectGetChild(parent, "irqData", 0);
    SerialEventCallbackData *data = (SerialEventCallbackData *)jsvGetFlatStringPointer(dataVar);
    if (data) {
      if (data->bits!=1) {
        JsSysTime time = jshGetSystemTime();
        JsSysTime timeDiff = time - data->lastTime;
        int bitCnt = (int)((jshGetMillisecondsFromTime(timeDiff) * data->baudRate / 1000)+0.5);
        if (bitCnt>2) {
          if ((data->bits<<bitCnt) >= data->bitMax) {
            while (data->bits < data->bitMax)
              data->bits=(data->bits<<1)|1;
            int ch = jswrap_espruino_reverseByte((data->bits>>1)&255);
            /*char buf[20];
            itostr(data->bits, buf, 2);
            jsiConsolePrintf("=]%d %s %d\n", data->bits, buf, ch);*/
            if (data->bufLen < sizeof(data->buf))
              data->buf[data->bufLen++] = (char)ch;
            data->bits = 1;
          }
        } else
          busy = true; // waiting for this byte to finish
      }
      if (data->bufLen) {
        JsVar *stringData = jsvNewStringOfLength(data->bufLen, data->buf);
        data->bufLen = 0;
        if (stringData) {
          jswrap_stream_pushData(parent, stringData, true);
          jsvUnLock(stringData);
        }
      }
    }
    jsvUnLock2(dataVar, parent);
    jsvObjectIteratorNext(&it);
  }
  jsvObjectIteratorFree(&it);
  jsvUnLock(list);
  return busy; // keep us from sleeping if the byte hadn't finished
}

/* This is used with jshSetEventCallback to allow Serial data to be received in software.
 * It's called from an IRQ
 */
void jsserialEventCallback(bool state, IOEventFlags channel) {
  JsVar *list = jsserialGetSerialList(false);
  if (!list) return;
  JsVar *parent = jsvGetArrayItem(list, (JsVarInt)channel);
  if (!parent) return;
  JsVar *dataVar = jsvObjectGetChild(parent, "irqData", 0);
  SerialEventCallbackData *data = (SerialEventCallbackData *)jsvGetFlatStringPointer(dataVar);
  if (!data) return;
  // work out time difference
  JsSysTime time = jshGetSystemTime();
  JsSysTime timeDiff = time - data->lastTime;
  data->lastTime = time;
  int bitCnt = (int)((jshGetMillisecondsFromTime(timeDiff) * data->baudRate / 1000)+0.5);
  if (bitCnt<0 || bitCnt>=10) {
    // We're starting again - reset
    bitCnt = 0;
    if (data->bits != 1)
      while (data->bits < data->bitMax)
        data->bits=(data->bits<<1)|1;
  }
  data->bits <<= bitCnt;
  if (!state) data->bits |= (1<<bitCnt)-1; // add 1s if we need to
  if (data->bits >= data->bitMax) {
    int ch = jswrap_espruino_reverseByte((data->bits>>1)&255);
    if (data->bufLen < sizeof(data->buf))
      data->buf[data->bufLen++] = (char)ch;
    data->bits = 1;
  }
  jshHasEvents();
}
