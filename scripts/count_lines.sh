#!/bin/bash

cd `dirname $0`
cd ..

find . | grep -f scripts/count_lines.include | grep -v -f scripts/count_lines.exclude | xargs cat | wc -l
