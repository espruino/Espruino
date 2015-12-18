Lightsensor example using LESENSE.

This example project uses the EFM32 CMSIS
and demonstrates the use of the LESENSE peripheral on the board.

This demo has two different modes.
To change between them, press PB0. In Mode0 (default). The LESENSE
module will wake up whenever a "dark" event is detected by the light sensor
below the "EFM32" sign on the right from the LCD on the STK. In Mode 1,
EFM32 will only wake up on every fifth "dark" event.

Board: Silicon Labs EFM32GG_STK3700 Development Kit
Device: EFM32GG990F1024
