Hardware Timer
===
Espruino uses Highspeed hardware timer for functions like waveform.

##General##
ESP32 supports 2 groups of timer, both having 2 timer.
esp-idf supports driver for timer with components/driver/include/driver/timer.h

##Conection to Espruino##
An interrupt needs to be defined with IRAM_ATTR. To connect to Espruino, the isr (interrupt service routine) calls jstUtilTimerInterruptHandler.

##Problem##
This function is not defined  as IRAM_ATTR or something similiar. Due to this, or whatever other reason we get a reset after some calls. There could be many reasons for that like:
- problems with floating point
- running out of memory, since isr use their own stack
- running out of time for isr
- something else ???

##Solution##
As long as we don't know better, the solution is to seperate isr from calling jstUtilTimerInterruptHandler.
For that we create a new task, which waits for a notification and after receiving one, calls the function in Espruino. Inside the isr we now have to send the notification. Sending a notification is done with vTaskNotifyGiveFromISR, a function of FREErtos.
Waiting for the notification uses ulTaskNotifyTake, another function in FREErtos.

##Status##
A first step with a simple waveform on an DAC port (GPIO25/GPIO26) runs without any crashes now. Thats good part of the answer, badly it does not change output :-(
We are working on that......