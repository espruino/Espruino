PWM
===
Espruino supports Analog output using PWM

##General##
ESP32 supports (at least) 2 optiins for PWM. Both have pros and cons.
- sigmadelta
driver for sigmadelta is very easy to use. Problem is a limitation in frequency.
Base of frequency is 80Mhz, which can be divided by a values from 1 to 255.
Therefore a frequency of 30khz is impossible this way
- ledc
driver for ledc is designed to control leds. Not a big surprise, if you see the name :-)
Driver allows a lot of options, like fadein and fadeout. There are 8 channel, which can be assigned to a GPIO. In the background are 4 timer, where each channel needs to be assigned to a specific timer. Therefore only 4 different frequencies are possible.
Last not least, Frequency is limited to about 75Khz

##Usage for Espruino##
First version of PWM uses ledc driver. Usually, this matches needs for frequency much more than sigmadelta. Mapping of PWM channel to timer is done in a special way.
There are 5 channel usable, as long as 5Khz are ok for frequency. Channels are internally controlled. If you try to use more than 5, you will get a message.
Left 3 channels are designed to support a frequency of your choice, as long as it is between 1hz and 78 Khz. If you try to use a 4th channel with a frequency, other than 5Khz you will a meesage.

##Problem##
Handling of PWM with those 5/3 method is not always easy to follow. As long as we don't have a better solution, its better to have this than nothing.
Limitation of frequency is boring, but as long as we use it for analog output, it should be acceptable.
Please have in mind, PWM via analog always need a lowpass filter, usually a simple RC.

##Status##
As mentioned above, this is a first step. Hopefully we get a better solution in the future. We could build our own solution, using RMT. If somebody wants to go into this direction, you are welcome. In that case please take care on implementation of digitalPulse, see jshardwarPulse for more information.