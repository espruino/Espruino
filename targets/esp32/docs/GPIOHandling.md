GPIO
====
ESP32 has a lot of GPIOs and supports different protocols etc. In this document we focus on simple GPIO binary handling. Documents for SPI, Analog, PWM etc. will follow.

###Available GPIOs / pins###
Even having GPIO named from 0 to 39, does not mean, we have all available. Some of them don't even exist, and only a few are available on DevkitC.

| CPU pin | GPIO | Devkit C|
| --- | --- | --- |
| 5 | 36 | X |
| 6 | 37 |  |
| 7 | 38 |  |
| 8 | 39 | X |
| 10 | 34 | X |
| 11 | 35 | X |
| 12 | 32 | X |
| 13 | 33 | X |
| 14 | 25 | X |
| 15 | 26 | X |
| 16 | 27 | X |
| 17 | 14 | X |
| 18 | 12 | X |
| 20 | 13 | X |
| 21 | 15 | X |
| 22 | 2 |  X|
| 23 | 0 |  X|
| 24 | 4 |  X|
| 25 | 16 | X |
| 27 | 17 | X |
| 28 | 9 |  |
| 29 | 10 |  |
| 30 | 11 |  |
| 31 | 6 |  |
| 32 | 7 |  |
| 33 | 8 |  |
| 34 | 5 | X |
| 35 | 18 | X |
| 36 | 23 | X |
| 38 | 19 | X |
| 39 | 22 | X |
| 40 | 32 |  |
| 41 | 10 |  |
| 42 | 21 | X |

###Binary I/O###
Espruino/targets/esp32/jshardware.c is the main connection between Espruino Core and special sources to handle ESP32 specific commands.
For simple GPIO handling we have to fill following functions. For more information/detailed please go to source

| Function | Description |
| --- | --- |
| jshPinSetState | Set the state of the specific pin. |
| jshPinGetState | return current State of pin |
| jshPinSetValue | set the value of pin |
| jshPinGetValue | get the value of pin |

###Interrupts###
For commands like setWatch fast response of changes on pin is a must. Fastest way is catching interrupts. Following functions will do this particular job

| Function | Description |
| --- | --- |
| in jshInit | add a line to register GPIO interrupts (gpio_isr_register) |
| gpio_intr_test | function to handle interrupts, calls jshPushIOWatchEvent for each changed pin that is enabled for interrupt |
| jshGetWatchedPinState | Get the state of the pin associated with the event flag. Take care on parameter its IOEventFlags not pin |
| jshPinWatch | Enables or disables interrupt for pin |
| jshIsEventForPin | Determine if a given event is associated with a given pin |

###General functions###
Please have in mind, there are pins and IOEventFlags. Handling them is simple, we only have to add EV_EXTIO. There are some definitions in Espruino Core. In jshardware.c we handle it this way. May be in future we find a better way.

| Function | Description |
| --- | --- |
| pinToEV_EXTI | maps a pin to an event |
|  | maping an event to a pin is done by subtracting EV_EXTI0 from IOEventFlag |
| pinToESP32Pin | converts a number to a gpio_num_t |
