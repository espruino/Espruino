The Espruino environment requires a board to provide a file called `jshardware.c` that contains within it the functions that are expected to be hardware specific.   This page describes the ESP8266 implementation of these functions.

----

######jshUARTKick

This function is called to inform the UART that it has some work to do.  In the ESP8266 implementation, it asks Espruino for each character to write and keeps looping over them until the buffer is drained.

----

######jshGetSystemTime

Return the system time in microseconds.

`JsSysTime jshGetSystemTime()`

For the ESP8266 we can leverage the SDK supplied `system_get_time()`.

----

######jshDelayMicroseconds

Delay for a period of microseconds.

`void jshDelayMicroseconds(int microsec)`

For the ESP8266, we might think that we can use a simple mapping to the SDK supplied `os_delay_us()` function however, the rules of ESP8266 say to try not sleep for more than 10 milliseconds and nothing in this spec for the function says that it couldn't
sleep for much longer.  As such we need special work to make sure we keep yielding often enough to satisfy everyone.

----

######jshInit

This function is called by the user application to initialize hardware however it is architected to be part of the `jshardware.c` file.  The rules say that it must call `jshInitDevices` which is a provided function.  This is also where we would do any ESP8266 hardware initialization.

`void jshInit()`