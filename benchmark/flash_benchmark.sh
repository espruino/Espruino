#!/bin/bash
# Automatically Flash and benchmark an Espruino board
# Needs:
#  1x Normal Espruino board
#  1x Board, flashed with `USB_PRODUCT_ID=0x5741 ESPRUINO_1V3=1 make serialflash`
# Connect A0 to normal board's RST pin
# Connect A1 to normal board's B12 pin

# copy the following lines to /etc/udev/rules.d/45-espruino-test.rules
#
# # special tester
# ATTRS{idProduct}=="5741", ATTRS{idVendor}=="0483", MODE="664", GROUP="plugdev", SYMLINK+="espruino_tester"
# # normal espruino
# ATTRS{idProduct}=="5740", ATTRS{idVendor}=="0483", MODE="664", GROUP="plugdev", SYMLINK+="espruino"
#
# And you're done


cd `dirname $0`
# Now in benchmark dir

BINARY=`ls  ../espruino*espruino_1r3.bin | sort -n | tail -1`

echo Using Binary $BINARY

echo "Resetting device into boot mode"
echo "var RST=A0,BT=A1;digitalWrite(BT,1);digitalPulse(RST,0,10);setTimeout(function() { digitalRead(BT); }, 100);\n" > /dev/espruino_tester
sleep 1s
echo "Flashing"
python ../scripts/stm32loader.py -k -b 460800 -a 0x8002800 -ew -p /dev/espruino $BINARY
echo "Resetting device out of boot mode"
echo "var RST=A0;digitalPulse(RST,0,10);\n" > /dev/espruino_tester
sleep 1s

# Run all benchmarks
for fn in `ls *.js`; do
  echo "var RST=A0;digitalPulse(RST,0,10);\n" > /dev/espruino_tester
  sleep 1s
  /usr/bin/python benchmark.py /dev/espruino $fn
done
