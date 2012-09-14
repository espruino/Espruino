#!/bin/bash
sed -ne 's/.*\*JS\*\(.*\)/\1/p' src/jsfunctions.c | sed -e 's/^ \([^ ]\)/\n\1/'
sed -ne 's/.*\*JS\*\(.*\)/\1/p' src/jsinteractive.c | sed -e 's/^ \([^ ]\)/\n\1/'
