For the ESP8266 project, we will need native functions.  These are functions exposed to the JS programmer that are implemented as native C code as opposed to JS.  These functions can then leverage the implementation capabilities provided by the ESP8266 SDK.

Examples of objects containing such functions will include:

* `ESP8266WiFi` - Access to native ESP8266 WiFi.

See also:
* [Espruino native functions](https://github.com/espruino/Espruino/tree/master/libs)


To define a new native set of functions, we need to annotate the source code of functions we wish to expose.  The format is:

    /*JSON{
      "type"     : "staticmethod",
      "class"    : "ESP8266WiFi",
      "name"     : "setAutoconnect",
      "generate" : "jswrap_ESP8266WiFi_setAutoconnect",
      "generate_full": ???,
      "params"   : [
        ["autoconnect","JsVar","True if we wish to autoconnect."]
      ],
      "return"   : ["JsVar","A boolean representing our auto connect status"],
      "return_object" : "Restart"
    }*/

It is not yet known the meanings and values of these nor which are optional vs mandatory nor whether there are additional settings not yet discovered.

* `type` - Values seen include `class`, `staticmethod`, `method`, `library`, `event`, `idle`, `init`, `kill`.