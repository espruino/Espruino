#!/bin/bash

# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# -----------------------------------------------------------------------------
# Check that the size of the binary is within limits
# -----------------------------------------------------------------------------

cd `dirname $0`
cd ..

FILE=$1

# just a random check...
MAXSIZE=`grep FLASH_AVAILABLE_FOR_CODE gen/platform_config.h | sed "s/[^0-9]*\([0-9][0-9]*\).*/\1/"`

ACTUALSIZE=$(wc -c < "$FILE")

if [ $ACTUALSIZE -gt $MAXSIZE ]; then
    echo FAIL - size of $ACTUALSIZE is over $MAXSIZE bytes
    exit 1
else
    echo PASS - size of $ACTUALSIZE is under $MAXSIZE bytes
    exit 0
fi
