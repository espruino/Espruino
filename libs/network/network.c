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
 * Contains functions for handling JsNetwork and doing common networking tasks
 * ----------------------------------------------------------------------------
 */
#include "network.h"
#include "jsparse.h"

#if defined(USE_CC3000)
  #include "network_cc3000.h"
#endif
#if defined(USE_WIZNET)
  #include "network_wiznet.h"
#endif
#if defined(LINUX)
  #include "network_linux.h"
#endif

JsNetworkState networkState =
#if defined(USE_CC3000) || defined(USE_WIZNET)
    NETWORKSTATE_OFFLINE
#else
    NETWORKSTATE_ONLINE
#endif
    ;

static unsigned long parseIPAddress(const char *ip) {
  int n = 0;
  unsigned long addr = 0;
  while (*ip) {
    if (*ip>='0' && *ip<='9') {
      n = n*10 + (*ip-'0');
    } else if (*ip>='.') {
      addr = (addr>>8) | (unsigned long)(n<<24);
      n=0;
    } else {
      return 0; // not an ip address
    }
    ip++;
  }
  addr = (addr>>8) | (unsigned long)(n<<24);
  return addr;
}

/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void networkGetHostByName(JsNetwork *net, char * hostName, unsigned long* out_ip_addr) {
  assert(out_ip_addr);
  *out_ip_addr = 0;

  *out_ip_addr = parseIPAddress(hostName); // first try and simply parse the IP address
  if (!*out_ip_addr)
    net->gethostbyname(net, hostName, out_ip_addr);
}



void networkCreate(JsNetwork *net) {
  net->networkVar = jsvNewStringOfLength(sizeof(JsNetworkData));
  jsvUnLock(jsvObjectSetChild(execInfo.root, NETWORK_VAR_NAME, net->networkVar));
  networkGetFromVar(net);
}

bool networkGetFromVar(JsNetwork *net) {
  net->networkVar = jsvObjectGetChild(execInfo.root, NETWORK_VAR_NAME, 0);
  if (!net->networkVar) {
#ifdef LINUX
    networkCreate(net);
    net->data.type = JSNETWORKTYPE_SOCKET;
    networkSet(net);
    return true;
#else
    return false;
#endif
  }
  jsvGetString(net->networkVar, (char *)&net->data, sizeof(JsNetworkData)+1/*trailing zero*/);

#if defined(USE_CC3000)
  netSetCallbacks_cc3000(net);
#elif defined(USE_WIZNET)
  netSetCallbacks_wiznet(net);
#else
  netSetCallbacks_linux(net);
#endif
  return true;
}

bool networkGetFromVarIfOnline(JsNetwork *net) {
  bool found = networkGetFromVar(net);
  if (!found || networkState != NETWORKSTATE_ONLINE) {
    jsError("Not connected to the internet");
    if (found) networkFree(net);
    return false;
  }
  return true;
}

void networkSet(JsNetwork *net) {
  jsvSetString(net->networkVar, (char *)&net->data, sizeof(JsNetworkData));
}

void networkFree(JsNetwork *net) {
  jsvUnLock(net->networkVar);
}
