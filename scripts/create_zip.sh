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
# Creates a Zip file of all common Espruino builds
# ----------------------------------------------------------------------------------------

cd `dirname $0`
cd ..

VERSION=`sed -ne "s/^.*JS_VERSION.*\"\(.*\)\"/\1/p" src/jsutils.h`
DIR=`pwd`
ZIPDIR=$DIR/zipcontents
ZIPFILE=$DIR/archives/espruino_${VERSION}.zip
rm -rf $ZIPDIR
mkdir $ZIPDIR

echo ------------------------------------------------------
echo                          Building Version $VERSION
echo ------------------------------------------------------

RELEASE=1 OLIMEX=1 make clean
RELEASE=1 OLIMEX=1 make || { echo 'Build failed' ; exit 1; }
cp espruino_olimexino_stm32.bin  $ZIPDIR/espruino_${VERSION}_olimexino_stm32.bin || { echo 'Build failed' ; exit 1; }

# Don't bother now - this is too big to fit sensibly in flash
#RELEASE=1 OLIMEX_BOOTLOADER=1 make clean
#RELEASE=1 OLIMEX_BOOTLOADER=1 make || { echo 'Build failed' ; exit 1; }
#cp espruino_olimexino_bootloader_stm32.bin  $ZIPDIR/espruino_${VERSION}_olimexino_bootloader_stm32.bin  || { echo 'Build failed' ; exit 1; }

RELEASE=1 HYSTM32_24=1 make clean
RELEASE=1 HYSTM32_24=1 make || { echo 'Build failed' ; exit 1; }
cp espruino_hystm32_24_ve.bin  $ZIPDIR/espruino_${VERSION}_hystm32_24_ve.bin  || { echo 'Build failed' ; exit 1; }

RELEASE=1 HYSTM32_28=1 make clean
RELEASE=1 HYSTM32_28=1 make || { echo 'Build failed' ; exit 1; }
cp espruino_hystm32_28_rb.bin  $ZIPDIR/espruino_${VERSION}_hystm32_28_rb.bin  || { echo 'Build failed' ; exit 1; }

RELEASE=1 HYSTM32_32=1 make clean
RELEASE=1 HYSTM32_32=1 make || { echo 'Build failed' ; exit 1; }
cp espruino_hystm32_32_vc.bin  $ZIPDIR/espruino_${VERSION}_hystm32_32_vc.bin  || { echo 'Build failed' ; exit 1; }

RELEASE=1 STM32VLDISCOVERY=1 make clean
RELEASE=1 STM32VLDISCOVERY=1 make || { echo 'Build failed' ; exit 1; }
cp espruino_stm32vldiscovery.bin  $ZIPDIR/espruino_${VERSION}_stm32vldiscovery.bin  || { echo 'Build failed' ; exit 1; }

RELEASE=1 STM32F3DISCOVERY=1 make clean
RELEASE=1 STM32F3DISCOVERY=1 make || { echo 'Build failed' ; exit 1; } 
cp espruino_stm32f3discovery.bin  $ZIPDIR/espruino_${VERSION}_stm32f3discovery.bin  || { echo 'Build failed' ; exit 1; }

RELEASE=1 STM32F4DISCOVERY=1 make clean
RELEASE=1 STM32F4DISCOVERY=1 make || { echo 'Build failed' ; exit 1; } 
cp espruino_stm32f4discovery.bin  $ZIPDIR/espruino_${VERSION}_stm32f4discovery.bin  || { echo 'Build failed' ; exit 1; }

RELEASE=1 RASPBERRYPI=1 make clean
RELEASE=1 RASPBERRYPI=1 make || { echo 'Build failed' ; exit 1; } 
cp espruino_raspberrypi  $ZIPDIR/espruino_${VERSION}_raspberrypi  || { echo 'Build failed' ; exit 1; }

cd $DIR

sed 's/$/\r/' dist_readme.txt > $ZIPDIR/readme.txt
bash scripts/extract_changelog.sh | sed 's/$/\r/' > $ZIPDIR/changelog.txt
#bash scripts/extract_todo.sh  >  $ZIPDIR/todo.txt
python scripts/build_docs.py  || { echo 'Build failed' ; exit 1; } 
mv $DIR/functions.html $ZIPDIR/functions.html
cp $DIR/dist_licences.txt $ZIPDIR/licences.txt

rm -f $ZIPFILE
cd zipcontents
zip $ZIPFILE *
