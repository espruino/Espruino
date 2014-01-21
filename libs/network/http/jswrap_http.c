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
        "class" : "url",
        "description" : ["This class helps to convert URLs into Objects of information ready for http.request/get" ]
}*/

bool checkOnline() {
  if (networkState != NETWORKSTATE_ONLINE) {
    jsError("Not connected to the internet");
    return false;
  }
  return true;
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"staticmethod",
         "class" : "http", "name" : "createServer",
         "generate" : "jswrap_http_createServer",
         "description" : ["Create an HTTP Server" ],
         "params" : [ [ "callback", "JsVarName", "A function(req,res) that will be called when a connection is made"] ],
         "return" : ["JsVar", "Returns a new httpSrv object"]
}*/

JsVar *jswrap_http_createServer(JsVar *callback) {
  if (!checkOnline()) return 0;
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
                       [ "callback", "JsVarName", "A function(res) that will be called when a connection is made"] ],
         "return" : ["JsVar", "Returns a new httpCRq object"]
}*/

JsVar *jswrap_http_request(JsVar *options, JsVar *callback) {
  if (!checkOnline()) return 0;
  bool unlockOptions = false;
  if (jsvIsString(options)) {
    options = jswrap_url_parse(options);
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
                       [ "callback", "JsVarName", "A function(res) that will be called when a connection is made"] ],
         "return" : ["JsVar", "Returns a new httpCRq object"]
}*/
JsVar *jswrap_http_get(JsVar *options, JsVar *callback) {
  if (!checkOnline()) return 0;
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
  httpClientRequestEnd(cliReq);
  return cliReq;
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"method",
         "class" : "httpSrv", "name" : "listen",
         "generate" : "jswrap_httpSrv_listen",
         "params" : [ [ "port", "int", "The port to listen on"] ]
}*/

void jswrap_httpSrv_listen(JsVar *parent, int port) {
  if (!checkOnline()) return;
  httpServerListen(parent, port);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"method",
         "class" : "httpSRs", "name" : "write",
         "generate" : "jswrap_httpSRs_write",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ]
}*/
void jswrap_httpSRs_write(JsVar *parent, JsVar *data) {
  if (!checkOnline()) return;
  httpServerResponseData(parent, data);
}

/*JSON{ "type":"method",
         "class" : "httpSRs", "name" : "end",
         "generate" : "jswrap_httpSRs_end",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ]
}*/
void jswrap_httpSRs_end(JsVar *parent, JsVar *data) {
  if (!checkOnline()) return;
  if (!jsvIsUndefined(data)) jswrap_httpSRs_write(parent, data);
  httpServerResponseEnd(parent);
}


/*JSON{ "type":"method",
         "class" : "httpSRs", "name" : "writeHead",
         "generate" : "jswrap_httpSRs_writeHead",
         "params" : [ [ "statusCode", "int", "The HTTP status code"],
                      [ "headers", "JsVar", "An object containing the headers"] ]
}*/
void jswrap_httpSRs_writeHead(JsVar *parent, int statusCode, JsVar *headers) {
  if (!checkOnline()) return;
  httpServerResponseWriteHead(parent, statusCode, headers);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"method",
         "class" : "httpCRq", "name" : "write",
         "generate" : "jswrap_httpCRq_write",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ]
}*/
void jswrap_httpCRq_write(JsVar *parent, JsVar *data) {
  if (!checkOnline()) return;
  httpClientRequestWrite(parent, data);
}

/*JSON{ "type":"method",
         "class" : "httpCRq", "name" : "end",
         "description" : ["Finish this HTTP request - optional data to append as an argument" ],
         "generate" : "jswrap_httpCRq_end",
         "params" : [ [ "data", "JsVar", "A string containing data to send"] ]
}*/
void jswrap_httpCRq_end(JsVar *parent, JsVar *data) {
  if (!checkOnline()) return;
  if (!jsvIsUndefined(data)) jswrap_httpCRq_write(parent, data);
  httpClientRequestEnd(parent);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{ "type":"staticmethod",
         "class" : "url", "name" : "parse",
         "generate" : "jswrap_url_parse",
         "params" : [ [ "urlStr", "JsVar", "A URL to be parsed"] ],
         "return" : ["JsVar", "An object containing options for ```http.request``` or ```http.get```"]
}*/
JsVar *jswrap_url_parse(JsVar *url) {
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
  int charIdx = 0;
  int portNumber = 0;
  while (jsvStringIteratorHasChar(&it)) {
    char ch = jsvStringIteratorGetChar(&it);
    if (ch == '/') {
      slashes++;
      if (addrStart>=0)
        pathStart = charIdx;
      if (colons==1 && slashes==2 && addrStart<0)
        addrStart = charIdx;
    }
    if (ch == ':') {
      colons++;
      if (addrStart>=0 && pathStart<0)
        portStart = charIdx;
    }

    if (portStart>=0 && charIdx>portStart && pathStart<0 && ch >= '0' && ch <= '9') {
      portNumber = portNumber*10 + (ch-'0');
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
  JsVar *method = jsvNewFromString("GET");
  jsvUnLock(jsvAddNamedChild(obj, method, "method"));
  jsvUnLock(method);
  JsVar *host = jsvNewFromEmptyString();
  jsvAppendStringVar(host, url, addrStart+1, addrEnd-(addrStart+1));
  jsvUnLock(jsvAddNamedChild(obj, host, "host"));
  jsvUnLock(host);
  JsVar *path = jsvNewFromEmptyString();
  jsvAppendStringVar(path, url, pathStart, JSVAPPENDSTRINGVAR_MAXLENGTH);
  if (jsvGetStringLength(path)==0) jsvAppendString(path, "/");
  jsvUnLock(jsvAddNamedChild(obj, path, "path"));
  jsvUnLock(path);
  if (portNumber<=0 || portNumber>65535) portNumber=80;
  JsVar *port = jsvNewFromInteger(portNumber);
  jsvUnLock(jsvAddNamedChild(obj, port, "port"));
  jsvUnLock(port);

  return obj;
}

