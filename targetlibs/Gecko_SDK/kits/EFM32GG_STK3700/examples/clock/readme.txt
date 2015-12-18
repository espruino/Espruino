Wall Clock example using the segment LCD.

This example project uses the EFM32 CMSIS  and demonstrates  the use of
the LCD controller, RTC (real time counter), VCMP, GPIO and various
Energy Modes (EM).

The RTC is set up to wake the EFM32 from EM2 every minute. During this
interval, the LCD is updated to reflect the current time. The animation
feature is used to show the seconds. This is done by using the frame
event counter to drive the animation feature every second.

The VCMP is used to measure the input voltage on every wakeup. If the
voltage drops too low, voltage boost is enabled on the LCD. This makes
the LCD clear and readable even when running on low batteries.

Board:  Silicon Labs EFM32GG_STK3700 Development Kit
Device: EFM32GG990F1024
