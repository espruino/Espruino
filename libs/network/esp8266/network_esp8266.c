
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
 * Contains ESP8266 board network specific functions.
 * ----------------------------------------------------------------------------
 */

// ESP8266 specific includes
#include <c_types.h>
#include <user_interface.h>
#include <mem.h>
#include "osapi_release.h"
#include <espconn.h>
#include <espmissingincludes.h>

#define _GCC_WRAP_STDINT_H
typedef long long int64_t;

#include "network_esp8266.h"
#include "socketerrors.h"
#include "esp8266_board_utils.h"
#include "pktbuf.h"

//#define espconn_abort espconn_disconnect

// Set NET_DBG to 0 to disable debug printf's, to 1 for important printf's
#ifdef RELEASE
  #define DBG(format, ...) do { } while(0)
#else
  // Normal debug
  #if NET_DBG > 0
    #define DBG(format, ...) os_printf(format, ## __VA_ARGS__)
    // #include "jsinteractive.h"
    // #define DBG(format, ...) jsiConsolePrintf(format, ## __VA_ARGS__)
    static char DBG_LIB[] = "net_esp8266"; // library name
  #else
    #define DBG(format, ...) do { } while(0)
  #endif
#endif

static struct socketData *getSocketData(int s);

/**
 * The next socketId to be used.
 */
static int g_nextSocketId = 0;

static int  getServerSocketByLocalPort(unsigned short port);
static void setSocketInError(struct socketData *pSocketData, int code);
static void dumpEspConn(struct espconn *pEspConn);
static struct socketData *allocateNewSocket();
static int connectSocket(struct socketData *pSocketData);
static void doClose(struct socketData *pSocketData);
static void releaseSocket(struct socketData *pSocketData);
static void resetSocket(struct socketData *pSocketData);
static void esp8266_dumpSocketData(struct socketData *pSocketData);

static void esp8266_callback_connectCB_inbound(void *arg);
static void esp8266_callback_connectCB_outbound(void *arg);
static void esp8266_callback_disconnectCB(void *arg);
static void esp8266_callback_sentCB(void *arg);
static void esp8266_callback_writeFinishedCB(void *arg);
static void esp8266_callback_recvCB(void *arg, char *pData, unsigned short len);
static void esp8266_callback_reconnectCB(void *arg, sint8 err);

/** Socket data structure
 *
 * We maintain an array of socketData structures.  The number of such structures is defined in the
 * MAX_SOCKETS define.  The private variable that contains the array is called "socketArray".
 * Each one of these array instances represents a possible socket structure that we can use.
 *
 * Each socket maintains state and its creation purpose.
 *
 * The trickiest part is closing. If the socket lib closes a socket it forgets about the socket
 * as soon as we return. We then have to issue a disconnect to espconn and await the disconnect
 * call-back in the SOCKET_STATE_DISCONNECTING. Once that's done, we can deallocate everything.
 * If we receive a disconnect from the remote end, we free the espconn struct and we transition
 * to SOCKET_STATE_CLOSED until we can respond to a send/recv call from the
 * socket library with -1 and it then calls close.
 * In summary, the rules are:
 * - SOCKET_STATE_DISCONNECTING: the socket lib has closed, we close when we get disconCB
 * - SOCKET_STATE_CLOSED: we have closed espconn, awaiting socket lib to issue close
 * - SOCKET_STATE_ABORTING: we await reconCB/disconCB, we also await socket lib to issue close
 */

/**
 * The potential states for a socket.
 * See the socket state diagram.
 */
enum SOCKET_STATE {
  SOCKET_STATE_UNUSED,         //!< Unused socket "slot"
  SOCKET_STATE_UNACCEPTED,     //!< New inbound connection that Espruino hasn't accepted yet
  SOCKET_STATE_HOST_RESOLVING, //!< Resolving a hostname, happens before CONNECTING
  SOCKET_STATE_CONNECTING,     //!< In the process of connecting
  SOCKET_STATE_IDLE,           //!< Connected but nothing in tx buffers
  SOCKET_STATE_TRANSMITTING,   //!< Connected and espconn_send has been called, awaiting CB
  SOCKET_STATE_DISCONNECTING,  //!< Did disconnect, awaiting discon callback from espconn
  SOCKET_STATE_ABORTING,       //!< Did abort, awaiting discon callback from espconn
  SOCKET_STATE_TO_ABORT,       //!< Need to abort asap (couldn't do it in callback)
  SOCKET_STATE_CLOSED,         //!< Closed, espconn struct freed, awaiting close from socket lib
};

/**
 * How was the socket created.
 */
enum SOCKET_CREATION_TYPE {
  SOCKET_CREATED_NONE,      //!< The socket has not yet been created.
  SOCKET_CREATED_SERVER,    //!< Listening socket ("server socket")
  SOCKET_CREATED_OUTBOUND,  //!< Outbound connection
  SOCKET_CREATED_INBOUND    //!< Inbound connection
};

/**
 * The core socket structure.
 * The structure is initialized by resetSocket.
 */
struct socketData {
  int                       socketId;     //!< The id of THIS socket.
  enum SOCKET_STATE         state;        //!< What is the socket state?
  enum SOCKET_CREATION_TYPE creationType; //!< How was the socket created?

  struct  espconn *pEspconn;              //!< The ESPConn structure.

  uint8    *currentTx;        //!< Data currently being transmitted.
  PktBuf   *rxBufQ;           //!< Queue of received buffers

  short    errorCode;         //!< Error code, 0=no error

  uint32_t        host;
  unsigned short  port;

  uint32_t  multicastGrpIp;
  uint32_t  multicastIp;
};


/**
 * An array of socket data structures.
 */
static struct socketData socketArray[MAX_SOCKETS];

/**
 * Flag the sockets as initially NOT initialized.
 */
static bool g_socketsInitialized = false;

/**
 * Dump all the socket structures.
 * This is used exclusively for debugging.  It walks through each of the
 * socket structures and dumps their state to the debug log.
 */
void esp8266_dumpAllSocketData() {
  for (int i=0; i<MAX_SOCKETS; i++) {
    esp8266_dumpSocketData(&socketArray[i]);
  }
}


/**
 * Write the details of a socket to the debug log.
 * The data associated with the socket is dumped to the debug log.
 */
void esp8266_dumpSocket(
    int socketId //!< The ID of the socket data structure to be logged.
  ) {
  struct socketData *pSocketData = getSocketData(socketId);
  esp8266_dumpSocketData(pSocketData);
}


/**
 * Write the details of a socketData to the debug log.
 * The data associated with the socketData is dumped to the debug log.
 */
static void esp8266_dumpSocketData(
    struct socketData *pSocketData //!< The socket data structure to be logged
  ) {
  DBG("=== socket %d", pSocketData->socketId);
  char *creationTypeMsg;
  switch(pSocketData->creationType) {
  case SOCKET_CREATED_NONE:
    creationTypeMsg = "none";
    break;
  case SOCKET_CREATED_INBOUND:
    creationTypeMsg = "inbound";
    break;
  case SOCKET_CREATED_OUTBOUND:
    creationTypeMsg = "outbound";
    break;
  case SOCKET_CREATED_SERVER:
    creationTypeMsg = "server";
    break;
  }
  DBG(" type=%s, txBuf=%p", creationTypeMsg, pSocketData->currentTx);
  char *stateMsg;
  switch(pSocketData->state) {
  case SOCKET_STATE_CLOSED:
    stateMsg = "closing";
    break;
  case SOCKET_STATE_CONNECTING:
    stateMsg = "connecting";
    break;
  case SOCKET_STATE_DISCONNECTING:
    stateMsg = "disconnecting";
    break;
  case SOCKET_STATE_ABORTING:
    stateMsg = "aborting";
    break;
  case SOCKET_STATE_IDLE:
    stateMsg = "idle";
    break;
  case SOCKET_STATE_TRANSMITTING:
    stateMsg = "transmitting";
    break;
  case SOCKET_STATE_HOST_RESOLVING:
    stateMsg = "resolving";
    break;
  case SOCKET_STATE_UNACCEPTED:
    stateMsg = "unaccepted";
    break;
  case SOCKET_STATE_UNUSED:
    stateMsg = "unused";
    break;
  default:
    stateMsg = "Unexpected state!!";
    break;
  }
  DBG(", state=%s, espconn=%p, err=%d", stateMsg, pSocketData->pEspconn, pSocketData->errorCode);
  DBG(", rx:");
  for (PktBuf *b=pSocketData->rxBufQ; b; b=b->next) {
    DBG(" %d@%p", b->filled, b);
  }
  DBG("\n");
}


/**
 * Dump a struct espconn (for debugging purposes).
 */
#if 0
static void dumpEspConn(
    struct espconn *pEspConn //!<
  ) {
  char ipString[20];
  LOG("Dump of espconn: 0x%x\n", (int)pEspConn);
  if (pEspConn == NULL) {
    return;
  }
  switch(pEspConn->type) {
  case ESPCONN_TCP:
    LOG(" - type = TCP\n");
    LOG("   - local address    = %d.%d.%d.%d [%d]\n",
        pEspConn->proto.tcp->local_ip[0],
        pEspConn->proto.tcp->local_ip[1],
        pEspConn->proto.tcp->local_ip[2],
        pEspConn->proto.tcp->local_ip[3],
        pEspConn->proto.tcp->local_port);
    LOG("   - remote address   = %d.%d.%d.%d [%d]\n",
        pEspConn->proto.tcp->remote_ip[0],
        pEspConn->proto.tcp->remote_ip[1],
        pEspConn->proto.tcp->remote_ip[2],
        pEspConn->proto.tcp->remote_ip[3],
        pEspConn->proto.tcp->remote_port);
    break;
  case ESPCONN_UDP:
    LOG(" - type = UDP\n");
    LOG("   - local_port  = %d\n", pEspConn->proto.udp->local_port);
    LOG("   - local_ip    = %d.%d.%d.%d\n",
        pEspConn->proto.udp->local_ip[0],
        pEspConn->proto.udp->local_ip[1],
        pEspConn->proto.udp->local_ip[2],
        pEspConn->proto.udp->local_ip[3]);
    LOG("   - remote_port = %d\n", pEspConn->proto.udp->remote_port);
    LOG("   - remote_ip   = %d.%d.%d.%d\n",
        pEspConn->proto.udp->remote_ip[0],
        pEspConn->proto.udp->remote_ip[1],
        pEspConn->proto.udp->remote_ip[2],
        pEspConn->proto.udp->remote_ip[3]);
    break;
  default:
    LOG(" - type = Unknown!! 0x%x\n", pEspConn->type);
  }
  switch(pEspConn->state) {
  case ESPCONN_NONE:
    LOG(" - state=NONE");
    break;
  case ESPCONN_WAIT:
    LOG(" - state=WAIT");
    break;
  case ESPCONN_LISTEN:
    LOG(" - state=LISTEN");
    break;
  case ESPCONN_CONNECT:
    LOG(" - state=CONNECT");
    break;
  case ESPCONN_WRITE:
    LOG(" - state=WRITE");
    break;
  case ESPCONN_READ:
    LOG(" - state=READ");
    break;
  case ESPCONN_CLOSE:
    LOG(" - state=CLOSE");
    break;
  default:
    LOG(" - state=unknown!!");
    break;
  }
  LOG(", link_cnt=%d", pEspConn->link_cnt);
  LOG(", reverse=0x%x\n", (unsigned int)pEspConn->reverse);
}
#endif


/**
 * Get the next new global socket id.
 * \return A new socketId that is assured to be unique.
 */
static int getNextGlobalSocketId() {
  return ++g_nextSocketId;
}


/**
 * Allocate a new socket
 * Look for the first free socket in the array of sockets and return the first one
 * that is available.  The socketId property is set to a unique and new socketId value
 * that will not previously have been seen.
 * \return The socketData structure for the returned socket.
 */
static struct socketData *allocateNewSocket() {
  // Walk through each of the sockets in the array of possible sockets and stop
  // at the first one that is flagged as not in use.  For that socket, set its
  // socketId to the next global socketId value.
  for (int i=0; i<MAX_SOCKETS; i++) {
    if (socketArray[i].state == SOCKET_STATE_UNUSED) {
      socketArray[i].socketId = getNextGlobalSocketId();
      return &socketArray[i];
    }
  }
  esp8266_dumpAllSocketData();
  return(NULL);
}


/**
 * Retrieve the socketData for the given socket index.
 * \return The socket data for the given socket or NULL if there is no matching socket.
 */
static struct socketData *getSocketData(int socketId) {
  struct socketData *pSocketData = socketArray;
  for (int socketArrayIndex=0; socketArrayIndex<MAX_SOCKETS; socketArrayIndex++) {
    if (pSocketData->socketId == socketId) {
      return pSocketData;
    }
    pSocketData++;
  }
  DBG("%s: socket %d not found\n", DBG_LIB, socketId);
  return NULL;
}


/**
 * Find the server socket that is bound to the given local port.
 * \return The socket id of the socket listening on the given port or -1 if there is no
 * server socket that matches.
 */
static int getServerSocketByLocalPort(
    unsigned short port //!< The port number on which a server socket is listening.
  ) {
  // Loop through each of the sockets in the socket array looking for a socket
  // that is inuse, a server and has a local_port of the passed in port number.
  int socketArrayIndex;
  struct socketData *pSocketData = socketArray;
  for (socketArrayIndex=0; socketArrayIndex<MAX_SOCKETS; socketArrayIndex++) {
    if (pSocketData->state  != SOCKET_STATE_UNUSED &&
      pSocketData->creationType == SOCKET_CREATED_SERVER &&
      pSocketData->pEspconn->proto.tcp->local_port == port)
    {
      return pSocketData->socketId;
    }
    pSocketData++;
  } // End of for each socket
  return -1;
}

/**
 * Release the socket and return it to the free pool.
 * The connection (espconn) must be closed and deallocated before calling releaseSocket.
 */
static void releaseSocket(
    struct socketData *pSocketData //!< The socket to release
  ) {
  assert(pSocketData != NULL);
  //DBG("%s: freeing socket %d\n", DBG_LIB, pSocketData->socketId);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);
  assert(pSocketData->pEspconn == NULL);

  // free any unconsumed receive buffers
  while (pSocketData->rxBufQ != NULL)
    pSocketData->rxBufQ = PktBuf_ShiftFree(pSocketData->rxBufQ);

  if (pSocketData->currentTx != NULL) {
    //DBG("%s: freeing tx buf %p\n", DBG_LIB, pSocketData->currentTx);
    os_free(pSocketData->currentTx);
    pSocketData->currentTx = NULL;
  }

  os_memset(pSocketData, 0, sizeof(struct socketData));
}

/**
 * Release the espconn structure
 */
static void releaseEspconn(
    struct socketData *pSocketData
) {
  if (pSocketData->pEspconn == NULL) return;
  // if the socket is an inbound connection then espconn will free the struct, else we do it now
  if (pSocketData->creationType != SOCKET_CREATED_INBOUND) {
    //DBG("%s: freeing espconn %p/%p for socket %d\n", DBG_LIB,
    //    pSocketData->pEspconn, pSocketData->pEspconn->proto.tcp, pSocketData->socketId);
    os_free(pSocketData->pEspconn->proto.tcp);
    pSocketData->pEspconn->proto.tcp = NULL;
    os_free(pSocketData->pEspconn);
  }
  pSocketData->pEspconn = NULL;
}


/**
 * Initialize the entire socket array
 */
void netInit_esp8266_board() {
  if (g_socketsInitialized) return;
  g_socketsInitialized = true;
  os_memset(socketArray, 0, sizeof(socketArray));
}


/**
 * Perform an actual closure of the socket by calling the ESP8266 disconnect API.
 */
static void doClose(
    struct socketData *pSocketData //!< The socket to be closed.
) {
  if (pSocketData == NULL) return; // just in case

  // if we're already closing, then don't do anything
  if (pSocketData->state == SOCKET_STATE_CLOSED ||
      pSocketData->state == SOCKET_STATE_DISCONNECTING)
  {
    return;
  }

  // if we're in the name resolution phase, we don't have much to do
  // if we're in aborting ditto
  if (pSocketData->state == SOCKET_STATE_HOST_RESOLVING ||
      pSocketData->state == SOCKET_STATE_ABORTING)
  {
    pSocketData->state = SOCKET_STATE_DISCONNECTING;
    return;
  }

  // if we need to abort, then do that
  if (pSocketData->state == SOCKET_STATE_TO_ABORT) {
    espconn_abort(pSocketData->pEspconn);
    pSocketData->state = SOCKET_STATE_DISCONNECTING;
    return;
  }

  // Tell espconn to disconnect/delete the connection
  if (pSocketData->creationType == SOCKET_CREATED_SERVER) {
    //dumpEspConn(pSocketData->pEspconn);
    int rc = espconn_delete(pSocketData->pEspconn);
    if (rc != 0) {
      setSocketInError(pSocketData, rc);
    }
    // we do not get a disconnected callback so we go straight to SOCKET_STATE_UNUSED
    pSocketData->state = SOCKET_STATE_UNUSED;
    pSocketData->creationType = SOCKET_CREATED_NONE;

  } else {
    int rc = espconn_disconnect(pSocketData->pEspconn);
    if (rc == 0) {
      pSocketData->state = SOCKET_STATE_DISCONNECTING;
    } else {
      setSocketInError(pSocketData, rc);
      pSocketData->state = SOCKET_STATE_UNUSED; // don't expect a callback
      pSocketData->creationType = SOCKET_CREATED_NONE;
    }
  }
}


/**
 * Set the given socket as being in error supplying the espconn code.
 * This translates the espconn code to an Espruino socket error code.
 */
static void setSocketInError(
    struct socketData *pSocketData, //!< The socket that is being flagged as in error.
    int code                        //!< The espconn error code
  ) {
  assert(pSocketData != NULL);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);

  if (pSocketData->errorCode != 0) return; // don't overwrite previous error
  int err = code;
  switch (code) {
  case ESPCONN_MEM:       err = SOCKET_ERR_MEM; break;
  case ESPCONN_ABRT:      err = SOCKET_ERR_RESET; break;
  case ESPCONN_CLSD:      err = SOCKET_ERR_CLOSED; break;
  case ESPCONN_IF:        err = SOCKET_ERR_UNKNOWN; break;
  case ESPCONN_ISCONN:    err = SOCKET_ERR_BUSY; break;
  case ESPCONN_HANDSHAKE: err = SOCKET_ERR_SSL_HAND; break;
  case ESPCONN_SSL_INVALID_DATA: err = SOCKET_ERR_SSL_INVALID; break;
  }
  pSocketData->errorCode = err;
  DBG("%s: error %d->%d on socket %d: %s\n", DBG_LIB,
      code, err, pSocketData->socketId, socketErrorString(err));
}

/**
 * Callback function registered to the ESP8266 environment that is
 * invoked when a new inbound connection has been formed.
 */
static void esp8266_callback_connectCB_inbound(
    void *arg //!<
) {
  struct espconn *pEspconn = (struct espconn *)arg;
  assert(pEspconn != NULL);

  struct socketData *pClientSocketData = allocateNewSocket();
  if (pClientSocketData == NULL) {
    DBG("%s: out of sockets, dropping inbound connection\n", DBG_LIB);
    espconn_disconnect(pEspconn);
    return;
  }
  DBG("%s: accepted socket %d inbound to port %d from %d.%d.%d.%d:%d\n", DBG_LIB,
      pClientSocketData->socketId, pEspconn->proto.tcp->local_port,
      IP2STR(pEspconn->proto.tcp->remote_ip), pEspconn->proto.tcp->remote_port);
  //dumpEspConn(pEspconn);

  // register callbacks on the new connection
  if (pEspconn->type == ESPCONN_TCP) {
    espconn_regist_disconcb(pEspconn, esp8266_callback_disconnectCB);
    espconn_regist_reconcb(pEspconn, esp8266_callback_reconnectCB);
  }
  espconn_regist_sentcb(pEspconn, esp8266_callback_sentCB);
  espconn_regist_recvcb(pEspconn, esp8266_callback_recvCB);

  pClientSocketData->pEspconn          = pEspconn;
  pClientSocketData->pEspconn->reverse = pClientSocketData;
  pClientSocketData->creationType      = SOCKET_CREATED_INBOUND;
  pClientSocketData->state             = SOCKET_STATE_UNACCEPTED;
}

/**
 * Callback function registered to the ESP8266 environment that is
 * invoked when a new outbound connection has been formed.
 */
static void esp8266_callback_connectCB_outbound(
    void *arg //!< A pointer to a `struct espconn`.
) {
  struct espconn *pEspconn = (struct espconn *)arg;
  assert(pEspconn != NULL);
  struct socketData *pSocketData = (struct socketData *)pEspconn->reverse;
  if (pSocketData == NULL) return; // stray callback (possibly after a disconnect)
  DBG("%s: socket %d connected\n", DBG_LIB, pSocketData->socketId);

  // if we're connecting, then move on, else ignore (could be that we're disconnecting)
  if (pSocketData->state == SOCKET_STATE_CONNECTING) {
    pSocketData->state = SOCKET_STATE_IDLE;
  }
}


/**
 * Callback function registered to the ESP8266 environment that is
 * Invoked when a connection has been disconnected. This does get invoked if we
 * initiated the disconnect (new since SDK 1.5?).
 */
static void esp8266_callback_disconnectCB(
    void *arg //!< A pointer to a `struct espconn`.
) {
  struct espconn *pEspconn = (struct espconn *)arg;
  struct socketData *pSocketData = (struct socketData *)pEspconn->reverse;
  if (pSocketData == NULL) return;
  //if (pEspconn != pSocketData->pEspconn) DBG("%s: pEspconn changed in disconnectCB ***\n", DBG_LIB);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);
  DBG("%s: socket %d disconnected\n", DBG_LIB, pSocketData->socketId);

  // we can deallocate the espconn structure
  releaseEspconn(pSocketData);

  // if we were in SOCKET_STATE_DISCONNECTING the socket lib is already done with this socket,
  // so we can free the whole thing. Otherwise, we transition to SOCKET_STATE_CLOSED because
  // we will need to tell the socket lib about the disconnect.
  if (pSocketData->state == SOCKET_STATE_DISCONNECTING) {
    releaseSocket(pSocketData);
  } else {
    // we can deallocate the tx buffer
    if (pSocketData->currentTx != NULL) {
      //DBG("%s: freeing tx buf %p\n", DBG_LIB, pSocketData->currentTx);
      os_free(pSocketData->currentTx);
      pSocketData->currentTx = NULL;
    }

    pSocketData->state = SOCKET_STATE_CLOSED;
  }
}


/**
 * Error handler callback.
 * Although this is called `reconnect` by Espressif, this is really a connection reset callback.
 */
static void esp8266_callback_reconnectCB(
    void *arg, //!< A pointer to a `struct espconn`.
    sint8 err  //!< The error code.
) {
  struct espconn *pEspconn = (struct espconn *)arg;
  struct socketData *pSocketData = (struct socketData *)pEspconn->reverse;
  if (pSocketData == NULL) return; // we already closed this.
  //if (pEspconn != pSocketData->pEspconn) DBG("%s: pEspconn changed in reconnectCB ***\n", DBG_LIB);
  DBG("%s: socket %d connection reset: Err %d - %s\n", DBG_LIB,
      pSocketData->socketId, err, esp8266_errorToString(err));
  // Do the same as for a disconnect
  esp8266_callback_disconnectCB(arg);
  // Set the socket state as in error (unless it got freed by esp8266_callback_disconnectCB)
  if (pSocketData->state != SOCKET_STATE_UNUSED)
    setSocketInError(pSocketData, err);
  //DBG("%s: ret from reconnectCB\n", DBG_LIB);
}


/**
 * Callback function registered to the ESP8266 environment that is
 * invoked when a send operation has been completed. This signals that we can reuse the tx buffer
 * and that we can send the next chunk of data.
 */
static void esp8266_callback_sentCB(
    void *arg //!< A pointer to a `struct espconn`.
) {
  struct espconn *pEspconn = (struct espconn *)arg;
  struct socketData *pSocketData = (struct socketData *)pEspconn->reverse;
  if (pSocketData == NULL) return; // we already closed this.
  //if (pEspconn != pSocketData->pEspconn) DBG("%s: pEspconn changed in sentCB ***\n", DBG_LIB);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);

  //DBG("%s: socket %d send completed\n", DBG_LIB, pSocketData->socketId);

  // We have transmitted the data ... which means that the data that was in the transmission
  // buffer can be released.
  if (pSocketData->currentTx != NULL) {
    os_free(pSocketData->currentTx);
    pSocketData->currentTx = NULL;
  }

  if (pSocketData->state == SOCKET_STATE_TRANSMITTING) {
    pSocketData->state = SOCKET_STATE_IDLE;
  }
}


/**
 * ESP8266 callback function that is invoked when new data has arrived over
 * the TCP/IP connection.
 */
static void esp8266_callback_recvCB(
    void *arg,         //!< A pointer to a `struct espconn`.
    char *pData,       //!< A pointer to data received over the socket.
    unsigned short len //!< The length of the data.
) {
  struct espconn *pEspconn = (struct espconn *)arg;
  struct socketData *pSocketData = (struct socketData *)pEspconn->reverse;
  if (pSocketData == NULL) return; // we closed this socket
  //if (pEspconn != pSocketData->pEspconn) DBG("%s: pEspconn changed in recvCB ***\n", DBG_LIB);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);

  //DBG("%s: socket %d recv %d\n", DBG_LIB, pSocketData->socketId, len);
  //DBG("%s: recv data: %p\n", DBG_LIB, pData);

  // if this is a dead connection then just ignore the callback
  if (pSocketData->state == SOCKET_STATE_ABORTING ||
      pSocketData->state == SOCKET_STATE_TO_ABORT ||
      pSocketData->state == SOCKET_STATE_CLOSED ||
      pSocketData->state == SOCKET_STATE_DISCONNECTING) {
    return;
  }

  bool isUDP = pEspconn->type == ESPCONN_UDP;
  if (isUDP && len > 536/2 /*chunkSize*/) {
    jsWarn("Recv %d too long (UDP)!\n", len);
    return;
  }

  // Allocate a buffer and add to the receive queue
  PktBuf *buf = PktBuf_New(len);
  if (!buf) {
    // handle out of memory condition
    DBG("%s: Out of memory allocating %d for recv\n", DBG_LIB, len);
    // at this point we're gonna deallocate all receive buffers as a panic measure
    while (pSocketData->rxBufQ != NULL)
      pSocketData->rxBufQ = PktBuf_ShiftFree(pSocketData->rxBufQ);
    // save the error
    setSocketInError(pSocketData, ESPCONN_MEM);
    // now reset the connection
    //espconn_abort(pEspconn); // can't do this: espconn crashes!
    pSocketData->state = SOCKET_STATE_TO_ABORT; // some function called from socket lib will abort
    //DBG("%s: ret from recvCB\n", DBG_LIB);
    return;
  }

  if (isUDP) {
    // UDP remote host/port
    remot_info *premot = NULL;
    if (espconn_get_connection_info(pEspconn, &premot, 0) == ESPCONN_OK) {
      pSocketData->host = *(uint32_t *)&premot->remote_ip;
      pSocketData->port = *(unsigned short *)&premot->remote_port;
    }
  }

  DBG("%s: Recv %d to %x:%d\n", DBG_LIB, len, pSocketData->host, pSocketData->port);

  // if this is the second buffer then stop the flood!
  if (pSocketData->rxBufQ != NULL) espconn_recv_hold(pEspconn);
  // got buffer, fill it
  os_memcpy(buf->data, pData, len);
  buf->filled = len;
  pSocketData->rxBufQ = PktBuf_Push(pSocketData->rxBufQ, buf);
}


// -------------------------------------------------

/**
 * Define the implementation functions for the logical network functions.
 */
void netSetCallbacks_esp8266_board(
    JsNetwork *net //!< The Network we are going to use.
  ) {
    net->idle          = net_ESP8266_BOARD_idle;
    net->checkError    = net_ESP8266_BOARD_checkError;
    net->createsocket  = net_ESP8266_BOARD_createSocket;
    net->closesocket   = net_ESP8266_BOARD_closeSocket;
    net->accept        = net_ESP8266_BOARD_accept;
    net->gethostbyname = net_ESP8266_BOARD_gethostbyname;
    net->recv          = net_ESP8266_BOARD_recv;
    net->send          = net_ESP8266_BOARD_send;
    // The TCP MSS is 536, we use half that 'cause otherwise we easily run out of JSvars memory
    net->chunkSize     = 536/2;
}

/**
 * Determine if there is a new client connection on the server socket.
 * This function is called to poll to see if the serverSckt has a new
 * accepted connection (socket) and, if it does, return it else return -1 to indicate
 * that there was no new accepted socket.
 */
int net_ESP8266_BOARD_accept(
    JsNetwork *net, //!< The Network we are going to use to create the socket.
    int serverSckt  //!< The socket that we are checking to see if there is a new client connection.
) {
  struct socketData *pSocketData = getSocketData(serverSckt);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);
  assert(pSocketData->creationType == SOCKET_CREATED_SERVER);

  // iterate through all sockets and see whether there is one in the UNACCEPTED state that is for
  // the server socket's local port.
  uint16_t serverPort = pSocketData->pEspconn->proto.tcp->local_port;
  for (uint8_t i=0; i<MAX_SOCKETS; i++) {
    if (socketArray[i].state == SOCKET_STATE_UNACCEPTED &&
        socketArray[i].pEspconn != NULL &&
        socketArray[i].pEspconn->proto.tcp->local_port == serverPort)
    {
      DBG("%s: Accepted socket %d\n", DBG_LIB, socketArray[i].socketId);
      socketArray[i].state = SOCKET_STATE_IDLE;
      return socketArray[i].socketId;
    }
  }

  return -1;
}


/**
 * Receive data from the network device.
 * Returns the number of bytes received which may be 0 and <0 if there was an error.
 */
int net_ESP8266_BOARD_recv(
    JsNetwork *net, //!< The Network we are going to use to create the socket.
    SocketType socketType, //!< The socket type bitmask
    int sckt,       //!< The socket from which we are to receive data.
    void *buf,      //!< The storage buffer into which we will receive data.
    size_t len      //!< The length of the buffer.
) {
  //DBG("%s:recv\n", DBG_LIB);
  struct socketData *pSocketData = getSocketData(sckt);
  assert(pSocketData);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);

  // handle socket that needs aborting
  if (pSocketData->state == SOCKET_STATE_TO_ABORT) {
    espconn_abort(pSocketData->pEspconn);
    return pSocketData->errorCode;
  }

  // If there is no data in the receive buffer, then all we need do is return
  // 0 bytes as the length of data moved or -1 if the socket is actually closed.
  if (pSocketData->rxBufQ == NULL) {
    switch (pSocketData->state) {
    case SOCKET_STATE_CLOSED:
      return pSocketData->errorCode != 0 ? pSocketData->errorCode : SOCKET_ERR_CLOSED;
    case SOCKET_STATE_DISCONNECTING:
    case SOCKET_STATE_ABORTING:
      return pSocketData->errorCode;
    case SOCKET_STATE_HOST_RESOLVING:
    case SOCKET_STATE_CONNECTING:
      return SOCKET_ERR_NO_CONN;
    default:
      return 0; // we just have no data
    }
  }
  PktBuf *rxBuf = pSocketData->rxBufQ;

  size_t delta = 0;
  if (socketType & ST_UDP) {
    // TODO: Use JsNetUDPPacketHeader here to tidy this up
    delta = sizeof(uint32_t) + sizeof(unsigned short) + sizeof(uint16_t);
    uint32_t *host = (uint32_t*)buf;
    unsigned short *port = (unsigned short*)&host[1];
    uint16_t *size = (uint16_t*)&port[1];
    buf += delta;
    len -= delta;
    // UDP remote host/port
    *host = pSocketData->host;
    *port = pSocketData->port;
    *size = rxBuf->filled;
  }

  // If the receive buffer is able to completely fit in the buffer
  // passed into us then we can copy all the data and the receive buffer will be clear.
  if (rxBuf->filled <= len) {
    os_memcpy(buf, rxBuf->data, rxBuf->filled);
    uint16_t retLen = rxBuf->filled;
    pSocketData->rxBufQ = PktBuf_ShiftFree(rxBuf);
    // if we now have exactly one buffer enqueued we need to re-enable the flood
    if (pSocketData->rxBufQ != NULL && pSocketData->rxBufQ->next == NULL)
      espconn_recv_unhold(pSocketData->pEspconn);
    //DBG("%s: socket %d JS recv %d\n", DBG_LIB, sckt, retLen);
    return retLen + delta;
  }

  // If we are here, then we have more data in the receive buffer than is available
  // to be returned in this request for data.  So we have to copy the amount of data
  // that is allowed to be returned and then strip that from the beginning of the
  // receive buffer.

  // First we copy the data we are going to return.
  os_memcpy(buf, rxBuf->data, len);
  // Next we shift up the remaining data
  uint16_t newLen = rxBuf->filled - len;
  os_memmove(rxBuf->data, rxBuf->data + len, newLen);
  rxBuf->filled = newLen;
  //DBG("%s: socket %d JS recv %d\n", DBG_LIB, sckt, len);
  return len + delta;
}


/**
 * Send data to the partner.
 * The return is the number of bytes actually transmitted which may also be
 * 0 to indicate no bytes sent or -1 to indicate an error.  For the ESP8266 implementation we
 * will return 0 if the socket is not connected or we are in the `SOCKET_STATE_TRANSMITTING`
 * state.
 */
int net_ESP8266_BOARD_send(
    JsNetwork *net,  //!< The Network we are going to use to create the socket.
    SocketType socketType, //!< The socket type bitmask
    int sckt,        //!< The socket over which we will send data.
    const void *buf, //!< The buffer containing the data to be sent.
    size_t len       //!< The length of data in the buffer to send.
) {
  //DBG("%s:send\n", DBG_LIB);
  struct socketData *pSocketData = getSocketData(sckt);
  assert(pSocketData->state != SOCKET_STATE_UNUSED);

  DBG("%s:send state:%d err:%d\n", DBG_LIB, pSocketData->state, pSocketData->errorCode);

  // If the socket is in error or it is closing return -1
  switch (pSocketData->state) {
  case SOCKET_STATE_CLOSED:
  case SOCKET_STATE_DISCONNECTING:
    return pSocketData->errorCode != 0 ? pSocketData->errorCode : SOCKET_ERR_CLOSED;
  case SOCKET_STATE_ABORTING:
    return pSocketData->errorCode;
  case SOCKET_STATE_TO_ABORT:
    espconn_abort(pSocketData->pEspconn);
    return pSocketData->errorCode;
  default:
    break;
  }

  // Unless we are in the idle state, we can't send more shtuff
  if (pSocketData->state != SOCKET_STATE_IDLE) {
    return 0;
  }

  // Log the content of the data we are sending.
  //esp8266_board_writeString(buf, len);
  //os_printf("\n");
  size_t delta = 0;
  if (socketType & ST_UDP) {
    // TODO: Use JsNetUDPPacketHeader here to tidy this up
    delta = sizeof(uint32_t) + sizeof(unsigned short) + sizeof(uint16_t);
    uint32_t *host = (uint32_t*)buf;
    unsigned short *port = (unsigned short*)&host[1];
    uint16_t *size = (uint16_t*)&port[1];

    // UDP remote IP/port need to be set everytime we call espconn_send
    *(uint32_t *)&pSocketData->pEspconn->proto.tcp->remote_ip = *host;
    pSocketData->pEspconn->proto.tcp->remote_port = *port;

    buf += delta;
    len = *size;
    DBG("%s: Sendto %d to %x:%d\n", DBG_LIB, len, *host, *port);
  }

  // Copy the data to be sent into a transmit buffer we hand off to espconn
  assert(pSocketData->currentTx == NULL);
  pSocketData->currentTx = (uint8_t *)os_malloc(len);
  if (pSocketData->currentTx == NULL) {
    DBG("%s: Out of memory sending %d on socket %d\n", DBG_LIB, len, sckt);
    setSocketInError(pSocketData, ESPCONN_MEM);
    espconn_abort(pSocketData->pEspconn);
    pSocketData->state = SOCKET_STATE_ABORTING;
    return pSocketData->errorCode;
  }
  memcpy(pSocketData->currentTx, buf, len);

  // Set transmitting now as the sentCB is called synchronously from inside
  // espconn_send for UDP
  pSocketData->state = SOCKET_STATE_TRANSMITTING;

  // Send the data over the ESP8266 SDK.
  int rc = espconn_send(pSocketData->pEspconn, pSocketData->currentTx, len);
  if (rc < 0) {
    setSocketInError(pSocketData, rc);
    os_free(pSocketData->currentTx);
    pSocketData->currentTx = NULL;
    espconn_abort(pSocketData->pEspconn);
    pSocketData->state = SOCKET_STATE_ABORTING;
    return rc;
  }

  //DBG("%s: socket %d JS send %d\n", DBG_LIB, sckt, len);
  return len + delta;
}


/**
 * Perform idle processing.
 * There is the possibility that we may wish to perform logic when we are idle.  For the
 * ESP8266 there is no specific idle network processing needed.
 */
void net_ESP8266_BOARD_idle(
    JsNetwork *net //!< The Network we are part of.
  ) {
  // Don't echo here because it is called continuously
  //os_printf("> net_ESP8266_BOARD_idle\n");
}


/**
 * Check for errors.
 * Returns true if there are NO errors.
 */
bool net_ESP8266_BOARD_checkError(
    JsNetwork *net //!< The Network we are checking.
  ) {
  //os_printf("> net_ESP8266_BOARD_checkError\n");
  return true;
}

/* Static variable hack to support async DNS resolutions. This is not great, but it works.
 * There is only one call to net_ESP8266_BOARD_gethostbyname and it is immediately followed
 * by a call to net_ESP8266_BOARD_createSocket, so we save the hostname from the first call
 * in a global variable and then use it in the second to actually kick off the name resolution.
 */
static char *savedHostname = 0;

/**
 * Get an IP address from a name. See the hack description above. This always returns -1
 */
void net_ESP8266_BOARD_gethostbyname(
    JsNetwork *net, //!< The Network we are going to use to create the socket.
    char *hostname, //!< The string representing the hostname we wish to lookup.
    uint32_t *outIp //!< The address into which the resolved IP address will be stored.
  ) {
  assert(hostname != NULL);
  savedHostname = hostname;
  *outIp = -1;
}

/**
 * Callback handler for espconn_gethostbyname.
 */
static void dnsFoundCallback(const char *hostName, ip_addr_t *ipAddr, void *arg) {
  assert(arg != NULL); // arg points to the espconn struct where the resolved IP address needs to go
  struct espconn *pEspconn = arg;
  struct socketData *pSocketData = pEspconn->reverse;

  if (pSocketData->state == SOCKET_STATE_DISCONNECTING) {
    // the sockte library closed the socket while we were resolving, we now need to deallocate
    releaseEspconn(pSocketData);
    releaseSocket(pSocketData);
    return;
  }
  if (pSocketData->state != SOCKET_STATE_HOST_RESOLVING) return; // not sure what happened

  // ipAddr will be NULL if the IP address can not be resolved.
  if (ipAddr != NULL) {
    *(uint32_t *)&pEspconn->proto.tcp->remote_ip = ipAddr->addr;
    if (pSocketData != NULL) connectSocket(pSocketData);
  } else {
    releaseEspconn(pSocketData);
    if (pSocketData != NULL) {
      setSocketInError(pSocketData, SOCKET_ERR_NOT_FOUND);
      pSocketData->state = SOCKET_STATE_CLOSED;
    }
  }
}


/**
 * Create a new socket.
 * if `ipAddress == 0`, creates a server otherwise creates a client (and automatically connects).
 * Returns >=0 on success.
 */
int net_ESP8266_BOARD_createSocket(
    JsNetwork *net,     //!< The Network we are going to use to create the socket.
    SocketType socketType, //!< The socket type bitmask
    uint32_t ipAddress, //!< The address of the partner of the socket or 0 if we are to be a server.
    unsigned short port,//!< The port number that the partner is listening upon.
    JsVar *options
) {
  // allocate a socket data structure
  struct socketData *pSocketData = allocateNewSocket();
  if (pSocketData == NULL) { // No free socket
    DBG("%s: No free sockets for outbound connection\n", DBG_LIB);
    return SOCKET_ERR_MAX_SOCK;
  }

  // allocate espconn data structure and initialize it
  struct espconn *pEspconn = os_zalloc(sizeof(struct espconn));

  esp_tcp *tcp = os_zalloc(sizeof(esp_tcp));
  if (pEspconn == NULL || tcp == NULL) {
    DBG("%s: Out of memory for outbound connection\n", DBG_LIB);
    if (pEspconn != NULL) os_free(pEspconn);
    if (tcp != NULL) os_free(tcp);
    releaseSocket(pSocketData);
    return SOCKET_ERR_MEM;
  }

  if (socketType & ST_UDP) {
    pEspconn->type    = ESPCONN_UDP;

    // esp_tcp and esp_udp start identically (up to remote_ip)
    // so we can leave the proto.tcp as an alias to proto.udp
  } else {
    pEspconn->type    = ESPCONN_TCP;

    espconn_set_opt(pEspconn, ESPCONN_NODELAY); // disable nagle, don't need the extra delay
  }

  pSocketData->pEspconn = pEspconn;
  pEspconn->state     = ESPCONN_NONE;
  pEspconn->proto.tcp = tcp;
  tcp->remote_port    = port;
  tcp->local_port     = espconn_port(); // using 0 doesn't work
  pEspconn->reverse   = pSocketData;

  // multicast support
  // FIXME: perhaps extend the JsNetwork with addmembership/removemembership instead of using options
  JsVar *mgrpVar = jsvObjectGetChild(options, "multicastGroup", 0);
  if (mgrpVar) {
      char ipStr[18];

      jsvGetString(mgrpVar, ipStr, sizeof(ipStr));
      jsvUnLock(mgrpVar);
      uint32_t grpip = networkParseIPAddress(ipStr);

      JsVar *ipVar = jsvObjectGetChild(options, "multicastIp", 0);
      jsvGetString(ipVar, ipStr, sizeof(ipStr));
      jsvUnLock(ipVar);
      uint32_t ip = networkParseIPAddress(ipStr);

      pSocketData->multicastGrpIp = grpip;
      pSocketData->multicastIp = ip;
  }

  if (ipAddress == (uint32_t)-1) {
    // We need DNS resolution, kick it off
    int rc = espconn_gethostbyname(pEspconn, savedHostname,
        (void*)&pEspconn->proto.tcp->remote_ip, dnsFoundCallback);
    if (rc < 0) {
    }
    DBG("%s: resolving %s\n", DBG_LIB, savedHostname);
    pSocketData->state = SOCKET_STATE_HOST_RESOLVING;
    return pSocketData->socketId;
  } else {
    // No DNS resolution needed, go right ahead
    *(uint32_t *)&pEspconn->proto.tcp->remote_ip = ipAddress;
    return connectSocket(pSocketData);
  }
}

/**
 * Continue creating a socket, the name resolution having completed
 */
static int connectSocket(
    struct socketData *pSocketData //!< Allocated socket data structure
) {
  struct espconn *pEspconn = pSocketData->pEspconn;
  bool isServer = *(uint32_t *)&pEspconn->proto.tcp->remote_ip == 0;
  int rc;

  int newSocket = pSocketData->socketId;
  assert(pSocketData->rxBufQ == NULL);
  assert(pSocketData->currentTx == NULL);

  espconn_regist_sentcb(pEspconn, esp8266_callback_sentCB);
  espconn_regist_recvcb(pEspconn, esp8266_callback_recvCB);

  // If we are a client
  if (!isServer) {
    pSocketData->state = SOCKET_STATE_CONNECTING;
    pSocketData->creationType = SOCKET_CREATED_OUTBOUND;

    if (pEspconn->type == ESPCONN_TCP) {
      espconn_regist_connectcb(pEspconn, esp8266_callback_connectCB_outbound);
      espconn_regist_disconcb(pEspconn, esp8266_callback_disconnectCB);
      espconn_regist_reconcb(pEspconn, esp8266_callback_reconnectCB);

      // Make a call to espconn_connect.
#if 0
      DBG("%s: connecting socket %d/%p/%p to %d.%d.%d.%d:%d from :%d\n",
          DBG_LIB, pSocketData->socketId, pSocketData, pEspconn,
          IP2STR(pEspconn->proto.tcp->remote_ip), pEspconn->proto.tcp->remote_port,
          pEspconn->proto.tcp->local_port);
#endif
      rc = espconn_connect(pEspconn);
    } else {
      rc = espconn_create(pEspconn);
    }
    if (rc != 0) {
      DBG("%s: error %d connecting socket %d: %s\n", DBG_LIB,
          rc, pSocketData->socketId, esp8266_errorToString(rc));
      releaseEspconn(pSocketData);
      releaseSocket(pSocketData);
      return rc;
    }
    DBG("%s: connecting socket %d to %d.%d.%d.%d:%d\n", DBG_LIB, pSocketData->socketId,
        IP2STR(pEspconn->proto.tcp->remote_ip), pEspconn->proto.tcp->remote_port);
  }

  // If the ipAddress IS 0 ... then we are a server.
  else {
    // We are going to set ourselves up as a server
    pSocketData->state        = SOCKET_STATE_IDLE;
    pSocketData->creationType = SOCKET_CREATED_SERVER;
    pEspconn->proto.tcp->local_port = pEspconn->proto.tcp->remote_port;
    pEspconn->proto.tcp->remote_port = 0;

    if (pEspconn->type == ESPCONN_TCP) {
      espconn_regist_connectcb(pEspconn, esp8266_callback_connectCB_inbound);

      // Make a call to espconn_accept (this should really be called espconn_listen, sigh)
      rc = espconn_accept(pEspconn);
      if (rc != 0) {
        DBG("%s: error %d creating listening socket %d: %s\n", DBG_LIB,
            rc, pSocketData->socketId, esp8266_errorToString(rc));
        releaseEspconn(pSocketData);
        releaseSocket(pSocketData);
        return rc;
      }
      espconn_regist_time(pEspconn, 600, 0);
    } else {
      rc = espconn_create(pEspconn);
      if (rc != 0) {
        DBG("%s: error %d creating listening socket %d: %s\n", DBG_LIB,
            rc, pSocketData->socketId, esp8266_errorToString(rc));
        releaseEspconn(pSocketData);
        releaseSocket(pSocketData);
        return rc;
      }
    }

    if (pSocketData->multicastGrpIp) {
      // multicast support
      espconn_igmp_join((ip_addr_t *)&pSocketData->multicastIp,
                        (ip_addr_t *)&pSocketData->multicastGrpIp);
    }

    DBG("%s: listening socket %d on port %d\n", DBG_LIB,
        pSocketData->socketId, pEspconn->proto.tcp->local_port);
  }

  return newSocket;
}

/**
 * Close a socket.
 * This gets called in two situations: when the user requests the close of a socket and as
 * an acknowledgment after we signal the socket library that a connection has closed by
 * returning <0 to a send or recv call.
 */
void net_ESP8266_BOARD_closeSocket(
    JsNetwork *net, //!< The Network we are going to use to create the socket.
    int socketId    //!< The socket to be closed.
) {
  struct socketData *pSocketData = getSocketData(socketId);
  assert(pSocketData != NULL); // We had better have found a socket to be closed.
  assert(pSocketData->state != SOCKET_STATE_UNUSED);  // Shouldn't be closing an unused socket.

  if (pSocketData->state == SOCKET_STATE_CLOSED) {
    // In these states we have already freed the espconn structures, so all that's left is to
    // free the socket structure
    //DBG("%s: socket %d close acknowledged\n", DBG_LIB, pSocketData->socketId);
    releaseSocket(pSocketData);
  } else {
    // Looks like this is the user telling us to close a connection, let's do it.
    DBG("%s: socket %d to be closed\n", DBG_LIB, pSocketData->socketId);
    doClose(pSocketData);
  }
}
