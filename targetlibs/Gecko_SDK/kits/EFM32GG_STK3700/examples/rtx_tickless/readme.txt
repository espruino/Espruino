Keil RTX RTOS - tick-less example

This example project uses the Keil RTX RTOS, and gives a basic demonstration
of using two tasks; one sender generating number and one receiver that displays
the number on LCD. The RTX is configured in tick-less mode, going into EM2 when
no tasks are active. This example is intended as a skeleton for new projects
using Keil RTX for energy aware applications.

Board:  Silicon Labs EFM32GG-STK3700
Device: EFM32GG990F1024
