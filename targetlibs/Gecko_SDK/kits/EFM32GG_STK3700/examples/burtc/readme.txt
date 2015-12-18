Backup power domain RTC example.

This example demonstrate use of the backup power domain and the
backup real time clock (BURTC).

Start the example with the Power Source Selector switch in the "DBG" position.
When the clock is running you can adjust time by pressing pushbuttons
PB0 and PB1.
When you move the Power Source Selector switch to the "USB" position, the
EFM is turned off and the BURTC peripheral runs on power from the BU capacitor.
(See User manual for EFM32GG-STK3700 starter kit for details on the hardware).
When you move the Power Source Selector switch back to the "DBG" position, you
can verify that time has been recorded correctly.

Board:  Silicon Labs EFM32GG-STK3700 Starter Kit
Device: EFM32GG990F1024
