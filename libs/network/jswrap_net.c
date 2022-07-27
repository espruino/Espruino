/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2014 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript Net (socket) functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_net.h"
#include "jsvariterator.h"
#include "jsinteractive.h"
#include "jsparse.h"
#include "socketserver.h"
#include "network.h"

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_net_idle"
}*/
bool jswrap_net_idle() {
  JsNetwork net;
  if (!networkGetFromVar(&net)) return false;
  net.idle(&net);
  bool b = socketIdle(&net);
  networkFree(&net);
  return b;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_net_init"
}*/
void jswrap_net_init() {
  socketInit();
}

/*JSON{
  "type"     : "kill",
  "generate" : "jswrap_net_kill"
}*/
void jswrap_net_kill() {
  JsNetwork net;
  if (networkWasCreated()) {
    if (!networkGetFromVar(&net)) return;
    socketKill(&net);
    networkFree(&net);
  }
}


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------

/*JSON{
  "type" : "class",
  "class" : "url"
}
This class helps to convert URLs into Objects of information ready for
http.request/get
*/


/*JSON{
  "type" : "staticmethod",
  "class" : "url",
  "name" : "parse",
  "generate" : "jswrap_url_parse",
  "params" : [
    ["urlStr","JsVar","A URL to be parsed"],
    ["parseQuery","bool","Whether to parse the query string into an object not (default = false)"]
  ],
  "return" : ["JsVar","An object containing options for ```http.request``` or ```http.get```. Contains `method`, `host`, `path`, `pathname`, `search`, `port` and `query`"]
}
A utility function to split a URL into parts

This is useful in web servers for instance when handling a request.

For instance `url.parse("/a?b=c&d=e",true)` returns
`{"method":"GET","host":"","path":"/a?b=c&d=e","pathname":"/a","search":"?b=c&d=e","port":80,"query":{"b":"c","d":"e"}}`
*/
JsVar *jswrap_url_parse(JsVar *url, bool parseQuery) {
  if (!jsvIsString(url)) return 0;
  JsVar *obj = jsvNewObject();
  if (!obj) return 0; // out of memory

  // scan string to try and pick stuff out
  JsvStringIterator it;
  jsvStringIteratorNew(&it, url, 0);
  int slashes = 0;
  int colons = 0;
  int addrStart = -1;
  int portStart = -1;
  int pathStart = -1;
  int searchStart = -1;
  int charIdx = 0;
  int portNumber = 0;
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetCharAndNext(&it);
    if (ch == '/') {
      slashes++;
      if (pathStart<0) pathStart = charIdx;
      if (colons==1 && slashes==2 && addrStart<0) {
        addrStart = charIdx;
        pathStart = -1;
        searchStart = -1;
      }
    }
    if (ch == ':') {
      colons++;
      if (addrStart>=0 && pathStart<0)
        portStart = charIdx;
    }

    if (portStart>=0 && charIdx>portStart && pathStart<0 && ch >= '0' && ch <= '9') {
      portNumber = portNumber*10 + (ch-'0');
    }

    if (ch == '?' && pathStart>=0) {
      searchStart = charIdx;
    }
    charIdx++;
  }
  jsvStringIteratorFree(&it);
  // try and sort stuff out
  if (pathStart<0) pathStart = charIdx;
  if (pathStart<0) pathStart = charIdx;
  int addrEnd = (portStart>=0) ? portStart : pathStart;
  // pull out details
  if (addrStart>0)
    jsvObjectSetChildAndUnLock(obj, "protocol", jsvNewFromStringVar(url, 0, (size_t)addrStart-1));
  jsvObjectSetChildAndUnLock(obj, "method", jsvNewFromString("GET"));
  jsvObjectSetChildAndUnLock(obj, "host", jsvNewFromStringVar(url, (size_t)(addrStart+1), (size_t)(addrEnd-(addrStart+1))));

  JsVar *v;

  v = jsvNewFromStringVar(url, (size_t)pathStart, JSVAPPENDSTRINGVAR_MAXLENGTH);
  if (jsvGetStringLength(v)==0) jsvAppendString(v, "/");
  jsvObjectSetChildAndUnLock(obj, "path", v);

  v = jsvNewFromStringVar(url, (size_t)pathStart, (size_t)((searchStart>=0)?(searchStart-pathStart):JSVAPPENDSTRINGVAR_MAXLENGTH));
  if (jsvGetStringLength(v)==0) jsvAppendString(v, "/");
  jsvObjectSetChildAndUnLock(obj, "pathname", v);

  jsvObjectSetChildAndUnLock(obj, "search", (searchStart>=0)?jsvNewFromStringVar(url, (size_t)searchStart, JSVAPPENDSTRINGVAR_MAXLENGTH):jsvNewNull());

  jsvObjectSetChildAndUnLock(obj, "port", (portNumber<=0 || portNumber>65535) ? jsvNewWithFlags(JSV_NULL) : jsvNewFromInteger(portNumber));

  JsVar *query = (searchStart>=0)?jsvNewFromStringVar(url, (size_t)(searchStart+1), JSVAPPENDSTRINGVAR_MAXLENGTH):jsvNewNull();
  if (parseQuery && !jsvIsNull(query)) {
    JsVar *queryStr = query;
    jsvStringIteratorNew(&it, query, 0);
    query = jsvNewObject();

    JsVar *key = jsvNewFromEmptyString();
    JsVar *val = jsvNewFromEmptyString();
    bool hadEquals = false;

    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetCharAndNext(&it);
      if (ch=='&') {
        if (jsvGetStringLength(key)>0 || jsvGetStringLength(val)>0) {
          key = jsvAsArrayIndexAndUnLock(key); // make sure "0" gets made into 0
          jsvMakeIntoVariableName(key, val);
          jsvAddName(query, key);
          jsvUnLock2(key, val);
          key = jsvNewFromEmptyString();
          val = jsvNewFromEmptyString();
          hadEquals = false;
        }
      } else if (!hadEquals && ch=='=') {
        hadEquals = true;
      } else {
        // decode percent escape chars
        if (ch=='%') {
          char hi = jsvStringIteratorGetCharAndNext(&it);
          char lo = jsvStringIteratorGetCharAndNext(&it);
          ch = (char)hexToByte(hi,lo);
        }

        if (hadEquals) jsvAppendCharacter(val, ch);
        else jsvAppendCharacter(key, ch);
      }
      charIdx++;
    }
    jsvStringIteratorFree(&it);
    jsvUnLock(queryStr);

    if (jsvGetStringLength(key)>0 || jsvGetStringLength(val)>0) {
      key = jsvAsArrayIndexAndUnLock(key); // make sure "0" gets made into 0
      jsvMakeIntoVariableName(key, val);
      jsvAddName(query, key);
    }
    jsvUnLock2(key, val);
  }
  jsvObjectSetChildAndUnLock(obj, "query", query);

  return obj;
}


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------


/*JSON{
  "type" : "library",
  "class" : "net"
}
This library allows you to create TCPIP servers and clients

In order to use this, you will need an extra module to get network connectivity.

This is designed to be a cut-down version of the [node.js
library](http://nodejs.org/api/net.html). Please see the [Internet](/Internet)
page for more information on how to use it.
*/

/*JSON{
  "type" : "class",
  "library" : "net",
  "class" : "Server"
}
The socket server created by `require('net').createServer`
*/
/*JSON{
  "type" : "class",
  "library" : "net",
  "class" : "Socket"
}
An actual socket connection - allowing transmit/receive of TCP data
*/
/*JSON{
  "type" : "event",
  "class" : "Socket",
  "name" : "data",
  "params" : [
    ["data","JsVar","A string containing one or more characters of received data"]
  ]
}
The 'data' event is called when data is received. If a handler is defined with
`X.on('data', function(data) { ... })` then it will be called, otherwise data
will be stored in an internal buffer, where it can be retrieved with `X.read()`
*/
/*JSON{
  "type" : "event",
  "class" : "Socket",
  "name" : "close",
  "params" : [
    ["had_error","JsVar","A boolean indicating whether the connection had an error (use an error event handler to get error details)."]
  ]
}
Called when the connection closes.
*/
/*JSON{
  "type" : "event",
  "class" : "Socket",
  "name" : "error",
  "params" : [
    ["details","JsVar","An error object with an error code (a negative integer) and a message."]
  ]
}
There was an error on this socket and it is closing (or wasn't opened in the
first place). If a "connected" event was issued on this socket then the error
event is always followed by a close event. The error codes are:

* -1: socket closed (this is not really an error and will not cause an error
  callback)
* -2: out of memory (typically while allocating a buffer to hold data)
* -3: timeout
* -4: no route
* -5: busy
* -6: not found (DNS resolution)
* -7: max sockets (... exceeded)
* -8: unsent data (some data could not be sent)
* -9: connection reset (or refused)
* -10: unknown error
* -11: no connection
* -12: bad argument
* -13: SSL handshake failed
* -14: invalid SSL data

*/
/*JSON{
  "type" : "method",
  "class" : "Socket",
  "name" : "available",
  "generate" : "jswrap_stream_available",
  "return" : ["int","How many bytes are available"]
}
Return how many bytes are available to read. If there is already a listener for
data, this will always return 0.
*/
/*JSON{
  "type" : "method",
  "class" : "Socket",
  "name" : "read",
  "generate" : "jswrap_stream_read",
  "params" : [
    ["chars","int","The number of characters to read, or undefined/0 for all available"]
  ],
  "return" : ["JsVar","A string containing the required bytes."]
}
Return a string containing characters that have been received
*/
/*JSON{
  "type" : "method",
  "class" : "Socket",
  "name" : "pipe",
  "ifndef" : "SAVE_ON_FLASH",
  "generate" : "jswrap_pipe",
  "params" : [
    ["destination","JsVar","The destination file/stream that will receive content from the source."],
    ["options","JsVar",["An optional object `{ chunkSize : int=32, end : bool=true, complete : function }`","chunkSize : The amount of data to pipe from source to destination at a time","complete : a function to call when the pipe activity is complete","end : call the 'end' function on the destination when the source is finished"]]
  ]
}
Pipe this to a stream (an object with a 'write' method)
*/
/*JSON{
  "type" : "event",
  "class" : "Socket",
  "name" : "drain"
}
An event that is fired when the buffer is empty and it can accept more data to
send.
*/



// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "staticmethod",
  "class" : "net",
  "name" : "createServer",
  "generate" : "jswrap_net_createServer",
  "params" : [
    ["callback","JsVar","A `function(connection)` that will be called when a connection is made"]
  ],
  "return" : ["JsVar","Returns a new Server Object"],
  "return_object" : "Server"
}
Create a Server

When a request to the server is made, the callback is called. In the callback
you can use the methods on the connection to send data. You can also add
`connection.on('data',function() { ... })` to listen for received data
*/

JsVar *jswrap_net_createServer(JsVar *callback) {
  JsVar *skippedCallback = jsvSkipName(callback);
  if (!jsvIsFunction(skippedCallback)) {
    jsError("Expecting Callback Function but got %t", skippedCallback);
    jsvUnLock(skippedCallback);
    return 0;
  }
  jsvUnLock(skippedCallback);
  return serverNew(ST_NORMAL, callback);
}



/*JSON{
  "type" : "staticmethod",
  "class" : "net",
  "name" : "connect",
  "generate_full" : "jswrap_net_connect(options, callback, ST_NORMAL)",
  "params" : [
    ["options","JsVar","An object containing host,port fields"],
    ["callback","JsVar","A `function(sckt)` that will be called  with the socket when a connection is made. You can then call `sckt.write(...)` to send data, and `sckt.on('data', function(data) { ... })` and `sckt.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","Returns a new net.Socket object"],
  "return_object" : "Socket"
}
Create a TCP socket connection
*/
JsVar *jswrap_net_connect(JsVar *options, JsVar *callback, SocketType socketType) {
  bool unlockOptions = false;
  if (jsvIsString(options)) {
    options = jswrap_url_parse(options, false);
    unlockOptions = true;
  }
  if (!jsvIsObject(options)) {
    jsError("Expecting Options to be an Object but it was %t", options);
    return 0;
  }
#ifdef USE_TLS
  if ((socketType&ST_TYPE_MASK) == ST_HTTP) {
    JsVar *protocol = jsvObjectGetChild(options, "protocol", 0);
    if (protocol && jsvIsStringEqual(protocol, "https:")) {
      socketType |= ST_TLS;
    }
    jsvUnLock(protocol);
  }
#endif

  // Make sure we have a function as callback, or nothing (which is OK too)
  if (!jsvIsUndefined(callback) && !jsvIsFunction(callback)) {
    jsError("Expecting Callback Function but got %t", callback);
    return 0;
  }

  if ((socketType&ST_TYPE_MASK) == ST_UDP) {
    JsNetwork net;
    if (networkGetFromVar(&net)) {
      int recvBufferSize = jsvGetIntegerAndUnLock(jsvObjectGetChild(options, "recvBufferSize", 0));
      if (recvBufferSize > net.data.recvBufferSize) {
        net.data.recvBufferSize = recvBufferSize;
        networkSet(&net);
      }
      networkFree(&net);
    }
  }

  JsVar *rq = clientRequestNew(socketType, options, callback);
  if (unlockOptions) jsvUnLock(options);

  if ((socketType&ST_TYPE_MASK) == ST_NORMAL) {
    JsNetwork net;
    if (networkGetFromVarIfOnline(&net)) {
      clientRequestConnect(&net, rq);
      networkFree(&net);
    }
  }

  return rq;
}

/*JSON{
  "type" : "library",
  "class" : "dgram"
}
This library allows you to create UDP/DATAGRAM servers and clients

In order to use this, you will need an extra module to get network connectivity.

This is designed to be a cut-down version of the [node.js
library](http://nodejs.org/api/dgram.html). Please see the [Internet](/Internet)
page for more information on how to use it.
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "dgram",
  "name" : "createSocket",
  "generate_full" : "jswrap_dgram_createSocket(type, callback)",
  "params" : [
    ["type","JsVar","Socket type to create e.g. 'udp4'. Or options object { type: 'udp4', reuseAddr: true, recvBufferSize: 1024 }"],
    ["callback","JsVar","A `function(sckt)` that will be called  with the socket when a connection is made. You can then call `sckt.send(...)` to send data, and `sckt.on('message', function(data) { ... })` and `sckt.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","Returns a new dgram.Socket object"],
  "return_object" : "dgramSocket"
}
Create a UDP socket
*/
JsVar *jswrap_dgram_createSocket(JsVar *type, JsVar *callback) {
  NOT_USED(type);
  JsVar *options = jsvIsObject(type) ? type : jsvNewObject();
  JsVar *connection = jswrap_net_connect(options, callback, ST_UDP);
  if (options != type) jsvUnLock(options);
  return connection;
}

/*JSON{
  "type" : "class",
  "library" : "dgram",
  "class" : "dgramSocket"
}
An actual socket connection - allowing transmit/receive of TCP data
*/
/*JSON{
  "type" : "method",
  "class" : "dgramSocket",
  "name" : "send",
  "generate" : "jswrap_dgram_socket_send",
  "params" : [
    ["buffer","JsVar","A string containing message to send"],
    ["offset","JsVar","Offset in the passed string where the message starts [optional]"],
    ["length","JsVar","Number of bytes in the message [optional]"],
    ["args","JsVarArray","Destination port number, Destination IP address string"]
  ]
}*/
// There are futher arguments within the 'args' JsVarArray:
//  ["port","JsVar","Destination port number to send the message to"],
//  ["address","JsVar","Destination hostname or IP address string"]
void jswrap_dgram_socket_send(JsVar *parent, JsVar *buffer, JsVar *offset, JsVar *length, JsVar *args) {
  assert(jsvIsObject(parent));
  assert(jsvIsString(buffer));
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  JsVar *msg;
  JsVar *address;
  JsVar *port = jsvGetArrayItem(args, 0);
  if (jsvIsNumeric(port)) {
    int from = jsvGetInteger(offset);
    int count = jsvGetInteger(length);
    msg = jsvNewFromEmptyString();
    if (!msg) {
      networkFree(&net);
      return; // out of memory
    }
    jsvAppendStringVar(msg, buffer, (size_t)from, (size_t)count);
    address = jsvGetArrayItem(args, 1);
  } else {
    jsvUnLock(port);

    msg = buffer;
    port = offset;
    address = length;
    assert(jsvIsNumeric(port));
  }
  clientRequestWrite(&net, parent, msg, address, (unsigned short)jsvGetInteger(port));
  if (msg != buffer) {
    jsvUnLock3(msg, port, address);
  }
  networkFree(&net);
}

/*JSON{
  "type" : "event",
  "class" : "dgramSocket",
  "name" : "message",
  "params" : [
    ["msg","JsVar","A string containing the received message"],
    ["rinfo","JsVar","Sender address,port containing information"]
  ]
}
The 'message' event is called when a datagram message is received. If a handler
is defined with `X.on('message', function(msg) { ... })` then it will be called`
*/

/*JSON{
  "type" : "method",
  "class" : "dgramSocket",
  "name" : "bind",
  "generate" : "jswrap_dgramSocket_bind",
  "params" : [
    ["port","int32","The port to bind at"],
    ["callback","JsVar","A function(res) that will be called when the socket is bound. You can then call `res.on('message', function(message, info) { ... })` and `res.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","The dgramSocket instance that 'bind' was called on"]
}
*/
JsVar *jswrap_dgramSocket_bind(JsVar *parent, unsigned short port, JsVar *callback) {
  parent = jsvLockAgain(parent); // we're returning the parent, so need to re-lock it

  jsvObjectSetChild(parent, DGRAM_ON_BIND_NAME, callback);
  jsvUnLock(jswrap_net_server_listen(parent, port, ST_UDP)); // create bound socket
  jsiQueueObjectCallbacks(parent, DGRAM_ON_BIND_NAME, &parent, 1);

  return parent;
}

/*JSON{
  "type" : "method",
  "class" : "dgramSocket",
  "name" : "close",
  "generate" : "jswrap_dgram_close"
}
Close the socket
*/
void jswrap_dgram_close(JsVar *parent) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  clientRequestEnd(&net, parent);
  networkFree(&net);
}

/*JSON{
  "type" : "method",
  "class" : "dgramSocket",
  "name" : "addMembership",
  "generate" : "jswrap_dgram_addMembership",
  "params" : [
    ["group","JsVar","A string containing the group ip to join"],
    ["ip","JsVar","A string containing the ip to join with"]
  ]
}
*/
void jswrap_dgram_addMembership(JsVar *parent, JsVar *group, JsVar *ip) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  serverAddMembership(&net, parent, group, ip);
}

/*JSON{
  "type" : "event",
  "class" : "dgramSocket",
  "name" : "close",
  "params" : [
    ["had_error","JsVar","A boolean indicating whether the connection had an error (use an error event handler to get error details)."]
  ]
}
Called when the connection closes.
*/


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------

/*JSON{
  "type" : "library",
  "class" : "tls",
  "ifdef" : "USE_TLS"
}
This library allows you to create TCPIP servers and clients using TLS encryption

In order to use this, you will need an extra module to get network connectivity.

This is designed to be a cut-down version of the [node.js
library](http://nodejs.org/api/tls.html). Please see the [Internet](/Internet)
page for more information on how to use it.
*/

/*JSON{
  "type" : "staticmethod",
  "class" : "tls",
  "name" : "connect",
  "generate_full" : "jswrap_net_connect(options, callback, ST_NORMAL | ST_TLS)",
  "params" : [
    ["options","JsVar","An object containing host,port fields"],
    ["callback","JsVar","A function(res) that will be called when a connection is made. You can then call `res.on('data', function(data) { ... })` and `res.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","Returns a new net.Socket object"],
  "return_object" : "Socket",
  "ifdef" : "USE_TLS"
}
Create a socket connection using TLS

Options can have `ca`, `key` and `cert` fields, which should be the decoded
content of the certificate.

```
var options = url.parse("localhost:1234");
options.key = atob("MIIJKQ ... OZs08C");
options.cert = atob("MIIFi ... Uf93rN+");
options.ca = atob("MIIFgDCC ... GosQML4sc=");
require("tls").connect(options, ... );
```

If you have the certificates as `.pem` files, you need to load these files, take
the information between the lines beginning with `----`, remove the newlines
from it so you have raw base64, and then feed it into `atob` as above.

You can also:
* Just specify the filename (<=100 characters) and it will be loaded and parsed
  if you have an SD card connected. For instance `options.key = "key.pem";`
* Specify a function, which will be called to retrieve the data. For instance
  `options.key = function() { eeprom.load_my_info(); };

For more information about generating and using certificates, see:

https://engineering.circle.com/https-authorized-certs-with-node-js/

(You'll need to use 2048 bit certificates as opposed to 4096 bit shown above)
*/

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------

/*JSON{
  "type" : "method",
  "class" : "Server",
  "name" : "listen",
  "generate_full" : "jswrap_net_server_listen(parent, port, ST_NORMAL)",
  "params" : [
    ["port","int32","The port to listen on"]
  ],
  "return" : ["JsVar","The HTTP server instance that 'listen' was called on"]
}
Start listening for new connections on the given port
*/

JsVar *jswrap_net_server_listen(JsVar *parent, int port, SocketType socketType) {
  parent = jsvLockAgain(parent); // we're returning the parent, so need to re-lock it
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return parent;

  serverListen(&net, parent, (unsigned short)port, socketType);
  networkFree(&net);
  return parent;
}

/*JSON{
  "type" : "method",
  "class" : "Server",
  "name" : "close",
  "generate" : "jswrap_net_server_close"
}
Stop listening for new connections
*/

void jswrap_net_server_close(JsVar *parent) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  serverClose(&net, parent);
  networkFree(&net);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "method",
  "class" : "Socket",
  "name" : "write",
  "generate" : "jswrap_net_socket_write",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ],
  "return" : ["bool","For node.js compatibility, returns the boolean false. When the send buffer is empty, a `drain` event will be sent"]
}
This function writes the `data` argument as a string. Data that is passed in
(including arrays) will be converted to a string with the normal JavaScript
`toString` method.

If you wish to send binary data then you need to convert that data directly to a
String. This can be done with `String.fromCharCode`, however it's often easier
and faster to use the Espruino-specific `E.toString`, which will read its
arguments as an array of bytes and convert that to a String:

```
socket.write(E.toString([0,1,2,3,4,5]));
```

If you need to send something other than bytes, you can use 'Typed Arrays', or
even `DataView`:

```
var d = new DataView(new ArrayBuffer(8)); // 8 byte array buffer
d.setFloat32(0, 765.3532564); // write float at bytes 0-3
d.setInt8(4, 42); // write int8 at byte 4
socket.write(E.toString(d.buffer))
```
*/
bool jswrap_net_socket_write(JsVar *parent, JsVar *data) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return false;
  clientRequestWrite(&net, parent, data, NULL, 0);
  networkFree(&net);
  return false;
}

/*JSON{
  "type" : "method",
  "class" : "Socket",
  "name" : "end",
  "generate" : "jswrap_net_socket_end",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ]
}
Close this socket - optional data to append as an argument.

See `Socket.write` for more information about the data argument
*/
void jswrap_net_socket_end(JsVar *parent, JsVar *data) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  if (!jsvIsUndefined(data)) jswrap_net_socket_write(parent, data);
  clientRequestEnd(&net, parent);
  networkFree(&net);
}
