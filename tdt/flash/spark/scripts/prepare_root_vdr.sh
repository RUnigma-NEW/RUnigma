#!/bin/bash

CURDIR=$1
RELEASEDIR=$2

TMPROOTDIR=$3
TMPKERNELDIR=$4

cp -a $RELEASEDIR/* $TMPROOTDIR

cd $TMPROOTDIR/dev/
$TMPROOTDIR/etc/init.d/makedev start
cd -

# --- BOOT ---
mv $TMPROOTDIR/boot/uImage $TMPKERNELDIR/uImage