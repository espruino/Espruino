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
DIR=`pwd`
ZIPDIR=$DIR/zipcontents
ZIPFILE=$DIR/archives/espruino_${VERSION}.zip
rm -rf $ZIPDIR
mkdir $ZIPDIR

# Tidy up
# ESP8266
rm -rf esp_iot_sdk_v2.0.0*
rm -rf xtensa-lx106-elf
# ESP32
rm -rf esp-idf
rm -rf app
rm -rf xtensa-esp32-elf
# Install everything
source scripts/provision.sh ALL



echo ------------------------------------------------------
echo                          Building Version $VERSION
echo ------------------------------------------------------
# The following have been removed because it's too hard to keep the build going:
# STM32F3DISCOVERY OLIMEXINO_STM32 HYSTM32_32
# 
for BOARDNAME in ESPRUINO_1V3 ESPRUINO_1V3_WIZ  PICO_1V3_CC3000 PICO_1V3_WIZ ESPRUINOWIFI PUCKJS PIXLJS MDBT42Q NUCLEOF401RE NUCLEOF411RE STM32VLDISCOVERY STM32F4DISCOVERY STM32L496GDISCOVERY HYSTM32_24 HYSTM32_28  RASPBERRYPI MICROBIT ESP8266_BOARD ESP8266_4MB RUUVITAG ESP32 WIO_LTE NRF52832DK THINGY52 RAK8211 RAK8212 SMARTIBOT
do
  echo ------------------------------
  echo                  $BOARDNAME
  echo ------------------------------
  EXTRADEFS=
  EXTRANAME=
  if [ "$BOARDNAME" == "ESPRUINO_1V3" ]; then
    BOARDNAME=ESPRUINOBOARD
    EXTRADEFS=
  fi
  if [ "$BOARDNAME" == "ESPRUINO_1V3_WIZ" ]; then
    BOARDNAME=ESPRUINOBOARD
    EXTRADEFS="WIZNET=1 USE_CRYPTO=0"
    # we must now disable crypto in order to get WIZnet support in on the Original board
    EXTRANAME=_wiznet
  fi
  if [ "$BOARDNAME" == "PICO_1V3_CC3000" ]; then
    BOARDNAME=PICO_R1_3
    EXTRADEFS=CC3000=1
    EXTRANAME=_cc3000
  fi
  if [ "$BOARDNAME" == "PICO_1V3_WIZ" ]; then
    BOARDNAME=PICO_R1_3
    EXTRADEFS=WIZNET=1
    EXTRANAME=_wiznet
  fi

  # actually build
  ESP_BINARY_NAME=`python scripts/get_board_info.py $BOARDNAME "common.get_board_binary_name(board)"`
  if [ "$BOARDNAME" == "PUCKJS" ]; then
    ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
    EXTRADEFS=DFU_UPDATE_BUILD=1
  fi
  if [ "$BOARDNAME" == "PIXLJS" ]; then
    ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
    EXTRADEFS=DFU_UPDATE_BUILD=1
  fi
  if [ "$BOARDNAME" == "MDBT42Q" ]; then
    ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
    EXTRADEFS=DFU_UPDATE_BUILD=1
  fi
  if [ "$BOARDNAME" == "RUUVITAG" ]; then
    ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
    EXTRADEFS=DFU_UPDATE_BUILD=1
  fi

  echo "Building $ESP_BINARY_NAME"
  echo
  rm -f $BINARY_NAME
  if [ "$BOARDNAME" == "ESPRUINOBOARD" ]; then
    bash -c "$EXTRADEFS scripts/create_espruino_image_1v3.sh" || { echo "Build of $EXTRADEFS $BOARDNAME failed" ; exit 1; }
  elif [ "$BOARDNAME" == "PICO_R1_3" ]; then
    bash -c "$EXTRADEFS scripts/create_pico_image_1v3.sh" || { echo "Build of $EXTRADEFS $BOARDNAME failed" ; exit 1; }
  elif [ "$BOARDNAME" == "ESPRUINOWIFI" ]; then
    bash -c "$EXTRADEFS scripts/create_espruinowifi_image.sh" || { echo "Build of $EXTRADEFS $BOARDNAME failed" ; exit 1; }
  else
    bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make clean"
    bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make" || { echo "Build of $EXTRADEFS $BOARDNAME failed" ; exit 1; }
  fi
  # rename binary if needed
  if [ -n "$EXTRANAME" ]; then
    NEW_BINARY_NAME=`basename ${ESP_BINARY_NAME} .bin`$EXTRANAME.bin
  else
    NEW_BINARY_NAME=${ESP_BINARY_NAME}
  fi
  # copy...
  if [ "$BOARDNAME" == "ESP8266_BOARD" ]; then
    tar -C $ZIPDIR -xzf ${ESP_BINARY_NAME}.tgz || { echo "Build of $BOARDNAME failed" ; exit 1; }
    # build a combined image
    bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make combined" || { echo "Build of $BOARDNAME failed" ; exit 1; }
    cp ${ESP_BINARY_NAME}_combined_512.bin $ZIPDIR || { echo "Build of $BOARDNAME failed" ; exit 1; }
  elif [ "$BOARDNAME" == "ESP8266_4MB" ]; then
    tar -C $ZIPDIR -xzf ${ESP_BINARY_NAME}.tgz || { echo "Build of $BOARDNAME failed" ; exit 1; }
    # build a combined image
    bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make combined" || { echo "Build of $BOARDNAME failed" ; exit 1; }
    cp ${ESP_BINARY_NAME}_combined_4096.bin $ZIPDIR || { echo "Build of $BOARDNAME failed" ; exit 1; }
  else
    echo Copying ${ESP_BINARY_NAME} to $ZIPDIR/$NEW_BINARY_NAME
    cp ${ESP_BINARY_NAME} $ZIPDIR/$NEW_BINARY_NAME || { echo "Build of $BOARDNAME failed" ; exit 1; }
    if [ "$BOARDNAME" == "ESP32" ]; then
      tar -C $ZIPDIR -xzf  `basename $ESP_BINARY_NAME .bin`.tgz || { echo "Build of $BOARDNAME failed" ; exit 1; }
    fi
  fi
done



cd $DIR

sed 's/$/\r/' dist_readme.txt | sed "s/#v##/$VERSION/" > $ZIPDIR/readme.txt
bash scripts/extract_changelog.sh | sed 's/$/\r/' > $ZIPDIR/changelog.txt
#bash scripts/extract_todo.sh  >  $ZIPDIR/todo.txt
python scripts/build_docs.py  || { echo 'Build failed' ; exit 1; }
mv $DIR/functions.html $ZIPDIR/functions.html
cp $DIR/dist_licences.txt $ZIPDIR/licences.txt

rm -f $ZIPFILE
cd zipcontents
echo zip -r $ZIPFILE *
zip -r $ZIPFILE *
