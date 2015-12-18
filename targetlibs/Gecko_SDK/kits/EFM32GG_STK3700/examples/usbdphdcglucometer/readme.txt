USB PHDC (Personal Healthcare Device Class) example.

This example is supplied in binary form only. Install the "HealthLink" PC
application available from http://www.lnihealth.com to check out this demo.

This example contains source code for PHDC support in
the file phdc_descriptors.c and phdc_descriptors.h.
The PHDC interface is quite simple. It supports the
BULK IN and BULK OUT channels on a single interface
and does not support metadata (so far an unused
feature in existing PHDC devices).

The code consists primarily of filling out the
descriptors and a set of handlers for the class
GET STATUS, SET and CLEAR FEATURE functions.
Since metadata is not supported, the responses
to these host requests are very simple.

USE WITH THE DEMO GLUCOMETER

There are three buttons
Reset, PB0, and PB1
Put the selector switch in the position 'USB'
Reset causes the board to start as if one first plugged in the cable.
This assumes that the USB host driver for Continua devices is installed.

If the manager is not running, plugging in the device will cause the
agent to attempt to associate. This involves sending an AARQ and
waiting for the manager to send an AARE. The agent waits 10 seconds
and tries again. It repeats the attempt four times. After the forth
time the agent sends an abort. The LCD displays the 10 second
count between each attempt.

To restart the sequence press RESET.

With a manager connected the manager should respond to the AARQ.
The screen will display LINKED.
The agent loads fake data into a PM Store on every startup which
HealthLink automatically gets. To send an individual measurement
press PB0. To disassociate from the manager press PB1. The LCD
will state 'ReLinks' which is 7-chars to state 'Return (PB0)
relinks'. So to re-associate with the manager press PB0.

Board:  Silicon Labs EFM32GG-STK3700 Development Kit
Device: EFM32GG990F1024
