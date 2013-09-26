#!/bin/bash

cd `dirname $0`
cd  ..

arm-none-eabi-gcc -S src/jsparse.c
grep -B 2 "frame = " jsparse.s | sed -ne "N;N;N;s/\([^:]*\):.*frame = \(.*\)\n.*/\2\t\1/p" | sort -n
rm jsparse.s
