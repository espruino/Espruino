# Bangle.js 3 LCD Controller firmware

Due to the large amount of IO needed to drive the Memory LCD screen on Bangle.js 3
(it's 6 bit parallel) and strict timing requirements, we've opted to use a separate
microcontroller (a PY32F040) to handle the LCD (and ancilliaries) on Bangle.js 3, which is connected
via an SPI bus.

This is the source code for that PY32 microcontroller.

The LCD and buttons are connected direct to the microcontroller (as are the nRF54 SWD pins),
allowing it to act as a supervisor to the nRF54, ensuring that it can always be factory reset.

------------

We've used the Puya headers, libs, and fragments of Makefile provided in https://github.com/IOsetting/py32f0-template as a base for this work.
