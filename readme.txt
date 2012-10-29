 _____                 _                                                        
|   __|___ ___ ___ _ _|_|___ ___                                                
|   __|_ -| . |  _| | | |   | . |                                               
|_____|___|  _|_| |___|_|_|_|___|                                               
          |_|                                                               
   Copyright 2012 Gordon Williams
                                               
http://www.espruino.com

This version of Espruino is provided free for personal
use only. This must not be sold either as software or
pre-programmed hardware without the author's permission.

For more information on how to program this onto your
STM32VLDISCOVERY board, please see the website above.



Please see functions.txt for a description of functions
available to you.



NOTES:

* On the STM32F4DISCOVERY the default USART is USART2 (because
USART1 shares some pins with USB). This means you must connect
serial connections to PA2/PA3 NOT PA9/PA10 as you would for
the STM32VLDISCOVERY.

* When you've got your code as you want it, type 'save()' and
the state of the VM will be written to flash. The next time 
Espruino powers on it'll resume from where it left off.

* If you've used save() and you want the device to run some
code the first time it starts (for instance to initialise
some external hardware like an LCD), create a function or
string called 'onInit'.

