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
#include "httpserver.h"
#include "../network.h"

/*

http.createServer(function (req, res) {
  console.log("Connected");
  res.writeHead(200, {'Content-Type': 'text/plain'});
  res.end('Hello World\n');
}).listen(8080);

 */

/*JSON{ "type":"idle", "generate" : "jswrap_http_idle" }*/
bool jswrap_http_idle() {
  JsNetwork net;
  if (!networkGetFromVar(&net)) return false;
  bool b = httpIdle(&net);
  networkFree(&net);
  return b;
}

/*JSON{ "type":"init", "generate" : "jswrap_http_init" }*/
void jswrap_http_init() {
  httpInit();
}

/*JSON{ "type":"kill", "generate" : "jswrap_http_kill" }*/
void jswrap_http_kill() {
  JsNetwork net;
  if (!networkGetFromVar(&net)) return;
  httpKill(&net);
  networkFree(&net);
}


/*JSON{ "type":"library",
        "class" : "http",
        "description" : [
                         "This library allows you to create http servers and make http requests",
                         "NOTE: This is currently only available in the Raspberry Pi version",
                         "This is a cut-down version of node.js's library",
                         "Please see http://nodemanual.org/latest/nodejs_ref_guide/http.html",
                         "To use this, you must type ```var http = require('http')``` to get access to the library",
                         "NOTE: The HTTP client + server send in ~8 byte chunks. This is normally fine but big servers - eg. Google will reject requests made like this (DDoS protection?)"
                          ]
}*/
/*JSON{ "type":"class",
        "class" : "httpSrv",
        "description" : ["The HTTP server created by http.createServer" ]
}*/
/*JSON{ "type":"class",
        "class" : "httpSRq",
        "description" : ["The HTTP server request" ]
}*/
/*JSON{ "type":"class",
        "class" : "httpSRs",
        "description" : ["The HTTP server response" ]
}*/
/*JSON{ "type":"class",
        "class" : "httpCRq",
        "description" : ["The HTTP client request" ]
}*/
/*JSON{ "type":"class",
        "class" : "httpCRs",
        "description" : ["The HTTP client response" ]
}*/
/*JSON{ "type":"class",
        "class" : "url",
        "description" : ["This class helps to convert URLs into Objects of information ready for http.request/get" ]
}*/


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"staticmethod",
         "class" : "http", "name" : "createServer",
         "generate" : "jswrap_http_createServer",
         "description" : ["Create an HTTP Server", "When a request to the server is made, the callback is called. In the callback you can use the methods on the response (httpSRs) to send data. You can also add `request.on('data',function() { ... })` to listen for POSTed data" ],
         "params" : [ [ "callback", "JsVar", "A function(request,response) that will be called when a connection is made"] ],
         "return" : ["JsVar", "Returns a new httpSrv object"]
}*/

JsVar *jswrap_http_createServer(JsVar *callback) {
  JsVar *skippedCallback = jsvSkipName(callback);
  if (!jsvIsFunction(skippedCallback)) {
    jsError("Expecting Callback Function but got %t", skippedCallback);
    jsvUnLock(skippedCallback);
    return 0;
  }
  jsvUnLock(skippedCallback);
  return httpServerNew(callback);
}

/*JSON{ "type":"staticmethod",
         "class" : "http", "name" : "request",
         "generate" : "jswrap_http_request",
         "description" : ["Create an HTTP Request - end() must be called on it to complete the operation" ],
         "params" : [  [ "options", "JsVar", "An object containing host,port,path,method fields"],
                       [ "callback", "JsVar", "A function(res) that will be called when a connection is made"] ],
         "return" : ["JsVar", "Returns a new httpCRq object"]
}*/

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
  JsVar *rq = httpClientRequestNew(options, callback);
  if (unlockOptions) jsvUnLock(options);
  return rq;
}

/*JSON{ "type":"staticmethod",
         "class" : "http", "name" : "get",
         "generate" : "jswrap_http_get",
         "description" : ["Create an HTTP Request - convenience function for ```http.request()```. options.method is set to 'get', and end is called automatically" ],
         "params" : [  [ "options", "JsVar", "An object containing host,port,path,method fields"],
                       [ "callback", "JsVar", "A function(res) that will be called when a connection is made"] ],
         "return" : ["JsVar", "Returns a new httpCRq object"]
}*/
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
  httpClientRequestEnd(&net, cliReq);
  networkFree(&net);
  return cliReq;
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"method",
         "class" : "httpSrv", "name" : "listen",
         "description" : [ "Start listening for new HTTP connections on the given port" ],
         "generate" : "jswrap_httpSrv_listen",
         "params" : [ [ "port", "int32", "The port to listen on"] ]
}*/

void jswrap_httpSrv_listen(JsVar *parent, int port) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  httpServerListen(&net, parent, port);
  networkFree(&net);
}

/*JSON{ "type":"method",
         "class" : "httpSrv", "name" : "close",
         "description" : [ "Stop listening for new HTTP connections" ],
         "generate" : "jswrap_httpSrv_close"
}*/

void jswrap_httpSrv_close(JsVar *parent) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  httpServerClose(&net, parent);
  networkFree(&net);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"method",
         "class" : "httpSRs", "name" : "write",
         "generate" : "jswrap_httpSRs_write",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ],
         "return" : ["bool", "For note compatibility, the boolean false. When the send buffer is empty, a `drain` event will be sent" ]
}*/
bool jswrap_httpSRs_write(JsVar *parent, JsVar *data) {
  httpServerResponseData(parent, data);
  return false;
}

/*JSON{ "type":"method",
         "class" : "httpSRs", "name" : "end",
         "generate" : "jswrap_httpSRs_end",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ]
}*/
void jswrap_httpSRs_end(JsVar *parent, JsVar *data) {
  if (!jsvIsUndefined(data)) jswrap_httpSRs_write(parent, data);
  httpServerResponseEnd(parent);
}


/*JSON{ "type":"method",
         "class" : "httpSRs", "name" : "writeHead",
         "generate" : "jswrap_httpSRs_writeHead",
         "params" : [ [ "statusCode", "int32", "The HTTP status code"],
                      [ "headers", "JsVar", "An object containing the headers"] ]
}*/
void jswrap_httpSRs_writeHead(JsVar *parent, int statusCode, JsVar *headers) {
  httpServerResponseWriteHead(parent, statusCode, headers);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"method",
         "class" : "httpCRq", "name" : "write",
         "generate" : "jswrap_httpCRq_write",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ],
         "return" : ["bool", "For note compatibility, the boolean false. When the send buffer is empty, a `drain` event will be sent" ]
}*/
bool jswrap_httpCRq_write(JsVar *parent, JsVar *data) {
  httpClientRequestWrite(parent, data);
  return false;
}

/*JSON{ "type":"method",
         "class" : "httpCRq", "name" : "end",
         "description" : ["Finish this HTTP request - optional data to append as an argument" ],
         "generate" : "jswrap_httpCRq_end",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ]
}*/
void jswrap_httpCRq_end(JsVar *parent, JsVar *data) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return;

  if (!jsvIsUndefined(data)) jswrap_httpCRq_write(parent, data);
  httpClientRequestEnd(&net, parent);
  networkFree(&net);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"staticmethod",
         "class" : "url", "name" : "parse",
         "generate" : "jswrap_url_parse",
         "description" : ["A utility function to split a URL into parts", 
                          "This is useful in web servers for instance when handling a request.",
                          "For instance `url.parse(\"/a?b=c&d=e\",true)` returns `{\"method\":\"GET\",\"host\":\"\",\"path\":\"/a?b=c&d=e\",\"pathname\":\"/a\",\"search\":\"?b=c&d=e\",\"port\":80,\"query\":{\"b\":\"c\",\"d\":\"e\"}}`"],
         "params" : [ [ "urlStr", "JsVar", "A URL to be parsed"],
                      [ "parseQuery", "bool", "Whether to parse the query string into an object not (default = false)"] ],
         "return" : ["JsVar", "An object containing options for ```http.request``` or ```http.get```. Contains `method`, `host`, `path`, `pathname`, `search`, `port` and `query`" ]
}*/
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
        if (hadEquals) jsvAppendCharacter(val, ch);
        else jsvAppendCharacter(key, ch);
      }
      jsvStringIteratorNext(&it);
      charIdx++;
    }
    jsvStringIteratorFree(&it);
    jsvUnLock(queryStr);

    if (jsvGetStringLength(key)>0 || jsvGetStringLength(val)>0) {
      jsvMakeIntoVariableName(key, val);
      jsvAddName(query, key);
    }
    jsvUnLock(key);
    jsvUnLock(val);
  }
  jsvUnLock(jsvObjectSetChild(obj, "query", query));

  return obj;
}


