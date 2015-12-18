USB device enumerator example.

This example project uses the USB host stack to implement
a simple device enumerator. When a device is attached its descriptors are read
and the vendor and product id's (VID/PID) are displayed on the LCD.
The device attached will not be configured.

Note that USB disk drives may fail, as some of them draw too much current
when attached.

Board:  Silicon Labs EFM32GG-STK3700 Development Kit
Device: EFM32GG990F1024
