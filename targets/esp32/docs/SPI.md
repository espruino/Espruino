# SPI
The ESP32 has three SPI buses called HSPI, VSPI and FSPI.  Of these three, HSPI
and VSPI are available for our use.  The default pin mappings for these
buses are:

| SPI  | MOSI | MISO | CLK | SS |
+------+------+------+-----+----+
| HSPI | 13   | 12   | 14  | 15 |
| VSPI | 23   | 19   | 18  | 5  |

In the world of Espruino, there are logical SPIs called SPI1 and SPI2.
We have mapped SPI1 -> HSPI and SPI2 -> VSPI.

As of 2016-10-26 the ESP-IDF SPI drivers have NOT yet been released however
we have a technique that is available to us through the use of the Arduino-ESP32
project.  We have linked the Arduino-ESP32 code with our current Espruino to achieve
SPI.  This will keep us going until the proper libraries are released.  The Espruino
externals will not be affected by our internal SPI implementation.