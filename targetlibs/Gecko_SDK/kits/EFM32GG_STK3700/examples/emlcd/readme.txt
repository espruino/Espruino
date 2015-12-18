Energy Modes with segment LCD example.

This example project use EFM32 CMSIS and the emlib peripheral library to
demonstrate the use of the LCD controller, RTC (real time counter), GPIO and
various Energy Modes (EM) on the starter kit.

EM2 is used for delays in between refreshing the LCD display, and a lot of
"eye candy" are present to show off the MCU module's LCD display.

The LCD controller drives the display down to EM2. In the demo, EM2 is used
for "most" delays to show off this feature. The user can press PB0 or PB1 to
activate EM3 and EM4. EM4 requires a system reset, while PB0 again will wake
up the system  from Energy Mode 3.

Board:  Silicon Labs EFM32STG_STK3700 Starter Kit
Device: EFM32GG990F1024

