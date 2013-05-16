#!/bin/sh

[ $? -ne 2 ] && echo "Usage: $0 <path to the VDR recording> <rating 0 - 10>" && exit 1

rm ${2}/rated.vdr >/dev/null 2>&1
if [ ! ${1} = "0" ]; then
    echo "${1}" > $2/rated.vdr
fi
