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
 * Implementation of JsNetwork for ESP8266 devices
 * ----------------------------------------------------------------------------
 */
#include "jsinteractive.h"
#include "network.h"
#include "network_esp8266.h"
#include "jswrap_stream.h"

#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)

void esp8266_send(JsVar *msg) {
  JsvStringIterator it;
  jsvStringIteratorNew(&it, msg, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    jshTransmit(networkGetCurrent()->data.device, (unsigned char)ch);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
}

bool esp8266_wait_for(const char *text, int milliseconds) {
  JsSysTime endTime = jshGetSystemTime() + jshGetTimeFromMilliseconds(milliseconds);
  JsVar *usartClass = jsvSkipNameAndUnLock(jsiGetClassNameFromDevice(networkGetCurrent()->data.device));
  if (!jsvIsObject(usartClass)) {
    assert(0);
    return false;
  }

  bool found = false;

  while (!found && jshGetSystemTime() < endTime) {
    IOEvent event;
    // drag any data out of the event queue
    while (jshPopIOEventOfType(networkGetCurrent()->data.device, &event)) {
      jsiHandleIOEventForUSART(usartClass, &event);
    }
    // Chomp newlines
    JsVar *buf = jsvObjectGetChild(usartClass, STREAM_BUFFER_NAME, 0);
    if (jsvIsString(buf)) {
      int idx = jsvGetStringIndexOf(buf, '\r');
      bool hasChanged = false;
      while (!found && idx>=0) {
        hasChanged = true;
        JsVar *line = jsvNewFromStringVar(buf,0,idx);
        jsiConsolePrintf("ESP8266> %q", line);
        if (jsvIsStringEqual(line, text))
          found = true;
        jsvUnLock(line);
        JsVar *newBuf = jsvNewFromStringVar(buf, idx+1, JSVAPPENDSTRINGVAR_MAXLENGTH);
        jsvUnLock(buf);
        buf = newBuf;
        idx = jsvGetStringIndexOf(buf, '\r');
      }
      if (hasChanged)
        jsvObjectSetChild(usartClass, STREAM_BUFFER_NAME, buf);
    }
    jsvUnLock(buf);
  }
  // timeout
  return found;
}

typedef enum {
  ESP8266_IDLE,
  ESP8266_RESET_WAIT,
} ESP8266State;



/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_esp8266_gethostbyname(JsNetwork *net, char * hostName, unsigned long* out_ip_addr) {
}

/// Called on idle. Do any checks required for this device
void net_esp8266_idle(JsNetwork *net) {
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_esp8266_checkError(JsNetwork *net) {
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_esp8266_createsocket(JsNetwork *net, unsigned long host, unsigned short port) {
}

/// destroys the given socket
void net_esp8266_closesocket(JsNetwork *net, int sckt) {
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_esp8266_accept(JsNetwork *net, int sckt) {
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp8266_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp8266_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
}

void netSetCallbacks_esp8266(JsNetwork *net) {
  net->idle = net_esp8266_idle;
  net->checkError = net_esp8266_checkError;
  net->createsocket = net_esp8266_createsocket;
  net->closesocket = net_esp8266_closesocket;
  net->accept = net_esp8266_accept;
  net->gethostbyname = net_esp8266_gethostbyname;
  net->recv = net_esp8266_recv;
  net->send = net_esp8266_send;
}

