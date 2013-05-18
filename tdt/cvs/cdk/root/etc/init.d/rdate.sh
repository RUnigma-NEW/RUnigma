#! /bin/sh
#
# rdate.sh
#
# chkconfig: S 99 0
#

RDATE0=ntp.time.in.ua
RDATE=ntp.time.in.ua
RDATE_ALT=ntp.time.in.ua

#Date
echo "Running rdate -s $RDATE..."
rdate -s $RDATE || (echo "Running rdate -s $RDATE_ALT..."; rdate -s $RDATE_ALT)

gmterr=0
file="/etc/enigma2/settings"
text="config.timezone.val"
str=`cat $file | grep $text`
gmt=$(($gmterr${str:24:3}))
fp_control -gmt $gmt
