#!/bin/bash

N="$1"
ON="$2"

if [ "$ON" == "0" ]; then
    echo "F00199998 F00199999"
else
    OUT=`head -n "$N" /c/cs223/hw2/Tests/100K_airports | tr '\n' ' '`
    echo $OUT F00199997
fi
