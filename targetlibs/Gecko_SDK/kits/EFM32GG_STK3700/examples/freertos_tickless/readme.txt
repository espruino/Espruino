FreeRTOS - Tickless example

This example project uses the FreeRTOS, and gives a basic demonstration of using two tasks,
one sender generating number and one receiver that displays the number on the LCD. The FreeRTOS is 
configured in tickless mode, going into EM2 when no tasks are active.

EM3 mode cannot be used in this example because the LCD driver and timer are not available in EM3.
For more details, see the configuraiton in FreeRTOSConfig.h file.

Board:  Silicon Labs EFM32GG_STK3700 Starter Kit
Device: EFM32GG990F1024
