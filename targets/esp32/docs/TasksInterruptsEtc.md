Internals
===
RTOS and the SDK for ESP32 bring up some new screws and bolts.
In this document, we will list at least some of them.

##Tasks##
Rtos supports a simple form of tasks. These tasks get a priority on start and some other information.

| Task | Stack | Priority |
| --- | --- | --- |
| espruinoTask | 10000 | 5 |


##Interrupt Level##
Each realtime OS uses interrupts, and so does RTOS. There are some predefined or reserved for future use. For more info see esp-idf/components/esp32/include/soc/soc.h line 259ff..
In this list we have all interrupts handled for Espruino

| Intr num | level | descripton |
| --- | --- | --- |
| 18 | 1 | GPIO interrupt used for setWatch |

