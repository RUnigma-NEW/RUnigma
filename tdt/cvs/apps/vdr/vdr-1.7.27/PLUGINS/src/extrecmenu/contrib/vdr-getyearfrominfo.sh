#!/bin/sh

# old info file
infofile="$1/info.vdr"

# use new info file if available
if [ -e "$1/info" ]; then
  infofile="$1/info"
fi

#try "|Year: XXXX|" first
year=`cat "${infofile}" | grep '|Year:' | sed -e 's/.*|Year: \([0-9]*\)|.*/\1/g'`
#echo "year1:>${year}<"


if [ ! "$year" == "" ]; then
  echo "${year}" > "$1/year.vdr"
  echo "${year}"
fi
