Internals
===
RTOS and the SDK for ESP32 bring up some new screws and bolts.
In this document, we will list at least some of them.

##Tasks##
Rtos supports a simple form of tasks. These tasks get a priority on start and some other information.

| Task | Stack | Priority | CPU |
| --- | --- | --- | --- |
| EspruinoTask | 10000 | 5 | 0 |
| ConsoleTask | 2048 | 20 | 0 |
| TimerTask | 2048 | 19 | 0 |


##Interrupt Level##
Each realtime OS uses interrupts, and so does RTOS. There are some predefined or reserved for future use. For more info see esp-idf/components/esp32/include/soc/soc.h line 259ff..

Based on actual version of esp-idf, interrupt levels are assigned automatically.

##Timer##
ESP32 generally support 2 groups of timers each having 2 timer. Timer is used for interrupt driven handling inside Espruino like Waveforms.

| Group | Timer | used for |
| --- | --- | --- |
| 0 | 0 | Espruino Util Timer |
| 0 | 1 | testing only |
| 1 | 0 | free |
| 1 | 1 | free |

##Uarts##
ESP32 supports 3 uarts, which can be assigned to GPIO ports. Espruino right now only uses uart_num_0 for console.

| Uart | used for |
| --- | --- |
| uart_num_0 | espruino console |
| uart_num_1 | free |
| uart_num_2 | free |
