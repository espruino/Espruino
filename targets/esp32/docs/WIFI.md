# ESP32 and WiFi
Using the ESP8266 board as a model, we must implement the ESP32 WiFi functions.  This will entail
the creation of a new source file called `jswrap_esp32_network.c` which will live in
`libs/network/esp32`.  There will also be a corresponding `jswrap_esp32_network.h`.

The majority of the WiFi API is documented in the Espruino docs here:

http://www.espruino.com/Reference#Wifi

This will expose a module called `Wifi` which will have the following static methods defined
upon it:

* `disconnect` 
* `stopAP`
* `connect` - Connect to an access point
* `scan`
* `startAP`
* `getStatus`
* `setConfig`
* `getDetails`
* `getAPDetails`
* `save`
* `restore`
* `getIP`
* `getAPIP`
* `getHostByName`
* `getHostname`
* `setHostname`
* `ping`

# Implementation status

## connect
Working.

## disconnect
Working.

## All others
Not yet implemented