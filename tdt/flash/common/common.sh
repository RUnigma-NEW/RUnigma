#!/bin/bash
CURDIR=$1
BASEDIR=$CURDIR/../..

echo "Checking if pad exists ($CURDIR/pad)..."
if [ ! -e $CURDIR/pad ]; then
  echo "pad is missing, trying to compile it..."
  cd $CURDIR/pad.src
  $CURDIR/pad.src/compile.sh
  mv $CURDIR/pad.src/pad $CURDIR/pad
  cd $CURDIR
  if [ ! -e $CURDIR/pad ]; then
    echo "Compiling failed! Exiting..."
    exit 3
  else
    echo "Compiling successfull"
  fi
fi

echo "Checking if mksquashfs3.3 exists ($CURDIR/mksquashfs3.3)..."
if [ ! -e $CURDIR/mksquashfs3.3 ]; then
  echo "mksquashfs3.3 is missing, trying to compile it..."
  cd $CURDIR
  rm -rf $CURDIR/squashfs-tools
  tar -xzf $CURDIR/squashfs3.3.tar.gz
  cd $CURDIR/squashfs-tools
  make all
  mv $CURDIR/squashfs-tools/mksquashfs $CURDIR/mksquashfs3.3
  mv $CURDIR/squashfs-tools/unsquashfs $CURDIR/unsquashfs3.3
  cd $CURDIR
  rm -rf $CURDIR/squashfs-tools
  if [ ! -e $CURDIR/mksquashfs3.3 ]; then
    echo "Compiling failed! Exiting..."
    exit 3
  else
    echo "Compiling successfull"
  fi
fi

echo "Checking if mksquashfs4.0 exists ($CURDIR/mksquashfs4.0)..."
if [ ! -e $CURDIR/mksquashfs4.0 ]; then
  echo "mksquashfs4.0 is missing, trying to compile it..."
  cd $CURDIR
  rm -rf $CURDIR/squashfs4.0
  if [ ! -e $CURDIR/squashfs4.0.tar.gz ]; then
    #wget "http://heanet.dl.sourceforge.net/sourceforge/squashfs/squashfs4.0.tar.gz"
    wget "http://pkgs.fedoraproject.org/repo/pkgs/squashfs-tools/squashfs4.0.tar.gz/a3c23391da4ebab0ac4a75021ddabf96/squashfs4.0.tar.gz"
  fi
  if [ ! -e $CURDIR/lzma465.tar.bz2 ]; then
    #wget "http://heanet.dl.sourceforge.net/sourceforge/sevenzip/lzma465.tar.bz2"
    wget "http://pkgs.fedoraproject.org/repo/pkgs/SevenZip/lzma465.tar.bz2/29d5ffd03a5a3e51aef6a74e9eafb759/lzma465.tar.bz2"
  fi
  mkdir $CURDIR/squashfs-tools
  cd $CURDIR/squashfs-tools
  tar -xzf $CURDIR/squashfs4.0.tar.gz
  tar -xjf $CURDIR/lzma465.tar.bz2
  cd $CURDIR/squashfs-tools/squashfs4.0/squashfs-tools
  echo "patch -p1 < $BASEDIR/cvs/cdk/Patches/squashfs-tools-4.0-lzma.patch"
  patch -p1 < $BASEDIR/cvs/cdk/Patches/squashfs-tools-4.0-lzma.patch
  make all
  mv $CURDIR/squashfs-tools/squashfs4.0/squashfs-tools/mksquashfs $CURDIR/mksquashfs4.0
  mv $CURDIR/squashfs-tools/squashfs4.0/squashfs-tools/unsquashfs $CURDIR/unsquashfs4.0
  cd $CURDIR
  rm -rf $CURDIR/squashfs-tools
  if [ ! -e $CURDIR/mksquashfs4.0 ]; then
    echo "Compiling failed! Exiting..."
    exit 3
  else
    echo "Compiling successfull"
  fi
fi
