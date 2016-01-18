#!/bin/bash

if [ $# -ne 1 ]
then
  echo "USAGE:"
  echo "scripts/flash_nrf51_openocd.sh espruino_XXXX.hex"
  exit 1
fi

FIRMWARE=$1
# must be hex file

OPENOCD=`dirname $0`/../openocd-0.9.0
# OpenOCD from https://github.com/adafruit/Adafruit_nRF51822_Flasher
$OPENOCD/ubuntu/openocd -s $OPENOCD/scripts -f interface/stlink-v2.cfg -f target/nrf51.cfg -c init -c "reset init" -c halt -c "nrf51 mass_erase" -c "program $FIRMWARE verify" -c reset -c exit
