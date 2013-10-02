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

BOOTLOADER=1 RELEASE=1 ESPRUINO_1V1=1 make clean
BOOTLOADER=1 RELEASE=1 ESPRUINO_1V1=1 make || { echo 'Build failed' ; exit 1; }

# TODO: use RELEASE=1 here
ESPRUINO_1V1=1 make clean
ESPRUINO_1V1=1 make || { echo 'Build failed' ; exit 1; }


ESPRUINOFILE=espruino_espruino_1v1.bin
BOOTLOADERFILE=bootloader_espruino_1v1.bin
IMGFILE=espruino_full.bin
BOOTLOADERSIZE=10240
IMGSIZE=$(expr $BOOTLOADERSIZE + $(stat -c%s "$ESPRUINOFILE"))

echo ---------------------
echo Image Size = $IMGSIZE

echo ---------------------
echo Create blank image
echo ---------------------
tr "\000" "\377" < /dev/zero | dd bs=1 count=$IMGSIZE of=$IMGFILE

echo Add bootloader
echo ---------------------
dd bs=1 if=$BOOTLOADERFILE of=$IMGFILE conv=notrunc

echo Add espruino
echo ---------------------
dd bs=1 seek=$BOOTLOADERSIZE if=$ESPRUINOFILE of=$IMGFILE conv=notrunc

echo ---------------------
echo Finished! Written to $IMGFILE
echo ---------------------

python scripts/stm32loader.py -ewv espruino_full.bin
