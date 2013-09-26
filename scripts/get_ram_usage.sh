#!/bin/bash

cd `dirname $0`
cd ..
sed -ne "s/^20.*\t\([0-9a-f]*.*\)/\1/p" espruino_olimexino_stm32.lst | sort 
