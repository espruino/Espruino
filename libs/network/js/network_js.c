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

// ------------------------------------------------------------------------------------------------------------------------

/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_js_gethostbyname(JsNetwork *net, char * hostName, unsigned long* out_ip_addr) {
  // hacky - save the last checked name so we can put it straight into the request
  *out_ip_addr = 0xFFFFFFFF;
  jsvUnLock(jsvObjectSetChild(execInfo.hiddenRoot, JSNET_DNS_NAME, jsvNewFromString(hostName)));
}

/// Called on idle. Do any checks required for this device
void net_js_idle(JsNetwork *net) {
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_js_checkError(JsNetwork *net) {
  return true;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_js_createsocket(JsNetwork *net, uint32_t host, unsigned short port) {
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
  JsVar *netObj = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_NAME, 0);
  JsVar *args[2] = {
      hostVar,
      jsvNewFromInteger(port)
  };
  int sckt = jsvGetIntegerAndUnLock(jspCallNamedFunction(net, "create", 2, args));
  jsvUnLock(args[0]);
  jsvUnLock(args[1]);
  jsvUnLock(netObj);
  return sckt;
}

/// destroys the given socket
void net_js_closesocket(JsNetwork *net, int sckt) {
  JsVar *netObj = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_NAME, 0);
  JsVar *args[1] = {
      jsvNewFromInteger(sckt)
  };
  jspCallNamedFunction(net, "recv", 2, args);
  jsvUnLock(args[0]);
  jsvUnLock(netObj);
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_js_accept(JsNetwork *net, int serverSckt) {
  JsVar *netObj = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_NAME, 0);
  JsVar *args[1] = {
      jsvNewFromInteger(serverSckt)
  };
  int sckt = jsvGetIntegerAndUnLock(jspCallNamedFunction(net, "accept", 1, args));
  jsvUnLock(args[0]);
  jsvUnLock(netObj);
  return sckt;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_js_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  JsVar *netObj = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_NAME, 0);
  JsVar *args[1] = {
      jsvNewFromInteger(sckt)
  };
  int r = jsvGetIntegerAndUnLock(jspCallNamedFunction(net, "recv", 1, args));
  jsvUnLock(args[0]);
  jsvUnLock(netObj);
  return r;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_js_send(JsNetwork *net, int sckt, const void *buf, size_t len) {
  JsVar *netObj = jsvObjectGetChild(execInfo.hiddenRoot, JSNET_NAME, 0);
  JsVar *args[2] = {
      jsvNewFromInteger(sckt),
      jsvNewFromEmptyString()
  };
  jsvAppendStringBuf(args[1], buf, len);
  int r = jsvGetIntegerAndUnLock(jspCallNamedFunction(net, "send", 2, args));
  jsvUnLock(args[0]);
  jsvUnLock(args[1]);
  jsvUnLock(netObj);
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
}

