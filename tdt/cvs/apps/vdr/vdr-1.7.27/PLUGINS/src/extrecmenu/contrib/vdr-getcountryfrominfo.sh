#!/bin/sh

# old info file
infofile="$1/info.vdr"

# use new info file if available
if [ -e "$1/info" ]; then
  infofile="$1/info"
fi

# try "Land: XXX 2010" first
country=`cat "${infofile}" | grep '|Land:' | sed -e 's/.*|Land: \([^ ]*\).*/\1/g'`
#echo "country1:>${country}<"

if [ "$country" == "" ]; then
  country=`cat ${infofile} | grep '|Country:' | sed -e 's/.*|Country: \([^|]*\).*/\1/g'`
  #echo "country2:>${country}<"
fi

if [ "$country" == "" ]; then
  country=`cat ${infofile} | grep -e '^D .*' | sed -e 's/^D [^, ]*[, ]*\([A-Z/]*\).*/\1/g'`
  #echo "country3:>${country}<"
fi

if [ ! "$country" == "" ]; then
  echo "${country}" > "$1/country.vdr"
  echo "${country}"
fi
