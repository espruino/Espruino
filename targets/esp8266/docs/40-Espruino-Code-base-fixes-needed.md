The following items have been learned about the Espruino code base that need addressed:

* The python code contains `print <value>` where we seem to need `print(<value>)`.  Change made.
* the __WORDSIZE is not defined but is attempted to be checked for equality to 64.
* The scripts/build_platform_config.py needed changed to add a family check for ESP8266.
* jsnative.c - Addition of matching XTENSA architecture to ARM architecture.  Change made.
* Addition of `JSNETWORKTYPE_ESP8266_BOARD` into `network.h`.
* Within `jsutils.h` there is a definition for `ALWAYS_INLINE`.  For the ESP8266 this needs to be set to
defined but with no value.