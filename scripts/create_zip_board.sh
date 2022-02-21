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
# Creates a build of espruino for a zip file - some targets have different 'flavours'
# exit 255 allows this to work with xargs
# ----------------------------------------------------------------------------------------
#


cd `dirname $0`
cd .. # Espruino

if [ "$#" -ne 1 ]; then
  echo "Usage:"
  echo "  scripts/create_zip_board.sh TARGET"
  echo ""
  echo "Targets are:"
  echo "ESPRUINO_1V3 ESPRUINO_1V3_AT ESPRUINO_1V3_WIZ PICO_1V3 PICO_1V3_CC3000 PICO_1V3_WIZ "
  echo "ESPRUINOWIFI PUCKJS PIXLJS BANGLEJS BANGLEJS2 MDBT42Q NUCLEOF401RE NUCLEOF411RE STM32VLDISCOVERY" 
  echo "STM32F4DISCOVERY STM32L496GDISCOVERY MICROBIT2 ESP8266_BOARD ESP8266_4MB ESP32 WIO_LTE RAK5010 SMARTIBOT MICROBIT1"
  echo "STM32F3DISCOVERY OLIMEXINO_STM32 HYSTM32_32 HYSTM32_28 HYSTM32_24 RAK8211 RAK8212 RUUVITAG THINGY52 RASPBERRYPI"
  exit 255
fi

BOARDNAME=$1
VERSION=`sed -ne "s/^.*JS_VERSION.*\"\(.*\)\"/\1/p" src/jsutils.h | head -1`
echo "VERSION $VERSION"
DIR=`pwd`
ZIPDIR=$DIR/zipcontents

# We assume all setup has been done by create_zip
OBJDIR=obj_$BOARDNAME
rm -rf $OBJDIR
mkdir $OBJDIR

echo ------------------------------------------------------
echo                          Building Version $VERSION
echo    $BOARDNAME
echo ------------------------------------------------------
EXTRADEFS="OBJDIR=$OBJDIR GENDIR=$OBJDIR "
EXTRANAME=
if [ "$BOARDNAME" == "ESPRUINO_1V3" ]; then
  BOARDNAME=ESPRUINOBOARD
fi
if [ "$BOARDNAME" == "ESPRUINO_1V3_AT" ]; then
  BOARDNAME=ESPRUINOBOARD
  EXTRADEFS+="USE_NET=1 DEFINES=-DNO_VECTOR_FONT=1 BLACKLIST=boards/ESPRUINOBOARD.net.blocklist"
  EXTRANAME=_at
fi
if [ "$BOARDNAME" == "ESPRUINO_1V3_WIZ" ]; then
  BOARDNAME=ESPRUINOBOARD
  EXTRADEFS+="USE_NET=1 WIZNET=1 USE_CRYPTO=0 USE_DEBUGGER=0 USE_TAB_COMPLETE=0 USE_NETWORK_JS=0 DEFINES='-DNO_VECTOR_FONT=1 -DNO_DUMP_HARDWARE_INITIALISATION=1' BLACKLIST=boards/ESPRUINOBOARD.net.blocklist"
  # we must now disable crypto in order to get WIZnet support in on the Original board
  EXTRANAME=_wiznet
fi
if [ "$BOARDNAME" == "PICO_1V3_CC3000" ]; then
  BOARDNAME=PICO_R1_3
  EXTRADEFS+="CC3000=1 USE_DEBUGGER=0 USE_NETWORK_JS=0 USE_TV=0 DEFINES='-DNO_VECTOR_FONT=1 -DNO_DUMP_HARDWARE_INITIALISATION=1'"
  EXTRANAME=_cc3000
fi
if [ "$BOARDNAME" == "PICO_1V3_WIZ" ]; then
  BOARDNAME=PICO_R1_3
  EXTRADEFS+="WIZNET=1 USE_DEBUGGER=0 USE_NETWORK_JS=0 USE_TV=0 DEFINES='-DNO_VECTOR_FONT=1 -DNO_DUMP_HARDWARE_INITIALISATION=1'"
  EXTRANAME=_wiznet
fi
if [ "$BOARDNAME" == "PICO_1V3" ]; then
  BOARDNAME=PICO_R1_3
fi

# actually build
ESP_BINARY_NAME=`python scripts/get_board_info.py $BOARDNAME "common.get_board_binary_name(board)"`
if [ "$BOARDNAME" == "PUCKJS" ]; then
  ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD=1
fi
if [ "$BOARDNAME" == "PIXLJS" ]; then
  ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD=1
fi
if [ "$BOARDNAME" == "BANGLEJS" ]; then
  ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD=1
fi
if [ "$BOARDNAME" == "BANGLEJS2" ]; then
  ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD=1
fi
if [ "$BOARDNAME" == "SMARTIBOT" ]; then
  ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD=1
fi
if [ "$BOARDNAME" == "MDBT42Q" ]; then
  ESP_BINARY_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD=1
fi
if [ "$BOARDNAME" == "RUUVITAG" ]; then
  ESP_BINARY2_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD_WITH_HEX=1
fi
if [ "$BOARDNAME" == "THINGY52" ]; then
  ESP_BINARY2_NAME=`basename $ESP_BINARY_NAME .hex`.zip
  EXTRADEFS+=DFU_UPDATE_BUILD_WITH_HEX=1
fi

echo "Building $ESP_BINARY_NAME"
echo "EXTRADEFS $EXTRADEFS"
echo
rm -f $BINARY_NAME
if [ "$BOARDNAME" == "ESPRUINOBOARD" ]; then
  bash -c "$EXTRADEFS scripts/create_espruino_image_1v3.sh" || { echo "Build of '$EXTRADEFS BOARD=$BOARDNAME make' failed" ; exit 255; }
elif [ "$BOARDNAME" == "PICO_R1_3" ]; then
  bash -c "$EXTRADEFS scripts/create_pico_image_1v3.sh" || { echo "Build of '$EXTRADEFS BOARD=$BOARDNAME make' failed" ; exit 255; }
elif [ "$BOARDNAME" == "ESPRUINOWIFI" ]; then
  bash -c "$EXTRADEFS scripts/create_espruinowifi_image.sh" || { echo "Build of '$EXTRADEFS BOARD=$BOARDNAME make' failed" ; exit 255; }
else
  bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make clean"
  bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make" || { echo "Build of '$EXTRADEFS BOARD=$BOARDNAME make' failed" ; exit 255; }
fi
# rename binary if needed
if [ -n "$EXTRANAME" ]; then
  NEW_BINARY_NAME=`basename ${ESP_BINARY_NAME} .bin`$EXTRANAME.bin
else
  NEW_BINARY_NAME=${ESP_BINARY_NAME}
fi
# copy...
if [ "$BOARDNAME" == "ESP8266_BOARD" ]; then
  tar -C $ZIPDIR -xzf bin/${ESP_BINARY_NAME}.tgz || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
  # build a combined image
  bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make combined" || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
  cp bin/${ESP_BINARY_NAME}_combined_512.bin $ZIPDIR || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
elif [ "$BOARDNAME" == "ESP8266_4MB" ]; then
  tar -C $ZIPDIR -xzf bin/${ESP_BINARY_NAME}.tgz || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
  # build a combined image
  bash -c "$EXTRADEFS RELEASE=1 BOARD=$BOARDNAME make combined" || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
  cp bin/${ESP_BINARY_NAME}_combined_4096.bin $ZIPDIR || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
else
  echo Copying bin/${ESP_BINARY_NAME} to $ZIPDIR/$NEW_BINARY_NAME
  cp bin/${ESP_BINARY_NAME} $ZIPDIR/$NEW_BINARY_NAME || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
  if [ "$BOARDNAME" == "ESP32" ]; then
    tar -C $ZIPDIR -xzf  bin/`basename $ESP_BINARY_NAME .bin`.tgz || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
  fi
fi
# Copy second binary
if [ -n "$ESP_BINARY2_NAME" ]; then
  cp bin/${ESP_BINARY2_NAME} $ZIPDIR || { echo "Build of 'BOARD=$BOARDNAME make' failed" ; exit 255; }
fi


