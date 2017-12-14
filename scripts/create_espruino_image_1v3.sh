#!/bin/bash

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# Creates a binary file containing both Espruino and the bootloader
# ----------------------------------------------------------------------------------------

cd `dirname $0` # scripts
cd ..            # main dir
BASEDIR=`pwd`

BOARDNAME=ESPRUINOBOARD
ESPRUINOFILE=`python scripts/get_board_info.py $BOARDNAME "common.get_board_binary_name(board)"`
BOOTLOADERFILE=bootloader_$ESPRUINOFILE
IMGFILE=espruino_full.bin
rm -f $ESPRUINOFILE $BOOTLOADERFILE $IMGFILE

export BOARD=ESPRUINOBOARD
# export USB_PRODUCT_ID=0x5741 # For test harness board only
# export DEBUG=1
export RELEASE=1

BOOTLOADER=1 make clean
BOOTLOADER=1 make || { echo 'Build failed' ; exit 1; }

make clean
make || { echo 'Build failed' ; exit 1; }

BOOTLOADERSIZE=`python scripts/get_board_info.py $BOARDNAME "common.get_bootloader_size(board)"`
IMGSIZE=$(expr $BOOTLOADERSIZE + $(stat -c%s "$ESPRUINOFILE"))

echo ---------------------
echo Image Size = $IMGSIZE

echo ---------------------
echo Create blank image
echo ---------------------
tr "\000" "\377" < /dev/zero | dd bs=1 count=$IMGSIZE of=$IMGFILE || { echo 'Build failed' ; exit 1; }

echo Add bootloader
echo ---------------------
dd bs=1 if=$BOOTLOADERFILE of=$IMGFILE conv=notrunc || { echo 'Build failed' ; exit 1; }

echo Add espruino
echo ---------------------
dd bs=1 seek=$BOOTLOADERSIZE if=$ESPRUINOFILE of=$IMGFILE conv=notrunc || { echo 'Build failed' ; exit 1; }


cp $IMGFILE $ESPRUINOFILE || { echo 'Build failed' ; exit 1; }
echo ---------------------
echo Finished! Written to $IMGFILE and copied to $ESPRUINOFILE
echo python scripts/stm32loader.py -p /dev/ttyUSB0 -b 460800 -ewv $ESPRUINOFILE
echo ---------------------

#echo python scripts/stm32loader.py -b 460800 -ewv $IMGFILE
#python scripts/stm32loader.py -b 460800 -ewv $IMGFILE
