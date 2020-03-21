#!/bin/sh

if [ $# -eq 0 ]; then 
    echo "Usage: $(basename $0) bmpfile"
    exit 1
fi

makeicon < $1 > tmp-icon
../bin2c/bin2c tmp-icon $1-icon
rm tmp-icon
