#!/bin/bash

PRGNAME=$(basename $0)

usage() {
    echo "usage: ${PRGNAME} (all|head|tail)"
}

what=${1}
if [[ -z ${what} ]]; then
    usage
    exit
fi



# set values for the positions of each column and the sortpositions
. vdr_set_values_for_recording_lists.sh



# crop vertically / lines
if   [ "$what" == "head" ] ; then
    LINEFILTER=" head -n ${LINES_PER_PAGE} |"
elif [ "$what" == "tail" ] ; then
    LINEFILTER=" tail -n ${LINES_PER_PAGE} |"
else
    LINEFILTER=""
fi


COMMAND="cat /dev/stdin | ${LINEFILTER} cut -b 1-${CHARS_PER_LINE}"
#echo "cmd: ${COMMAND}"
eval "${COMMAND}"
