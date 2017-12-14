#!/bin/bash

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2016 Gordon Williams <gw@pur3.co.uk>
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

BOARDNAME=PUCKJS
ESPRUINOFILE=`python scripts/get_board_info.py $BOARDNAME "common.get_board_binary_name(board)"`

rm -f $ESPRUINOFILE

export BOARD=PUCKJS
export RELEASE=1

BOOTLOADER=1 make clean
BOOTLOADER=1 make || { echo 'Build failed' ; exit 1; }

make clean
make || { echo 'Build failed' ; exit 1; }

echo ---------------------
echo Finished! Written to $ESPRUINOFILE
echo nrfjprog --family NRF52 --clockspeed 50000 --program $ESPRUINOFILE --chiperase --reset
echo ---------------------


