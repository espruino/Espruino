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
if [[ ! -v BINDIR ]]; then 
  echo "BINDIR not set, assuming 'bin'";
  BINDIR=bin
fi
BOARDNAME=PICO_R1_3
ESPRUINOBINARY=`python scripts/get_board_info.py $BOARDNAME "common.get_board_binary_name(board)"`
BOOTLOADERFILE=$BINDIR/bootloader_$ESPRUINOBINARY
IMGFILE=$BINDIR/full_$ESPRUINOBINARY.bin
ESPRUINOFILE=$BINDIR/$ESPRUINOBINARY
rm -f $ESPRUINOFILE $BOOTLOADERFILE $IMGFILE

export BOARD=PICO_R1_3
# export DEBUG=1
export RELEASE=1

BOOTLOADER=1 make clean
BOOTLOADER=1 make || { echo 'ERROR Build failed' ; exit 255; }

make clean
make || { echo 'ERROR Build failed' ; exit 255; }

BOOTLOADERSIZE=`python scripts/get_board_info.py $BOARDNAME "common.get_espruino_binary_address(board)"`
IMGSIZE=$(expr $BOOTLOADERSIZE + $(du "$ESPRUINOFILE" | cut -f1))

echo ---------------------
echo Image Size = $IMGSIZE

echo ---------------------
echo Create blank image
echo ---------------------
tr "\000" "\377" < /dev/zero | dd bs=1 count=$IMGSIZE of=$IMGFILE || { echo 'ERROR Build failed' ; exit 255; }

echo Add bootloader
echo ---------------------
dd bs=1 if=$BOOTLOADERFILE of=$IMGFILE conv=notrunc || { echo 'ERROR Build failed' ; exit 255; }

echo Add espruino
echo ---------------------
dd bs=1 seek=$BOOTLOADERSIZE if=$ESPRUINOFILE of=$IMGFILE conv=notrunc || { echo 'ERROR Build failed' ; exit 255; }


mv $IMGFILE $ESPRUINOFILE || { echo 'ERROR Build failed' ; exit 255; }
echo ---------------------
echo Finished! Written to $IMGFILE and copied to $ESPRUINOFILE
echo dfu-util -a 0 -s 0x08000000 -D ${ESPRUINOFILE}
echo ---------------------

#echo python scripts/stm32loader.py -b 460800 -ewv $IMGFILE
#python scripts/stm32loader.py -b 460800 -ewv $IMGFILE


