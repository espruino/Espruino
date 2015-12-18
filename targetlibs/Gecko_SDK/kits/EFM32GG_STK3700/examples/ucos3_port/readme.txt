uC/OS-III RTOS on EFM32 using example.

This is a port of the uC/OS-III RTOS from Micrium Inc, http://www.micrium.com

The port has been adapted by Silicon Labs for use with our Development Kit..
It requires the standard EFM32 CMSIS package (including emlib).

The example has three tasks:
 - Task1: LED blink task
 - Task2: Receives characters from serial port and posts message to Task3
 - Task3: Receives message from Task2 and writes it on LCD and serial port

Serial port can be chosen between:
USART1 - location 1 is used, GPIO port D pins 0 and 1, baudrate 115200-8-N-1.
LEUART0 - location 0 is used, GPIO port D pins 4 and 5, baudrate 9600-8-N-1.

These USARTs are by default not connected to any serial port, so this needs
to be connected to external RS232 ports to be usable.

uC/OS-III is provided in source form for FREE evaluation, for educational use
or for peaceful research.

If you plan on using uC/OS-III in a commercial product you need to contact
Micrium to properly license its use in your product. We provide ALL the
source code for your convenience and to help you experience uC/OS-III.
The fact that the source is provided does NOT mean that you can use it without
paying a licensing fee.

For commercial use, contact Micrium, http://www.micrium.com for details.

Board:  Silicon Labs EFM32GG_STK3700 Starter Kit
Device: EFM32GG990F1024
