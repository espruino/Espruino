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

#define ESP8266_IPD_NAME JS_HIDDEN_CHAR_STR"IPD"

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

const char *esp8266_idle_compare = 0;
size_t esp8266_ipd_buffer_size = 0;
bool esp8266_idle_compare_only_start = false;

void esp8266_got_data(JsVar *data) {
  JsVar *ipd = jsvObjectGetChild(execInfo.hiddenRoot, ESP8266_IPD_NAME, 0);
  if (!ipd) {
    jsvObjectSetChild(execInfo.hiddenRoot, ESP8266_IPD_NAME, data);
  } else {
    jsvAppendStringVar(ipd, data, 0, JSVAPPENDSTRINGVAR_MAXLENGTH);
    jsvUnLock(ipd);
  }
}

bool esp8266_idle(JsVar *usartClass) {
  bool found = false;
  bool hasChanged = false;
  JsVar *buf = jsvObjectGetChild(usartClass, STREAM_BUFFER_NAME, 0);
  if (jsvIsString(buf)) {
    if (esp8266_ipd_buffer_size) {
      size_t nChars = jsvGetStringLength(buf);
      if (nChars > esp8266_ipd_buffer_size)
        nChars = esp8266_ipd_buffer_size;
      esp8266_ipd_buffer_size -= nChars;
      JsVar *data = jsvNewFromStringVar(buf,0,(size_t)nChars);
      JsVar *newBuf = jsvNewFromStringVar(buf, (size_t)(nChars+1), JSVAPPENDSTRINGVAR_MAXLENGTH);
      jsvUnLock(buf);
      buf = newBuf;
      hasChanged = true;
      esp8266_got_data(data);
    } else {
      char chars[10];
      jsvGetStringChars(buf, 0, chars, sizeof(chars));
      if (chars[0]=='+' && chars[1]=='I' && chars[2]=='P' && chars[3]=='D' && chars[4]==',') {
        size_t i = 5;
        while (i<sizeof(chars)-1 && chars[i]!=':' && chars[i]!=0) i++;
        if (chars[i]==':') {
          chars[i]=0;
          esp8266_ipd_buffer_size = (size_t)stringToInt(&chars[5]);
          size_t len = jsvGetStringLength(buf);
          if (len > i) {
            size_t nChars = len-i;
            if (nChars > esp8266_ipd_buffer_size)
              nChars = esp8266_ipd_buffer_size;
            esp8266_ipd_buffer_size -= nChars;
            JsVar *data = jsvNewFromStringVar(buf,(size_t)(i+1),(size_t)nChars);
            JsVar *newBuf = jsvNewFromStringVar(buf, (size_t)(i+nChars+1), JSVAPPENDSTRINGVAR_MAXLENGTH);
            jsvUnLock(buf);
            buf = newBuf;
            hasChanged = true;
            esp8266_got_data(data);
            jsvUnLock(data);
          } else {
            // Do nothing - our string isn't big enough
          }
        } else if (chars[i]!=0) {
          // invalid data. Kill it.
          jsWarn("ESP8266 expecting +IPD string, got %q", buf);
          jsvUnLock(buf);
          buf = 0;
          hasChanged = true;
        }
      } else { // string doesn't start with '+IPD'
        int idx = jsvGetStringIndexOf(buf, '\n');
        while (!found && idx>0/* because we want a \r before it */) {
          hasChanged = true;
          JsVar *line = jsvNewFromStringVar(buf,0,(size_t)(idx-1)); // \r\n - so idx is of '\n' and we want to remove '\r' too
          jsiConsoleRemoveInputLine();
          jsiConsolePrintf("ESP8266> %q\n", line);
          if (esp8266_idle_compare && jsvIsStringEqualOrStartsWith(line, esp8266_idle_compare, esp8266_idle_compare_only_start))
            found = true;
          jsvUnLock(line);
          JsVar *newBuf = jsvNewFromStringVar(buf, (size_t)(idx+1), JSVAPPENDSTRINGVAR_MAXLENGTH);
          jsvUnLock(buf);
          buf = newBuf;
          idx = jsvGetStringIndexOf(buf, '\n');
        }
      }
    }
  }
  if (hasChanged)
    jsvObjectSetChild(usartClass, STREAM_BUFFER_NAME, buf);
  jsvUnLock(buf);
  return found;
}

bool esp8266_wait_for(const char *text, int milliseconds, bool justTheStart) {
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
    // Search for newlines
    esp8266_idle_compare = text;
    esp8266_idle_compare_only_start = justTheStart;
    found = esp8266_idle(usartClass);
    esp8266_idle_compare = 0;
  }

  jsvUnLock(usartClass);
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
  JsVar *usartClass = jsvSkipNameAndUnLock(jsiGetClassNameFromDevice(networkGetCurrent()->data.device));
  if (jsvIsObject(usartClass)) {
    esp8266_idle(usartClass);
  }
  jsvUnLock(usartClass);
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_esp8266_checkError(JsNetwork *net) {
  return true;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_esp8266_createsocket(JsNetwork *net, unsigned long host, unsigned short port) {
  JsVar *hostStr = networkGetAddressAsString((unsigned char *)&host, 4,10,'.');
  JsVar *msg = jsvVarPrintf("AT+CIPSTART=\"TCP\",%q,%d\r", hostStr, port);
  jsvUnLock(hostStr);
  esp8266_send(msg);
  jsvUnLock(msg);
  if (!esp8266_wait_for("OK",100,false))
    return -1;
  if (!esp8266_wait_for("Linked",4000,false))
    return -1;
  return 0;
}

/// destroys the given socket
void net_esp8266_closesocket(JsNetwork *net, int sckt) {
  JsVar *msg = jsvVarPrintf("AT+CIPCLOSE\r");
  esp8266_send(msg);
  jsvUnLock(msg);
  esp8266_wait_for("OK",100,false);
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_esp8266_accept(JsNetwork *net, int sckt) {
  return -1;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp8266_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  JsVar *ipd = jsvObjectGetChild(execInfo.hiddenRoot, ESP8266_IPD_NAME, 0);
  size_t chars = 0;
  if (ipd) {
    chars = jsvGetStringLength(ipd);
    jsvGetStringChars(ipd, 0, buf, len);

    JsVar *newIpd = (chars>len) ? jsvNewFromStringVar(ipd, len, JSVAPPENDSTRINGVAR_MAXLENGTH) : 0;
    jsvUnLock(ipd);
    jsvUnLock(jsvObjectSetChild(execInfo.hiddenRoot, ESP8266_IPD_NAME, newIpd));
  }
  return (chars>len) ? len : chars;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp8266_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  JsVar *msg = jsvVarPrintf("AT+CIPSEND=%d\r",len);
  esp8266_send(msg);
  jsvUnLock(msg);

  // doesn't seem to return this until it's done, but we need to wait anyway
  esp8266_wait_for("AT+CIPSEND",100,true);
  // -------------------------------------------------

  size_t i;
  for (i=0;i<len;i++)
    jshTransmit(networkGetCurrent()->data.device, ((unsigned char *)buf)[i]);
  jshTransmit(networkGetCurrent()->data.device, '\r');
  esp8266_wait_for(">",1000,false); // ideally we wouldn't wait for the newline...
  return len;
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

