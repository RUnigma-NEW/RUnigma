#!/bin/bash

CURDIR=$1
TUFSBOXDIR=$2
OUTDIR=$3
TMPBOOTDIR=$4
TMPFWDIR=$5

echo "CURDIR       = $CURDIR"
echo "TUFSBOXDIR   = $TUFSBOXDIR"
echo "OUTDIR       = $OUTDIR"
echo "TMPBOOTDIR   = $TMPBOOTDIR"
echo "TMPFWDIR     = $TMPFWDIR"

MKFSYAFFS2=$CURDIR/mkyaffs2  
#MUP=$CURDIR/mup

OUTFILE=$OUTDIR/e2yaffs2.img 

if [ ! -e $OUTDIR ]; then
  mkdir $OUTDIR
fi

if [ -e $OUTFILE ]; then
  rm -f $OUTFILE
fi

cp $TMPBOOTDIR/uImage $OUTDIR/uImage

# Create a kathrein update file for fw's 
# Offset on NAND Disk = 0x00400000
$MKFSYAFFS2 -o spark_oob.img $TMPFWDIR $OUTFILE << EOF

EOF

#$OUTDIR/* zip $OUTFILE.zip $OUTDIR