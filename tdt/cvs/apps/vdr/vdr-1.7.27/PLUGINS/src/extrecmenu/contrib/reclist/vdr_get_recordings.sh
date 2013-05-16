#!/bin/sh

# set values for the length of each column
. vdr_set_values_for_recording_lists.sh

#echo "LEN_TITLE    : ${LEN_TITLE}"
#echo "LEN_GENRE    : ${LEN_GENRE}"
#echo "LEN_COUNTRY  : ${LEN_COUNTRY}"
#echo "LEN_YEAR     : ${LEN_YEAR}"
#echo "LEN_LENGTH   : ${LEN_LENGTH}"
#echo "LEN_DATE     : ${LEN_DATE}"
#echo "LEN_SIZE     : ${LEN_SIZE}"
#echo "LEN_MEDIA    : ${LEN_MEDIA}"
#echo "LEN_PATH     : ${LEN_PATH}"

#echo "VIDEODIR     : ${VIDEODIR}"

# find the info.vdr files and let awk create a recording list entry
rm ${RECLISTDIR}/${RECLISTFILE} 2>/dev/null

find ${VIDEODIR} -name 'info.vdr' -exec vdr_add_recording.sh \{\} \;
find ${VIDEODIR} -name 'info' -exec vdr_add_recording.sh \{\} \;

#find ${VIDEODIR} -name info.vdr -exec echo "{}" \; | \
#	awk -f /usr/local/bin/vdr_create_recording_list_entry.awk basedir="${VIDEODIR}" Seperator="${SEPERATOR}" \
#	LenTitle="${LEN_TITLE}" \
#	LenGenre="${LEN_GENRE}" \
#	LenCountry="${LEN_COUNTRY}" \
#	LenYear="${LEN_YEAR}" \
#	LenLength="${LEN_LENGTH}" \
#	LenDate="${LEN_DATE}" \
#	LenSize="${LEN_SIZE}" \
#	LenMedia="${LEN_MEDIA}" >> ${RECLISTDIR}/${RECLISTFILE}