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
cd .. # Espruino

VERSION=`sed -ne "s/^.*JS_VERSION.*\"\(.*\)\"/\1/p" src/jsutils.h | head -1`
echo "VERSION $VERSION"
ESPRUINODIR=`pwd`
ZIPDIR=$ESPRUINODIR/zipcontents
ZIPFILE=$ESPRUINODIR/archives/espruino_${VERSION}.zip
rm -rf $ZIPDIR
mkdir $ZIPDIR

# Tidy up
# Binaries
rm -f bootloader_espruino_$VERSION* espruino_$VERSION*
# ESP8266
#rm -rf esp_iot_sdk_v2.0.0*
#rm -rf xtensa-lx106-elf
# ESP32
#rm -rf esp-idf
#rm -rf app
#rm -rf xtensa-esp32-elf

# create docs before ESP32 provisioning creates a venv for python which then ensures markdown2 isn't installed
echo Creating Documentation
scripts/build_docs.py || { echo 'Build failed' ; exit 1; }
mv $ESPRUINODIR/functions.html $ZIPDIR/functions.html

# Install all 'normal boards'
source scripts/provision.sh ESPRUINOBOARD
source scripts/provision.sh PIXLJS

echo ------------------------------------------------------
echo                          Building Version $VERSION
echo ------------------------------------------------------
# The following have been removed because it's too hard to keep the build going:
# STM32F3DISCOVERY OLIMEXINO_STM32 HYSTM32_32 HYSTM32_28 HYSTM32_24 RAK8211 RAK8212 RUUVITAG THINGY52 RASPBERRYPI RAK5010
 
for BOARDNAME in ESPRUINO_1V3 ESPRUINO_1V3_AT ESPRUINO_1V3_WIZ PICO_1V3 PICO_1V3_CC3000 PICO_1V3_WIZ ESPRUINOWIFI PUCKJS PUCKJS_MINIMAL PUCKJS_NETWORK PIXLJS PIXLJS_WIZ JOLTJS BANGLEJS BANGLEJS2 MDBT42Q NUCLEOF401RE NUCLEOF411RE STM32VLDISCOVERY STM32F4DISCOVERY STM32L496GDISCOVERY MICROBIT1 MICROBIT2 SMARTIBOT
do
  scripts/create_zip_board.sh $BOARDNAME
done

# Install Espressif stuff as it screws with Python

source scripts/provision.sh ESP8266_4MB
source scripts/provision.sh ESP32
source scripts/provision.sh ESP32C3_IDF4
source scripts/provision.sh ESP32S3_IDF4
 
for BOARDNAME in ESP8266_BOARD ESP8266_4MB ESP32 ESP32C3_IDF4 ESP32S3_IDF4
do
  scripts/create_zip_board.sh $BOARDNAME
done


cd $ESPRUINODIR

echo Copying README
sed 's/$/\r/' $ESPRUINODIR/scripts/create_zip_dist_readme.txt | sed "s/#v##/$VERSION/" > $ZIPDIR/readme.txt
cp $ESPRUINODIR/scripts/create_zip_dist_licences.txt $ZIPDIR/licences.txt
echo Copying ChangeLog
bash scripts/extract_changelog.sh | sed 's/$/\r/' > $ZIPDIR/changelog.txt
#bash scripts/extract_todo.sh  >  $ZIPDIR/todo.txt

echo Compressing...
rm -f $ZIPFILE
cd zipcontents
echo zip -r $ZIPFILE *
zip -r $ZIPFILE *
