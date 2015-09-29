The Espruino networking has an architecture model that appears to be implemented in `network.c`.  Various network support types can be compiled into the core code including:

* `JSNETWORKTYPE_CC3000` - The CC3000 device.
* `JSNETWORKTYPE_W5500` - The W5500 device.
* `JSNETWORKTYPE_ESP8266` - A piggybacked ESP8266 attached to the MCU.
* `JSNETWORKTYPE_LINUX` - The Linux OS
* `JSNETWORKTYPE_JS` - Unknown

For the ESP8266 board project, we will create a new type called `JSNETWORKTYPE_ESP8266_BOARD`.

Among the core functions exposed we seem to have:

* networkCreate() - create the network object (ONLY to be used by network drivers)
* networkWasCreated()
* networkGetFromVar()
* networkGetFromVarIfOnline()
* networkSet()
* networkFree()
* networkGetCurrent() - Get the currently active network structure. can be 0!
* networkParseIPAddress()
* networkParseMACAddress()
* networkGetAddressAsString() - Get an address as a string.
* networkPutAddressAsString()
* networkFlipIPAddress()
* networkGetHostByName() - Use this for getting the hostname, as it parses the name to see if it is an IP address first.


A logical data type called `JsNetwork` contains the entry points for many of the functions.

This includes:

* `createsocket` - if host=0, creates a server socket otherwise creates a client socket (and automatically connects). Returns >=0 on success.
* `closesocket` - destroys the given socket.
* `accept` - If the given server socket can accept a connection, return it (or return < -1).
* `gethostbyname` - Get an IP address from a name.
* `recv` - Receive data if possible. returns nBytes on success, 0 on no data, or -1 on failure.
* `send` - Send data if possible. returns nBytes on success, 0 on no data, or -1 on failure.
* `idle` - Called on idle. Do any checks required for this device.
* `checkError` - Call just before returning to idle loop. This checks for errors and tries to recover. Returns true if no errors.

Each of these functions **must** be implemented by a network provider.

##createsocket

`int net_<board>_createSocket(JsNetwork *net, uint32_t ipAddress, unsigned short port)`

If host=0, creates a server otherwise creates a client (and automatically connects).

* `net` - The Network we are going to use to create the socket.
* `ipAddress` - The address of the partner of the socket.
* `port` - The port number that the partner is listening upon.

 Returns >=0 on success and -1 on an error.  The return is the new socket id.

##closesocket

Destroys the given socket.

`void net_<board>_closeSocket(JsNetwork *net, int sckt)`

* `net` - The Network we are going to use to create the socket.
* `sckt` - The socket to be closed.

##accept

If the given server socket can accept a connection, return it (or return < 0).

`int net_<board>_accept(JsNetwork *net, int serverSckt)`

* `net` - The Network we are going to use to create the socket.
* `serverSckt` - The socket that we are now going to start accepting requests upon.

Returns a new conversation socket or -1 on an error.

##gethostbyname

Get an IP address from a name. Sets `outIp` to 0 on failure.

`void net_<board>_gethostbyname(JsNetwork *net, char *hostName, uint32_t *outIp)`

* `net` - The Network we are going to use to create the socket.
* `hostName` - The string representing the hostname we wish to lookup.
* `outIp` - The address into which the resolved IP address will be stored.

##recv

`int net_<board>_recv(JsNetwork *net, int sckt, void *buf, size_t len)`

* `net` - The Network we are going to use to create the socket.
* `recv` - The socket from which we are to receive data.
* `buf` - The storage buffer into which we will receive data.
* `len` - The length of the buffer.

Returns the number of bytes received which may be 0 and -1 if there was an error.

##send
Send data if possible.

`int net_<board>_send(JsNetwork *net, int sckt, const void *buf, size_t len)`

* `net` - The Network we are going to use to create the socket.
* `sckt` - The socket over which we will send data.
* `buf` - The buffer containing the data to be sent.
* `len` - The length of data in the buffer to send.

 Returns number of bytes sent on success, 0 on no data, or -1 on failure

##idle

`void net_<board>_idle(JsNetwork *net)`

* `net` - The Network we are going to use to create the socket.

##checkError

`bool net_<board>_checkError(JsNetwork *net)`

* `net` - The Network we are going to use to create the socket.

This function returns `true` if there are **no** errors.

----

#The Socket Server library

The socket server library is a suite of components used to provide a sockets like abstraction within
the Espruino environment.  In addition to pure communication, there is much function in here for being
either an HTTP client or an HTTP server.  To ensure that we understand what that later part means ...
an HTTP client is playing the part of a browser and will send HTTP requests **to** and HTTP server such
as a Web Server.

The HTTP server part of this library plays the part of **being** an HTTP server that responds to client
requests from browsers (or other HTTP clients).

To make this technology work, the library created __hidden__ variables in the JS variable root.  These are
real JS variables but they are not visible or available to normal JS programs.  However, from an internals
perspective, they can be worked with just fine.  These JS variables (and there are a couple) are actually
instances of JS arrays where each element of the array is an object that represents a connection to
a partner over the network.  When working with the arrays, there is a method called `socketGetArray()`
that returns (and creates) the array for us so we don't have to work with the raw arrays ourselves
as a consumer of the library.

The `Socket` class has the following additions to it:

* `data` - Event
* `close` - Event
* `drain` - event
* `available` - method - `jswrap_stream_available`
* `read` - method - `jswrap_stream_end`
* `write` - method - `jswrap_jswrap_net_socket_write`
* `pipe` - method - `jswrap_pipe`

----

###httpAppendHeaders
Add the HTTP headers object to the string.

`static void httpAppendHeaders(JsVar *string, JsVar *headerObject)`

The `headersObject` is a JS object with name/value properties.  Here we walk through each of those properties
and append them to the string supplied as `string` and add each one as an HTTP header of the form
`<name>: <value>`.

----

###httpParseHeaders
Parse out the HTTP headers from the data.

`bool httpParseHeaders(JsVar **receiveData, JsVar *objectForData, bool isServer)`

The `receiveData` is the address of a JS variable that contains the string received from the partner.  It
is parsed to look for headers.  These are added as new property of the object passed in as `objectForData`.  The
name of the new property that is added is called `headers`.  Special treatment is given for HTTP client vs server requests.
If we are a server, then `method` and `url` are especially parsed and added and if we are a client then
`httpVersion`, `statusCode` and `statusMessage` are pulled out.

----

###httpStringGet
Get a C string from a JS Var.

`size_t httpStringGet(JsVar *v, char *str, size_t len)`

Retrieve a string from the variable `v` and store it at the buffer pointed to by `str` for a
maximum size of `len`.  It appears that there is no null terminator and that the return is confusing.

----

###socketGetArray
Return the hidden variable array.

`static JsVar *socketGetArray(const char *name, bool create)`

A JS array is returned called `name` where it is a hidden root variable.  If it doesn't exist then it
can be optionally created.

----

###socketGetType
Retrieve the type of the socket variable.

`static SocketType socketGetType(JsVar *var)`

Given a socket variable, retrieve its type.  The property in the socket variable is currently `type` and is an
integer encoded as:

* 0 - ST_NORMAL
* 1 - ST_HTTP

----

###socketSetType

Set the type of the socket variable.

`static void socketSetType(JsVar *var, SocketType socketType)`

Given a socket variable and its type, set the type on that socket variable.  This sets a property that is currently
called `type` and is an integer encoded as:

* 0 - ST_NORMAL
* 1 - ST_HTTP

----

###_socketConnectionKill
Close (kill) the socket for a specific socket variable.

`void _socketConnectionKill(JsNetwork *net, JsVar *connection)`

Kill a socket connection that is the socket contained within `connection`.

----

###_socketCloseAllConnectionsFor

Close ALL the sockets associated with __something__.

`static void _socketCloseAllConnectionsFor(JsNetwork *net, char *name)`

Close all the sockets associated with a named hidden variable.

----

###_socketCloseAllConnections
Close all the sockets associated with some socket sets.

`static void _socketCloseAllConnections(JsNetwork *net)`

Close all the sockets associated with the socket sets called `HttpSC`, `HttpCC` and `HttpS`.

----

###socketSendData
Send data through the socket.

`bool socketSendData(JsNetwork *net, JsVar *connection, int sckt, JsVar **sendData)`

The implementation of this one is tricky.  It appears that `sendData` must be a string as we perform
string lengths against it.  Can it hold binary?  We also appear to pass in a `connection` which I thought
would know its own socket ... but yet it takes a socket integer as a parameter as well.

The `sendData` is shrunk on return having pruned off what was actually sent.  However, this rountine
does **not** assure to send all the data ... just __some__ data ... so a caller should not assume that
once it is called, the data has been sent.


----

###socketInit
Initialize the socket server environment.

`void socketInit()`

Only seems to do something on windows.()

----

###socketKill
Shutdown the socket subsystem.

`void socketKill(JsNetwork *net)`

----

###socketServerConnectionsIdle
Do idle processing on server connections.

`bool socketServerConnectionsIdle(JsNetwork *net)`

Walk through each of the socket connections for servers `HTTP_ARRAY_HTTP_SERVER_CONNECTIONS (HttpSC)` and look for 
properties on them that are interpreted as instructions.

Instructions that are processed include:

* `HTTP_NAME_CLOSENOW (closeNow)` - Close a socket.
* `HTTP_NAME_SEND_DATA (dSnd)` - Send data.
* `HTTP_NAME_CLOSE (close)` - Close a socket when done.

----

###socketClientPushReceiveData
Push received data into a stream.

`void socketClientPushReceiveData(JsVar *connection, JsVar *socket, JsVar **receiveData)`

----

###socketClientConnectionsIdle
Do idle processing on HTTP client connections.

`bool socketClientConnectionsIdle(JsNetwork *net)`

Walk through each of the socket connections for HTTP clients `HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS (HttpCC)` and look
for properties on them that are interpreted as instructions.

* `HTTP_NAME_CLOSENOW (closeNow)` - Close a socket.
* `HTTP_NAME_RECEIVE_DATA (dRcv)` - Receive data.
* `HTTP_NAME_CLOSE (close)` - Close a socket when done.

This function also polls the receive from the board.

----

###socketIdle
Do idle processing on ??? connections.

`bool socketIdle(JsNetwork *net)`

Walk through each of the socket connections for ??? `HTTP_ARRAY_HTTP_SERVERS (HttpS)`.

We examine the socket to see if there is a pending connection upon it.

----

###serverNew
Create a new server socket.

`JsVar *serverNew(SocketType socketType, JsVar *callback)`

Create a new server socket of the given socketType (one of `ST_NORMAL` or `ST_HTTP`).  Add the
callback function as a property of the new socket variable.  The property is `HTTP_NAME_ON_CONNECT (#onconnect)`.
Return the new socket variable.  Note that this does NO network work other than create the socket JS var.

The variable returned is of class type `httpSrv` or `Server` depending on the socket type.

----

###serverListen

Start listening.

`void serverListen(JsNetwork *net, JsVar *server, int port)`

Create a real network connection for the socket JS variable supplied in `server` and cause it to start listening
on the given port.  The new socket is added to the list of server sockets `HTTP_ARRAY_HTTP_SERVERS (HttpS)`.

----

###serverClose
Close the server socket.

`void serverClose(JsNetwork *net, JsVar *server)`

Close the server socket given by the socket JS variable supplied in `server`.

----

###clientRequestNew

Create a new client socket.

`JsVar *clientRequestNew(SocketType socketType, JsVar *options, JsVar *callback)`

The new socket is added to the list of client sockets `HTTP_ARRAY_HTTP_CLIENT_CONNECTIONS (HttpCC)`.
We register the callback function named in `callback` with the `HTTP_NAME_ON_CONNECT (#onconnect)` method of the
new socket variable.

The returned object is of class type `httpCCRs`, `httpCRq` or `Socket` depending on the socket type.

----

###clientRequestWrite
Write some data to the client socket.

`void clientRequestWrite(JsVar *httpClientReqVar, JsVar *data)`

Write the data supplied by `data` to the client socket.  This is implemented by adding the data to the property called `HTTP_NAME_SEND_DATA (dSnd)` property.  The data is **not** actually sent by this function but is instead built and made available to be sent later.

----

###clientRequestConnect

Connect a socket to the server.

`void clientRequestConnect(JsNetwork *net, JsVar *httpClientReqVar)`

Connect a socket to the server.  The variable supplies as a socket variable is expected to have a property on it called
`HTTP_NAME_OPTIONS_VAR (opt)` which has a property called `port` which is the port number to connect with.  In addition
it should have a property called `host` that is the target host.

----

###clientRequestEnd

Signal the end of the socket.

`void clientRequestEnd(JsNetwork *net, JsVar *httpClientReqVar)`

Things get strange here based on the type of socket with which we are working.  If the type is
`ST_HTTP` we actually perform a connect request!!!   For normal sockets, we register that we are
ready to close after all the data has been sent.  This is done by setting the `HTTP_NAME_CLOSE (close)` flag.

----

###serverResponseWriteHead

Set the HTTP response code and headers.

`void serverResponseWriteHead(JsVar *httpServerResponseVar, int statusCode, JsVar *headers)`

Record the HTTP response code and headers to be sent back to the partner.

----

###serverResponseWrite

Set the data to be sent back to the partner.

`void serverResponseWrite(JsVar *httpServerResponseVar, JsVar *data)`

Set the data to be sent back to the partner.

----

###serverResponseEnd

Write the data and flag the close of the connection.

`void serverResponseEnd(JsVar *httpServerResponseVar)`

Write the data and flag the close of the connection.

----

#ESP8266 Implementation Notes

It looks like we are going to have a problem with `gethostbyname()`.  This appears to be a blocking call in Espruino while in the ESP8266 it is a callback function.
The equivalent ESP8266 API is `espconn_gethostbyname()` which takes a callback that will be invoked when the host name is resolved.

----

#The net library

The net library brings together the network routines and the socket routines into a
high level composed form.

 * jswrap\_net\_idle - Type: idle
 * jswrap\_net\_init - Type: init
 * jswrap\_net\_kill - Type: kill
 * jswrap\_url\_parse - Type: staticmethod, class: url
 * jswrap\_net\_createServer - Type: staticmethod, class: net, return: Server
 * jswrap\_net\_connect - Type: staticmethod, class: net, return: Socket
 * jswrap\_net\_server_listen - Type: method, class: Server
 * jswrap\_net\_server_close - Type: method, Class: Server
 * jswrap\_net\_socket_write - Type: method, Class: Socket
 * jswrap\_net\_socket_end - Type: method, Class: Socket
 * jswrap\_stream\_available - Type: method, Class: Socket
 * jswrap\_stream\_read - Type: method, Class: Socket

----
##jswrap\_net\_idle

Perform the idle processing.

`bool jswrap_net_idle()`

----

##jswrap\_net\_init

Initialize the network stuff.

`void jswrap_net_init()`

----

##jswrap\_net\_kill

Destory the network.

`void jswrap_net_kill()`

----
 
##jswrap\_url\_parse

Parse a URL string.

`JsVar *jswrap_url_parse(JsVar *url, bool parseQuery)`

The `url` variable is a String that is to be parsed.  If `parseQuery` is true, we also parse
any query data passed in.  The object that is returned contains:

* method
* host
* path
* pathname
* port
* search
* query

----

##jswrap\_net\_createServer

Create a new ST_NORMAL type server.

`JsVar *jswrap_net_createServer(JsVar *callback)`

Mostly a pass through to `serverNew`.  The return type is a JS object of type `Server`.

----

##jswrap\_net\_connect

Form a connection to a client connection to a partner.

`JsVar *jswrap_net_connect(JsVar *options, JsVar *callback, SocketType socketType)`

If `options` is a string, then we parse it as a URL string otherwise it should be
an object such as that returned by `jswrap_url_parse`.

The socket type is one of:

* `ST_NORMAL` - 0
* `ST_HTTP` - 1

The variable returned is a socket object.  The logic in this function calls 
`clientRequestNew` and if we are **not** an ST_HTTP socket, then we call
`clientRequestConnect`.  The return is the object returned from `clientRequestNew` which means it
is an instance of class type `httpCCRs`, `httpCRq` or `Socket` depending on the socket type.

----

##jswrap\_net\_server\_listen

Start a server listening.

`void jswrap_net_server_listen(JsVar *parent, int port)`

Set the server listening.  The port on which it will listen is supplied by `port`.  This is
mostly a pass through to `serverListen`.

This function is mapped to the following JavaScript methods:

* `httpSrv.listen()`

----

##jswrap\_net\_server\_close

`void jswrap_net_server_close(JsVar *parent)`

Close the server.  This is mostly a pass through to `serverClose`.

----

##jswrap\_net\_socket\_write

Write data through the socket.

`bool jswrap_net_socket_write(JsVar *socketVar, JsVar *data)`

The `data` is the new data to write.  This is a pass through to `clientRequestWrite`.

----

##jswrap\_net\_socket\_end

Close the socket with optional data.

`void jswrap_net_socket_end(JsVar *parent, JsVar *data)`

This is a pass through to `clientRequestEnd`.

----

#The HTTP Subsystem
A library is provided that allows Espruino to perform HTTP services.  There are two flavors to this.  The first is that the Espruino can behave as an HTTP client (i.e. a browser or REST caller) and transmit and receive HTTP request.

Secondly, the Espruino can become an HTTP server and listen for incoming requests from browsers or REST clients.

To be a server, one would use:

	var http = require("http");
	var httpSrv = http.createServer(function(request, response) {
		// We have a new request here!!
	});
	httpSrv.listen(80);

Should we wish the server to stop listening for new incomming connections we can call `httpSrv.close()`.

On the other side of the coin, we may wish the ESP8266 to be an HTTP client making HTTP requests in the same fashion as a browser or as a REST client.

We can perform a

	http.get({
		host: "ipAddress",
		port: <portNumber>
	}, function(response) {
		// Handle response
	});

The HTTP subsystem is implemented in `jswrap_http.c`.

* `jswrap_http_createServer`
* `jswrap_http_get`
* `jswrap_httpSRs_write`
* `jswrap_httpSRs_end`
* `jswrap_httpSRs_writeHead`

----

##jswrap\_http\_createServer


`JsVar *jswrap_http_createServer(JsVar *callback)`

The callback function supplied in the `callback` parameter is invoked when a new browse connection is received.  The callback function
has the form:


    function(request, response)

Where request is the connection for the incoming data and response is the connection for the outgoing data.  For example, we can write data to the "response" object
and read data from the "request" object.

The return from the `jswrap_http_createServer` function is an instance of an `httpSrv` object.

----

##jswrap\_http\_get

`JsVar *jswrap_http_get(JsVar *options, JsVar *callback)`

Send an HTTP request to an HTTP server.  The `options` variable defines the parameters of the connection and includes properties for `host` and `port`.  The `callback` function is a function that will be invoked when a response is received.  The signature of that functions is:

    function(response)

Where response can be used to retrieve data and determine when the response connection is closed.

The `jswrap_http_get` returns an instance of an `httpCRq` objetct.

----

##jswrap\_httpSRs\_write

`bool jswrap_httpSRs_write(JsVar *parent, JsVar *data)`

----

##jswrap\_httpSRs\_end

`void jswrap_httpSRs_end(JsVar *parent, JsVar *data)`

----

##jswrap\_httpSRs\_writeHead

`void jswrap_httpSRs_writeHead(JsVar *parent, int statusCode, JsVar *headers)`

----

#End User network programming
From a programmers perspective who is writing JS networking, there is a library exposed called `net`.  I has the following exposed methods:

* `net.connect` - Connect to a partner
* `net.createServer` - Become a server

There also appears to be classes that leverage this library.  These classes are `Socket` and `Server`.

##Socket
###Socket.available
Socket.available()

Returns how many bytes are available to be read.

###Socket.end
Close the socket.

###Socket.pipe
Pipe the data to a stream.

###Socket.read
Read some characters from the socket.

###Socket.write
Write some data to the socket.

###Socket.close
Event: Called when the connection closes.

###Socket.data
Event: Called when data has been received and is available to be read.

###Socket.drain
Event: Called when the data has been transmitted and new data can be sent.

##Server

----
#Adding a new network device
To add a new network device, the device must initialize the Espruino environment by calling

networkCreate and then networkSet.  Here is an example piece of code:

    JsNetwork net;
    networkCreate(&net, JSNETWORKTYPE_ESP8266_BOARD);
    networkSet(&net);

In the network.c file there is a bootstrap mechanism in the function called `networkGetFromVar` which switches on the network type and calls a function called `netSetCallbacks_<networkType>`.

It appears that we have to supply the following functions
* `int net_<board>_accept(JsNetwork *net, int serverSckt)`
* `int net_<board>_recv(JsNetwork *net, int sckt, void *buf, size_t len)` - Return the number of
bytes actually sent.
* `int net_<board>_send(JsNetwork *net, int sckt, const void *buf, size_t len)`
* `void net_<board>_idle(JsNetwork *net)`
* `bool net_<board>_checkError(JsNetwork *net)`
* `int net_<board>_createSocket(JsNetwork *net, uint32_t host, unsigned short port)`
* `void net_<board>_closeSocket(JsNetwork *net, int sckt)`
* `void net_<board>_gethostbyname(JsNetwork *net, char *hostName, uint32_t *outIp)`

----

#API Reference

----

####networkGetAddressAsString

#####Call type:
`JsVar *networkGetAddressAsString(unsigned char *ip, int nBytes, unsigned int base, char separator)`

#####Description
Return a JS variable that represents the IP address.

#####Parameters
* `ip` - The IP address in memory.
* `nBytes` - The size of the IP address (usually 4).
* `base` - The base representation (usually 10 for decimal).
* `separator` - The separator character (usually '.').

#####Returns
A representation of the IP address.

----

####networkParseIPAddress

#####Call Type:

`uint32_t networkParseIPAddress(const char *ip)`
 
#####Description

Parse a string representation of an IP address and return an IP address.

#####Parameters

A string representation of an IP address.

#####Returns

A 4 byte IP address.