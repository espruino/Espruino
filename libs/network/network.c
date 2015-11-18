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
#include "jsinteractive.h"

#if defined(USE_CC3000)
  #include "network_cc3000.h"
#endif
#if defined(USE_WIZNET)
  #include "network_wiznet.h"
#endif
#if defined(USE_ESP8266)
  #include "network_esp8266.h"
#endif
#if defined(LINUX)
  #include "network_linux.h"
#endif
#if defined(USE_HTTPS)
  #include "mbedtls/ssl.h"
  #include "mbedtls/ctr_drbg.h"
#endif
#include "network_js.h"

JsNetworkState networkState =
#ifdef LINUX
    NETWORKSTATE_ONLINE
#else
    NETWORKSTATE_OFFLINE
#endif
    ;

JsNetwork *networkCurrentStruct = 0;

uint32_t networkParseIPAddress(const char *ip) {
  int n = 0;
  uint32_t addr = 0;
  while (*ip) {
    if (*ip>='0' && *ip<='9') {
      n = n*10 + (*ip-'0');
    } else if (*ip=='.') {
      addr = (addr>>8) | (uint32_t)(n<<24);
      n=0;
    } else {
      return 0; // not an ip address
    }
    ip++;
  }
  addr = (addr>>8) | (uint32_t)(n<<24);
  return addr;
}

/* given 6 pairs of 8 bit hex numbers separated by ':', parse them into a
 * 6 byte array. returns false on failure */
bool networkParseMACAddress(unsigned char *addr, const char *ip) {
  int n = 0;
  int i = 0;
  while (*ip) {
    int v = chtod(*ip);
    if (v>=0 && v<16) {
      n = n*16 + v;
    } else if (*ip==':') {
      addr[i++] = (unsigned char)n;
      n=0;
      if (i>5) return false; // too many items!
    } else {
      return false; // not a mac address
    }
    ip++;
  }
  addr[i] = (unsigned char)n;
  return i==5;
}

JsVar *networkGetAddressAsString(unsigned char *ip, int nBytes, unsigned int base, char separator) {
  char data[64] = "";
  int i = 0, dir = 1, l = 0;
  if (nBytes<0) {
    i = (-nBytes)-1;
    nBytes = -1;
    dir=-1;
  }
  for (;i!=nBytes;i+=dir) {
    if (base==16) {
      data[l++] = itoch(ip[i]>>4);
      data[l++] = itoch(ip[i]&15);
    } else {
      itostr((int)ip[i], &data[l], base);
    }
    l = (int)strlen(data);
    if (i+dir!=nBytes && separator) {
      data[l++] = separator;
      data[l] = 0;
    }
  }

  return jsvNewFromString(data);
}

void networkPutAddressAsString(JsVar *object, const char *name,  unsigned char *ip, int nBytes, unsigned int base, char separator) {
  jsvObjectSetChildAndUnLock(object, name, networkGetAddressAsString(ip, nBytes, base, separator));
}

/** Some devices (CC3000) store the IP address with the first element last, so we must flip it */
unsigned long networkFlipIPAddress(unsigned long addr) {
  return
      ((addr&0xFF)<<24) |
      ((addr&0xFF00)<<8) |
      ((addr&0xFF0000)>>8) |
      ((addr&0xFF000000)>>24);
}

/**
 * Get the IP address of a hostname.
 * Retrieve the IP address of a hostname and return it in the address of the
 * ip address passed in.  If the hostname is as dotted decimal string, we will
 * decode that immediately otherwise we will use the network adapter's `gethostbyname`
 * function to resolve the hostname.
 *
 * A value of 0 returned for an IP address means we could NOT resolve the hostname.
 * A value of 0xFFFFFFFF for an IP address means that we haven't found it YET.
 */
void networkGetHostByName(
    JsNetwork *net,        //!< The network we are using for resolution.
    char      *hostName,   //!< The hostname to be resolved.
    uint32_t  *out_ip_addr //!< The address where the returned IP address will be stored.
  ) {
  assert(hostName    != NULL);
  assert(out_ip_addr != NULL);

  // Set the default IP address returned to be 0 that indicates not found.
  *out_ip_addr = 0;

  // first try and simply parse the IP address as a string
  *out_ip_addr = networkParseIPAddress(hostName);

  // If we did not get an IP address from the string, then try and resolve it by
  // calling the network gethostbyname.
  if (!*out_ip_addr) {
    net->gethostbyname(net, hostName, out_ip_addr);
  }
}



void networkCreate(JsNetwork *net, JsNetworkType type) {
  net->networkVar = jsvNewStringOfLength(sizeof(JsNetworkData));
  if (!net->networkVar) return;
  net->data.type = type;
  net->data.device = EV_NONE;
  net->data.pinCS = PIN_UNDEFINED;
  net->data.pinIRQ = PIN_UNDEFINED;
  net->data.pinEN = PIN_UNDEFINED;
  jsvObjectSetChildAndUnLock(execInfo.hiddenRoot, NETWORK_VAR_NAME, net->networkVar);
  networkSet(net);
  networkGetFromVar(net);
}

bool networkWasCreated() {
  JsVar *v = jsvObjectGetChild(execInfo.hiddenRoot, NETWORK_VAR_NAME, 0);
  if (v) {
    jsvUnLock(v);
    return true;
  } else {
    return false;
  }
}

bool networkGetFromVar(JsNetwork *net) {
  // Retrieve a reference to the JsVar that represents the network and save in the
  // JsNetwork C structure.
  net->networkVar = jsvObjectGetChild(execInfo.hiddenRoot, NETWORK_VAR_NAME, 0);

  // Validate that we have a network variable.
  if (!net->networkVar) {
#ifdef LINUX
    networkCreate(net, JSNETWORKTYPE_SOCKET);
    return net->networkVar != 0;
#else
    return false;
#endif
  }

  // Retrieve the data for the network var and save in the data property of the JsNetwork
  // structure.
  jsvGetString(net->networkVar, (char *)&net->data, sizeof(JsNetworkData)+1/*trailing zero*/);

  // Now we know which kind of network we are working with, invoke the corresponding initialization
  // function to set the callbacks for this network tyoe.
  switch (net->data.type) {
#if defined(USE_CC3000)
  case JSNETWORKTYPE_CC3000 : netSetCallbacks_cc3000(net); break;
#endif
#if defined(USE_WIZNET)
  case JSNETWORKTYPE_W5500 : netSetCallbacks_wiznet(net); break;
#endif
#if defined(USE_ESP8266)
  case JSNETWORKTYPE_ESP8266_BOARD : netSetCallbacks_esp8266_board(net); break;
#endif
#if defined(LINUX)
  case JSNETWORKTYPE_SOCKET : netSetCallbacks_linux(net); break;
#endif
  case JSNETWORKTYPE_JS : netSetCallbacks_js(net); break;
  default:
    jsError("Unknown network device %d", net->data.type);
    networkFree(net);
    return false;
  }

  // Save the current network as a global.
  networkCurrentStruct = net;
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
  networkCurrentStruct = 0;
  jsvUnLock(net->networkVar);
}

JsNetwork *networkGetCurrent() {
  // The value of this global is set in networkGetFromVar.
  return networkCurrentStruct;
}

// ------------------------------------------------------------------------------
#ifdef USE_HTTPS

typedef struct {
  int sckt;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_x509_crt cacert;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
} SSLSocketData;

SSLSocketData _sd;
SSLSocketData *sd = &_sd; // make it easier to use pointers later

BITFIELD_DECL(socketIsHTTPS, 32);

static void ssl_debug( void *ctx, int level,
                      const char *file, int line, const char *str )
{
    ((void) ctx);
    ((void) level);
    jsiConsolePrintf( "%s:%d: %s", file, line, str );
}

int ssl_send(void *ctx, const unsigned char *buf, size_t len) {
  JsNetwork *net = networkGetCurrent();
  assert(net);
  int sckt = *(int *)ctx;
  int r = net->send(net, sckt, buf, len);
  if (r==0) return MBEDTLS_ERR_SSL_WANT_WRITE;
  return r;
}
int ssl_recv(void *ctx, unsigned char *buf, size_t len) {
  JsNetwork *net = networkGetCurrent();
  assert(net);
  int sckt = *(int *)ctx;
  int r = net->recv(net, sckt, buf, len);
  if (r==0) return MBEDTLS_ERR_SSL_WANT_READ;
  return r;
}

int ssl_entropy( void *data, unsigned char *output, size_t len ) {
  NOT_USED(data);
  size_t i;
  unsigned int r;
  for (i=0;i<len;i++) {
    if (!(i&3)) r = jshGetRandomNumber();
    output[i] = (unsigned char)r;
    r>>=8;
  }
  return 0;
}

/*SSLSocketData *ssl_getSocketData(int sckt) {
  JsVar *ssl = jsvObjectGetChild(execInfo.root, "ssl", 0);
  if (!ssl) return 0;
  JsVar *sslData = jsvGetArrayItem(ssl, sckt);
  SSLSocketData *data = 0;
  if (jsvIsFlatString(sslData))
    data = jsvGetFlatStringPointer(sslData);
  jsvUnLock(sslData);
  return data;
}*/

#endif
// ------------------------------------------------------------------------------

bool netCheckError(JsNetwork *net) {
  return net->checkError(net);
}

int netCreateSocket(JsNetwork *net, uint32_t host, unsigned short port, NetCreateFlags flags) {
#ifdef USE_HTTPS
  if (flags & NCF_TLS) {
    jsiConsolePrintf( "Connecting with TLS..." );
    int ret;

    const char *pers = "ssl_client1";
    mbedtls_ssl_init( &sd->ssl );
    mbedtls_ssl_config_init( &sd->conf );
    mbedtls_x509_crt_init( &sd->cacert );
    mbedtls_ctr_drbg_init( &sd->ctr_drbg );
    if( ( ret = mbedtls_ctr_drbg_seed( &sd->ctr_drbg, ssl_entropy, 0,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 ) {
        jsiConsolePrintf("HTTPS init failed! mbedtls_ctr_drbg_seed returned %d\n", ret );
        return -1;
    }

    sd->sckt = net->createsocket(net, host, port);
    if (sd->sckt<0) {
      jsiConsolePrintf("HTTPS Connect failed");
      return -1;
    }
    assert(sd->sckt>=0 && sd->sckt<32);

    if( ( ret = mbedtls_ssl_config_defaults( &sd->conf,
                    MBEDTLS_SSL_IS_CLIENT, // or MBEDTLS_SSL_IS_SERVER
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 ) {
      jsiConsolePrintf( "HTTPS init failed! mbedtls_ssl_config_defaults returned %d\n\n", ret );
      return -1;
    }

    // FIXME no cert checking!
    mbedtls_ssl_conf_authmode( &sd->conf, MBEDTLS_SSL_VERIFY_NONE );
    mbedtls_ssl_conf_ca_chain( &sd->conf, &sd->cacert, NULL );
    mbedtls_ssl_conf_rng( &sd->conf, mbedtls_ctr_drbg_random, &sd->ctr_drbg );
    mbedtls_ssl_conf_dbg( &sd->conf, ssl_debug, 0 );

    if( ( ret = mbedtls_ssl_setup( &sd->ssl, &sd->conf ) ) != 0 )
     {
      jsiConsolePrintf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
      return -1;
     }

    if( ( ret = mbedtls_ssl_set_hostname( &sd->ssl, "mbed TLS Server 1" ) ) != 0 ) {
      jsiConsolePrintf( "HTTPS init failed! mbedtls_ssl_set_hostname returned %d\n\n", ret );
      return -1;
    }

    mbedtls_ssl_set_bio( &sd->ssl, &sd->sckt, ssl_send, ssl_recv, NULL );

    jsiConsolePrintf( "  . Performing the SSL/TLS handshake..." );

    while( ( ret = mbedtls_ssl_handshake( &sd->ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
          jsiConsolePrintf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
          return -1;
        }
    }

    jsiConsolePrintf( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    jsiConsolePrintf( "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &sd->ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        jsiConsolePrintf( " failed\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        jsiConsolePrintf( "%s\n", vrfy_buf );
    }
    else
      jsiConsolePrintf( " ok\n" );


    BITFIELD_SET(socketIsHTTPS, sd->sckt, 1);
    return sd->sckt;
  } else {
    int sckt = net->createsocket(net, host, port);
    assert(sckt>=0 && sckt<32);
    BITFIELD_SET(socketIsHTTPS, sckt, 0);
    return sckt;
  }
#else
  return net->createsocket(net, host, port);
#endif
}

void netCloseSocket(JsNetwork *net, int sckt) {
  net->closesocket(net, sckt);
#ifdef USE_HTTPS
  /*
  mbedtls_net_free( &server_fd );
  mbedtls_ssl_free( &ssl );
  mbedtls_ssl_config_free( &conf );
  mbedtls_ctr_drbg_free( &ctr_drbg );
  mbedtls_entropy_free( &entropy );
   */
#endif
}

int netAccept(JsNetwork *net, int sckt) {
  return net->accept(net, sckt);
}

void netGetHostByName(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
  net->gethostbyname(net, hostName, out_ip_addr);
}

int netRecv(JsNetwork *net, int sckt, void *buf, size_t len) {
#ifdef USE_HTTPS
  if (BITFIELD_GET(socketIsHTTPS, sckt)) {
    int ret = mbedtls_ssl_read( &sd->ssl, buf, len );
    if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
      return 0;
    return ret;
  } else
#endif
  {
    return net->recv(net, sckt, buf, len);
  }
}

int netSend(JsNetwork *net, int sckt, const void *buf, size_t len) {
#ifdef USE_HTTPS
  if (BITFIELD_GET(socketIsHTTPS, sckt)) {
    int ret = mbedtls_ssl_write( &sd->ssl, buf, len );
    if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
      return 0;
    return ret;
  } else
#endif
  {
    return net->send(net, sckt, buf, len);
  }
}
