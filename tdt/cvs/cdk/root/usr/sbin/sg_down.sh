#!/bin/sh

#see http://www.nslu2-info.de/showthread.php?t=3207&page=6
#see Homepage: http://rusi.is-a-geek.org

# start-stop-daemon --start -v --background --pidfile /var/run/rusi_sg_downd.pid --make-pidfile --exec /usr/local/sbin/rusi_sg_down

# This shell script uses sg3_utils (http://sg.torque.net/sg/) to
# automatically put a SCSI device into standby mode after
# a configurable idle time period. This also works with some USB
# storage devices

# time in seconds of device inactivity until it will be 
# put to standby mode (15 minutes)
IDLE_UNTIL_STANDBY=300
# device to use
DEVICE=sda

CONFIGDEFAULTFILE="/etc/default/sg_down"
# Reads config file (will override defaults above)
[ -r "$CONFIGDEFAULTFILE" ] && . $CONFIGDEFAULTFILE

state=`echo -n 'ibase=16;$( cat /sys/block/$DEVICE/device/iorequest_cnt ) | bc`
count=$IDLE_UNTIL_STANDBY
up=1
while [ 1 ]; do
   sleep 30
   count=$(($count-30))
   newstate=`echo -n 'ibase=16;$( cat /sys/block/$DEVICE/device/iorequest_cnt ) | bc`
   if [ "$state" = "$newstate" ]; then
      if [ $count -lt 0 ]; then
         count=300
         if [ $up = 1 ]; then
            sync
            state=`echo -n 'ibase=16;$( cat /sys/block/$DEVICE/device/iorequest_cnt ) | bc`
            /usr/bin/sg_start 0 -pc=3 /dev/$DEVICE
            echo "Drive sent down\t" `date`
            up=0
         fi
      fi
   else
      echo -e "drive is up\t" `date` >> /opt/drivelog.txt
      count=300
      state="$newstate"
      up=1
   fi
done


#lastRequestCount = 0
#lastAccessTime = 0
#sleepTime = 0
#
#while true
#  f = File.open('/sys/block/' + DEVICE + '/device/iorequest_cnt', 'r');
#  requestCount = f.readline.chomp.hex
#  f.close
#  now = Time.now.to_f
#
#  if requestCount != lastRequestCount
#    lastAccessTime = now
#  end
#
#  # if we are after the timeout and there were new
#  # accesses after the last sleep time -> go to sleep (again)
#  if now > lastAccessTime + IDLE_UNTIL_STANDBY \
#        && lastAccessTime > sleepTime
#
#    sleepTime = now
#    system '/usr/bin/sg_start 0 -pc=3 /dev/' + DEVICE
#
#    # increase the request count by one for the command we just sent
#    requestCount += 1
#  end
#
#  lastRequestCount = requestCount
#
#  # sleep 1 minute
#  sleep 60
#end
