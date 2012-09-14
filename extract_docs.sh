#!/bin/bash
sed -ne 's/.*\*JS\*\(.*\)/\1/p' src/jsfunctions.c
sed -ne 's/.*\*JS\*\(.*\)/\1/p' src/jsinteractive.c
