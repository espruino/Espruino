This is the page where the User Guide for using the Espruino ESP8266 will be documented.  It is **vital** that you realize that this is still a work in progress and hence if you start coding to this guide be prepared to make changes to your own code should the APIs and semantics change before the project completes and becomes part of Espruino as whole.

By completion, this documentation will be polished and placed in the code itself so that automated
documentation generation will work.



##Listing access points
An access point is a potential WiFi device that an ESP8266 can connect to as a client.  In order to connect, you will need to know  the identity of the network (the SSID) and the password (if needed).  To list access points, we can use the function called `ESP8266WiFi.getAccessPoints()`.  The syntax of the function is:

`ESP8266WiFi.getAccessPoints(callback)`

Where callback is a function that takes a single parameter that is an array of objects.  Each object in the array corresponds to an available access point and contains the following properties:

* `ssid` - The network id
* `authMode` - The authentication mode
* `rssi` - The signal strength
* `channel` - The network channel
* `isHidden` - Is hidden from discovery

Here is an example of use:

    ESP8266WiFi.getAccessPoints(function(arrayOfAcessPoints) {
      for (var i=0; i<arrayOfAcessPoints.length; i++) {
        print("Access point: " + i + " = " + JSON.stringify(arrayOfAcessPoints[i]));
      }
    });

with a resulting output of:

    Access point: 0 = {"rssi":-48,"channel":7,"authMode":3,"isHidden":false,"ssid":"mySsid"}
    Access point: 1 = {"rssi":-84,"channel":9,"authMode":4,"isHidden":false,"ssid":"2WIRE874"}

##Connect to an access point
We can connect to an access point using `ESP8266WiFi.connect()`.  This function takes an SSID and password of the access point to which we wish to connect.  We can also supply an optional callback function that will be invoked when we have been assigned an IP address and are ready for work.

##Determining your own IP address
To determine the ESP8266's current IP address, we can use the `ESP8266WiFi.getIPInfo()` function.

    var ipInfo = ESP8266WiFi.getIPInfo();
    print("Current IP address is: " + ESP8266WiFi.getAddressAsString(ipInfo.ip));

##Disconnect from the access point
When connected to an access point, you can disconnect from it with a call to `ESP8266WiFi.disconnect()`.

##Forming a TCP connection
Assuming that the ESP8266 is now connected to the network, we can form a TCP connection to a partner.  To do this, we need to know the IP address and port
number on which the partner is listening.  From there, we can use the Espruino supplied library called "net".  We gain access to this library using:

	var net = require("net");

From this, we can now start calling the functions available within that library.  For example, to connect to a partner using TCP we can issue:

	net.connect(<connectionDetails>, function(conn) {
	   // Work with connection here
	});

The connection details can either be a URL of the form `http://<IP Address>:<port>` or an object with the properties:

* `host` - The IP address of the target
* `port` - The target IP address

We can close the socket with a call to `end()`.

##Becoming an HTTP server
We can use the Espruino libraries to become an HTTP server.  Our first step is to load the appropriate library using:

    var http = require("http");

From here, we can call createServer() to create an instance of an HTTP server.  Note that once created
we are not yet listening for incomming connections.  That comes later.

    var httpSrv = http.createServer(callbackFunction);

The `callbackFunction` is a function that will be called when an HTTP client request is received.
The function takes two parameters:

* `request` - An instance of a 
* `response`

##Being an HTTP client
We can use the Espruino libraries to be an HTTP client.  Our first step is to load
the appropriate library using:

    var http = require("http");

From here, we can call `get()` to send an HTTP get request and get the response.

For example:


    http.get({
       host: "184.168.192.49",
       port: 80,
       path: "/"
    }, function(response) {
       print("get callback!");
    });

This will send a request to the IP address specified by host and, when a response is received,
the callback function will be invoked.  The `response` data object contains details
of the response.

From the response object, we can register callbacks to be informed when new data is available and also when the connection is closed.  For example:

    response.on("data", function(data) {
       // Data available here.
    });

and

    response.on("close", function() {
       // Connection was closed.
    });

----

#Reference

----

##ESP8266WiFi.init

`ESP8266WiFi.init()`

Initialize the ESP8266 WiFi and TCP subsystems.  This is undoubtedly NOT going to be in the final releases
however until we understand more about the architecture, this is needed as scafolding (if nothing else).

----

##ESP8266WiFi.getAccessPoints

Get a list of the access points and pass them as an array into the callback.

`ESP8266WiFi.getAccessPoints(callback)`

----

##ESP8266WiFi.connect

Connect to a named access point.

`ESP8266WiFi.connect(ssid, password, [callback])`

When called, this function places the ESP8266 in station mode.  This means that we will not be an access point.
Once done, we then connect to the named access point using the network and password parameters supplied by `ssid` and `password`.  The optional callback is a function that is invoked when an IP address has been assigned to us meaning that we are now ready for TCP/IP based work.

* `ssid` - The network id of the access point.
* `password` - The password to use to connect to the access point.
* `callback` - An optional JavaScript function that is called when we are ready for TCP/IP based work.

----

##ESP8266WiFi.disconnect
Disconnect the ESP8266 from the access point.

`ESP8266WiFi.disconnect()`

----

##ESP8266WiFi.restart

Restart the ESP8266.  Purely for debug and will be removed.

`ESP8266WiFi.restart()`

----

##ESP8266WiFi.getRstInfo

`ESP8266WiFi.getRstInfo()`


Returns an object that contains the details of the last ESP8266 restart.


----

##ESP8266WiFi.getIPInfo

`ESP8266WiFi.getIPInfo()`

Returns an object that contains the details of the current IP address.  The object contains:

* `ip` - The current IP address
* `gw` - The current Gateway address
* `netmask` - The current Netmask value

Each of these properties are 32 bit integers (4 bytes corresponding to the IP address).  To convert these
into string dotted decimal format one can use the `ESP8266WiFi.getAddressAsString` function.

----

##ESP8266WiFi.getAutoConnect

Get the current value of the auto connect mode.

`ESP8266WiFi.getAutoConnect()`

Returns `true` if we are going to auto connect on next restart.

----

##ESP8266WiFi.setAutoConnect

Set the auto connect mode on **next** restart.

`ESP8266WiFi.setAutoConnect(autoconnect)`

* `autoconnect` - A value of `true` to perform an autoconnect and false otherwise.

----

##ESP8266WiFi.getStationConfig

Get the station configuration settings.

`ESP8266WiFi.getStationConfig()`

The return is an object containing:

* `ssid` - The network name of the access point.
* `password` - The password used to connect to the access point.

----

##ESP8266WiFi.onWiFiEvent

Set the WiFi event handler.

`ESP8266WiFi.onWiFiEvent(callback)`

* `callback` - A callback function that is invoked with the details of the WiFi event.

----

##ESP8266WiFi.getAddressAsString

Get a string representation of an IP address.

`ESP8266WiFi.getAddressAsString(address)`

* `address` - An integer representation of a string.

The return is a JS string that represents our IP address.

----

##ESP8266WiFi.getConnectStatus

Get the current connection status.

`ESP8266WiFi.getConnectStatus()`

Retrieve the connection status.  The return is an object that contains:

* status - The status code from ESP8266
* statusMsg - The description of the code

The status is a JS integer that describes the current connection status which will be one of:

 * 0 - `STATION_IDLE`
 * 1 - `STATION_CONNECTING`
 * 2 - `STATION_WRONG_PASSWORD`
 * 3 - `STATION_NO_AP_FOUND`
 * 4 - `STATION_CONNECT_FAIL`
 * 5 - `STATION_GOT_IP`
 * 255 - Not in station mode

----

##ESP8266WiFi.beAccessPoint

Become an access point.

`ESP8266WiFi.beAccessPoint(ssid, password)`

Become an access point for the network supplied by `ssid` with a password of `password`.

----

##ESP8266WiFi.getConnectedStations

List the WiFi stations connected to the ESP8266 assuming it is being
an access point.

`ESP8266WiFI.getConnectedStations()`

The return is an array of objects where each object contains:

* `ip` - The IP address of the connected station.

If no stations are connected then the return is an array with zero elements.

----

##ESP8266WiFi.getRSSI

Get the RSSI (signal strength) of the WiFi signal.

`ESP8266WiFi.getRSSI()`

The return is an integer representing the signal strength of the connected WiFi
network.

----

##ESP8266WiFi.ping

Ping an IP address.

`ESP8266WiFi.ping(ipAddress, callback)`

Ping the TCP/IP device given by the address.  The address can be either a dotted
decimal string or a 32bit numeric.  A callback function can be supplied which is invoked for each ping response.  A parameter is supplied to the callback which is a JS object that contains the following fields:

 * totalCount
 * totalBytes
 * totalTime
 * respTime
 * seqNo
 * timeoutCount
 * bytes
 * error
 
 
 An example of calling this function would be:
 
 
    ESP8266WiFi.ping("192.168.1.31", function(resp) {
        print("Ping response: " + JSON.stringify(resp));
    });

----

##ESP8266WiFi.getState

Return an object that represents the state and details of the ESP8266 device.

`ESP8266WiFi.getState()`

The returned object contains the following fields:

* `sdkVersion` - The version of the ESP8266 SDK used to build this release.
* `cpuFrequency` - The CPU operating frequency in MHz.
* `freeHeap` - The amount of free heap in bytes.