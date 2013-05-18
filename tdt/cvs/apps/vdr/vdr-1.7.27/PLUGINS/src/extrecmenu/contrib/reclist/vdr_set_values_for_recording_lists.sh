#!/bin/sh

source /etc/conf.d/vdr
export VIDEODIR="${VIDEO}/"
export RECLISTDIR="$(dirname ${EPGFILE})/"
#export RECLISTDIR="/root/t/"
export RECLISTFILE="vdr_recordings.lst"

PAGE_WIDTH=1139
PAGE_HEIGHT=395
FONTFIXSIZE=$[$(grep "FontFixSize " /etc/vdr/setup.conf | awk 'BEGIN{FS="="}{print $2}')]

#export CHARS_PER_LINE=$[${PAGE_WIDTH}/(${FONTFIXSIZE}+1)]
export CHARS_PER_LINE=59
#echo "CHARS_PER_LINE: ${CHARS_PER_LINE}"
#export LINES_PER_PAGE=$[${PAGE_HEIGHT}/${FONTFIXSIZE}]
export LINES_PER_PAGE=21
#echo "LINES_PER_PAGE: ${LINES_PER_PAGE}"


export SEPERATOR="|"


export LEN_TITLE=50
export LEN_GENRE=20
export LEN_COUNTRY=8
export LEN_YEAR=4
export LEN_LENGTH=4
export LEN_DATE=10
export LEN_SIZE=9
export LEN_MEDIA=8
