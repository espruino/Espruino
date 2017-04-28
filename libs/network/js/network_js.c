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
* Implementation of JsNetwork that calls into JavaScript
 * ----------------------------------------------------------------------------
 */
#include "jsinteractive.h"
#include "network.h"
#include "network_js.h"
#include "jswrap_stream.h"

#define JSNET_NAME "JSN"
#define JSNET_DNS_NAME "DNS"

// Set the built-in object for network access
void net_js_setObj(JsVar *obj) {
  jsvObjectSetChild(execInfo.hiddenRoot, JSNET_NAME, obj);
}

/// Call the named function on the object - whether it's built in, or predefined. Returns the return value of the function.
JsVar *callFn(char* name, int argCount, JsVar **argPtr) {
  JsVar *netObj = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_NAME, 0);
  JsVar *child = jspGetNamedField(netObj, name, false);

  JsVar *r = 0;
  if (jsvIsFunction(child)) {
    JsExecInfo oldExecInfo = execInfo;
    execInfo.execute = EXEC_YES;
    r = jspeFunctionCall(child, 0, netObj, false, argCount, argPtr);
    execInfo = oldExecInfo;
  }
  jsvUnLock2(child, netObj);
  return r;
}

// ------------------------------------------------------------------------------------------------------------------------

/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_js_gethostbyname(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
  NOT_USED(net);
  // hacky - save the last checked name so we can put it straight into the request
  *out_ip_addr = 0xFFFFFFFF;
  jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, JSNET_DNS_NAME, jsvNewFromString(hostName));
}

/// Called on idle. Do any checks required for this device
void net_js_idle(JsNetwork *net) {
  NOT_USED(net);
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_js_checkError(JsNetwork *net) {
  NOT_USED(net);
  return true;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_js_createsocket(JsNetwork *net, uint32_t host, unsigned short port) {
  NOT_USED(net);
  JsVar *hostVar = 0;
  if (host!=0) {
    // client
    if (host==0xFFFFFFFF) {
      hostVar = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_DNS_NAME, 0);
    }
    if (!hostVar)
      hostVar = networkGetAddressAsString((unsigned char *)&host, 4,10,'.');
  }
  // else server, hostVar=0
  JsVar *args[2] = {
      hostVar,
      jsvNewFromInteger(port)
  };
  int sckt = jsvGetIntegerAndUnLock(callFn("create", 2, args));
  jsvUnLockMany(2, args);
  return sckt;
}

/// destroys the given socket
void net_js_closesocket(JsNetwork *net, int sckt) {
  NOT_USED(net);
  JsVar *args[1] = {
      jsvNewFromInteger(sckt)
  };
  jsvUnLock2(callFn("close", 1, args), args[0]);
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_js_accept(JsNetwork *net, int serverSckt) {
  NOT_USED(net);
  JsVar *netObj = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_NAME, 0);
  JsVar *args[1] = {
      jsvNewFromInteger(serverSckt)
  };

  int sckt = jsvGetIntegerAndUnLock(callFn("accept", 1, args));
  jsvUnLock2(args[0], netObj);
  return sckt;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_js_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  NOT_USED(net);
  JsVar *args[2] = {
      jsvNewFromInteger(sckt),
      jsvNewFromInteger((JsVarInt)len),
  };
  JsVar *res = callFn( "recv", 2, args);
  jsvUnLockMany(2, args);
  int r = -1; // fail
  if (jsvIsString(res)) {
    r = (int)jsvGetStringLength(res);
    if (r>(int)len) { r=(int)len; assert(0); }
    jsvGetStringChars(res, 0, (char*)buf, (size_t)r);
    // FIXME: jsvGetStringChars adds a 0 - does that actually write past the end of the array, or clip the data we get?
  } else if (jsvIsInt(res)) {
    r = jsvGetInteger(res);
    if (r>=0) {
      jsExceptionHere(JSET_ERROR, "JSNetwork.recv returned >=0");
      r=-1;
    }
  }
  jsvUnLock(res);
  return r;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_js_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  NOT_USED(net);
  JsVar *args[2] = {
      jsvNewFromInteger(sckt),
      jsvNewFromEmptyString()
  };
  jsvAppendStringBuf(args[1], buf, len);
  int r = jsvGetIntegerAndUnLock(callFn( "send", 2, args));
  jsvUnLockMany(2, args);
  return r;
}

// ------------------------------------------------------------------------------------------------------------------------

void netSetCallbacks_js(JsNetwork *net) {
  net->idle = net_js_idle;
  net->checkError = net_js_checkError;
  net->createsocket = net_js_createsocket;
  net->closesocket = net_js_closesocket;
  net->accept = net_js_accept;
  net->gethostbyname = net_js_gethostbyname;
  net->recv = net_js_recv;
  net->send = net_js_send;
  net->chunkSize = 536;
}

