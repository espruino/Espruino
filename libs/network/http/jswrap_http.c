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
 * This file is designed to be parsed during the build process
 *
 * Contains JavaScript HTTP Functions
 * ----------------------------------------------------------------------------
 */
#include "jswrap_http.h"
#include "jsvariterator.h"
#include "socketserver.h"

#include "../network.h"

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_http_idle"
}*/
bool jswrap_http_idle() {
  JsNetwork net;
  if (!networkGetFromVar(&net)) return false;
  net.idle(&net);
  bool b = socketIdle(&net);
  networkFree(&net);
  return b;
}

/*JSON{
  "type" : "init",
  "generate" : "jswrap_http_init"
}*/
void jswrap_http_init() {
  socketInit();
}

/*JSON{
  "type" : "kill",
  "generate" : "jswrap_http_kill"
}*/
void jswrap_http_kill() {
  JsNetwork net;
  if (!networkGetFromVar(&net)) return;
  socketKill(&net);
  networkFree(&net);
}


/*JSON{
  "type" : "library",
  "class" : "http"
}
This library allows you to create http servers and make http requests

In order to use this, you will need an extra module to get network connectivity such as the [TI CC3000](/CC3000) or [WIZnet W5500](/WIZnet).

This is designed to be a cut-down version of the [node.js library](http://nodejs.org/api/http.html). Please see the [Internet](/Internet) page for more information on how to use it.
*/

/*JSON{
  "type" : "class",
  "class" : "httpSrv"
}
The HTTP server created by http.createServer
*/
// there is a 'connect' event on httpSrv, but it's used by createServer and isn't node-compliant

/*JSON{
  "type" : "class",
  "class" : "httpSRq"
}
The HTTP server request
*/
/*JSON{
  "type" : "event",
  "class" : "httpSRq",
  "name" : "data",
  "params" : [
    ["data","JsVar","A string containing one or more characters of received data"]
  ]
}
The 'data' event is called when data is received. If a handler is defined with `X.on('data', function(data) { ... })` then it will be called, otherwise data will be stored in an internal buffer, where it can be retrieved with `X.read()`
*/
/*JSON{
  "type" : "event",
  "class" : "httpSRq",
  "name" : "close"
}
Called when the connection closes.
*/
/*JSON{
  "type" : "method",
  "class" : "httpSRq",
  "name" : "available",
  "generate" : "jswrap_stream_available",
  "return" : ["int","How many bytes are available"]
}
Return how many bytes are available to read. If there is already a listener for data, this will always return 0.
*/
/*JSON{
  "type" : "method",
  "class" : "httpSRq",
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
  "class" : "httpSRq",
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
  "type" : "class",
  "class" : "httpSRs"
}
The HTTP server response
*/
/*JSON{
  "type" : "event",
  "class" : "httpSRs",
  "name" : "drain"
}
An event that is fired when the buffer is empty and it can accept more data to send. 
*/
/*JSON{
  "type" : "event",
  "class" : "httpSRs",
  "name" : "close"
}
Called when the connection closes.
*/

/*JSON{
  "type" : "class",
  "class" : "httpCRq"
}
The HTTP client request
*/
/*JSON{
  "type" : "event",
  "class" : "httpCRq",
  "name" : "drain"
}
An event that is fired when the buffer is empty and it can accept more data to send. 
*/

/*JSON{
  "type" : "class",
  "class" : "httpCRs"
}
The HTTP client response
*/
/*JSON{
  "type" : "event",
  "class" : "httpCRs",
  "name" : "data",
  "params" : [
    ["data","JsVar","A string containing one or more characters of received data"]
  ]
}
The 'data' event is called when data is received. If a handler is defined with `X.on('data', function(data) { ... })` then it will be called, otherwise data will be stored in an internal buffer, where it can be retrieved with `X.read()`
*/
/*JSON{
  "type" : "event",
  "class" : "httpCRs",
  "name" : "close"
}
Called when the connection closes.
*/
/*JSON{
  "type" : "method",
  "class" : "httpCRs",
  "name" : "available",
  "generate" : "jswrap_stream_available",
  "return" : ["int","How many bytes are available"]
}
Return how many bytes are available to read. If there is already a listener for data, this will always return 0.
*/
/*JSON{
  "type" : "method",
  "class" : "httpCRs",
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
  "class" : "httpCRs",
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
  "type" : "class",
  "class" : "url"
}
This class helps to convert URLs into Objects of information ready for http.request/get
*/


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "staticmethod",
  "class" : "http",
  "name" : "createServer",
  "generate" : "jswrap_http_createServer",
  "params" : [
    ["callback","JsVar","A function(request,response) that will be called when a connection is made"]
  ],
  "return" : ["JsVar","Returns a new httpSrv object"],
  "return_object" : "httpSrv"
}
Create an HTTP Server

When a request to the server is made, the callback is called. In the callback you can use the methods on the response (httpSRs) to send data. You can also add `request.on('data',function() { ... })` to listen for POSTed data
*/

JsVar *jswrap_http_createServer(JsVar *callback) {
  JsVar *skippedCallback = jsvSkipName(callback);
  if (!jsvIsFunction(skippedCallback)) {
    jsError("Expecting Callback Function but got %t", skippedCallback);
    jsvUnLock(skippedCallback);
    return 0;
  }
  jsvUnLock(skippedCallback);
  return serverNew(callback);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "http",
  "name" : "request",
  "generate" : "jswrap_http_request",
  "params" : [
    ["options","JsVar","An object containing host,port,path,method fields"],
    ["callback","JsVar","A function(res) that will be called when a connection is made. You can then call `res.on('data', function(data) { ... })` and `res.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","Returns a new httpCRq object"],
  "return_object" : "httpCRq"
}
Create an HTTP Request - end() must be called on it to complete the operation
*/

JsVar *jswrap_http_request(JsVar *options, JsVar *callback) {
  bool unlockOptions = false;
  if (jsvIsString(options)) {
    options = jswrap_url_parse(options, false);
    unlockOptions = true;
  }
  if (!jsvIsObject(options)) {
    jsError("Expecting Options to be an Object but it was %t", options);
    return 0;
  }
  JsVar *skippedCallback = jsvSkipName(callback);
  if (!jsvIsFunction(skippedCallback)) {
    jsError("Expecting Callback Function but got %t", skippedCallback);
    jsvUnLock(skippedCallback);
    return 0;
  }
  jsvUnLock(skippedCallback);
  JsVar *rq = clientRequestNew(options, callback);
  if (unlockOptions) jsvUnLock(options);
  return rq;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "http",
  "name" : "get",
  "generate" : "jswrap_http_get",
  "params" : [
    ["options","JsVar","An object containing host,port,path,method fields"],
    ["callback","JsVar","A function(res) that will be called when a connection is made. You can then call `res.on('data', function(data) { ... })` and `res.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","Returns a new httpCRq object"],
  "return_object" : "httpCRq"
}
Create an HTTP Request - convenience function for ```http.request()```. `options.method` is set to 'get', and end is called automatically. See [the Internet page](/Internet) for more usage examples.
*/
JsVar *jswrap_http_get(JsVar *options, JsVar *callback) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return 0;

  if (jsvIsObject(options)) {
    // if options is a string - it will be parsed, and GET will be set automatically
    JsVar *method = jsvNewFromString("GET");
    jsvUnLock(jsvAddNamedChild(options, method, "method"));
    jsvUnLock(method);
  }
  JsVar *skippedCallback = jsvSkipName(callback);
  if (!jsvIsUndefined(skippedCallback) && !jsvIsFunction(skippedCallback)) {
    jsError("Expecting Callback Function but got %t", skippedCallback);
    jsvUnLock(skippedCallback);
    return 0;
  }
  jsvUnLock(skippedCallback);
  JsVar *cliReq = jswrap_http_request(options, callback);
  if (cliReq) clientRequestEnd(&net, cliReq);
  networkFree(&net);
  return cliReq;
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "method",
  "class" : "httpSrv",
  "name" : "listen",
  "generate" : "jswrap_httpSrv_listen",
  "params" : [
    ["port","int32","The port to listen on"]
  ]
}
Start listening for new HTTP connections on the given port
*/

void jswrap_httpSrv_listen(JsVar *parent, int port) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  serverListen(&net, parent, port);
  networkFree(&net);
}

/*JSON{
  "type" : "method",
  "class" : "httpSrv",
  "name" : "close",
  "generate" : "jswrap_httpSrv_close"
}
Stop listening for new HTTP connections
*/

void jswrap_httpSrv_close(JsVar *parent) {
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
  "class" : "httpSRs",
  "name" : "write",
  "generate" : "jswrap_httpSRs_write",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ],
  "return" : ["bool","For note compatibility, the boolean false. When the send buffer is empty, a `drain` event will be sent"]
}*/
bool jswrap_httpSRs_write(JsVar *parent, JsVar *data) {
  serverResponseData(parent, data);
  return false;
}

/*JSON{
  "type" : "method",
  "class" : "httpSRs",
  "name" : "end",
  "generate" : "jswrap_httpSRs_end",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ]
}*/
void jswrap_httpSRs_end(JsVar *parent, JsVar *data) {
  if (!jsvIsUndefined(data)) jswrap_httpSRs_write(parent, data);
  serverResponseEnd(parent);
}


/*JSON{
  "type" : "method",
  "class" : "httpSRs",
  "name" : "writeHead",
  "generate" : "jswrap_httpSRs_writeHead",
  "params" : [
    ["statusCode","int32","The HTTP status code"],
    ["headers","JsVar","An object containing the headers"]
  ]
}*/
void jswrap_httpSRs_writeHead(JsVar *parent, int statusCode, JsVar *headers) {
  serverResponseWriteHead(parent, statusCode, headers);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "method",
  "class" : "httpCRq",
  "name" : "write",
  "generate" : "jswrap_httpCRq_write",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ],
  "return" : ["bool","For note compatibility, the boolean false. When the send buffer is empty, a `drain` event will be sent"]
}*/
bool jswrap_httpCRq_write(JsVar *parent, JsVar *data) {
  clientRequestWrite(parent, data);
  return false;
}

/*JSON{
  "type" : "method",
  "class" : "httpCRq",
  "name" : "end",
  "generate" : "jswrap_httpCRq_end",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ]
}
Finish this HTTP request - optional data to append as an argument
*/
void jswrap_httpCRq_end(JsVar *parent, JsVar *data) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  if (!jsvIsUndefined(data)) jswrap_httpCRq_write(parent, data);
  clientRequestEnd(&net, parent);
  networkFree(&net);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
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

For instance `url.parse("/a?b=c&d=e",true)` returns `{"method":"GET","host":"","path":"/a?b=c&d=e","pathname":"/a","search":"?b=c&d=e","port":80,"query":{"b":"c","d":"e"}}`
*/
JsVar *jswrap_url_parse(JsVar *url, bool parseQuery) {
  if (!jsvIsString(url)) return 0;
  JsVar *obj = jsvNewWithFlags(JSV_OBJECT);
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
    char ch = jsvStringIteratorGetChar(&it);
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

    jsvStringIteratorNext(&it);
    charIdx++;
  }
  jsvStringIteratorFree(&it);
  // try and sort stuff out
  if (pathStart<0) pathStart = charIdx;
  if (pathStart<0) pathStart = charIdx;
  int addrEnd = (portStart>=0) ? portStart : pathStart;
  // pull out details
  jsvUnLock(jsvObjectSetChild(obj, "method", jsvNewFromString("GET")));
  jsvUnLock(jsvObjectSetChild(obj, "host", jsvNewFromStringVar(url, (size_t)(addrStart+1), (size_t)(addrEnd-(addrStart+1)))));

  JsVar *v;

  v = jsvNewFromStringVar(url, (size_t)pathStart, JSVAPPENDSTRINGVAR_MAXLENGTH);
  if (jsvGetStringLength(v)==0) jsvAppendString(v, "/");
  jsvUnLock(jsvObjectSetChild(obj, "path", v));

  v = jsvNewFromStringVar(url, (size_t)pathStart, (size_t)((searchStart>=0)?(searchStart-pathStart):JSVAPPENDSTRINGVAR_MAXLENGTH));
  if (jsvGetStringLength(v)==0) jsvAppendString(v, "/");
  jsvUnLock(jsvObjectSetChild(obj, "pathname", v));

  jsvUnLock(jsvObjectSetChild(obj, "search", (searchStart>=0)?jsvNewFromStringVar(url, (size_t)searchStart, JSVAPPENDSTRINGVAR_MAXLENGTH):jsvNewNull()));

  if (portNumber<=0 || portNumber>65535) portNumber=80;
  jsvUnLock(jsvObjectSetChild(obj, "port", jsvNewFromInteger(portNumber)));

  JsVar *query = (searchStart>=0)?jsvNewFromStringVar(url, (size_t)(searchStart+1), JSVAPPENDSTRINGVAR_MAXLENGTH):jsvNewNull();
  if (parseQuery && !jsvIsNull(query)) {
    JsVar *queryStr = query;
    jsvStringIteratorNew(&it, query, 0);
    query = jsvNewWithFlags(JSV_OBJECT);

    JsVar *key = jsvNewFromEmptyString();
    JsVar *val = jsvNewFromEmptyString();
    bool hadEquals = false;

    while (jsvStringIteratorHasChar(&it)) {
      char ch = jsvStringIteratorGetChar(&it);
      if (ch=='&') {
        if (jsvGetStringLength(key)>0 || jsvGetStringLength(val)>0) {
          key = jsvAsArrayIndexAndUnLock(key); // make sure "0" gets made into 0
          jsvMakeIntoVariableName(key, val);
          jsvAddName(query, key);
          jsvUnLock(key);
          jsvUnLock(val);
          key = jsvNewFromEmptyString();
          val = jsvNewFromEmptyString();
          hadEquals = false;
        }
      } else if (!hadEquals && ch=='=') {
        hadEquals = true;
      } else {
        // decode percent escape chars
        if (ch=='%') {
          jsvStringIteratorNext(&it);
          ch = jsvStringIteratorGetChar(&it);
          jsvStringIteratorNext(&it);
          ch = (char)((chtod(ch)<<4) | chtod(jsvStringIteratorGetChar(&it)));
        }

        if (hadEquals) jsvAppendCharacter(val, ch);
        else jsvAppendCharacter(key, ch);
      }
      jsvStringIteratorNext(&it);
      charIdx++;
    }
    jsvStringIteratorFree(&it);
    jsvUnLock(queryStr);

    if (jsvGetStringLength(key)>0 || jsvGetStringLength(val)>0) {
      key = jsvAsArrayIndexAndUnLock(key); // make sure "0" gets made into 0
      jsvMakeIntoVariableName(key, val);
      jsvAddName(query, key);
    }
    jsvUnLock(key);
    jsvUnLock(val);
  }
  jsvUnLock(jsvObjectSetChild(obj, "query", query));

  return obj;
}


