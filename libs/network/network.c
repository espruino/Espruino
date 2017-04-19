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
#ifdef USE_FILESYSTEM
  #include "jswrap_functions.h"
  #include "jswrap_fs.h"
#endif

#if defined(USE_CC3000)
  #include "network_cc3000.h"
#endif
#if defined(USE_WIZNET)
  #include "network_wiznet.h"
#endif
#if defined(USE_ESP8266)
  #include "network_esp8266.h"
#endif
#if defined(USE_ESP32)
  #include "network_esp32.h"
#endif
#if defined(LINUX)
  #include "network_linux.h"
#endif
#if defined(USE_TLS)
  #include "mbedtls/ssl.h"
  #include "mbedtls/ctr_drbg.h"
  #include "jswrap_crypto.h"
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

/**
 * Convert a buffer of bytes pointed to by ip ... for a length of nBytes into a string where each
 * byte is separated by a separator character.  The numeric base of the bytes is given by the base
 * value.
 *
 * For example, to create a dotted decimal string, one would use:
 * networkGetAddressAsString(ip, 4, 10, '.')
 *
 * To create a Mac address, one might use:
 * networkGetAddressAsString(mac, 6, 16, ':')
 */
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

/**
 * Convert a buffer of bytes pointed to by ip ... for a length of nBytes into a string member of an object where each
 * byte is separated by a separator character.  The numeric base of the bytes is given by the base
 * value.
 *
 * For example, to create a dotted decimal string, one would use:
 * networkPutAddressAsString(myObject,"ip", ip, 4, 10, '.')
 *
 * To create a Mac address, one might use:
 * networkPutAddressAsString(myObject, "mac", mac, 6, 16, ':')
 */
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
#if defined(USE_ESP32)
  case JSNETWORKTYPE_ESP32 : netSetCallbacks_esp32(net); break;
#endif
#if defined(LINUX)
  case JSNETWORKTYPE_SOCKET : netSetCallbacks_linux(net); break;
#endif
  case JSNETWORKTYPE_JS : netSetCallbacks_js(net); break;
  default:
    jsExceptionHere(JSET_INTERNALERROR, "Unknown network device %d", net->data.type);
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
    jsExceptionHere(JSET_INTERNALERROR, "Not connected to the internet");
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
#ifdef USE_TLS

typedef struct {
  int sckt;
  bool connecting; // are we in the process of connecting?
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_pk_context pkey;
  mbedtls_x509_crt owncert;
  mbedtls_x509_crt cacert;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
} SSLSocketData;

BITFIELD_DECL(socketIsHTTPS, 32);

static void ssl_debug( void *ctx, int level,
                      const char *file, int line, const char *str )
{
    ((void) ctx);
    ((void) level);
    // jsiConsolePrintf( "%s:%d: %s", file, line, str );
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

int ssl_entropy(void *data, unsigned char *output, size_t len ) {
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

void ssl_freeSocketData(int sckt) {
  BITFIELD_SET(socketIsHTTPS, sckt, 0);

  JsVar *ssl = jsvObjectGetChild(execInfo.root, "ssl", 0);
  if (!ssl) return;
  JsVar *scktVar = jsvNewFromInteger(sckt);
  JsVar *sslDataVar = jsvFindChildFromVar(ssl, scktVar, false);
  jsvUnLock(scktVar);
  JsVar *sslData = jsvSkipName(sslDataVar);
  jsvRemoveChild(ssl, sslDataVar);
  jsvUnLock(sslDataVar);
  jsvUnLock(ssl);
  SSLSocketData *sd = 0;
  if (jsvIsFlatString(sslData)) {
    sd = (SSLSocketData *)jsvGetFlatStringPointer(sslData);
    mbedtls_ssl_free( &sd->ssl );
    mbedtls_ssl_config_free( &sd->conf );
    mbedtls_ctr_drbg_free( &sd->ctr_drbg );
    mbedtls_x509_crt_free( &sd->owncert );
    mbedtls_x509_crt_free( &sd->cacert );
    mbedtls_pk_free( &sd->pkey );
  }
  jsvUnLock(sslData);
}

#ifdef USE_FILESYSTEM
/* load cert file from filesystem */
JsVar *load_cert_file(JsVar *cert) {
  // jsiConsolePrintf("Loading certificate file: %q\n", cert);
  JsVar *fileContents = jswrap_fs_readFile(cert);
  if (!fileContents || jsvIsStringEqual(fileContents, "")) {
    jsWarn("File %q not found", cert);
    jsvUnLock(fileContents);
    return 0;
  }

  JSV_GET_AS_CHAR_ARRAY(pPtr, pLen, fileContents);
  bool started = false;
  unsigned int i = 0;
  unsigned int j = 0;

  /* parse the beginning, end, and line returns from cert file */
  for (i = 0; i < pLen; i++) {
    if (started && pPtr[i] == '-') {
      //jsvArrayBufferSet(fileContents, 1, '\0')
      pPtr[j] = '\0';
      break;  /* end of key */
    }
    if (!started && pPtr[i] == '\n') {
      started = true; /* found the start of the key */
      continue;
    }
    if (started && pPtr[i] != '\n' && pPtr[i] != '\r') {
      pPtr[j++] = pPtr[i];
    }
  }  

  JsVar *parsed = jsvNewFromString((const char *)pPtr);
  /* convert to binary */
  JsVar *buffer = jswrap_atob(parsed);
  jsvUnLock2(fileContents, parsed);

  if (!buffer) {
    jsWarn("Failed to process file %q", cert);
    return 0;
  }

  return buffer;
}
#endif /* USE_FILESYSTEM */


/** Given a variable:
 *
 * if it's a string <=100 chars long use it as a filename (if filesystem is enabled)
 * if it's a function, run it and return the result
 */
JsVar *decode_certificate_var(JsVar *var) {
  JsVar *decoded = 0;
  if (jsvIsFunction(var)) {
    decoded = jspExecuteFunction(var, 0, 0, 0);
  }
#ifdef USE_FILESYSTEM
  if (jsvIsString(var) && jsvGetStringLength(var) <= 100) {
    /* too short for cert data, so assume a file path*/
    decoded = load_cert_file(var);
  }
#endif /* USE_FILESYSTEM */
  if (!decoded) decoded = jsvLockAgain(var);
  return decoded;
}

bool ssl_load_key(SSLSocketData *sd, JsVar *options) {
  JsVar *keyVar = jsvObjectGetChild(options, "key", 0);
  if (!keyVar) {
    return true; // still ok - just no key
  }
  int ret = -1;
  // jsiConsolePrintf("Loading the Client Key...\n");

  JsVar *buffer = decode_certificate_var(keyVar);
  JSV_GET_AS_CHAR_ARRAY(keyPtr, keyLen, buffer);
  if (keyLen && keyPtr) {
    ret = mbedtls_pk_parse_key(&sd->pkey, (const unsigned char *)keyPtr, keyLen, NULL, 0 /*no password*/);
  }
  jsvUnLock(buffer);

  jsvUnLock(keyVar);
  if (ret != 0) {
    JsVar *e = jswrap_crypto_error_to_jsvar(ret);
    jsExceptionHere(JSET_INTERNALERROR, "HTTPS init failed! mbedtls_pk_parse_key: %v\n", e);
    jsvUnLock(e);
    return false;
  }

  return true;
}
bool ssl_load_owncert(SSLSocketData *sd, JsVar *options) {
  JsVar *certVar = jsvObjectGetChild(options, "cert", 0);
  if (!certVar) {
    return true; // still ok - just no cert
  }
  int ret = -1;
  // jsiConsolePrintf("Loading the Client certificate...\n");

  JsVar *buffer = decode_certificate_var(certVar);
  JSV_GET_AS_CHAR_ARRAY(certPtr, certLen, buffer);
  if (certLen && certPtr) {
    ret = mbedtls_x509_crt_parse(&sd->owncert, (const unsigned char *)certPtr, certLen);
  }
  jsvUnLock(buffer);

  jsvUnLock(certVar);
  if (ret != 0) {
    JsVar *e = jswrap_crypto_error_to_jsvar(ret);
    jsExceptionHere(JSET_INTERNALERROR, "HTTPS init failed! mbedtls_x509_crt_parse of 'cert': %v\n", e);
    jsvUnLock(e);
    return false;
  }
  return true;
}
bool ssl_load_cacert(SSLSocketData *sd, JsVar *options) {
  JsVar *caVar = jsvObjectGetChild(options, "ca", 0);
  if (!caVar) {
    return true; // still ok - just no ca
  }
  int ret = -1;
  // jsiConsolePrintf("Loading the CA root certificate...\n");

  JsVar *buffer = decode_certificate_var(caVar);
  JSV_GET_AS_CHAR_ARRAY(caPtr, caLen, buffer);
  if (caLen && caPtr) {
    ret = mbedtls_x509_crt_parse(&sd->cacert, (const unsigned char *)caPtr, caLen);
  }
  jsvUnLock(buffer);

  jsvUnLock(caVar);
  if (ret != 0) {
    JsVar *e = jswrap_crypto_error_to_jsvar(ret);
    jsExceptionHere(JSET_INTERNALERROR, "HTTPS init failed! mbedtls_x509_crt_parse of 'ca': %v\n", e);
    jsvUnLock(e);
    return false;
  }

  return true;
}

bool ssl_newSocketData(int sckt, JsVar *options) {
  /* FIXME Warning:
   *
   * MBEDTLS_SSL_MAX_CONTENT_LEN = 16kB, so we need over double this = 32kB memory
   * for just a single connection!!
   *
   * Also see https://tls.mbed.org/kb/how-to/reduce-mbedtls-memory-and-storage-footprint
   * */

  assert(sckt>=0 && sckt<32);
  // Create a new socketData using the variable
  JsVar *ssl = jsvObjectGetChild(execInfo.root, "ssl", JSV_OBJECT);
  if (!ssl) return false; // out of memory?
  JsVar *scktVar = jsvNewFromInteger(sckt);
  JsVar *sslDataVar = jsvFindChildFromVar(ssl, scktVar, true);
  jsvUnLock(scktVar);
  jsvUnLock(ssl);
  if (!sslDataVar) {
    return 0; // out of memory
  }
  JsVar *sslData = jsvNewFlatStringOfLength(sizeof(SSLSocketData));
  if (!sslData) {
    jsExceptionHere(JSET_INTERNALERROR, "Not enough memory to allocate SSL socket\n");
    jsvUnLock(sslDataVar);
    return false;
  }
  jsvSetValueOfName(sslDataVar, sslData);
  jsvUnLock(sslDataVar);
  SSLSocketData *sd = (SSLSocketData *)jsvGetFlatStringPointer(sslData);
  jsvUnLock(sslData);
  assert(sd);

  // Now initialise this
  sd->sckt = sckt;
  sd->connecting = true;

  // jsiConsolePrintf( "Connecting with TLS...\n" );

  int ret;

  const char *pers = "ssl_client1";
  mbedtls_ssl_init( &sd->ssl );
  mbedtls_ssl_config_init( &sd->conf );
  mbedtls_pk_init( &sd->pkey );
  mbedtls_x509_crt_init( &sd->owncert );
  mbedtls_x509_crt_init( &sd->cacert );
  mbedtls_ctr_drbg_init( &sd->ctr_drbg );
  if (( ret = mbedtls_ctr_drbg_seed( &sd->ctr_drbg, ssl_entropy, 0,
                             (const unsigned char *) pers,
                             strlen(pers))) != 0 ) {
    JsVar *e = jswrap_crypto_error_to_jsvar(ret);
    jsExceptionHere(JSET_INTERNALERROR, "HTTPS init failed! mbedtls_ctr_drbg_seed: %v\n", e );
    jsvUnLock(e);
    ssl_freeSocketData(sckt);
    return false;
  }

  if (jsvIsObject(options)) {
    if (!ssl_load_cacert(sd, options) ||
        !ssl_load_owncert(sd, options) ||
        !ssl_load_key(sd, options)) {
      ssl_freeSocketData(sckt);
      return false;
    }
  }

  if (( ret = mbedtls_ssl_config_defaults( &sd->conf,
                  MBEDTLS_SSL_IS_CLIENT, // or MBEDTLS_SSL_IS_SERVER
                  MBEDTLS_SSL_TRANSPORT_STREAM,
                  MBEDTLS_SSL_PRESET_DEFAULT )) != 0 ) {
    JsVar *e = jswrap_crypto_error_to_jsvar(ret);
    jsExceptionHere(JSET_INTERNALERROR, "HTTPS init failed! mbedtls_ssl_config_defaults returned: %v\n", e );
    jsvUnLock(e);
    ssl_freeSocketData(sckt);
    return false;
  }

  if (sd->pkey.pk_info) {
    // this would get set if options.key was set
    if (( ret = mbedtls_ssl_conf_own_cert(&sd->conf, &sd->owncert, &sd->pkey)) != 0 ) {
      JsVar *e = jswrap_crypto_error_to_jsvar(ret);
      jsExceptionHere(JSET_INTERNALERROR, "HTTPS init failed! mbedtls_ssl_conf_own_cert: %v\n", e );
      jsvUnLock(e);
      ssl_freeSocketData(sckt);
      return false;
    }
  }
  // FIXME no cert checking!
  mbedtls_ssl_conf_authmode( &sd->conf, MBEDTLS_SSL_VERIFY_NONE );
  mbedtls_ssl_conf_ca_chain( &sd->conf, &sd->cacert, NULL );
  mbedtls_ssl_conf_rng( &sd->conf, mbedtls_ctr_drbg_random, &sd->ctr_drbg );
  mbedtls_ssl_conf_dbg( &sd->conf, ssl_debug, 0 );

  if (( ret = mbedtls_ssl_setup( &sd->ssl, &sd->conf )) != 0) {
    JsVar *e = jswrap_crypto_error_to_jsvar(ret);
    jsExceptionHere(JSET_INTERNALERROR, "Failed! mbedtls_ssl_setup: %v\n", e );
    jsvUnLock(e);
    ssl_freeSocketData(sckt);
    return false;
  }

  if (( ret = mbedtls_ssl_set_hostname( &sd->ssl, "mbed TLS Server 1" )) != 0) {
    JsVar *e = jswrap_crypto_error_to_jsvar(ret);
    jsExceptionHere(JSET_INTERNALERROR, "HTTPS init failed! mbedtls_ssl_set_hostname: %v\n", e );
    jsvUnLock(e);
    ssl_freeSocketData(sckt);
    return false;
  }

  mbedtls_ssl_set_bio( &sd->ssl, &sd->sckt, ssl_send, ssl_recv, NULL );

  // jsiConsolePrintf("Performing the SSL/TLS handshake...\n" );

  return true;
}



SSLSocketData *ssl_getSocketData(int sckt) {
  // try and find the socket data variable
  JsVar *ssl = jsvObjectGetChild(execInfo.root, "ssl", 0);
  if (!ssl) return 0;
  JsVar *sslData = jsvGetArrayItem(ssl, sckt);
  jsvUnLock(ssl);
  SSLSocketData *sd = 0;
  if (jsvIsFlatString(sslData))
    sd = (SSLSocketData *)jsvGetFlatStringPointer(sslData);
  jsvUnLock(sslData);

  // now continue with connection
  if (sd->connecting) {
    int ret;

    if ( ( ret = mbedtls_ssl_handshake( &sd->ssl ) ) != 0 ) {
      if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE ) {
        JsVar *e = jswrap_crypto_error_to_jsvar(ret);
        jsExceptionHere(JSET_INTERNALERROR,  "Failed! mbedtls_ssl_handshake returned %v\n", e );
        jsvUnLock(e);
        return 0; // this signals an error
      }
      // else we just continue - connecting=true so other things should wait
    } else {
      // Verify the server certificate
      // jsiConsolePrintf("Verifying peer X.509 certificate...\n");

      /* In real life, we probably want to bail out when ret != 0 */
      uint32_t flags;
      if( ( flags = mbedtls_ssl_get_verify_result( &sd->ssl ) ) != 0 ) {
        char vrfy_buf[512];
        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );
        jsExceptionHere(JSET_INTERNALERROR, "Failed! %s\n", vrfy_buf );
        return 0;
      }
      sd->connecting = false;
    }
  }


  return sd;
}

#endif
// ------------------------------------------------------------------------------

bool netCheckError(JsNetwork *net) {
  return net->checkError(net);
}

int netCreateSocket(JsNetwork *net, uint32_t host, unsigned short port, NetCreateFlags flags, JsVar *options) {
  int sckt = net->createsocket(net, host, port);
  if (sckt<0) return sckt;

#ifdef USE_TLS
  assert(sckt>=0 && sckt<32);
  BITFIELD_SET(socketIsHTTPS, sckt, 0);
  if (flags & NCF_TLS) {
    if (ssl_newSocketData(sckt, options)) {
      BITFIELD_SET(socketIsHTTPS, sckt, 1);
    } else {
      return -1; // fail!
    }
  }
#endif
  return sckt;
}

void netCloseSocket(JsNetwork *net, int sckt) {
#ifdef USE_TLS
  if (BITFIELD_GET(socketIsHTTPS, sckt)) {
    ssl_freeSocketData(sckt);
  }
#endif
  net->closesocket(net, sckt);
}

int netAccept(JsNetwork *net, int sckt) {
  return net->accept(net, sckt);
}

void netGetHostByName(JsNetwork *net, char * hostName, uint32_t* out_ip_addr) {
  net->gethostbyname(net, hostName, out_ip_addr);
}

int netRecv(JsNetwork *net, int sckt, void *buf, size_t len) {
#ifdef USE_TLS
  if (BITFIELD_GET(socketIsHTTPS, sckt)) {
    SSLSocketData *sd = ssl_getSocketData(sckt);
    if (!sd) return -1;
    if (sd->connecting) return 0; // busy

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
#ifdef USE_TLS
  if (BITFIELD_GET(socketIsHTTPS, sckt)) {
    SSLSocketData *sd = ssl_getSocketData(sckt);
    if (!sd) return -1;
    if (sd->connecting) return 0; // busy

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
