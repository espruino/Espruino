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

for BOARDNAME in ESPRUINO_1V3 ESPRUINO_1V1 STM32VLDISCOVERY STM32F3DISCOVERY STM32F4DISCOVERY OLIMEXINO_STM32 HYSTM32_24 HYSTM32_28 HYSTM32_32 RASPBERRYPI
do
  BOARDNAMEX=$BOARDNAME
  if [ "$BOARDNAMEX" == "ESPRUINO_1V3" ]; then
    BOARDNAMEX=ESPRUINOBOARD
  fi
  if [ "$BOARDNAMEX" == "ESPRUINO_1V1" ]; then
    BOARDNAMEX=ESPRUINOBOARD_R1_1
  fi
  BINARY_NAME=`python scripts/get_binary_name.py $BOARDNAMEX`
  rm $BINARY_NAME
  if [ "$BOARDNAME" == "ESPRUINO_1V3" ]; then      
   scripts/create_espruino_image_1v3.sh
  elif [ "$BOARDNAME" == "ESPRUINO_1V1" ]; then      
   scripts/create_espruino_image_1v1.sh
  else 
    bash -c "RELEASE=1 $BOARDNAME=1 make clean"
    bash -c "RELEASE=1 $BOARDNAME=1 make" || { echo 'Build failed' ; exit 1; }
  fi
  cp $BINARY_NAME $ZIPDIR/$BINARY_NAME || { echo 'Build failed' ; exit 1; }
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
