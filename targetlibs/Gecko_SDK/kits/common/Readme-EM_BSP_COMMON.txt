====== Board Support Package and Drivers ======

This package includes the board support package and drivers
common to all Silicon Labs EFM32 kits.

====== Dependencies ======

This package _requires_ the EFM32 CMSIS package to be installed at the
same level as this package. If you did not get this as part of the
Simplicity Studio package, you should also download and install the
EFM32 CMSIS package. See the Changes file for required version.

The EFM32 CMSIS package includes the necessary EFM32 drivers, register,
and bit field definitions that are required for the included BSP and
example projects.

The CMSIS package requires C99 support, and so does this package.

====== File structure ======

common/bsp
   C source and header files for kit specific functionality, such as
   enabling kit specific peripherals to be accessible from the EFM32
   (configures on board analog switches - that are there to prevent
   current leakage, gives access to LEDs, dip switches, joystick, i2c
   devices and so on).

common/bspdoc
   Doxygen documentation of BSP and Drivers. Use a web browser and open the
   index.html file in the html directory.

common/drivers
   Various drivers for kit specific components.

====== Updates ======

Silicon Labs continually works to provide updated and improved example code,
header files and other software of use for our customers. Please check

http://www.silabs.com/support/pages/document-library.aspx?p=MCUs--32-bit

for the latest releases.

====== License ======

License information for use of the source code is given at the top of
all C files.

(C) Copyright Silicon Laboratories Inc. 2015. All rights reserved.
