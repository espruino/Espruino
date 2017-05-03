/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains WiFi functions
 * ----------------------------------------------------------------------------
 */

/*JSON{
   "type": "library",
   "class": "Wifi"
}
The wifi library is a generic cross-platform library to control the Wifi interface.  It supports functionality such as connecting to wifi networks, getting network information, starting and access point, etc.
**Currently this library is ESP8266 specific** and needs to be ported to other Espruino platforms.

To get started and connect to your local access point all you need is
```
var wifi = require("Wifi");
wifi.connect("my-ssid", {password:"my-pwd"}, function(ap){ console.log("connected:", ap); });
```
If you want the connection to happen automatically at boot, add `wifi.save();`.

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "associated",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'connected' event is called when an association with an access point has succeeded, i.e., a connection to the AP's network has been established.
The details include:

* ssid - The SSID of the access point to which the association was established
* mac - The BSSID/mac address of the access point
* channel - The wifi channel used (an integer, typ 1..14)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "disconnected",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'disconnected' event is called when an association with an access point has been lost.
The details include:

* ssid - The SSID of the access point from which the association was lost
* mac - The BSSID/mac address of the access point
* reason - The reason for the disconnection (string)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "auth_change",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'auth_change' event is called when the authentication mode with the associated access point changes.
The details include:

* oldMode - The old auth mode (string: open, wep, wpa, wpa2, wpa_wpa2)
* newMode - The new auth mode (string: open, wep, wpa, wpa2, wpa_wpa2)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "dhcp_timeout"
}
The 'dhcp_timeout' event is called when a DHCP request to the connected access point fails and thus no IP address could be acquired (or renewed).
*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "connected",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'connected' event is called when the connection with an access point is ready for traffic. In the case of a dynamic IP address configuration this is when an IP address is obtained, in the case of static IP address allocation this happens when an association is formed (in that case the 'associated' and 'connected' events are fired in rapid succession).
The details include:

* ip - The IP address obtained as string
* netmask - The network's IP range mask as string
* gw - The network's default gateway as string

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "sta_joined",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'sta_joined' event is called when a station establishes an association (i.e. connects) with the esp8266's access point.
The details include:

* mac - The MAC address of the station in string format (00:00:00:00:00:00)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "sta_left",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'sta_left' event is called when a station disconnects from the esp8266's access point (or its association times out?).
The details include:

* mac - The MAC address of the station in string format (00:00:00:00:00:00)

*/

/*JSON{
  "type" : "event",
  "class" : "Wifi",
  "name" : "probe_recv",
  "params" : [
    ["details","JsVar","An object with event details"]
  ]
}
The 'probe_recv' event is called when a probe request is received from some station by the esp8266's access point.
The details include:

* mac - The MAC address of the station in string format (00:00:00:00:00:00)
* rssi - The signal strength in dB of the probe request

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "disconnect",
  "generate" : "jswrap_wifi_disconnect",
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back on disconnection. The callback function receives no argument."]
  ]
}
Disconnect the wifi station from an access point and disable the station mode. It is OK to call `disconnect` to turn off station mode even if no connection exists (for example, connection attempts may be failing). Station mode can be re-enabled by calling `connect` or `scan`.
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "stopAP",
  "generate" : "jswrap_wifi_stopAP",
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back on successful stop. The callback function receives no argument."]
  ]
}
Stop being an access point and disable the AP operation mode. Ap mode can be re-enabled by calling `startAP`.
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "connect",
  "generate" : "jswrap_wifi_connect",
  "params"   : [
    ["ssid", "JsVar", "The access point network id."],
    ["options", "JsVar", "Connection options (optional)."],
    ["callback", "JsVar", "A function to be called back on completion (optional)."]
  ]
}
Connect to an access point as a station. If there is an existing connection to an AP it is first disconnected if the SSID or password are different from those passed as parameters. Put differently, if the passed SSID and password are identical to the currently connected AP then nothing is changed.
When the connection attempt completes the callback function is invoked with one `err` parameter, which is NULL if there is no error and a string message if there is an error. If DHCP is enabled the callback occurs once an IP addres has been obtained, if a static IP is set the callback occurs once the AP's network has been joined.  The callback is also invoked if a connection already exists and does not need to be changed.

The options properties may contain:

* `password` - Password string to be used to access the network.
* `dnsServers` (array of String) - An array of up to two DNS servers in dotted decimal format string.

Notes:

* the options should include the ability to set a static IP and associated netmask and gateway, this is a future enhancement.
* the only error reported in the callback is "Bad password", all other errors (such as access point not found or DHCP timeout) just cause connection retries. If the reporting of such temporary errors is desired, the caller must use its own timeout and the `getDetails().status` field.
* the `connect` call automatically enabled station mode, it can be disabled again by calling `disconnect`.

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "scan",
  "generate" : "jswrap_wifi_scan",
  "params"   : [
    ["callback", "JsVar", "A function to be called back on completion."]
  ]
}
Perform a scan for access points. This will enable the station mode if it is not currently enabled. Once the scan is complete the callback function is called with an array of APs found, each AP is an object with:

* `ssid`: SSID string.
* `mac`: access point MAC address in 00:00:00:00:00:00 format.
* `authMode`: `open`, `wep`, `wpa`, `wpa2`, or `wpa_wpa2`.
* `channel`: wifi channel 1..13.
* `hidden`: true if the SSID is hidden.
* `rssi`: signal strength in dB in the range -110..0.

Notes:
* in order to perform the scan the station mode is turned on and remains on, use Wifi.disconnect() to turn it off again, if desired.
* only one scan can be in progress at a time.

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "startAP",
  "generate" : "jswrap_wifi_startAP",
  "params"   : [
    ["ssid", "JsVar", "The network id."],
    ["options", "JsVar", "Configuration options (optional)."],
    ["callback", "JsVar", "Optional function to be called when the AP is successfully started."]
  ]
}
Create a WiFi access point allowing stations to connect. If the password is NULL or an empty string the access point is open, otherwise it is encrypted.
The callback function is invoked once the access point is set-up and receives one `err` argument, which is NULL on success and contains an error message string otherwise.

The `options` object can contain the following properties.

* `authMode` - The authentication mode to use.  Can be one of "open", "wpa2", "wpa", "wpa_wpa2". The default is open (but open access points are not recommended).
* `password` - The password for connecting stations if authMode is not open.
* `channel` - The channel to be used for the access point in the range 1..13. If the device is also connected to an access point as a station then that access point determines the channel.

Notes:

* the options should include the ability to set the AP IP and associated netmask, this is a future enhancement.
* the `startAP` call automatically enables AP mode. It can be disabled again by calling `stopAP`.

*/


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getStatus",
  "generate" : "jswrap_wifi_getStatus",
  "return"   : ["JsVar", "An object representing the current WiFi status, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the current Wifi status, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Retrieve the current overall WiFi configuration. This call provides general information that pertains to both station and access point modes. The getDetails and getAPDetails calls provide more in-depth information about the station and access point configurations, respectively. The status object has the following properties:

* `station` - Status of the wifi station: `off`, `connecting`, ...
* `ap` - Status of the wifi access point: `disabled`, `enabled`.
* `mode` - The current operation mode: `off`, `sta`, `ap`, `sta+ap`.
* `phy` - Modulation standard configured: `11b`, `11g`, `11n` (the esp8266 docs are not very clear, but it is assumed that 11n means b/g/n). This setting limits the modulations that the radio will use, it does not indicate the current modulation used with a specific access point.
* `powersave` - Power saving mode: `none` (radio is on all the time), `ps-poll` (radio is off between beacons as determined by the access point's DTIM setting). Note that in 'ap' and 'sta+ap' modes the radio is always on, i.e., no power saving is possible.
* `savedMode` - The saved operation mode which will be applied at boot time: `off`, `sta`, `ap`, `sta+ap`.

*/


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setConfig",
  "generate" : "jswrap_wifi_setConfig",
  "params"   : [
    ["settings", "JsVar", "An object with the configuration settings to change."]
  ]
}
Sets a number of global wifi configuration settings. All parameters are optional and which are passed determines which settings are updated.
The settings available are:

* `phy` - Modulation standard to allow: `11b`, `11g`, `11n` (the esp8266 docs are not very clear, but it is assumed that 11n means b/g/n).
* `powersave` - Power saving mode: `none` (radio is on all the time), `ps-poll` (radio is off between beacons as determined by the access point's DTIM setting). Note that in 'ap' and 'sta+ap' modes the radio is always on, i.e., no power saving is possible.

Note: esp8266 SDK programmers may be missing an "opmode" option to set the sta/ap/sta+ap operation mode. Please use connect/scan/disconnect/startAP/stopAP, which all set the esp8266 opmode indirectly.
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getDetails",
  "generate" : "jswrap_wifi_getDetails",
  "return"   : ["JsVar", "An object representing the wifi station details, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the wifi details, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Retrieve the wifi station configuration and status details. The details object has the following properties:

* `status` - Details about the wifi station connection, one of `off`, `connecting`, `wrong_password`, `no_ap_found`, `connect_fail`, or `connected`. The off, bad_password and connected states are stable, the other states are transient. The connecting state will either result in connected or one of the error states (bad_password, no_ap_found, connect_fail) and the no_ap_found and connect_fail states will result in a reconnection attempt after some interval.
* `rssi` - signal strength of the connected access point in dB, typically in the range -110 to 0, with anything greater than -30 being an excessively strong signal.
* `ssid` - SSID of the access point.
* `password` - the password used to connect to the access point.
* `authMode` - the authentication used: `open`, `wpa`, `wpa2`, `wpa_wpa2` (not currently supported).
* `savedSsid` - the SSID to connect to automatically at boot time, null if none.

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getAPDetails",
  "generate" : "jswrap_wifi_getAPDetails",
  "return"   : ["JsVar", "An object representing the current access point details, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the current access point details, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Retrieve the current access point configuration and status.  The details object has the following properties:

* `status` - Current access point status: `enabled` or `disabled`
* `stations` - an array of the stations connected to the access point.  This array may be empty.  Each entry in the array is an object describing the station which, at a minimum contains `ip` being the IP address of the station.
* `ssid` - SSID to broadcast.
* `password` - Password for authentication.
* `authMode` - the authentication required of stations: `open`, `wpa`, `wpa2`, `wpa_wpa2`.
* `hidden` - True if the SSID is hidden, false otherwise.
* `maxConn` - Max number of station connections supported.
* `savedSsid` - the SSID to broadcast automatically at boot time, null if the access point is to be disabled at boot.

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "save",
  "generate" : "jswrap_wifi_save",
  "params"   : [
    ["what", "JsVar", "An optional parameter to specify what to save, on the esp8266 the two supported values are `clear` and `sta+ap`. The default is `sta+ap`"]
  ]
}
Save the current wifi configuration (station and access point) to flash and automatically apply this configuration at boot time, unless `what=="clear"`, in which case the saved configuration is cleared such that wifi remains disabled at boot. The saved configuration includes:

* mode (off/sta/ap/sta+ap)
* SSIDs & passwords
* phy (11b/g/n)
* powersave setting
* DHCP hostname

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "restore",
  "generate" : "jswrap_wifi_restore"
}
Restores the saved Wifi configuration from flash. See `Wifi.save()`.
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getIP",
  "generate" : "jswrap_wifi_getIP",
  "return"   : ["JsVar", "An object representing the station IP information, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the IP information, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Return the station IP information in an object as follows:

* ip - IP address as string (e.g. "192.168.1.5")
* netmask - The interface netmask as string
* gw - The network gateway as string
* mac - The MAC address as string of the form 00:00:00:00:00:00

Note that the `ip`, `netmask`, and `gw` fields are omitted if no connection is established:
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getAPIP",
  "generate" : "jswrap_wifi_getAPIP",
  "return"   : ["JsVar", "An object representing the esp8266's Access Point IP information, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the the IP information, i.e. the same object as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Return the access point IP information in an object which contains:

* ip - IP address as string (typ "192.168.4.1")
* netmask - The interface netmask as string
* gw - The network gateway as string
* mac - The MAC address as string of the form 00:00:00:00:00:00

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getHostByName",
  "generate" : "jswrap_wifi_getHostByName",
    "params"   : [
    ["hostname", "JsVar", "The hostname to lookup."],
    ["callback", "JsVar", "The callback to invoke when the hostname is returned."]
  ]
}
Lookup the hostname and invoke a callback with the IP address as integer argument. If the lookup fails, the callback is invoked with a null argument.
**Note:** only a single hostname lookup can be made at a time, concurrent lookups are not supported.

*/


/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getDHCPHostname",
  "generate" : "jswrap_wifi_getHostname",
  "return"   : ["JsVar", "The currently configured DHCP hostname, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the hostname, i.e. the same string as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Deprecated, please use getHostname.
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "getHostname",
  "generate" : "jswrap_wifi_getHostname",
  "return"   : ["JsVar", "The currently configured hostname, if available immediately."],
  "params"   : [
    ["callback", "JsVar", "An optional function to be called back with the hostname, i.e. the same string as returned directly. The callback function is more portable than the direct return value."]
  ]
}
Returns the hostname announced to the DHCP server and broadcast via mDNS when connecting to an access point.
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setDHCPHostname",
  "generate" : "jswrap_wifi_setHostname",
  "params"   : [
    ["hostname", "JsVar", "The new DHCP hostname."]
  ]
}
Deprecated, please use setHostname instead.
*/
/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setHostname",
  "generate" : "jswrap_wifi_setHostname",
  "params"   : [
    ["hostname", "JsVar", "The new hostname."]
  ]
}
Set the hostname. Depending on implemenation, the hostname is sent with every DHCP request and is broadcast via mDNS. The DHCP hostname may be visible in the access point and may be forwarded into DNS as hostname.local.
If a DHCP lease currently exists changing the hostname will cause a disconnect and reconnect in order to transmit the change to the DHCP server.
The mDNS announcement also includes an announcement for the "espruino" service.
*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setSNTP",
  "generate" : "jswrap_wifi_setSNTP",
  "ifdef"    : "ESP8266",
  "params"   : [
    ["server", "JsVar", "The NTP server to query, for example, `us.pool.ntp.org`"],
    ["tz_offset", "JsVar", "Local time zone offset in the range -11..13."]
  ]
}
Starts the SNTP (Simple Network Time Protocol) service to keep the clock synchronized with the specified server. Note that the time zone is really just an offset to UTC and doesn't handle daylight savings time.
The interval determines how often the time server is queried and Espruino's time is synchronized. The initial synchronization occurs asynchronously after setSNTP returns.
*/



/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setIP",
  "generate" : "jswrap_wifi_setIP",
  "ifdef"    : "ESP8266",
  "params"   : [
    ["settings", "JsVar", "Configuration settings"],
    ["callback", "JsVar", "The callback to invoke when ip is set"]
  ]
}
The `settings` object must contain the following properties.

* `ip` IP address as string (e.g. "192.168.5.100")
* `gw`  The network gateway as string (e.g. "192.168.5.1")
* `netmask` The interface netmask as string (e.g. "255.255.255.0")

*/

/*JSON{
  "type"     : "staticmethod",
  "class"    : "Wifi",
  "name"     : "setAPIP",
  "generate" : "jswrap_wifi_setAPIP",
  "ifdef"    : "ESP8266",
  "params"   : [
    ["settings", "JsVar", "Configuration settings"],
    ["callback", "JsVar", "The callback to invoke when ip is set"]
  ]
}
The `settings` object must contain the following properties.

* `ip` IP address as string (e.g. "192.168.5.100")
* `gw`  The network gateway as string (e.g. "192.168.5.1")
* `netmask` The interface netmask as string (e.g. "255.255.255.0")
*/
