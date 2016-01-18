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


# ESP8266
export ESP8266_SDK_ROOT=$DIR/esp_iot_sdk_v1.5.0
export PATH=$PATH:$DIR/xtensa-lx106-elf/bin/

echo ------------------------------------------------------
echo                          Building Version $VERSION
echo ------------------------------------------------------

for BOARDNAME in PICO_1V3_CC3000 PICO_1V3_WIZ ESPRUINO_1V3 ESPRUINO_1V3_WIZ NUCLEOF401RE NUCLEOF411RE STM32VLDISCOVERY STM32F3DISCOVERY STM32F4DISCOVERY OLIMEXINO_STM32 HYSTM32_24 HYSTM32_28 HYSTM32_32 RASPBERRYPI MICROBIT ESP8266_BOARD
do
  echo ------------------------------
  echo                  $BOARDNAME
  echo ------------------------------
  EXTRADEFS=
  EXTRANAME=
  if [ "$BOARDNAME" == "ESPRUINO_1V3_WIZ" ]; then
    BOARDNAME=ESPRUINO_1V3
    EXTRADEFS=WIZNET=1
    EXTRANAME=_wiznet
  fi
  if [ "$BOARDNAME" == "PICO_1V3_CC3000" ]; then
    BOARDNAME=PICO_R1_3
    EXTRANAME=_cc3000
  fi
  if [ "$BOARDNAME" == "PICO_1V3_WIZ" ]; then
    BOARDNAME=PICO_R1_3
    EXTRADEFS=WIZNET=1
    EXTRANAME=_wiznet
  fi
  BOARDNAMEX=$BOARDNAME
  if [ "$BOARDNAME" == "ESPRUINO_1V3" ]; then
    BOARDNAMEX=ESPRUINOBOARD
  fi
  if [ "$BOARDNAME" == "MICROBIT" ]; then
    BINARY_NAME=`basename $BINARYNAME bin`.hex
  fi
  # actually build
  BINARY_NAME=`python scripts/get_board_info.py $BOARDNAMEX "common.get_board_binary_name(board)"`
  rm $BINARY_NAME
  if [ "$BOARDNAME" == "ESPRUINO_1V3" ]; then      
    bash -c "$EXTRADEFS scripts/create_espruino_image_1v3.sh" || { echo "Build of $BOARDNAME failed" ; exit 1; }
  elif [ "$BOARDNAME" == "PICO_R1_3" ]; then      
    bash -c "$EXTRADEFS scripts/create_pico_image_1v3.sh" || { echo "Build of $BOARDNAME failed" ; exit 1; }
  else 
    bash -c "$EXTRADEFS RELEASE=1 $BOARDNAME=1 make clean"
    bash -c "$EXTRADEFS RELEASE=1 $BOARDNAME=1 make" || { echo "Build of $BOARDNAME failed" ; exit 1; }
  fi
  # rename binary if needed
  if [ -n "$EXTRANAME" ]; then 
    NEW_BINARY_NAME=`basename $BINARY_NAME .bin`$EXTRANAME.bin
  else
    NEW_BINARY_NAME=$BINARY_NAME
  fi
  # copy...
  if [ "$BOARDNAME" == "ESP8266_BOARD" ]; then
    cp espruino_esp8266*.bin $ZIPDIR || { echo "Build of $BOARDNAME failed" ; exit 1; }
  else
    cp $BINARY_NAME $ZIPDIR/$NEW_BINARY_NAME || { echo "Build of $BOARDNAME failed" ; exit 1; }
  fi
done

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
