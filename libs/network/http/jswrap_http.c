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
#include "jswrap_net.h"
#include "jswrap_http.h"
#include "jsvariterator.h"
#include "socketserver.h"

#include "../network.h"

/*JSON{
  "type" : "library",
  "name" : "http"
}
This library allows you to create http servers and make http requests

In order to use this, you will need an extra module to get network connectivity such as the [TI CC3000](/CC3000) or [WIZnet W5500](/WIZnet).

This is designed to be a cut-down version of the [node.js library](http://nodejs.org/api/http.html). Please see the [Internet](/Internet) page for more information on how to use it.
*/

/*JSON{
  "type" : "object",
  "name" : "httpSrv",
  "memberOf" : "http"
}
The HTTP server created by `require('http').createServer`
*/
// there is a 'connect' event on httpSrv, but it's used by createServer and isn't node-compliant

/*JSON{
  "type" : "object",
  "name" : "httpSRq",
  "memberOf" : "http"
}
The HTTP server request
*/
/*JSON{
  "type" : "event",
  "name" : "data",
  "memberOf" : "httpSRq",
  "params" : [
    ["data","JsVar","A string containing one or more characters of received data"]
  ]
}
The 'data' event is called when data is received. If a handler is defined with `X.on('data', function(data) { ... })` then it will be called, otherwise data will be stored in an internal buffer, where it can be retrieved with `X.read()`
*/
/*JSON{
  "type" : "event",
  "name" : "close",
  "memberOf" : "httpSRq"
}
Called when the connection closes.
*/
/*JSON{
  "type" : "function",
  "name" : "available",
  "memberOf" : "httpSRq.prototype",
  "thisParam" : true,
  "generate" : "jswrap_stream_available",
  "return" : ["int","How many bytes are available"]
}
Return how many bytes are available to read. If there is already a listener for data, this will always return 0.
*/
/*JSON{
  "type" : "function",
  "name" : "read",
  "memberOf" : "httpSRq.prototype",
  "thisParam" : true,
  "generate" : "jswrap_stream_read",
  "params" : [
    ["chars","int","The number of characters to read, or undefined/0 for all available"]
  ],
  "return" : ["JsVar","A string containing the required bytes."]
}
Return a string containing characters that have been received
*/
/*JSON{
  "type" : "function",
  "name" : "pipe",
  "memberOf" : "httpSRq.prototype",
  "thisParam" : true,
  "generate" : "jswrap_pipe",
  "params" : [
    ["destination","JsVar","The destination file/stream that will receive content from the source."],
    ["options","JsVar",["An optional object `{ chunkSize : int=32, end : bool=true, complete : function }`","chunkSize : The amount of data to pipe from source to destination at a time","complete : a function to call when the pipe activity is complete","end : call the 'end' function on the destination when the source is finished"]]
  ],
  "if" : "!defined(SAVE_ON_FLASH)"
}
Pipe this to a stream (an object with a 'write' method)
*/

/*JSON{
  "type" : "object",
  "name" : "httpSRs",
  "memberOf" : "http"
}
The HTTP server response
*/
/*JSON{
  "type" : "event",
  "name" : "drain",
  "memberOf" : "httpSRs"
}
An event that is fired when the buffer is empty and it can accept more data to send.
*/
/*JSON{
  "type" : "event",
  "name" : "close",
  "memberOf" : "httpSRs"
}
Called when the connection closes.
*/

/*JSON{
  "type" : "object",
  "name" : "httpCRq",
  "memberOf" : "http"
}
The HTTP client request, returned by `http.request()` and `http.get()`.
*/
/*JSON{
  "type" : "event",
  "name" : "drain",
  "memberOf" : "httpCRq"
}
An event that is fired when the buffer is empty and it can accept more data to send.
*/
/*JSON{
  "type" : "event",
  "name" : "error",
  "memberOf" : "httpCRq"
}
An event that is fired if there is an error making the request and the response callback has not been invoked. In this case the error event concludes the request attempt. The error event function receives an error object as parameter with a `code` field and a `message` field.
*/

/*JSON{
  "type" : "object",
  "name" : "httpCRs",
  "memberOf" : "http"
}
The HTTP client response, passed to the callback of `http.request()` an `http.get()`.
*/
/*JSON{
  "type" : "event",
  "name" : "data",
  "memberOf" : "httpCRs",
  "params" : [
    ["data","JsVar","A string containing one or more characters of received data"]
  ]
}
The 'data' event is called when data is received. If a handler is defined with `X.on('data', function(data) { ... })` then it will be called, otherwise data will be stored in an internal buffer, where it can be retrieved with `X.read()`
*/
/*JSON{
  "type" : "event",
  "name" : "close",
  "memberOf" : "httpCRs"
}
Called when the connection closes with one `hadError` boolean parameter, which indicates whether an error occurred.
*/
/*JSON{
  "type" : "event",
  "name" : "error",
  "memberOf" : "httpCRs"
}
An event that is fired if there is an error receiving the response. The error event function receives an error object as parameter with a `code` field and a `message` field. After the error event the close even will also be triggered to conclude the HTTP request/response.
*/
/*JSON{
  "type" : "function",
  "name" : "available",
  "memberOf" : "httpCRs.prototype",
  "thisParam" : true,
  "generate" : "jswrap_stream_available",
  "return" : ["int","How many bytes are available"]
}
Return how many bytes are available to read. If there is a 'data' event handler, this will always return 0.
*/
/*JSON{
  "type" : "function",
  "name" : "read",
  "memberOf" : "httpCRs.prototype",
  "thisParam" : true,
  "generate" : "jswrap_stream_read",
  "params" : [
    ["chars","int","The number of characters to read, or undefined/0 for all available"]
  ],
  "return" : ["JsVar","A string containing the required bytes."]
}
Return a string containing characters that have been received
*/
/*JSON{
  "type" : "function",
  "name" : "pipe",
  "memberOf" : "httpCRs.prototype",
  "thisParam" : true,
  "generate" : "jswrap_pipe",
  "params" : [
    ["destination","JsVar","The destination file/stream that will receive content from the source."],
    ["options","JsVar",["An optional object `{ chunkSize : int=32, end : bool=true, complete : function }`","chunkSize : The amount of data to pipe from source to destination at a time","complete : a function to call when the pipe activity is complete","end : call the 'end' function on the destination when the source is finished"]]
  ],
  "if" : "!defined(SAVE_ON_FLASH)"
}
Pipe this to a stream (an object with a 'write' method)
*/


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "function",
  "name" : "createServer",
  "memberOf" : "http",
  "thisParam" : false,
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
  return serverNew(ST_HTTP, callback);
}

/*JSON{
  "type" : "function",
  "name" : "request",
  "memberOf" : "http",
  "thisParam" : false,
  "generate_full" : "jswrap_net_connect(options, callback, ST_HTTP)",
  "params" : [
    ["options","JsVar","An object containing host,port,path,method,headers fields (and also ca,key,cert if HTTPS is enabled)"],
    ["callback","JsVar","A function(res) that will be called when a connection is made. You can then call `res.on('data', function(data) { ... })` and `res.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","Returns a new httpCRq object"],
  "return_object" : "httpCRq"
}
Create an HTTP Request - `end()` must be called on it to complete the operation. `options` is of the form:

```
var options = {
    host: 'example.com', // host name
    port: 80,            // (optional) port, defaults to 80
    path: '/',           // path sent to server
    method: 'GET',       // HTTP command sent to server (must be uppercase 'GET', 'POST', etc)
    headers: { key : value, key : value } // (optional) HTTP headers
  };
require("http").request(options, function(res) {
  res.on('data', function(data) {
    console.log("HTTP> "+data);
  });
  res.on('close', function(data) {
    console.log("Connection closed");
  });
});
```

You can easily pre-populate `options` from a URL using `var options = url.parse("http://www.example.com/foo.html")`

**Note:** if TLS/HTTPS is enabled, options can have `ca`, `key` and `cert` fields. See `tls.connect` for
more information about these and how to use them.
*/

/*JSON{
  "type" : "function",
  "name" : "get",
  "memberOf" : "http",
  "thisParam" : false,
  "generate" : "jswrap_http_get",
  "params" : [
    ["options","JsVar","A simple URL, or an object containing host,port,path,method fields"],
    ["callback","JsVar","A function(res) that will be called when a connection is made. You can then call `res.on('data', function(data) { ... })` and `res.on('close', function() { ... })` to deal with the response."]
  ],
  "return" : ["JsVar","Returns a new httpCRq object"],
  "return_object" : "httpCRq"
}
Request a webpage over HTTP - a convenience function for `http.request()` that makes sure the HTTP command is 'GET', and that calls `end` automatically.

```
require("http").get("http://pur3.co.uk/hello.txt", function(res) {
  res.on('data', function(data) {
    console.log("HTTP> "+data);
  });
  res.on('close', function(data) {
    console.log("Connection closed");
  });
});
```

See `http.request()` and [the Internet page](/Internet) and ` for more usage examples.
*/
JsVar *jswrap_http_get(JsVar *options, JsVar *callback) {
  JsNetwork net;
  if (!networkGetFromVarIfOnline(&net)) return 0;

  if (jsvIsObject(options)) {
    // if options is a string - it will be parsed, and GET will be set automatically
    JsVar *method = jsvNewFromString("GET");
    jsvUnLock2(jsvAddNamedChild(options, method, "method"), method);
  }
  JsVar *skippedCallback = jsvSkipName(callback);
  if (!jsvIsUndefined(skippedCallback) && !jsvIsFunction(skippedCallback)) {
    jsError("Expecting Callback Function but got %t", skippedCallback);
    jsvUnLock(skippedCallback);
    return 0;
  }
  jsvUnLock(skippedCallback);
  JsVar *cliReq = jswrap_net_connect(options, callback, ST_HTTP);
  if (cliReq) clientRequestEnd(&net, cliReq);
  networkFree(&net);
  return cliReq;
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "function",
  "name" : "listen",
  "memberOf" : "httpSrv.prototype",
  "thisParam" : true,
  "generate" : "jswrap_net_server_listen",
  "params" : [
    ["port","int32","The port to listen on"]
  ]
}
Start listening for new HTTP connections on the given port
*/
// Re-use existing

/*JSON{
  "type" : "function",
  "name" : "close",
  "memberOf" : "httpSrv.prototype",
  "thisParam" : true,
  "generate" : "jswrap_net_server_close"
}
Stop listening for new HTTP connections
*/
// Re-use existing


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "function",
  "name" : "write",
  "memberOf" : "httpSRs.prototype",
  "thisParam" : true,
  "generate" : "jswrap_httpSRs_write",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ],
  "return" : ["bool","For note compatibility, the boolean false. When the send buffer is empty, a `drain` event will be sent"]
}

*/
bool jswrap_httpSRs_write(JsVar *parent, JsVar *data) {
  serverResponseWrite(parent, data);
  return false;
}

/*JSON{
  "type" : "function",
  "name" : "end",
  "memberOf" : "httpSRs.prototype",
  "thisParam" : true,
  "generate" : "jswrap_httpSRs_end",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ]
}

*/
void jswrap_httpSRs_end(JsVar *parent, JsVar *data) {
  if (!jsvIsUndefined(data)) jswrap_httpSRs_write(parent, data);
  serverResponseEnd(parent);
}


/*JSON{
  "type" : "function",
  "name" : "writeHead",
  "memberOf" : "httpSRs.prototype",
  "thisParam" : true,
  "generate" : "jswrap_httpSRs_writeHead",
  "params" : [
    ["statusCode","int32","The HTTP status code"],
    ["headers","JsVar","An object containing the headers"]
  ]
}

*/
void jswrap_httpSRs_writeHead(JsVar *parent, int statusCode, JsVar *headers) {
  serverResponseWriteHead(parent, statusCode, headers);
}

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
/*JSON{
  "type" : "function",
  "name" : "write",
  "memberOf" : "httpCRq.prototype",
  "thisParam" : true,
  "generate" : "jswrap_net_socket_write",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ],
  "return" : ["bool","For note compatibility, the boolean false. When the send buffer is empty, a `drain` event will be sent"]
}

*/
// Re-use existing

/*JSON{
  "type" : "function",
  "name" : "end",
  "memberOf" : "httpCRq.prototype",
  "thisParam" : true,
  "generate" : "jswrap_net_socket_end",
  "params" : [
    ["data","JsVar","A string containing data to send"]
  ]
}
Finish this HTTP request - optional data to append as an argument
*/
// Re-use existing


