#!/bin/bash
cd `dirname $0`
cd ..

awk '/\[CHANGELOG\]/{s=x}{s=s$0"\n"}/\[\/CHANGELOG\]/{print s}' src/jsutils.h | tail -n +2 | head -n -2
