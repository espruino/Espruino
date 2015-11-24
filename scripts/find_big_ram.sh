#!/bin/bash
#
# Extracts and orders symbol names so we can see how big they are
#

if [ $# -eq 0 ]
then
  echo "USAGE:"
  echo "scripts/find_big_ram.sh espruino_XXXX.lst"
  exit 1
fi

grep "^20...... [^<]" $1 |  sort --key=5 
