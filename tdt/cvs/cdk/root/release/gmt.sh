#!/bin/sh
/usr/sbin/rdate ntp.time.in.ua
gmterr=0
file="/etc/enigma2/settings"
text="config.timezone.val"
str=`cat $file | grep $text`
gmt=$(($gmterr${str:24:3}))
fp_control -gmt $gmt