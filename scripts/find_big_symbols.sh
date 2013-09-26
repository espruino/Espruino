#!/bin/bash
#
# Extracts and orders symbol names in flash so we can see how big they are
#
# use like this
#  scripts/find_big_symbols.sh espruino_hystm32_28.lst 


grep "^08...... [^<]" $1 |  sort --key=4 
