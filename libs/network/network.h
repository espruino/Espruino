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
 * Network access globals
 * ----------------------------------------------------------------------------
 */

#ifndef _NETWORK_H
#define _NETWORK_H

#include "jsutils.h"
#include "jsvar.h"

typedef enum {
  NETWORKSTATE_OFFLINE,
  NETWORKSTATE_CONNECTED, // connected but not online (no DHCP)
  NETWORKSTATE_ONLINE, // DHCP (or manual address)
} PACKED_FLAGS JsNetworkState;

extern JsNetworkState networkState;

// This is all potential code for handling multiple types of network access with one binary
#if 0

typedef enum {
  JSNETWORKTYPE_SOCKET,  ///< Standard linux socket API
  JSNETWORKTYPE_CC3000,  ///< CC3000 support
  // wiznet?
  // enc28j60?
} JsNetworkType;

typedef struct {
  JsNetworkType type;
} PACKED_FLAGS JsNetworkData;

typedef struct JsNetwork {
  JsVar *neworkVar; // this won't be locked again - we just know that it is already locked by something else
  JsNetworkData data;
  unsigned char _blank; ///< this is needed as jsvGetString for 'data' wants to add a trailing zero  

  int (*socket)(struct JsNetwork *gfx, long domain, long type, long protocol);
  long (*closesocket)(struct JsNetwork *gfx, long sd);
  long (*accept)(struct JsNetwork *gfx, long sd, sockaddr *addr, socklen_t *addrlen);
  long (*bind)(struct JsNetwork *gfx, long sd, const sockaddr *addr, long addrlen);
  long (*listen)(struct JsNetwork *gfx, long sd, long backlog);
  int (*gethostbyname)(struct JsNetwork *gfx, char * hostname, unsigned short usNameLen, unsigned long* out_ip_addr);
  long (*connect)(struct JsNetwork *gfx, long sd, const sockaddr *addr, long addrlen);
  int (*select)(struct JsNetwork *gfx, long nfds, fd_set *readsds, fd_set *writesds, fd_set *exceptsds, struct timeval *timeout);
  int (*recv)(struct JsNetwork *gfx, long sd, void *buf, long len);
  int (*send)(struct JsNetwork *gfx, long sd, const void *buf, long len);
} PACKED_FLAGS JsNetwork;

static inline void networkStructInit(JsNetwork *net) {  
}

// ---------------------------------- these are in network.c
// Access a JsVar and get/set the relevant info in JsGraphics
bool networkGetFromVar(JsNetwork *net, JsVar *parent);
void networkSetVar(JsNetwork *net);
// ----------------------------------------------------------------------------------------------

void networkIdle(); ///< called when idling

#endif

#endif // _NETWORK_H
