 _____                 _                                                        
|   __|___ ___ ___ _ _|_|___ ___                                                
|   __|_ -| . |  _| | | |   | . |                                               
|_____|___|  _|_| |___|_|_|_|___|                                               
          |_|                                                               
   Copyright 2015 Gordon Williams
                                               
http://www.espruino.com

--------------------------------------------------------------

There are a few different binaries in this ZIP file, for different
types of Microcontroller:


espruino_#v##_pico_1r3_cc3000.bin  
espruino_#v##_pico_1r3_wiznet.bin  
   - The firmware image for Espruino Pico Boards.
     We'd strongly suggest that you use the Web IDE to flash this.
     Each image is for a different type of networking device.
     If you don't want a network device, it doesn't matter which you choose.

espruino_#v##_espruino_1r3.bin  
espruino_#v##_espruino_1r3_wiznet.bin  
   - The firmware image for Original Espruino Boards
     We'd strongly suggest that you use the Web IDE to flash this.
     Each image is for a different type of networking device.
     If you don't want a network device, it doesn't matter which you choose.

espruino_#v##_wifi.bin  
   - The firmware image for Espruino WiFi Boards
     We'd strongly suggest that you use the Web IDE to flash this.

espruino_#v##_puckjs.zip
   - The firmware image for Espruino Puck.js Devices
     See http://www.espruino.com/Puck.js#firmware-updates for more information

espruino_#v##_hystm32_24_ve.bin 
   - 'HY'STM32F103VET6 ARM with 2.4" LCD display
     This is available from eBay

espruino_#v##_hystm32_28_rb.bin 
   - 'HY'STM32F103RBT6 ARM with 2.8" LCD display
     This is available from eBay

espruino_#v##_hystm32_32_vc.bin 
   - 'HY'STM32F103VCT6 ARM with 3.2" LCD display
     This is available from eBay
      
espruino_#v##_olimexino_stm32.bin
   - You will need to overwrite the Maple bootloader to install this.
     Espruino is now too large to fit in flash alongside it.
   - Olimexino-STM32 Arduino form factor board
   - Leaf Labs Maple Arduino form factor board

espruino_#v##_stm32vldiscovery.bin
   - STM32VLDISCOVERY board

espruino_#v##_stm32f3discovery.bin
   - STM32F3DISCOVERY board
   
espruino_#v##_stm32f4discovery.bin
   - STM32F4DISCOVERY board

espruino_#v##_nucleof401re.bin
   - ST NUCLEO-F401RE board

espruino_#v##_nucleof411re.bin
   - ST NUCLEO-F411RE board

espruino_#v##_raspberrypi
   - Raspberry Pi executable (just copy it to the device and run it)
   NOTE: There is GPIO support (which requires you to run Espruino as root)
   however there is no Serial, SPI, OneWire or I2C support at the moment so
   you're pretty limited!

espruino_#v##_microbit.hex
   - Espruino for the BBC micro:bit - just copy this file onto the
   flash drive that appears when you plug the micro:bit in.

espruino_#v##_ruuvitag.zip
   - The firmware image for Ruuvitag Devices
     See http://www.espruino.com/Ruuvitag for more information


ESP8266 / ESP32
---------------

See http://www.espruino.com/EspruinoESP8266 for more info

espruino_#v##_esp8266_combined_512.bin
   - ESP8266 'combined' port for 512k devices like ESP01
       Flash with: esptool.py write_flash 0 espruino_#v##_esp8266_combined_512.bin

espruino_#v##_esp8266_4mb_combined_4096.bin
   - ESP8266 'combined' port for 4MB devices like ESP12
       Flash with: esptool.py write_flash 0 espruino_#v##_esp8266_combined_4096.bin

espruino_#v##_esp8266
   - ESP8266 port as separate files - see README in directory for more information

espruino_#v##_esp8266_4mb
   - ESP8266 port for 4mb devices as separate files - see README in directory for more information

espruino_#v##_esp32.zip
   - The firmware image for ESP32 Devices
     See http://www.espruino.com/ESP32 for more information




For more information on devices, and on how to flash these binary files on to 
each device, please see our website, http://www.espruino.com

NOTES:

* On the STM32F4DISCOVERY the default USART is USART2 (because
USART1 shares some pins with USB). This means you must connect
serial connections to PA2/PA3 NOT PA9/PA10 as you would for
the STM32VLDISCOVERY.

