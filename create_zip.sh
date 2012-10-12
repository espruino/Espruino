#!/bin/bash

cd `dirname $0`
VERSION=`sed -ne "s/^.*JS_VERSION.*\"\(.*\)\"/\1/p" src/jsutils.h`
DIR=`pwd`

echo ------------------------------------------------------
echo                          Building Version $VERSION
echo ------------------------------------------------------

cd $DIR/../stm32/stinyjs

OLIMEX=1 make clean
OLIMEX=1 make 
cp main.bin  $DIR/espruino_${VERSION}_olimexino_stm32.bin

STM32VLDISCOVERY=1 make clean
STM32VLDISCOVERY=1 make 
cp main.bin  $DIR/espruino_${VERSION}_stm32vldiscovery.bin

STM32F4DISCOVERY=1 make clean
STM32F4DISCOVERY=1 make 
cp main.bin  $DIR/espruino_${VERSION}_stm32f4discovery.bin

cd $DIR

./extract_docs.sh > functions.txt
awk '/\[CHANGELOG\]/{s=x}{s=s$0"\n"}/\[\/CHANGELOG\]/{print s}' src/jsutils.h > changelog.txt
awk '/\[TODO\]/{s=x}{s=s$0"\n"}/\[\/TODO\]/{print s}' src/jsutils.h > todo.txt

rm -f espruino_${VERSION}.zip 
zip espruino_${VERSION}.zip readme.txt functions.txt changelog.txt todo.txt espruino_${VERSION}_stm32vldiscovery.bin espruino_${VERSION}_stm32f4discovery.bin espruino_olimexino_stm32.bin

rm  espruino_${VERSION}_stm32vldiscovery.bin espruino_${VERSION}_stm32f4discovery.bin espruino_olimexino_stm32.bin
