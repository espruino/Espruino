#include "network_w600.h"
#include "wm_sockets.h"
#include "lwip/errno.h"

void net_idle_w600(JsNetwork *net){
  NOT_USED(net);
}

bool net_checkError_w600(JsNetwork *net){
  NOT_USED(net);
  return false;
}

void net_gethostbyname_w600(JsNetwork *net,char *host_name,uint32_t *out_ip_addr){
  struct hostent *host_addr_p=gethostbyname(host_name);
  if(host_addr_p){
    *out_ip_addr=*(uint32_t *)*host_addr_p->h_addr_list;
  }
}

int net_createsocket_w600(JsNetwork *net,SocketType socketType,uint32_t host,unsigned short port,JsVar *options){
  int ippProto = socketType & ST_UDP ? IPPROTO_UDP : IPPROTO_TCP;
  int scktType = socketType & ST_UDP ? SOCK_DGRAM : SOCK_STREAM;
  int sckt = -1;

  if(host!=0){
    sckt=socket(AF_INET,scktType,ippProto);
    if (sckt<0) {
      jsError("Socket creation failed");
      return sckt; // error
    }

    if (scktType == SOCK_DGRAM) { // only for UDP
      // set broadcast
      int optval = 1;
      if (setsockopt(sckt,SOL_SOCKET,SO_BROADCAST,(const char *)&optval,sizeof(optval))<0){
        jsWarn("setsockopt(SO_BROADCAST) failed\n");
      }
    }else{
      struct sockaddr_in       sin;
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = (in_addr_t)host;
      sin.sin_port = htons( port );

      int res = connect(sckt,(struct sockaddr *)&sin, sizeof(struct sockaddr_in) );
      if (res == WM_FAILED) {
       int err = errno;
       if (err != EINPROGRESS &&
           err != EWOULDBLOCK) {
         jsError("Connect failed (err %d)", err);
         closesocket(sckt);
         return -1;
       }
      }
    }
  }else{
    sckt = socket(AF_INET, scktType, ippProto);
    if (sckt == WM_FAILED) {
      jsError("Socket creation failed");
      return 0;
    }

    if (scktType != SOCK_DGRAM ||jsvGetBoolAndUnLock(jsvObjectGetChild(options, "reuseAddr", 0))) {
      int optval = 1;
      if (setsockopt(sckt,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(optval)) < 0){
        jsWarn("setsockopt(SO_REUSADDR) failed\n");
      }

      #ifdef SO_REUSEPORT
      if (setsockopt(sckt,SOL_SOCKET,SO_REUSEPORT,(const char *)&optval,sizeof(optval)) < 0){
        jsWarn("setsockopt(SO_REUSPORT) failed\n");
      }
      #endif
    }

    int nret;
    struct sockaddr_in serverInfo;
    memset(&serverInfo, 0, sizeof(struct sockaddr_in));
    serverInfo.sin_family = AF_INET;
    //serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // allow only LOCAL clients to connect
    serverInfo.sin_addr.s_addr = INADDR_ANY; // allow anyone to connect
    serverInfo.sin_port = (unsigned short)htons((unsigned short)port); // port
    nret = bind(sckt, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
    if (nret == WM_FAILED) {
      jsError("Socket bind failed");
      closesocket(sckt);
      return -1;
    }

    // multicast support
    // FIXME: perhaps extend the JsNetwork with addmembership/removemembership instead of using options
    JsVar *mgrpVar = jsvObjectGetChild(options, "multicastGroup", 0);
    if (mgrpVar) {
        char ipStr[18];

        uint32_t grpip;
        jsvGetString(mgrpVar, ipStr, sizeof(ipStr));
        jsvUnLock(mgrpVar);
        net_gethostbyname_w600(net, ipStr, &grpip);

        JsVar *ipVar = jsvObjectGetChild(options, "multicastIp", 0);
        jsvGetString(ipVar, ipStr, sizeof(ipStr));
        jsvUnLock(ipVar);
        uint32_t ip;
        net_gethostbyname_w600(net, ipStr, &ip);

        struct ip_mreq mreq;
        mreq.imr_multiaddr = *(struct in_addr *)&grpip;
        mreq.imr_interface = *(struct in_addr *)&ip;
        setsockopt (sckt, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    }

    if (scktType == SOCK_STREAM) { // only for TCP
      // Make the socket listen
      nret = listen(sckt, 10); // 10 connections (but this ignored on CC30000)
      if (nret == WM_FAILED) {
        jsError("Socket listen failed");
        closesocket(sckt);
        return -1;
      }
    }

  }

  #ifdef SO_RCVBUF
  int rcvBufSize = net->data.recvBufferSize;
  if (rcvBufSize > 0) {
    if (setsockopt(sckt,SOL_SOCKET,SO_RCVBUF,(const char *)&rcvBufSize,sizeof(rcvBufSize))<0){
      jsWarn("setsockopt(SO_RCVBUF) failed\n");
    }
  }
  #endif

  #ifdef SO_NOSIGPIPE
  // disable SIGPIPE
  int optval = 1;
  if (setsockopt(sckt,SOL_SOCKET,SO_NOSIGPIPE,(const char *)&optval,sizeof(optval))<0){
    jsWarn("setsockopt(SO_NOSIGPIPE) failed\n");
  }
  #endif
  return sckt;
}

void net_closesocket_w600(JsNetwork *net,int sckt){
  NOT_USED(net);
  closesocket(sckt);
}

int net_accept_w600(JsNetwork *net,int sckt){
  NOT_USED(net);
  // TODO: look for unreffed servers?
  fd_set s;
  FD_ZERO(&s);
  FD_SET(sckt,&s);
  // check for waiting clients
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  int n = select(sckt+1,&s,NULL,NULL,&timeout);
  if (n>0) {
    // we have a client waiting to connect... try to connect and see what happens
    int theClient = accept(sckt,0,0);
    return theClient;
  }
  return -1;
}

int net_recv_w600(JsNetwork *net,SocketType socketType,int sckt,void *buf,size_t len){
  NOT_USED(net);
  struct sockaddr_in fromAddr;
  int fromAddrLen = sizeof(fromAddr);
  int num = 0;
  fd_set s;
  FD_ZERO(&s);
  FD_SET(sckt,&s);
  // check for waiting clients
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  int n = select(sckt+1,&s,NULL,NULL,&timeout);
  if (n==WM_FAILED) {
    // we probably disconnected
    return -1;
  } else if (n>0) {
    // receive data
    if (socketType & ST_UDP) {
      num = (int)recvfrom(sckt,buf+sizeof(JsNetUDPPacketHeader),len-sizeof(JsNetUDPPacketHeader),0,(struct sockaddr *)&fromAddr,(socklen_t *)&fromAddrLen);

      JsNetUDPPacketHeader *header = (JsNetUDPPacketHeader*)buf;
      *(in_addr_t*)&header->host = fromAddr.sin_addr.s_addr;
      header->port = ntohs(fromAddr.sin_port);
      header->length = num;

      if (num==0) return -1; // select says data, but recv says 0 means connection is closed
      num += sizeof(JsNetUDPPacketHeader);
    } else {
      num = (int)recvfrom(sckt,buf,len,0,(struct sockaddr *)&fromAddr,(socklen_t *)&fromAddrLen);
      if (num==0) return -1; // select says data, but recv says 0 means connection is closed
    }
  }

  return num;
}

int net_send_w600(JsNetwork *net,SocketType socketType,int sckt,const void *buf,size_t len){

  NOT_USED(net);
  fd_set writefds;
  FD_ZERO(&writefds);
  FD_SET(sckt, &writefds);
  struct timeval time;
  time.tv_sec = 0;
  time.tv_usec = 0;
  int n = select(sckt+1, 0, &writefds, 0, &time);
  if (n==WM_FAILED ) {
     // we probably disconnected so just get rid of this
    return -1;
  } else if (FD_ISSET(sckt, &writefds)) {
    int flags = 0;
    #if !defined(SO_NOSIGPIPE) && defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
    #endif
    if (socketType & ST_UDP) {
      JsNetUDPPacketHeader *header = (JsNetUDPPacketHeader*)buf;
      struct sockaddr_in sin;
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = *(in_addr_t*)&header->host;
      sin.sin_port = htons(header->port);

      n = (int)sendto(sckt, buf + sizeof(JsNetUDPPacketHeader), header->length, flags, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) + sizeof(JsNetUDPPacketHeader);
    } else {
      n = (int)send(sckt, buf, len, flags);
    }
    return n;
  } else{
    return 0; // just not ready
  }
}

void netSetCallbacks_w600(JsNetwork *net){
    net->idle=net_idle_w600;
    net->checkError=net_checkError_w600;
    net->gethostbyname=net_gethostbyname_w600;
    net->createsocket=net_createsocket_w600;
    net->closesocket=net_closesocket_w600;
    net->accept=net_accept_w600;
    net->recv=net_recv_w600;
    net->send=net_send_w600;
    net->chunkSize=536;
}