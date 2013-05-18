#!/bin/bash

CURDIR=$1
TUFSBOXDIR=$2
OUTDIR=$3
TMPKERNELDIR=$4
TMPROOTDIR=$5

echo "CURDIR       = $CURDIR"
echo "TUFSBOXDIR   = $TUFSBOXDIR"
echo "OUTDIR       = $OUTDIR"
echo "TMPKERNELDIR = $TMPKERNELDIR"
echo "TMPROOTDIR   = $TMPROOTDIR"

MKFSJFFS2=$TUFSBOXDIR/host/bin/mkfs.jffs2
MKSQUASHFS=$CURDIR/../common/mksquashfs4.0
SUMTOOL=$TUFSBOXDIR/host/bin/sumtool
PAD=$CURDIR/../common/pad

OUTFILE=$OUTDIR/e2jffs2.img

if [ ! -e $OUTDIR ]; then
  mkdir $OUTDIR
fi

if [ -e $OUTFILE ]; then
  rm -f $OUTFILE
fi

# --- KERNEL ---
# Size 8MB !
cp -f $TMPKERNELDIR/uImage $OUTDIR/uImage
#$PAD 0x800000 $CURDIR/uImage $CURDIR/mtd_kernel.pad.bin #padding of kernel uImage is not needed

# --- ROOT ---
# Size 64MB !
#echo "MKFSJFFS2 --qUfv -p0x4000000 -e0x20000 -r $TMPROOTDIR -o $CURDIR/mtd_root.bin"
#$MKFSJFFS2 -qUfv -p0x4000000 -e0x20000 -r $TMPROOTDIR -o $CURDIR/mtd_root.bin
#echo "SUMTOOL -v -p -e 0x20000 -i $CURDIR/mtd_root.bin -o $CURDIR/mtd_root.sum.bin"
#$SUMTOOL -v -p -e 0x20000 -i $CURDIR/mtd_root.bin -o $CURDIR/mtd_root.sum.bin
#echo "$PAD 0x4000000 $CURDIR/mtd_root.sum.bin $CURDIR/mtd_root.sum.pad.bin"
#$PAD 0x4000000 $CURDIR/mtd_root.sum.bin $CURDIR/mtd_root.sum.pad.bin

echo "MKFSJFFS2 --qUfv -e0x20000 -r $TMPROOTDIR -o $CURDIR/mtd_root.bin"
$MKFSJFFS2 -qUfv -e0x20000 -r $TMPROOTDIR -o $CURDIR/mtd_root.bin
echo "SUMTOOL -v -p -e 0x20000 -i $CURDIR/mtd_root.bin -o $CURDIR/mtd_root.sum.pad.bin"
$SUMTOOL -v -p -e 0x20000 -i $CURDIR/mtd_root.bin -o $CURDIR/mtd_root.sum.pad.bin

#rm -f $CURDIR/uImage
rm -f $CURDIR/mtd_root.bin
rm -f $CURDIR/mtd_root.sum.bin

#SIZE=`stat mtd_kernel.pad.bin -t --format %s`
#SIZE=`printf "0x%x" $SIZE`
#if [[ $SIZE > "0x800000" ]]; then
#  echo "KERNEL TO BIG. $SIZE instead of 0x800000" > /dev/stderr
#fi

#SIZE=`stat mtd_root.sum.pad.bin -t --format %s`
#SIZE=`printf "0x%x" $SIZE`
#if [[ $SIZE > "0x4000000" ]]; then
#  echo "ROOT TO BIG. $SIZE instead of 0x4000000" > /dev/stderr
#fi

#mv $CURDIR/mtd_kernel.pad.bin $OUTDIR/uImage
mv $CURDIR/mtd_root.sum.pad.bin $OUTDIR/e2jffs2.img

#rm -f $CURDIR/mtd_kernel.pad.bin
rm -f $CURDIR/mtd_root.sum.pad.bin

cd $OUTDIR;zip $OUTFILE.zip e2jffs2.img uImage
