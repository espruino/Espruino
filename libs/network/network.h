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

#ifndef _NETWORK_H
#define _NETWORK_H

#include "jsutils.h"
#include "jsvar.h"
#include "jshardware.h"

#define NETWORK_VAR_NAME "net"

typedef enum {
  NETWORKSTATE_OFFLINE,
  NETWORKSTATE_CONNECTED, // connected but not online (no DHCP)
  NETWORKSTATE_ONLINE, // DHCP (or manual address)
  NETWORKSTATE_INVOLUNTARY_DISCONNECT, // just randomly disconnected - maybe try and reconnect
} PACKED_FLAGS JsNetworkState;

extern JsNetworkState networkState; // FIXME put this in JsNetwork

// This is all code for handling multiple types of network access with one binary
typedef enum {
  JSNETWORKTYPE_SOCKET,  ///< Standard linux socket API
  JSNETWORKTYPE_CC3000,  ///< TI CC3000 support
  JSNETWORKTYPE_W5500,  ///< WIZnet W5500 support
  JSNETWORKTYPE_JS,  ///< JavaScript network type
  JSNETWORKTYPE_ESP8266_BOARD, ///< Espressif ESP8266 board support
  JSNETWORKTYPE_ESP32 /// < Espressif ESP32 board support
} JsNetworkType;

typedef struct {
  JsNetworkType type;
  // Info for accessing specific devices
  IOEventFlags device;
  Pin pinCS, pinIRQ, pinEN;
} PACKED_FLAGS JsNetworkData;


// Here we assume that IP addresses are stored IN ORDER - eg. 192.168.1.1 = [192,168,1,1] - CC3000 does it backwards
typedef struct JsNetwork {
  JsVar *networkVar; // this won't be locked again - we just know that it is already locked by something else
  JsNetworkData data;
  unsigned char _blank; ///< this is needed as jsvGetString for 'data' wants to add a trailing zero  

  int chunkSize; ///< Amount of memory to allocate for chunks of data when using send/recv

  /// Called on idle. Do any checks required for this device
  void (*idle)(struct JsNetwork *net);
  /// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
  bool (*checkError)(struct JsNetwork *net);

  /// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
  int (*createsocket)(struct JsNetwork *net, uint32_t host, unsigned short port);
  /// destroys the given socket
  void (*closesocket)(struct JsNetwork *net, int sckt);
  /// If the given server socket can accept a connection, return it (or return < 0)
  int (*accept)(struct JsNetwork *net, int sckt);
  /// Get an IP address from a name
  void (*gethostbyname)(struct JsNetwork *net, char * hostName, uint32_t* out_ip_addr);
  /// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
  int (*recv)(struct JsNetwork *net, int sckt, void *buf, size_t len);
  /// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
  int (*send)(struct JsNetwork *net, int sckt, const void *buf, size_t len);
} PACKED_FLAGS JsNetwork;

// ---------------------------------- these are in network.c
// Get the relevant info for JsNetwork (done from a var in root scope)
void networkCreate(JsNetwork *net, JsNetworkType type); // create the network object (ONLY to be used by network drivers)
bool networkWasCreated();
bool networkGetFromVar(JsNetwork *net);
bool networkGetFromVarIfOnline(JsNetwork *net); // only return true (and network) if we're online, otherwise warn
void networkSet(JsNetwork *net);
void networkFree(JsNetwork *net);

JsNetwork *networkGetCurrent(); ///< Get the currently active network structure. can be 0!
// ---------------------------------------------------------

/// Use this for getting the hostname, as it parses the name to see if it is an IP address first
void networkGetHostByName(JsNetwork *net, char * hostName, uint32_t* out_ip_addr);
uint32_t networkParseIPAddress(const char *ip);
/* given 6 pairs of 8 bit hex numbers separated by ':', parse them into a
 * 6 byte array. returns false on failure */
bool networkParseMACAddress(unsigned char *addr, const char *ip);
/// if nBytes<0, addresses are printed out backwards
JsVar *networkGetAddressAsString(unsigned char *ip, int nBytes, unsigned int base, char separator);
/// Given an address (pointed to by ip) put it in a string named 'name', in the given object. if nBytes<0, addresses are printed out backwards
void networkPutAddressAsString(JsVar *object, const char *name,  unsigned char *ip, int nBytes, unsigned int base, char separator);
/** Some devices (CC3000) store the IP address with the first element last, so we must flip it */
unsigned long networkFlipIPAddress(unsigned long addr);

typedef enum {
  NCF_NORMAL = 0,
  NCF_TLS = 1
} NetCreateFlags;

/// Check for any errors and try and recover (CC3000 only really)
bool netCheckError(JsNetwork *net);

/// Create a socket (server (host==0) or client)
int netCreateSocket(JsNetwork *net, uint32_t host, unsigned short port, NetCreateFlags flags, JsVar *options);

/// Ask this socket to close - it may not close immediately
void netCloseSocket(JsNetwork *net, int sckt);

/** If this is a server socket and we have an incoming connection then
 * accept and return the socket number - else return <0 */
int netAccept(JsNetwork *net, int sckt);

void netGetHostByName(JsNetwork *net, char * hostName, uint32_t* out_ip_addr);
int netRecv(JsNetwork *net, int sckt, void *buf, size_t len);
int netSend(JsNetwork *net, int sckt, const void *buf, size_t len);

#endif // _NETWORK_H
