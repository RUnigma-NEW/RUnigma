#!/bin/bash

usage() {
    echo "usage: ${PRGNAME} <type> <what>"
    echo "   where   <type> = (title|genre|country|year|length|date|size|media|path)"
    echo "     and   <what> = (all|head|tail)"
}

PRGNAME=$(basename $0)
type=${1}
what=${2}

if [[ -z ${type} ]]; then
    usage
    exit
fi
if [[ -z ${what} ]]; then
    usage
    exit
fi



# set values for the column length
. vdr_set_values_for_recording_lists.sh



# crop vertically / lines per page
if   [ "$what" == "head" ] ; then
    LINEFILTER=" head -n ${LINES_PER_PAGE} |"
elif [ "$what" == "tail" ] ; then
    LINEFILTER=" tail -n ${LINES_PER_PAGE} |"
else
    LINEFILTER=""
fi



if   [ "$type" == "title" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -d -b -f -k 1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$1; l1=32; \
	    c2=\$2; l2=10; \
	    c3=\$3; l3=3; \
	    c4=\$4; l4=4; \
	    c5=\$5; l5=4; \
	    c6=\$8; l6=1; \
	    printf (\"%-*s %-*s %-*s %-*s %-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2), \
		l3, substr(c3,1,l3), \
		l4, substr(c4,1,l4), \
		l5, substr(c5,1,l5), \
		l6, substr(c6,1,l6) ); \
	}'"

elif [ "$type" == "genre" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -d -b -f -k2,2 -k1,1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$2; l1=20; \
	    c2=\$1; l2=22; \
	    c3=\$3; l3=3; \
	    c4=\$4; l4=4; \
	    c5=\$5; l5=4; \
	    c6=\$8; l6=1; \
	    printf (\"%-*s %-*s %-*s %-*s %-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2), \
		l3, substr(c3,1,l3), \
		l4, substr(c4,1,l4), \
		l5, substr(c5,1,l5), \
		l6, substr(c6,1,l6) ); \
	}'"

elif [ "$type" == "country" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -d -b -f -k3,3 -k1,1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$3; l1=5; \
	    c2=\$1; l2=30; \
	    c3=\$2; l3=10; \
	    c4=\$4; l4=4; \
	    c5=\$5; l5=4; \
	    c6=\$8; l6=1; \
	    printf (\"%-*s %-*s %-*s %-*s %-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2), \
		l3, substr(c3,1,l3), \
		l4, substr(c4,1,l4), \
		l5, substr(c5,1,l5), \
		l6, substr(c6,1,l6) ); \
	}'"

elif [ "$type" == "year" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|'          -k4,4 -k1,1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$4; l1=4; \
	    c2=\$1; l2=32; \
	    c3=\$2; l3=10; \
	    c4=\$3; l4=3; \
	    c5=\$5; l5=4; \
	    c6=\$8; l6=1; \
	    printf (\"%-*s %-*s %-*s %-*s %-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2), \
		l3, substr(c3,1,l3), \
		l4, substr(c4,1,l4), \
		l5, substr(c5,1,l5), \
		l6, substr(c6,1,l6) ); \
	}'"

elif [ "$type" == "length" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -d -b -f -k5,5 -k1,1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$5; l1=4; \
	    c2=\$1; l2=32; \
	    c3=\$2; l3=10; \
	    c4=\$3; l4=3; \
	    c5=\$4; l5=4; \
	    c6=\$8; l6=1; \
	    printf (\"%-*s %-*s %-*s %-*s %-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2), \
		l3, substr(c3,1,l3), \
		l4, substr(c4,1,l4), \
		l5, substr(c5,1,l5), \
		l6, substr(c6,1,l6) ); \
	}'"

elif [ "$type" == "date" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -d -b -f -k6,6 -k1,1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$6; l1=10; \
	    c2=\$1; l2=22; \
	    c3=\$2; l3=9; \
	    c4=\$3; l4=3; \
	    c5=\$4; l5=4; \
	    c6=\$5; l6=4; \
	    c7=\$8; l7=1; \
	    printf (\"%-*s %-*s %-*s %-*s %-*s %-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2), \
		l3, substr(c3,1,l3), \
		l4, substr(c4,1,l4), \
		l5, substr(c5,1,l5), \
		l6, substr(c6,1,l6), \
		l7, substr(c7,1,l7) ); \
	}'"

elif [ "$type" == "size" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -n       -k7,7 -k1,1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$7; l1=9; \
	    c2=\$9; l2=49; \
	    printf (\"%-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2) ); \
	}'"

elif [ "$type" == "media" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -d -b -f -k8,8 -k1,1 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$8; l1=8; \
	    c2=\$1; l2=25; \
	    c3=\$2; l3=10; \
	    c4=\$3; l4=3; \
	    c5=\$4; l5=4; \
	    c6=\$5; l6=4; \
	    printf (\"%-*s %-*s %-*s %-*s %-*s %-*s\n\", \
		l1, substr(c1,1,l1), \
		l2, substr(c2,1,l2), \
		l3, substr(c3,1,l3), \
		l4, substr(c4,1,l4), \
		l5, substr(c5,1,l5), \
		l6, substr(c6,1,l6) ); \
	}'"

elif [ "$type" == "path" ] ; then
    COMMAND="cat ${RECLISTDIR}/${RECLISTFILE} | sort -t'|' -d -b -f -k9,9 | ${LINEFILTER} awk ' \
	BEGIN{FS=\"|\"} { \
	    c1=\$9; l1=59; \
	    printf (\"%-*s\n\", \
		l1, substr(c1,1,l1) ); \
	}'"

fi


# crop horizontally / chars per line
COMMAND="${COMMAND} | cut -b 1-${CHARS_PER_LINE}"


#echo "cmd: ${COMMAND}"
eval "${COMMAND}"
