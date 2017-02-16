# ESP32 Specific
A static class called "ESP32" is provided that supplies ESP32 specific APIs.
Note that there is no "requires" needed for these functions.  They are static
and belong to the class.  For example, to invoke the `reboot` method, one would
code:

```
ESP32.reboot();
```

These include:

## ESP32.neopixelWrite
Not yet implemented

## ESP32.getState
Returns a Java Script object that has the following properties filled in:
* `sdkVersion` - The version of the Espressif SDK that Espruino is using.
* `freeHeap` - The amount of heap storage unused.

##ESP32.setLogLevel
The ESP32 environment has built in diagnostics and logging.  Calling this
method changes the logging levels for either specific component or the
environment as a whole.  The signature of the function is:

`setLogLevel(tag, level)`

where:

* `tag` - The logging tag to specify.  Use `*` for all tags.
* `level` - The logging level to log at or better.  The levels are:
    * verbose
    * debug
    * info
    * warn
    * error
    * none

##ESP32.reboot
Reboot the ESP32 device.