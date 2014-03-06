/*
 * network.c
 *
 *  Created on: 21 Jan 2014
 *      Author: gw
 */
#include "network.h"
#include "jsparse.h"


#include <string.h> // for memset

#define INVALID_SOCKET ((SOCKET)(-1))
#define SOCKET_ERROR (-1)

#if defined(USE_CC3000) // ------------------------------------------------
 #include "spi.h"
 typedef int SOCKET;
 #include "socket.h"
 #include "board_spi.h"
 #include "cc3000_common.h"
 #include "jswrap_cc3000.h"

 #define MSG_NOSIGNAL 0x4000 /* don't raise SIGPIPE */ // IGNORED ANYWAY!
#elif defined(USE_WIZNET) // ------------------------------------------------
 #include "Ethernet/socket.h"
  typedef struct sockaddr_in sockaddr_in;
 #define closesocket(SOCK) close(SOCK)
 #define MSG_NOSIGNAL 0x4000 /* don't raise SIGPIPE */ // IGNORED ANYWAY!
 #define send(sock,ptr,len,flags) send(sock,(uint8_t*)(ptr),len) // throw away last arg of send
 #define recv(sock,ptr,len,flags) recv(sock,(uint8_t*)(ptr),len) // throw away last arg of send
 // name resolution
 #include "DNS/dns.h"
 extern uint8_t Server_IP_Addr[4];
#else                     // ------------------------------------------------
 #include <sys/stat.h>
 #include <errno.h>
#ifdef WIN32
 #include <winsock.h>
#else
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <resolv.h>
 typedef struct sockaddr_in sockaddr_in;
 typedef int SOCKET;
#endif

 #define closesocket(SOCK) close(SOCK)
#endif


JsNetworkState networkState =
#if defined(USE_CC3000) || defined(USE_WIZNET)
    NETWORKSTATE_OFFLINE
#else
    NETWORKSTATE_ONLINE
#endif
    ;

unsigned long parseIPAddress(const char *ip) {
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




/// Get an IP address from a name. Sets out_ip_addr to 0 on failure
void net_gethostbyname(JsNetwork *net, char * hostName, unsigned long* out_ip_addr) {
#if defined(USE_CC3000)
  gethostbyname(hostName, strlen(hostName), out_ip_addr);
#elif defined(USE_WIZNET)

  if (dns_query(0, getFreeSocket(), (uint8_t*)hostName) == 1) {
    *out_ip_addr = *(unsigned long*)&Server_IP_Addr[0];
  }
#else
  struct hostent * host_addr_p = gethostbyname(hostName);
  if (host_addr_p)
    *out_ip_addr = *(unsigned long*)*host_addr_p->h_addr_list;
#endif
  /* getaddrinfo is newer than this?
  *
  * struct addrinfo * result;
  * error = getaddrinfo("www.example.com", NULL, NULL, &result);
  * if (0 != error)
  *   fprintf(stderr, "error %s\n", gai_strerror(error));
  *
  */
}




#if defined(USE_WIZNET)
#define WIZNET_SERVER_CLIENT 256 // sockets are only 0-255 so this is masked out

uint8_t getFreeSocket() {
  uint8_t i;
  for (i=0;i<8;i++)
    if (getSn_SR(i) == SOCK_CLOSED) // it's free!
      return i;

  jsError("No free sockets found\n");
  // out of range will probably just make it error out
  return 8;
}
#endif




/// Called on idle. Do any checks required for this device
void net_idle(JsNetwork *net) {
#ifdef USE_CC3000
  cc3000_spi_check();
#endif
}

/// Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.
bool net_checkError(JsNetwork *net) {
  bool hadErrors = false;
#ifdef USE_CC3000
  while (jspIsInterrupted()) {
    hadErrors = true;
    jsiConsolePrint("Looks like CC3000 has died again. Power cycling...\n");
    jspSetInterrupted(false);
    // remove all existing connections
    networkState = NETWORKSTATE_OFFLINE; // ensure we don't try and send the CC3k anything
    httpKill(net);
    httpInit();
    // power cycle
    JsVar *wlan = jsvObjectGetChild(execInfo.root, CC3000_OBJ_NAME, 0);
    if (wlan) {
      jswrap_wlan_reconnect(wlan);
      jsvUnLock(wlan);
    } else jsErrorInternal("No CC3000 object!\n");
    // jswrap_wlan_reconnect could fail, which would mean we have to do this all over again
  }
#endif
  return hadErrors;
}

/// if host=0, creates a server otherwise creates a client (and automatically connects). Returns >=0 on success
int net_createsocket(JsNetwork *net, unsigned long host, unsigned short port) {
  int sckt = -1;
  if (host!=0) { // ------------------------------------------------- host (=client)
  #if defined(USE_CC3000)
    sockaddr       sin;
    sin.sa_family = AF_INET;
    sin.sa_data[0] = (unsigned char)((port & 0xFF00) >> 8);
    sin.sa_data[1] = (unsigned char)(port & 0x00FF);
  #elif defined(USE_WIZNET)
  #else
    sockaddr_in       sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons( port );
  #endif


  #if !defined(USE_WIZNET)
    sckt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  #else
    sckt = socket(getFreeSocket(), Sn_MR_TCP, port, 0); // we set nonblocking later
  #endif
    if (sckt<0) return sckt; // error

    // turn on non-blocking mode
    #ifdef WIN_OS
    u_long n = 1;
    ioctlsocket(sckt,FIONBIO,&n);
    #elif defined(USE_CC3000)
    int param;
    param = SOCK_ON;
    setsockopt(sckt, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &param, sizeof(param)); // enable nonblock
    param = 5; // ms
    setsockopt(sckt, SOL_SOCKET, SOCKOPT_RECV_TIMEOUT, &param, sizeof(param)); // set a timeout
    #elif defined(USE_WIZNET)
    // ...
    #else
    int flags = fcntl(sckt, F_GETFL);
    if (flags < 0) {
      jsError("Unable to retrieve socket descriptor status flags (%d)", flags);
      return -1001;
    }
    if (fcntl(sckt, F_SETFL, flags | O_NONBLOCK) < 0)
      jsError("Unable to set socket descriptor status flags");
    #endif

  #if defined(USE_CC3000)
    sin.sa_data[5] = (unsigned char)((host) & 0xFF);  // First octet of destination IP
    sin.sa_data[4] = (unsigned char)((host>>8) & 0xFF);   // Second Octet of destination IP
    sin.sa_data[3] = (unsigned char)((host>>16) & 0xFF);  // Third Octet of destination IP
    sin.sa_data[2] = (unsigned char)((host>>24) & 0xFF);  // Fourth Octet of destination IP
  #elif defined(USE_WIZNET)
  #else
    sin.sin_addr.s_addr = (in_addr_t)host;
  #endif

    //uint32_t a = sin.sin_addr.s_addr;
    //_DEBUG_PRINT( cout<<"Port :"<<sin.sin_port<<", Address : "<< sin.sin_addr.s_addr<<endl);
  #ifdef USE_WIZNET
    int res = connect((uint8_t)sckt,(uint8_t*)&host, port);
    // now we set nonblocking - so that connect waited for the connection
    uint8_t ctl = SOCK_IO_NONBLOCK;
    ctlsocket((uint8_t)sckt, CS_SET_IOMODE, &ctl);
  #else
    int res = connect(sckt,(struct sockaddr *)&sin, sizeof(sockaddr_in) );
  #endif

    if (res == SOCKET_ERROR) {
    #ifdef WIN_OS
     int err = WSAGetLastError();
    #elif defined(USE_WIZNET)
     int err = res;
    #else
     int err = errno;
    #endif
    #if !defined(USE_WIZNET)
     if (err != EINPROGRESS &&
         err != EWOULDBLOCK) {
    #else
     {
    #endif
       jsError("Connect failed (err %d)\n", err );
     }
    }

 } else { // ------------------------------------------------- no host (=server)

  #if !defined(USE_WIZNET)
    sckt = socket(AF_INET,           // Go over TCP/IP
                         SOCK_STREAM,       // This is a stream-oriented socket
                         IPPROTO_TCP);      // Use TCP rather than UDP
    if (sckt == INVALID_SOCKET) {
      jsError("Socket creation failed");
      return 0;
    }
  #if !defined(USE_CC3000)
    int optval = 1;
    if (setsockopt(sckt,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(optval)) < 0)
  #else
    int optval = SOCK_ON;
    if (setsockopt(sckt,SOL_SOCKET,SOCKOPT_ACCEPT_NONBLOCK,(const char *)&optval,sizeof(optval)) < 0)
  #endif
      jsWarn("setsockopt failed\n");

    int nret;
    sockaddr_in serverInfo;
    memset(&serverInfo, 0, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    //serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // allow only LOCAL clients to connect
    serverInfo.sin_addr.s_addr = INADDR_ANY; // allow anyone to connect
    serverInfo.sin_port = (unsigned short)htons((unsigned short)port); // port
    nret = bind(sckt, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
    if (nret == SOCKET_ERROR) {
      jsError("Socket bind failed");
      closesocket(sckt);
      return -1;
    }

    // Make the socket listen
    nret = listen(sckt, 10); // 10 connections (but this ignored on CC30000)
    if (nret == SOCKET_ERROR) {
      jsError("Socket listen failed");
      closesocket(sckt);
      return -1;
    }
  #else // USE_WIZNET
    sckt = socket(getFreeSocket(), Sn_MR_TCP, port, SF_IO_NONBLOCK);
  #endif
 }
  return sckt;
}

/// destroys the given socket
void net_closesocket(JsNetwork *net, int sckt) {
#if defined(USE_WIZNET)
    // close gracefully
    disconnect((uint8_t)sckt);
    JsSysTime timeout = jshGetSystemTime()+jshGetTimeFromMilliseconds(1000);
    uint8_t status;
    while ((status=getSn_SR((uint8_t)sckt)) != SOCK_CLOSED &&
           jshGetSystemTime()<timeout) ;
    // if that didn't work, force it
    if (status != SOCK_CLOSED)
      closesocket((uint8_t)sckt);
    // Wiznet is a bit strange in that it uses the same socket for server and client
    if (sckt & WIZNET_SERVER_CLIENT) {
      // so it's just closed, but put it into 'listen' mode again
      int port = 80; // FIXME
      sckt = socket((uint8_t)sckt, Sn_MR_TCP, port, SF_IO_NONBLOCK);
    }
#else
    closesocket(sckt);
#endif
}

/// If the given server socket can accept a connection, return it (or return < 0)
int net_accept(JsNetwork *net, int sckt) {
#if !defined(USE_CC3000) && !defined(USE_WIZNET)
  // TODO: look for unreffed servers?
  fd_set s;
  FD_ZERO(&s);
  FD_SET(sckt,&s);
  // check for waiting clients
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  int n = select(sckt+1,&s,NULL,NULL,&timeout);
  #else
  /* CC3000/WIZnet works a different way - we set accept as nonblocking,
   * and then we just call it and see if it works or not...
   */
  int n=1;
  #endif
  if (n>0) {
    // we have a client waiting to connect... try to connect and see what happens
  #if defined(USE_CC3000)
    // CC3000's implementation doesn't accept NULL like everyone else's :(
    sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    int theClient = accept(sckt,&addr,&addrlen);
  #elif defined(USE_WIZNET)
    // WIZnet's implementation doesn't use accept, it uses listen
    int theClient = listen((uint8_t)sckt);
    if (theClient==SOCK_OK)
      theClient = sckt | WIZNET_SERVER_CLIENT; // we deal with the client on the same socket (we use the flag so we know that it really is different!)
  #else
    int theClient = accept(sckt,0,0);
  #endif
    return theClient;
  }
  return -1;
}

/// Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_recv(JsNetwork *net, int sckt, void *buf, size_t len) {
  int num = 0;
  if (true
#if defined(USE_WIZNET)
      && getSn_SR(sckt)!=SOCK_LISTEN
#endif
        ) {
#if !defined(USE_WIZNET)
    fd_set s;
    FD_ZERO(&s);
    FD_SET(sckt,&s);
    // check for waiting clients
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int n = select(sckt+1,&s,NULL,NULL,&timeout);
    if (n==SOCKET_ERROR) {
      // we probably disconnected
      return -1;
    } else if (n>0) {
      // receive data
      num = (int)recv(sckt,buf,len,0);
      if (num==0) num=-1; // select says data, but recv says 0 means connection is closed
    }
#else // defined(USE_WIZNET)
    // receive data - if none available it'll just return SOCK_BUSY
    num = (int)recv(sckt,buf,len,0);
    if (num==SOCK_BUSY) num=0;
#endif
  }

#ifdef CC3000
  if (num==0 && cc3000_socket_has_closed(sckt))
    return -1;
#endif

  return num;
}

/// Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure
int net_send(JsNetwork *net, int sckt, const void *buf, size_t len) {

#ifdef CC3000
  if (cc3000_socket_has_closed(sckt))
    return -1;
#endif

#if !defined(USE_WIZNET)
  fd_set writefds;
  FD_ZERO(&writefds);
  FD_SET(sckt, &writefds);
  struct timeval time;
  time.tv_sec = 0;
  time.tv_usec = 0;
  int n = select(sckt+1, 0, &writefds, 0, &time);
  if (n==SOCKET_ERROR ) {
     // we probably disconnected so just get rid of this
    return -1;
  } else if (FD_ISSET(sckt, &writefds)) {
    n = send(sckt, buf, len, MSG_NOSIGNAL);
    return n;
  } else
    return 0; // just not ready
#else // defined(USE_WIZNET)



#endif
}


void networkCreate(JsNetwork *net) {
  net->data.type = JSNETWORKTYPE_SOCKET;

  net->networkVar = jsvNewStringOfLength(sizeof(JsNetworkData));
  networkSet(net);
  jsvUnLock(jsvObjectSetChild(execInfo.root, NETWORK_VAR_NAME, net->networkVar));
  networkGetFromVar(net);
}

bool networkGetFromVar(JsNetwork *net) {
  net->networkVar = jsvObjectGetChild(execInfo.root, NETWORK_VAR_NAME, 0);
  if (!net->networkVar) {
#ifdef LINUX
    networkCreate(net);
    return true;
#else
    return false;
#endif
  }
  jsvGetString(net->networkVar, &net->data, sizeof(JsNetworkData)+1/*trailing zero*/);

  net->idle = net_idle;
  net->checkError = net_checkError;
  net->createsocket = net_createsocket;
  net->closesocket = net_closesocket;
  net->accept = net_accept;
  net->gethostbyname = net_gethostbyname;
  net->recv = net_recv;
  net->send = net_send;
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
  jsvSetString(net->networkVar, &net->data, sizeof(JsNetworkData));
}

void networkFree(JsNetwork *net) {
  jsvUnLock(net->networkVar);
}
