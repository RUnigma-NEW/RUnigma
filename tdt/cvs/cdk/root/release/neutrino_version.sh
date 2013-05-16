#!/bin/sh

#################
RELEASE=0
BETA=1
INTERNAL=2
SNAPSHOT=$RELEASE
TIMESTAMP=`date +%Y%m%d%H%M`
#BUILDREV=`git describe | cut -f2 -d'-'`
BUILDREV="100"
#################

CURDIR=`pwd`
KATIDIR=${CURDIR%/cvs/cdk}
version=`git describe`
cat $KATIDIR/cvs/cdk/root/var/etc/.version | head -n 6 > $KATIDIR/cvs/cdk/root/var/etc/.version.new

#########################################
# Original Neutrino Release Cycle Format:
# SBBBYYYYMMTTHHMM
# | |   | | |  | |_MM=Minutes 0 -59
# | |   | | |  |___HH=Hours 0 - 23
# | |   | | |______TT=Day 0-31
# | |   | |________MM=Month 0-12
# | |   |__________YYYY=Year
# | |______________BBB=Build-Revision (interesting for Image-Groups!)
# |________________S=Snapshot Version: 0 = Release, 1 = Beta, 2 = Internal
#########################################

echo "version=$SNAPSHOT$BUILDREV$TIMESTAMP" >> $KATIDIR/cvs/cdk/root/var/etc/.version.new
echo "git=$version" >> $KATIDIR/cvs/cdk/root/var/etc/.version.new
mv $KATIDIR/cvs/cdk/root/var/etc/.version.new $KATIDIR/cvs/cdk/root/var/etc/.version
exit
