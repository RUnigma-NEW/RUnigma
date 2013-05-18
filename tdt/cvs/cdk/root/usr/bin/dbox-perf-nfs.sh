#!/bin/sh

#Version fuer newmake images

#Usage: dbox-perf.sh <server-ip>

#Zur Bedienung:
#You nedd sort and sed on dbox (# type sed; type sort)
#Im Yadi sind beide Busybox-Befehle enthalten!
#
#Das Programm erwartet keine Parameter.
#In den ersten Zeilen werden die persönlichen Einstellungen eingetragen:
#serverip: Hier kommt die IP des NFS-Servers rein
#exportdir: Hier kommt das exportierte Verzeichnis des NFS-Servers rein (mit führendem Slash)
#dboxmountpoint: Das Verzeichnis auf der d-box, in das gemountet werden soll.
#filesize: Die Größe der zu testenden Datei. Sie sollte 8MByte oder mehr sein, damit die Ergebnisse
# brauchbar werden. 32 oder 64 MByte sind ein guter Ansatz. Wenn man dann noch genauer werden will, kann man das ganze ja nochmal mit 128 MByte starten.
#blocksizelist: Hier können die zu testenden Blockgrößen durch Leerzeichen getrennt eingegeben werden.
# Mögliche Werte sind 1, 2, 4, 8, 16 und 32. Wenn man beispielsweise weiss, dass die Werte 1, 2 und 4 keine verwertbare
# Performance bringen, so kann man sich eine Menge Zeit sparen, wenn man diese Tests auslässt.
#enablesynctests: Im normalfall sollten die Uebertragungsratetn mit der Mountoption "sync" immer langsamer sein als die mit "async".
# Darum besteht hier die Moeglichkeit, Tests mit der sync-Option abzuschalten. Moegliche Werte sind "yes" (Tests ausführen) oder "no" (Tests nicht ausführen. 

# IP of your NFS server
serverip=$1
# exported directory on your NFS server
exportdir=/ahorn/test
# mount point on dbox
dboxmountpoint="/mnt/custom"
[ ! -d /mnt/custom ] && mkdir -p /mnt/custom
# filesize to transfer in MBytes.
# At least 8 MByte. Good values are 32 or 64 MByte.
# Try 128 to be more accurate (takes much longer!)
filesize=64
# block sizes to test in KBytes, possible values are 1 2 4 8 16 32.
# values have to be separated with spaces. See examples below.
# blocksizelist="4 8 32"
# blocksizelist="16"
#blocksizelist="4 8 16 32"
blocksizelist="32"
# wether to enable synchronous reading, writing. Possible values are "yes"
# or no. Normally synchronous reading or writing should be slower than
# asynchronous, so to save some time most people would say "no" here.
enablesynctests="no"


##################################################################
########             don't edit below this line           ########
##################################################################

bs=8192
count=`expr $filesize \* 1048576 / $bs`
wlist=""
rlist=""
synctests="async"
if [ $enablesynctests = "yes" ]; then
  synctests="sync "$synctests
fi

echo
echo "Measuring NFS throughput..."
for factor in $blocksizelist
do
  for protocol in udp tcp
  do
    for synchronized in $synctests
    do
       size=`expr $factor \* 1024`
       bitcount=`expr $bs \* $count \* 8`
       umount $dboxmountpoint 2>/dev/null
       mount -t nfs -o rw,soft,$protocol,nolock,$synchronized,rsize=$size,wsize=$size $serverip:$exportdir $dboxmountpoint
       echo "mount -t nfs -o rw,soft,$protocol,nolock,$synchronized,rsize=$size,wsize=$size $serverip:$exportdir $dboxmountpoint"
       mount |grep $dboxmountpoint
       
       #Writing...
       #readsize=8192
       echo "Mount options: "$protocol", "$synchronized", wsize="$size
       echo "writing "$filesize" MBytes..."
       a=`date +%s`
       dd if=/dev/zero of=$dboxmountpoint/test-dboxfile bs=$bs count=$count 2>/dev/null;
       if [ $? = "0" ]
       then
         z=`date +%s`
         duration=`expr $z - $a`
         throughput=`expr $bitcount / $duration`
         throughputinkb=`expr $throughput / 1024`
         throughputinkB=`expr $throughputinkb / 8`
         echo "Success after "$duration" seconds"
       else
         throughput="Failure"
         echo "Failure"
       fi
       wlist=$wlist$throughput" "$throughputinkb" kbps "$throughputinkB" kBps with "$protocol","$synchronized",wsize="$size"\n"
       
       #Reading...
       #writesize=8192
       echo "Mount options: "$protocol", "$synchronized", rsize="$size
       echo "reading "$filesize" MBytes..."
       a=`date +%s`
       dd of=/dev/null if=$dboxmountpoint/test-dboxfile bs=$bs count=$count 2>/dev/null;
       if [ $? = "0" ]
       then
         z=`date +%s`
         duration=`expr $z - $a`
         throughput=`expr $bitcount / $duration`
         throughputinkb=`expr $throughput / 1024`
         throughputinkB=`expr $throughputinkb / 8`
         echo "Success after "$duration" seconds"
       else
         throughput="Failure"
         echo "Failure"
       fi
       rlist=$rlist$throughput" "$throughputinkb" kbps "$throughputinkB" kBps with "$protocol","$synchronized",rsize="$size"\n"
       echo
    done
  done
done

echo "Results for write throughput:"
#echo -e $wlist | sort -nr | sed 's/^\([0-9]*\)\([0-9]\{3\}\)\([0-9]\{3\}\(.*\)\)/\1.\2 Mbit\/s\4/g'
#echo $wlist | sort -nr | sed 's/^\([0-9]*\)\([0-9]\{3\}\)\([0-9]\{3\}\(.*\)\)/\1.\2 Mbit\/s\4/g'
echo $wlist | sort -nr

echo "Results for read throughput:"
#echo -e $rlist | sort -nr | sed 's/^\([0-9]*\)\([0-9]\{3\}\)\([0-9]\{3\}\(.*\)\)/\1.\2 Mbit\/s\4/g' 
#echo $rlist | sort -nr | sed 's/^\([0-9]*\)\([0-9]\{3\}\)\([0-9]\{3\}\(.*\)\)/\1.\2 Mbit\/s\4/g' 
echo $rlist | sort -nr

(rm $dboxmountpoint/test-dboxfile) && umount $dboxmountpoint 2>/dev/null
