#!/bin/bash

echo ----------------------
echo telnet localhost 4444
echo ----------------------
echo   ... dump_image filename.bin 0x08000000 0x2ffff
echo       exit

OPENOCD=`dirname $0`/../openocd-0.9.0
# OpenOCD from https://github.com/adafruit/Adafruit_nRF51822_Flasher
$OPENOCD/ubuntu/openocd -s $OPENOCD/scripts -f interface/stlink-v2.cfg -f target/nrf51.cfg
