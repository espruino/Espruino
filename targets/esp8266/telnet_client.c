/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2015 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains ESP8266 board specific functions.
 * ----------------------------------------------------------------------------
 */
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include <telnet.h>

#define MAXLINE 200

static char line[MAXLINE];
static struct espconn conn1;
static esp_tcp tcp1;
static bool discard;
static void (*lineCB)(char *);
static struct espconn *pTelnetClientConn;

/**
 * Callback invoked when a new TCP/IP connection has been formed.
 * o arg - This is a pointer to the struct espconn that reflects this entry.
 */
static void connectCB(void *arg) {
  struct espconn *pConn = (struct espconn *)arg;

  // By default, an ESP TCP connection times out after 10 seconds
  // Change to the max value.
  espconn_regist_time(pConn, 7200, 1);

  os_printf("Connect cb!!\n");
  strcpy(line, "");
  discard = false;
  pTelnetClientConn = pConn;
}

static void disconnectCB(void *arg) {
  os_printf("Disconnect cb!!\n");
}


static void receiveCB(void *arg, char *pData, unsigned short len) {
  //os_printf("Receive cb!!  len=%d\n", len);
  if (strlen(line) + len >= MAXLINE) {
    discard = true;
    strcpy(line, "");
  }
  strncat(line, pData, len);
  if (strstr(line, "\r\n") != NULL) {
    if (discard) {
      os_printf("Discarded ... line too long");
      discard = false;
    } else {
      //os_printf("Found: %s", line);
      (*lineCB)(line);
    }
    strcpy(line, "");
  } else {
    //os_printf("Not found, current='%s'\n", line);
  }
}

/**
 * Register a telnet serever listener on port 23 to handle incoming telnet client
 * connections.  The parameter called pLineCB is a pointer to a callback function
 * that should be called when we have received a link of input.
 */
void telnet_startListening(void (*plineCB)(char *)) {
  lineCB= plineCB;
  tcp1.local_port = 23;
  conn1.type = ESPCONN_TCP;
  conn1.state = ESPCONN_NONE;
  conn1.proto.tcp = &tcp1;
  espconn_regist_connectcb(&conn1, connectCB);
  espconn_regist_disconcb(&conn1, disconnectCB);
  espconn_regist_recvcb(&conn1, receiveCB);
  espconn_accept(&conn1);
  os_printf("Now listening for telnet client connection ...\n");
}

/**
 * Send the string passed as a parameter to the telnet client.
 */
void telnet_send(char *text) {
  espconn_sent(pTelnetClientConn, (uint8_t *)text, strlen(text));
}
