/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Thorsten von Eicken
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript Telnet console
 * ----------------------------------------------------------------------------
 */
#include "jswrap_net.h"
#include "jswrap_http.h"
#include "jsvariterator.h"
#include "jsinteractive.h"
#include "socketserver.h"

#include "../network.h"

#if defined(ESP8266)
extern int os_printf_plus(const char *format, ...)  __attribute__((format(printf, 1, 2)));
#define printf os_printf_plus
#elif defined(ESP32) && !defined(RELEASE)
#define printf jsWarn
//#elif defined(LINUX)
//#define printf printf
#else
#define printf(X, ...) do{}while(0)
#endif

#ifdef LINUX
#define PORT 2323 // avoid needing root permissions
bool telnetEnabled = false; // whether telnet should be enabled or not. Set in main.c
#else
#define PORT 23
#endif

// Forward function declarations

void telnetStart(JsNetwork *net);
void telnetStop(JsNetwork *net);
bool telnetAccept(JsNetwork *net);
bool telnetSendBuf(JsNetwork *net);
bool telnetRecv(JsNetwork *net);

// Telnet console data structures

#define MODE_OFF 0    // telnet console is off
#define MODE_ON  1    // telnet console is on

#define TX_CHUNK 1072         // size of chunks read from JS, buffered, and sent on socket

// Data structure for a telnet console server
typedef struct {
  int          sock;             ///< listening server socket, 0=none (actual socket numbers are 1 less than this, as 0 is a valid socket)
  int          cliSock;          ///< active client socket, 0=none (actual socket numbers are 1 less than this, as 0 is a valid socket)
  char         txBuf[TX_CHUNK];  ///< transmit buffer
  uint16_t     txBufLen;         ///< number of chars in tx buffer
  IOEventFlags oldConsole;       ///< device the console was stolen from
} TelnetServer;

static TelnetServer tnSrv;        ///< the telnet server, only one right now
static uint8_t      tnSrvMode;    ///< current mode for the telnet server

/*JSON{
  "type"  : "library",
  "class" : "TelnetServer"
}
This library implements a telnet console for the Espruino interpreter. It requires a network
connection, e.g. Wifi, and **currently only functions on the ESP8266 and on Linux **. It uses
port 23 on the ESP8266 and port 2323 on Linux.

**Note:** To enable on Linux, run `./espruino --telnet`
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "TelnetServer",
  "name"     : "setOptions",
  "generate" : "jswrap_telnet_setOptions",
  "params": [
    [ "options", "JsVar", "Options controlling the telnet console server" ]
  ]
}
*/
void jswrap_telnet_setOptions(JsVar *jsOptions) {
  // Make sure jsOptions is an object
  if (!jsvIsObject(jsOptions)) {
    jsExceptionHere(JSET_ERROR, "Expecting options object but got %t", jsOptions);
    return;
  }

  // Get mode
  JsVar *jsMode = jsvObjectGetChild(jsOptions, "mode", 0);
  if (jsvIsString(jsMode)) {
    if (jsvIsStringEqual(jsMode, "on")) {
      tnSrvMode = MODE_ON;
    } else if (jsvIsStringEqual(jsMode, "off")) {
      tnSrvMode = MODE_OFF;
    } else {
      jsvUnLock(jsMode);
      jsExceptionHere(JSET_ERROR, "Unknown mode value");
      return;
    }
  }
  jsvUnLock(jsMode);
}

/*JSON{
  "type"     : "init",
  "class"    : "TelnetServer",
  "generate" : "jswrap_telnet_init"
}
*/
void jswrap_telnet_init(void) {
#ifdef LINUX
  tnSrvMode = telnetEnabled ? MODE_ON : MODE_OFF;
#else
  tnSrvMode = MODE_ON; // hardcoded for now
#endif
}

/*JSON{
  "type"     : "kill",
  "class"    : "TelnetServer",
  "generate" : "jswrap_telnet_kill"
}
*/
void jswrap_telnet_kill(void) {
  tnSrvMode = MODE_OFF;
}

/*JSON{
  "type"     : "idle",
  "class"    : "TelnetServer",
  "generate" : "jswrap_telnet_idle"
}
*/
bool jswrap_telnet_idle(void) {
  // get a handle to the network, no network -> can't do anything at all
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return false;

  // if we're supposed to be off, then make sure we're disconnected
  if (tnSrvMode == MODE_OFF) {
    if (tnSrv.sock > 0) {
      telnetStop(&net);
    }
    networkFree(&net);
    return false;
  }

  // we're supposed to be on, make sure we're listening
  if (tnSrv.sock <= 0) {
    memset(&tnSrv, 0, sizeof(TelnetServer));
    telnetStart(&net);
    if (tnSrv.sock <= 0) {
      networkFree(&net);
      return false; // seems like there's a problem...
    }
  }

  // looks like we are listening, now deal with actual connected sockets
  bool active = false;
  active |= telnetAccept(&net);
  active |= telnetRecv(&net);
  active |= telnetSendBuf(&net);
  //if (active) printf("tnSrv: idle=%d\n", active);

  networkFree(&net);
  return active;
}

//===== Internal functions

// Start the listening socket for the telnet console server.
void telnetStart(JsNetwork *net) {
  // create the listening socket
  printf("tnSrv: creating...\n");
  int sock = netCreateSocket(net, 0, PORT, NCF_NORMAL, NULL)+1;
  if (sock <= 0) {
    printf("tnSrv: cannot create listening socket\n");
    return;
  }
  tnSrv.sock = sock;
  printf("tnSrv: started sock=%d\n", sock);
}

// Terminate the telnet console
void telnetStop(JsNetwork *net) {
  printf("tnSrv: stopped sock=%d\n", tnSrv.sock);
  if (tnSrv.cliSock > 0) netCloseSocket(net, tnSrv.cliSock-1);
  tnSrv.cliSock = 0;
  if (tnSrv.sock > 0) netCloseSocket(net, tnSrv.sock-1);
  tnSrv.sock = 0;
}

// Attempt to accept a connection, returns true if it did something
bool telnetAccept(JsNetwork *net) {
  // we're gonna do a single accept per idle iteration for now
  if (tnSrv.sock <= 0) return false;
  int sock = netAccept(net, tnSrv.sock-1)+1;
  if (sock <= 0) return false; // nothing

  // if we already have a client, then disconnect it
  if (tnSrv.cliSock > 0) {
    netCloseSocket(net, tnSrv.cliSock-1);
  }
  // if the console is not already telnet, then change it
  IOEventFlags console = jsiGetConsoleDevice();
  if (console != EV_TELNET) {
    tnSrv.oldConsole = console;
    if (!jsiIsConsoleDeviceForced()) jsiSetConsoleDevice(EV_TELNET, false);
  }

  tnSrv.cliSock = sock;
  printf("tnSrv: accepted console on sock=%d\n", sock-1);
  return true;
}

// Close the connection and release the console device
void telnetRelease(JsNetwork *net) {
  if (!(tnSrv.sock && tnSrv.cliSock)) return;
  printf("tnSrv: released console from sock %d\n", tnSrv.cliSock-1);
  netCloseSocket(net, tnSrv.cliSock-1);
  tnSrv.cliSock = 0;
  IOEventFlags console = jsiGetConsoleDevice();
  // only switch away from telnet if the current console is TELNET, this allows the current
  // console to be set to something else while connected via telnet and then not have it
  // switched again when disconnecting from telnet
  if (console == EV_TELNET && !jsiIsConsoleDeviceForced())
    jsiSetConsoleDevice(tnSrv.oldConsole, false);
}

// Attempt to send buffer on an established client connection, returns true if it sent something
bool telnetSendBuf(JsNetwork *net) {
  if (tnSrv.sock == 0 || tnSrv.cliSock == 0) return false;

  // if we have nothing buffered, that's it
  if (tnSrv.txBufLen == 0) return false;

  // try to send the tx buffer
  int sent = netSend(net, tnSrv.cliSock-1, tnSrv.txBuf, tnSrv.txBufLen);
  if (sent == tnSrv.txBufLen) {
    tnSrv.txBufLen = 0;
  } else if (sent > 0) {
    // shift remaining chars up in the buffer
    memmove(tnSrv.txBuf, tnSrv.txBuf+sent, (size_t)(tnSrv.txBufLen-sent));
    tnSrv.txBufLen = (uint16_t)(tnSrv.txBufLen - sent);
  } else if (sent < 0) {
    telnetRelease(net);
  }
  if (sent != 0) {
    //printf("tnSrv: sent sock=%d, %d bytes, %d left\n", tnSrv.sock-1, sent, tnSrv.txBufLen);
  }
  return sent != 0;
}

static bool ovf;

void telnetSendChar(char ch) {
  if (tnSrv.sock == 0 || tnSrv.cliSock == 0) return;
  if (tnSrv.txBufLen >= TX_CHUNK) {
    // buffer overflow :-(
    if (!ovf) {
      printf("tnSrv: send overflow!\n");
      ovf = true;
    }
  } else {
    ovf = false;
    tnSrv.txBuf[tnSrv.txBufLen++] = ch;
  }

  // if the buffer has a bunch of chars then try to send, else it'll happen
  // at idle time.
  if (tnSrv.txBufLen < TX_CHUNK/4) return;
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;
  telnetSendBuf(&net);
  networkFree(&net);
}

// Attempt to receive on an established client connection, returns true if it received something
bool telnetRecv(JsNetwork *net) {
  if (tnSrv.sock == 0 || tnSrv.cliSock == 0) return false;

  char buff[256];
  int r = netRecv(net, tnSrv.cliSock-1, buff, 256);
  if (r > 0) {
    jshPushIOCharEvents(EV_TELNET, buff, (unsigned int)r);
  } else if (r < 0) {
    telnetRelease(net);
  }
  if (r != 0) {
    //printf("tnSrv: recv sock=%d, %d bytes\n", tnSrv.sock-1, r);
  }
  return r != 0;
}


