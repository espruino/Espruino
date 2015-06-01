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
 *
 * DEPRECATED - YOU SHOULD NOW USE THE ESP8266.js MODULE
 * ----------------------------------------------------------------------------
 */
#include "jsinteractive.h"
#include "network.h"
#include "network_esp8266.h"
#include "jswrap_stream.h"

#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)

#define ESP8266_IPD_NAME JS_HIDDEN_CHAR_STR"ID"
#define ESP8266_IPD_NAME_LEN 3 // strlen(ESP8266_IPD_NAME)
#define ESP8266_DNS_NAME JS_HIDDEN_CHAR_STR"DNS"

#define ESP8266_DEBUG

// ---------------------------------------------------------------------------
typedef enum {
  ESP8266_IDLE,
  ESP8266_DELAY, // just delay and execute after the timeout
  ESP8266_WAIT_OK,
  ESP8266_WAIT_OK_THEN_CWMODE_THEN_RESET_THEN_READY_THEN_CIPMUX,
  ESP8266_WAIT_CWMODE_THEN_RESET_THEN_READY_THEN_CIPMUX,
  ESP8266_WAIT_RESET_THEN_READY_THEN_CIPMUX,
  ESP8266_WAIT_READY_THEN_CIPMUX,
  ESP8266_WAIT_OK_THEN_LINKED,
  ESP8266_WAIT_OK_THEN_DELAY_10S,
  ESP8266_WAIT_LINKED,
  ESP8266_WAIT_OK_THEN_WAIT_CONNECTED,
  ESP8266_WAIT_SEND_OK,
  ESP8266_WAIT_SEND_OK_THEN_CLOSE,

  ESP8266_WAIT_CONNECTED_RESPONSE, // wait until we're connected (we can get an IP)
  ESP8266_DELAY_1S_WAIT_CONNECTED, // after ESP8266_WAIT_CONNECTED_RESPONSE fails, it waits 1s and tried again
  /*ESP8266_WAIT_OK_THEN_READY,
  ESP8266_WAIT_READY,*/
} ESP8266State;

ESP8266State esp8266State = ESP8266_IDLE; // current state - see above
JsSysTime stateTimeout; // the timout to allow
const char *stateTimeoutMessage; // the message to be displayed if we get a timeout
JsVar *stateSuccessCallback = 0; // if all goes well, this is what gets called
char currentSocket = -1; // if esp8266State!=IDLE, this could be a socket on which to report errors
unsigned char socketErrors = 0; // bit mask of errors on sockets
unsigned char socketUsage = 0; // bit mask of what sockets are used
#define SOCKET_COUNT 5 // 0..4

const char *esp8266_idle_compare = 0;
char esp8266_ipd_buffer_sckt = 0;
size_t esp8266_ipd_buffer_size = 0;
bool esp8266_idle_compare_only_start = false;
unsigned char connectionCheckCount;

void net_esp8266_setState(ESP8266State state, int timeout, JsVar *callback, const char *timeoutMessage) {
  esp8266State = state;
  if (timeout) {
    stateTimeout = jshGetSystemTime() + jshGetTimeFromMilliseconds(timeout);
    stateTimeoutMessage = timeoutMessage;
  } else {
    stateTimeout = -1;
    stateTimeoutMessage = 0;
  }
  callback = jsvLockAgainSafe(callback);
  if (stateSuccessCallback) jsvUnLock(stateSuccessCallback);
  stateSuccessCallback = callback;
}

void net_esp8266_execute_and_set_idle() {
  JsVar *cb = jsvLockAgainSafe(stateSuccessCallback);
  net_esp8266_setState(ESP8266_IDLE, 0, 0, 0);
  if (cb) {
    jsiQueueEvents(0, cb, 0, 0);
    jsvUnLock(cb);
  }
}

ESP8266State net_esp8266_getState() {
  return esp8266State;
}

// ---------------------------------------------------------------------------

void esp8266_send(JsVar *msg) {
#ifdef ESP8266_DEBUG
  jsiConsolePrintf(">>> %q\n", msg);
#endif
  JsvStringIterator it;
  jsvStringIteratorNew(&it, msg, 0);
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    jshTransmit(networkGetCurrent()->data.device, (unsigned char)ch);
    jsvStringIteratorNext(&it);
  }
  jsvStringIteratorFree(&it);
}

void esp8266_got_data(int sckt, JsVar *data) {
#ifdef ESP8266_DEBUG
  jsiConsolePrintf("%d<<< %q\n", sckt, data);
#endif
  if (sckt<0 || sckt>=SOCKET_COUNT) { // sanity check
    assert(0);
    return;
  }
  if (socketUsage & (1<<SOCKET_COUNT) && // is a server
      net_esp8266_getState() == ESP8266_WAIT_SEND_OK_THEN_CLOSE &&
      currentSocket==sckt) {
    /* Whoa. So we are a server, we served up a webpage, were waiting
     * to close the socket after we'd finished sending data, but it
     * looks like the client got there first *and then started another request*
     * which also went onto socket #0. So now we just try not to close the socket
     * and pray.
     */
    esp8266State = ESP8266_WAIT_SEND_OK;
  }

  char ipdName[ESP8266_IPD_NAME_LEN+2] = ESP8266_IPD_NAME"x";
  ipdName[ESP8266_IPD_NAME_LEN] = (char)('0' + sckt); // make ipdName different for each socket
  JsVar *ipd = jsvObjectGetChild(execInfo.hiddenRoot, ipdName, 0);
  if (!ipd) {
    jsvObjectSetChild(execInfo.hiddenRoot, ipdName, data);
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
      if (nChars>0) {
        if (nChars > esp8266_ipd_buffer_size)
          nChars = esp8266_ipd_buffer_size;
        esp8266_ipd_buffer_size -= nChars;
        JsVar *data = jsvNewFromStringVar(buf,0,(size_t)nChars);
        JsVar *newBuf = jsvNewFromStringVar(buf, (size_t)(nChars+1), JSVAPPENDSTRINGVAR_MAXLENGTH);
        //jsiConsolePrintf("%d,%d %q -> %q + %q\n", nChars,esp8266_ipd_buffer_size, buf, data, newBuf);
        jsvUnLock(buf);
        buf = newBuf;
        hasChanged = true;
        esp8266_got_data(esp8266_ipd_buffer_sckt, data);
      }
    } else {
      char chars[20];
      jsvGetStringChars(buf, 0, chars, sizeof(chars));
      if (chars[0]=='+' && chars[1]=='I' && chars[2]=='P' && chars[3]=='D' && chars[4]==',') {
        size_t sckt_idx = 5;
        while (sckt_idx<sizeof(chars)-1 && chars[sckt_idx]!=',' && chars[sckt_idx]!=0) sckt_idx++;
        size_t len_idx = sckt_idx;
        while (len_idx<sizeof(chars)-1 && chars[len_idx]!=':' && chars[len_idx]!=0) len_idx++;
        if (chars[sckt_idx]==',' && chars[len_idx]==':') {
          chars[sckt_idx]=0;
          esp8266_ipd_buffer_sckt = (char)stringToInt(&chars[5]);
          chars[len_idx]=0;
          esp8266_ipd_buffer_size = (size_t)stringToInt(&chars[sckt_idx+1]);
          size_t len = jsvGetStringLength(buf);
          if (len > len_idx) {
            size_t nChars = len-(len_idx+1);
            if (nChars > esp8266_ipd_buffer_size)
              nChars = esp8266_ipd_buffer_size;
            esp8266_ipd_buffer_size -= nChars;
            JsVar *data = jsvNewFromStringVar(buf,(size_t)(len_idx+1),(size_t)nChars);
            JsVar *newBuf = jsvNewFromStringVar(buf, (size_t)(len_idx+nChars+1), JSVAPPENDSTRINGVAR_MAXLENGTH);
            //jsiConsolePrintf("%d,%d %q -> %q + %q\n", nChars,esp8266_ipd_buffer_size, buf, data, newBuf);
            jsvUnLock(buf);
            buf = newBuf;
            hasChanged = true;
            esp8266_got_data(esp8266_ipd_buffer_sckt, data);
            jsvUnLock(data);
          } else {
            // Do nothing - our string isn't big enough
          }
        } else if (chars[len_idx]!=0) {
          // invalid data. Kill it.
          jsWarn("ESP8266 expecting +IPD string, got %q", buf);
          jsvUnLock(buf);
          buf = 0;
          hasChanged = true;
        }
      } else { // string doesn't start with '+IPD'
        int idx = jsvGetStringIndexOf(buf, '\r');
        if (idx<0 && esp8266_idle_compare && esp8266_idle_compare[0]=='>' && esp8266_idle_compare[1]==0 && buf && buf->varData.str[0]=='>')
          idx=2; // pretend we had /r/n - this only works because right now we're only expecting one char
        while ((!found) && idx>=0) {
          hasChanged = true;
          JsVar *line = idx ?
                        jsvNewFromStringVar(buf,0,(size_t)(idx - 1)) :  // \r\n - so idx is of '\n' and we want to remove '\r' too
                        jsvNewFromEmptyString();
#ifdef ESP8266_DEBUG
          jsiConsoleRemoveInputLine();
          jsiConsolePrintf("ESP8266> %q\n", line);
#endif
          switch (net_esp8266_getState()) {
            case ESP8266_IDLE: break; // ignore?
            case ESP8266_DELAY: break; // ignore here - see stateTimeout below
            case ESP8266_DELAY_1S_WAIT_CONNECTED: break; // ignore here - see stateTimeout below
            case ESP8266_WAIT_OK:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false)) {
                net_esp8266_execute_and_set_idle();
              }
              break;
            case ESP8266_WAIT_OK_THEN_CWMODE_THEN_RESET_THEN_READY_THEN_CIPMUX:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false)) {
                JsVar *cmd = jsvNewFromString("AT+CWMODE=1\r\n"); // set mode to 1 (client). 3 (both) works too
                esp8266_send(cmd);
                jsvUnLock(cmd);
                net_esp8266_setState(ESP8266_WAIT_CWMODE_THEN_RESET_THEN_READY_THEN_CIPMUX, 500, stateSuccessCallback, stateTimeoutMessage);
              }
              break;
            case ESP8266_WAIT_CWMODE_THEN_RESET_THEN_READY_THEN_CIPMUX:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false) || jsvIsStringEqualOrStartsWith(line, "no change", false)) {
                JsVar *cmd = jsvNewFromString("AT+RST\r\n"); // sent reset command
                esp8266_send(cmd);
                jsvUnLock(cmd);
                net_esp8266_setState(ESP8266_WAIT_RESET_THEN_READY_THEN_CIPMUX, 500, stateSuccessCallback, stateTimeoutMessage);
              }
              break;
            case ESP8266_WAIT_RESET_THEN_READY_THEN_CIPMUX:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false)) {
                net_esp8266_setState(ESP8266_WAIT_READY_THEN_CIPMUX, 6000, stateSuccessCallback, stateTimeoutMessage);
              }
              break;
            case ESP8266_WAIT_READY_THEN_CIPMUX:
              if (jsvIsStringEqualOrStartsWith(line, "ready", false)) {
                JsVar *cmd = jsvNewFromString("AT+CIPMUX=1\r\n"); // sent reset command
                esp8266_send(cmd);
                jsvUnLock(cmd);
                net_esp8266_setState(ESP8266_WAIT_OK, 500, stateSuccessCallback, stateTimeoutMessage);
              }
              break;

            case ESP8266_WAIT_OK_THEN_DELAY_10S:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false)) {
                // we got ok, now just delay for 5S
                net_esp8266_setState(ESP8266_DELAY, 10000, stateSuccessCallback, stateTimeoutMessage);
              }
              break;
            case ESP8266_WAIT_OK_THEN_LINKED:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false)) {
                net_esp8266_setState(ESP8266_WAIT_LINKED, 5000, stateSuccessCallback, stateTimeoutMessage);
                // FIXME what now?
              }
              break;
            case ESP8266_WAIT_LINKED:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false)) {
                net_esp8266_execute_and_set_idle();
              }
              break;
            case ESP8266_WAIT_OK_THEN_WAIT_CONNECTED:
              if (jsvIsStringEqualOrStartsWith(line, "OK", false)) {
                // wait 1s and then ask for an IP address
                connectionCheckCount = 60; // wait 60 seconds
                net_esp8266_setState(ESP8266_DELAY_1S_WAIT_CONNECTED, 3000, stateSuccessCallback, "WiFi network connection failed");
              }
              break;
            case ESP8266_WAIT_CONNECTED_RESPONSE:
              if (jsvIsStringEqualOrStartsWith(line, "ERROR", false)) {
                // ok, we got an error - keep waiting and polling
                net_esp8266_setState(ESP8266_DELAY_1S_WAIT_CONNECTED, 1000, stateSuccessCallback, stateTimeoutMessage);
              } else if (isNumeric(jsvGetCharInString(line,0))) {
                // finally - we have an IP
                net_esp8266_execute_and_set_idle();
              }
              break;
            case ESP8266_WAIT_SEND_OK:
              if (jsvIsStringEqualOrStartsWith(line, "SEND OK", false)) {
                net_esp8266_execute_and_set_idle();
              }
              break;
            case ESP8266_WAIT_SEND_OK_THEN_CLOSE:
              if (jsvIsStringEqualOrStartsWith(line, "SEND OK", false)) {
                JsVar *msg = jsvVarPrintf("AT+CIPCLOSE=%d\r\n", currentSocket);
                esp8266_send(msg);
                jsvUnLock(msg);
                net_esp8266_setState(ESP8266_WAIT_OK, 500, 0, "No acknowledgement");
              }
              break;
          }

          if (esp8266_idle_compare && jsvIsStringEqualOrStartsWith(line, esp8266_idle_compare, esp8266_idle_compare_only_start))
            found = true;
          jsvUnLock(line);
          JsVar *newBuf = jsvNewFromStringVar(buf, (size_t)(idx+1), JSVAPPENDSTRINGVAR_MAXLENGTH);
          jsvUnLock(buf);
          buf = newBuf;
          idx = jsvGetStringIndexOf(buf, '\r');
        }
      }
    }
  }
  if (hasChanged)
    jsvObjectSetChild(usartClass, STREAM_BUFFER_NAME, buf);
  jsvUnLock(buf);

  if (net_esp8266_getState() != ESP8266_IDLE && stateTimeout>=0 && jshGetSystemTime()>stateTimeout) {
    if (net_esp8266_getState() == ESP8266_DELAY) {
      net_esp8266_execute_and_set_idle();
    } else if (net_esp8266_getState() == ESP8266_DELAY_1S_WAIT_CONNECTED && connectionCheckCount) {
      connectionCheckCount--; // so we don't keep doing it
      // Keep asking for an IP address
      JsVar *msg = jsvNewFromString("AT+CIFSR\r\n");
      esp8266_send(msg);
      jsvUnLock(msg);
      net_esp8266_setState(ESP8266_WAIT_CONNECTED_RESPONSE, 1000, stateSuccessCallback, stateTimeoutMessage);
    } else {
      if (currentSocket>=0) {
        socketErrors = (unsigned char)(socketErrors | 1<<currentSocket);
        currentSocket = -1;
      }
      jsExceptionHere(JSET_ERROR, stateTimeoutMessage ? stateTimeoutMessage : "ESP8266 timeout");
      stateTimeoutMessage = 0;
      net_esp8266_setState(ESP8266_IDLE, 0, 0, 0);
    }
  }

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



bool net_esp8266_initialise(JsVar *callback) {
  JsVar *cmd = jsvNewFromString("AT+CWJAP=\"\",\"\"\r\n");
  esp8266_send(cmd);
  jsvUnLock(cmd);
  net_esp8266_setState(ESP8266_WAIT_OK_THEN_CWMODE_THEN_RESET_THEN_READY_THEN_CIPMUX, 500, callback, "No Acknowledgement");
  socketUsage = 0; // no sockets should be used now
  socketErrors = 0;
  return true;
}

bool net_esp8266_connect(JsVar *vAP, JsVar *vKey, JsVar *callback) {
  // 'AT+CWMODE=1\r' ? seems to be the default
  JsVar *msg = jsvVarPrintf("AT+CWJAP=%q,%q\r\n", vAP, vKey);
  esp8266_send(msg);
  jsvUnLock(msg);
  net_esp8266_setState(ESP8266_WAIT_OK_THEN_WAIT_CONNECTED, 500, callback, "No Acknowledgement");
  return true;
}
// ------------------------------------------------------------------------------------------------------------------------

/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_esp8266_gethostbyname(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
  // hacky - save the last checked name so we can put it straight into the request
  *out_ip_addr = 0xFFFFFFFF;
  jsvUnLock(jsvObjectSetChild(execInfo.hiddenRoot, ESP8266_DNS_NAME, jsvNewFromString(hostName)));
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
int net_esp8266_createsocket(JsNetwork *net, uint32_t host, unsigned short port) {
  char sckt;
  if (host!=0) {
    sckt = 0;
    if (socketUsage&(1<<SOCKET_COUNT)) {
      jsError("Can't create a client socket while we have a server");
      return -1;
    }
    while (sckt<SOCKET_COUNT && (socketUsage&(1<<sckt))) sckt++;
    if (sckt>=SOCKET_COUNT) {
      jsError("No available sockets");
      return -1;
    }
    socketErrors = (unsigned char)(socketErrors & ~(1<<sckt));
    socketUsage = (unsigned char)(socketUsage | 1<<sckt);
    JsVar *hostStr = 0;
    if (host==0xFFFFFFFF) {
      hostStr = jsvObjectGetChild(execInfo.hiddenRoot, ESP8266_DNS_NAME, 0);
      jsvObjectSetChild(execInfo.hiddenRoot, ESP8266_DNS_NAME, 0);
    }
    if (!hostStr)
      hostStr = networkGetAddressAsString((unsigned char *)&host, 4,10,'.');
    JsVar *msg = jsvVarPrintf("AT+CIPSTART=%d,\"TCP\",%q,%d\r\n", sckt, hostStr, port);
    jsvUnLock(hostStr);
    esp8266_send(msg);
    jsvUnLock(msg);
  } else { // server
    sckt = SOCKET_COUNT;
    if (socketUsage&(1<<sckt)) {
      jsError("Only one server allowed");
      return -1;
    }
    socketErrors = (unsigned char)(socketErrors & ~(1<<sckt));
    socketUsage = (unsigned char)(socketUsage | 1<<sckt);
    JsVar *msg = jsvVarPrintf("AT+CIPSERVER=1,%d\r\n", port);
    esp8266_send(msg);
    jsvUnLock(msg);
  }

  currentSocket = sckt;
  net_esp8266_setState(ESP8266_WAIT_OK, 10000, 0, "Couldn't create connection");
  return sckt;
}

/// destroys the given socket
void net_esp8266_closesocket(JsNetwork *net, int sckt) {
  JsVar *msg = 0;
  if (sckt!=SOCKET_COUNT) {
    if (net_esp8266_getState() == ESP8266_WAIT_SEND_OK && currentSocket==sckt) {
      esp8266State = ESP8266_WAIT_SEND_OK_THEN_CLOSE;
    } else {
      msg = jsvVarPrintf("AT+CIPCLOSE=%d\r\n", sckt);
    }
  } else { // server
    msg = jsvNewFromString("AT+CIPSERVER=0\r\n");
  }
  if (msg) {
    esp8266_send(msg);
    jsvUnLock(msg);
    net_esp8266_setState(ESP8266_WAIT_OK, 100, 0, "No Acknowledgement");
  }
  socketErrors = (unsigned char)(socketErrors & ~(1<<sckt)); // clear socket error
  socketUsage = (unsigned char)(socketUsage & ~(1<<sckt));
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_esp8266_accept(JsNetwork *net, int serverSckt) {
  NOT_USED(serverSckt); // only one server allowed, so no need to check
  assert(serverSckt==SOCKET_COUNT);
  char ipdName[ESP8266_IPD_NAME_LEN+2] = ESP8266_IPD_NAME"x";
  char sckt;
  // Basically we only have a connection if there's been data on it
  for (sckt=0;sckt<SOCKET_COUNT;sckt++) {
    if (socketUsage & (1<<sckt)) continue;
    ipdName[ESP8266_IPD_NAME_LEN] = (char)('0' + sckt); // make ipdName different for each socket
    JsVar *ipd = jsvObjectGetChild(execInfo.hiddenRoot, ipdName, 0);
    size_t l = ipd ? jsvGetStringLength(ipd) : 0;
    jsvUnLock(ipd);
    if (l) {
      //jsiConsolePrintf("Socket %d accepted\n", sckt);
      socketUsage = (unsigned char)(socketUsage | 1<<sckt);
      return sckt; // if we have data, return the socket
    }
  }

  return -1;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp8266_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  if (socketErrors & (1<<sckt)) return -1; // socket error

  char ipdName[ESP8266_IPD_NAME_LEN+2] = ESP8266_IPD_NAME"x";
  ipdName[ESP8266_IPD_NAME_LEN] = (char)('0' + sckt); // make ipdName different for each socket

  JsVar *ipd = jsvObjectGetChild(execInfo.hiddenRoot, ipdName, 0);
  size_t chars = 0;
  if (ipd) {
    chars = jsvGetStringLength(ipd);
    jsvGetStringChars(ipd, 0, buf, len);

    JsVar *newIpd = (chars>len) ? jsvNewFromStringVar(ipd, len, JSVAPPENDSTRINGVAR_MAXLENGTH) : 0;
    jsvUnLock(ipd);
    jsvUnLock(jsvObjectSetChild(execInfo.hiddenRoot, ipdName, newIpd));
  }
  return (int)((chars>len) ? len : chars);
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_esp8266_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  if (net_esp8266_getState() != ESP8266_IDLE) return 0; // can't send
  if (socketErrors & (1<<sckt)) return -1; // socket error

  JsVar *msg = jsvVarPrintf("AT+CIPSEND=%d,%d\r",sckt,len); // no \n !
  esp8266_send(msg);
  jsvUnLock(msg);

  // doesn't seem to return this until it's done, but we need to wait anyway
  if (esp8266_wait_for(">",1000,true)) {
    size_t i;
    for (i=0;i<len;i++)
      jshTransmit(networkGetCurrent()->data.device, ((unsigned char *)buf)[i]);
    currentSocket = sckt;
    net_esp8266_setState(ESP8266_WAIT_SEND_OK, 2000, 0, "Send failed");
    return (int)len;
  }
  return -1; // broken!
}

// ------------------------------------------------------------------------------------------------------------------------

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

