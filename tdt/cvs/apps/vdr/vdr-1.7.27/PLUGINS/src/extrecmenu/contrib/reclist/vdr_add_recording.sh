#!/bin/sh


echo "${1}" | \
	awk -f /usr/local/bin/vdr_create_recording_list_entry.awk basedir="${VIDEODIR}" Seperator="${SEPERATOR}" \
	LenTitle="${LEN_TITLE}" \
	LenGenre="${LEN_GENRE}" \
	LenCountry="${LEN_COUNTRY}" \
	LenYear="${LEN_YEAR}" \
	LenLength="${LEN_LENGTH}" \
	LenDate="${LEN_DATE}" \
	LenSize="${LEN_SIZE}" \
	LenMedia="${LEN_MEDIA}" >> ${RECLISTDIR}/${RECLISTFILE}