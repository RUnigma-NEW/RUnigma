#!/bin/bash

#LOGFILE=~/vdr_move_to_hdd.log

#ARCHIVEHDD=~/Archive-HDD
ARCHIVEHDD=/mnt/archive-hdd

#VDRSETTINGSFILE=~/vdr
VDRSETTINGSFILE=/etc/conf.d/vdr

source ${VDRSETTINGSFILE}

HDDMOUNTED=0


#echo "" > ${LOGFILE}

# Test, if recording has already moved?
if [ -f ${SRC_FULLPATH}/hdd.vdr ]; then
    svdrpsend.pl mesg "Recording has already been moved to Archive-HDD!"
#    echo "Recording has already been moved to Archive-HDD!" >> ${LOGFILE}
    exit 1
fi


# Test, if Archive-HDD can be mounted
if [ ! -f ${ARCHIVEHDD}/hdd.vdr ]; then
    mount ${ARCHIVEHDD}
    HDDMOUNTED=1
fi
if [ ! -f ${ARCHIVEHDD}/hdd.vdr ]; then
    svdrpsend.pl mesg "Archive-HDD could not be mounted!"
#    echo "Archive-HDD could not be mounted!" >> ${LOGFILE}
    exit 1
fi


#echo "PARAMETER      = $1" >> ${LOGFILE}
#echo "VDRSETTINGSFILE= ${VDRSETTINGSFILE}" >> ${LOGFILE}
#echo "VIDEO          = ${VIDEO}" >> ${LOGFILE}
#echo "ARCHIVEHDD     = ${ARCHIVEHDD}" >> ${LOGFILE}
#echo "" >> ${LOGFILE}

VID_FULLPATH="`cd \"$VIDEO\" 2>/dev/null && pwd || echo \"$VIDEO\"`/"
#echo "VID_FULLPATH   = ${VID_FULLPATH}" >> ${LOGFILE}

SRC_FULLPATH="`cd \"$1\" 2>/dev/null && pwd || echo \"${1}\"`/"
#echo "SRC_FULLPATH   = ${SRC_FULLPATH}" >> ${LOGFILE}
#echo "" >> ${LOGFILE}



#RELPATH=$(echo ${SRC_FULLPATH} | sed -e "s#$VID_FULLPATH##")
RELPATH=${SRC_FULLPATH#$VID_FULLPATH}
#echo "RELPATH        = ${RELPATH}"  >> ${LOGFILE}

MODPATH=$(echo "$RELPATH" | awk '
BEGIN{
	FS="/";
}
{
	MODPATH="";
	#printf "0: %s\n", MODPATH;
	for (i=1; i<=NF; i++) {
		if ((i==NF-2) && (index($i, "%")==1)) { # cutted movie
			#printf "replace\n";
			MODPATH=MODPATH "" substr ($i, 2);
		} else {
			#printf "original\n";
			MODPATH=MODPATH "" $i;
		}
		if (i<NF) MODPATH=MODPATH "/";
		#printf "%i:%s %s\n", i, $i, MODPATH;
	}
	printf "%s", MODPATH;
}' ARCHIVEHDD="$ARCHIVEHDD")
#echo "MODPATH        = ${MODPATH}"  >> ${LOGFILE}

mkdir -p ${ARCHIVEHDD}/${MODPATH}
for i in ${SRC_FULLPATH}/0??.vdr; do
    if [ -e "${i}" ]; then
	B=$(basename $i)
	svdrpsend.pl mesg "Moving $B..."
#	sleep 5
	mv ${i}  ${ARCHIVEHDD}/${MODPATH}
    fi
done
for i in ${SRC_FULLPATH}/0????.ts; do
    if [ -e "${i}" ]; then
	B=$(basename $i)
	svdrpsend.pl mesg "Moving $B..."
#	sleep 5
        mv ${i}  ${ARCHIVEHDD}/${MODPATH}
    fi
done
mv ${SRC_FULLPATH}/index.vdr ${ARCHIVEHDD}/${MODPATH}
mv ${SRC_FULLPATH}/index ${ARCHIVEHDD}/${MODPATH}
cp ${SRC_FULLPATH}/info.vdr  ${ARCHIVEHDD}/${MODPATH}
cp ${SRC_FULLPATH}/info  ${ARCHIVEHDD}/${MODPATH}
rm ${SRC_FULLPATH}/resume.vdr
rm ${SRC_FULLPATH}/resume
rm ${SRC_FULLPATH}/marks.vdr
rm ${SRC_FULLPATH}/marks
cp ${ARCHIVEHDD}/hdd.vdr ${SRC_FULLPATH}/


if [ "${HDDMOUNTED}" == "1" ]; then
#    svdrpsend.pl mesg "umount."
#    sleep 5
    umount ${ARCHIVEHDD}
#else
#    svdrpsend.pl mesg "NO umount."
#    sleep 5
fi

svdrpsend.pl mesg "Successfully moved Recording to Archive-HDD."
