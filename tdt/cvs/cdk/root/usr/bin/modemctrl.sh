#!/bin/sh

DEBUG=0
DEV=$1

[ $DEBUG -eq 1 ] && echo "DEVICE: $DEV" > /tmp/modemctrl.log  || rm -rf /tmp/modemctrl.log

LIST=`find /sys/devices -name "$DEV" | sort -r 2>/dev/null`
[ $DEBUG -eq 1 ] && echo "LIST: $LIST" >> /tmp/modemctrl.log
for path in $LIST; do
 DEVPATH=${path#/sys}
done

MODALIAS=$(cat /sys${DEVPATH}/modalias 2>/dev/null)
TYPE=$(echo $MODALIAS | printf '%d/%d/%d' $(sed 's/.*d[0-9]\{4\}//;s/ic.*//;s/[dscp]\+/ 0x/g'))
PRODUCT=$(echo $MODALIAS | sed 's!^usb:\(.*\)dc.*!\1!;s![vpd]!/!g;s!/0\{1,3\}!/!g;s!^/!!;y!ABCDEF!abcdef!')
INTERFACE=$(udevadm info --query=all  --path=$DEVPATH | grep INTERFACE | sed 's/E: INTERFACE=//')

for var in DEVPATH MODALIAS TYPE PRODUCT INTERFACE; do
 [ $DEBUG -eq 1 ] && echo "$var:" >> /tmp/modemctrl.log 
 [ $DEBUG -eq 1 ] && eval "echo \$${var}" >> /tmp/modemctrl.log 
 [ -z "$(eval "echo \$${var}")" ] && exit 1
done

[ -z "$DEVPATH" ] && exit 1
if [ -d /sys${DEVPATH} ]; then
    cd /sys${DEVPATH}/..
    for var in id[PV]*; do
        [ -r $var ] && eval "$var='$(cat $var)'"
    done
fi

[ 0 -eq "${TYPE%%/*}" ] && TYPE=$INTERFACE

[ $DEBUG ] && echo "TYPE: $TYPE" >> /tmp/modemctrl.log

case $TYPE in

    8/6/*)
	if [ -f "/usr/share/usb_modeswitch/${idVendor}:${idProduct}" ]; then
	    [ $DEBUG -eq 1 ] && echo "${idVendor}:${idProduct} may be 3G modem in zero CD mode" >> /tmp/modemctrl.log
	    usb_modeswitch -v ${idVendor} -p ${idProduct} -c /usr/share/usb_modeswitch/${idVendor}:${idProduct} && exit 0
	fi
        ;;
    255/255/255)
	if [ -f "/usr/share/usb_modeswitch/${idVendor}:${idProduct}" ]; then
	    [ $DEBUG -eq 1 ] && echo "${idVendor}:${idProduct} $TYPE may be 3G modem" >> /tmp/modemctrl.log
	    if [ "${idVendor}" != "0af0" ]; then
		if [ ! -d /sys/module/usbserial ]; then
		    modprobe -q usbserial vendor=0x${idVendor} product=0x${idProduct}
		fi
#	    else
#		if [ ! -d /sys/module/hso ]; then
#		    modprobe -q hso
#		fi
	    fi
	else
	    [ $DEBUG -eq 1 ] && echo "${idVendor}:${idProduct} $TYPE unknow serial device! Try load all serial drivers." >> /tmp/modemctrl.log
	    if [ ! -d /sys/module/usbserial ]; then
		modprobe -q usbserial
	    fi
	    if [ ! -d /sys/module/option ]; then
		modprobe -q option
	    fi
	    if [ ! -d /sys/module/cdc-acm ]; then
		    modprobe -q cdc-acm
	    fi
#	    if [ ! -d /sys/module/hso ]; then
#		modprobe -q hso
#	    fi
	fi
        ;;
    2/*/*)
	[ $DEBUG -eq 1 ] && echo "${idVendor}:${idProduct} $TYPE may be ACM device " >> /tmp/modemctrl.log
	    if [ ! -d /sys/module/cdc-acm ]; then
		    modprobe -q cdc-acm
	    fi
	;;	

    *)
	[ $DEBUG -eq 1 ] && echo "${idVendor}:${idProduct} $TYPE unknow device!" >> /tmp/modemctrl.log
        ;;
        
        

esac
