Board Support Package API demo for voltage and current readout.

This example project uses the EFM32 CMSIS and demonstrates the use of
the STK BSP.

The BSP is used to read out the current consumption and VMCU voltage level
from the board controller. The readings are printed to the display.

The BSP use a 115800-N-1 UART to communicate with the board controller,
if you do not need the board support functions, there is no need to
include the BSP in your project. Currently, the UART communication with
16x oversampling is used, limiting the communication channel to high
frequencies for the peripheral clock.

Board:  Silicon Labs EFM32GG-STK3700 Development Kit
Device: EFM32GG990F1024
