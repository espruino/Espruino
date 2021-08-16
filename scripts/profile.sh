#!/bin/bash
# Profiles an Espruino build to find areas that are slow

# First
# * Disable extra libs in LINUX.py
# * Specify an amount of variables (2000)
make clean;CFLAGS="-pg" LDFLAGS="-pg" make
./espruino benchmark/donut.js
gprof espruino > gprof.txt
cat gprof.txt

