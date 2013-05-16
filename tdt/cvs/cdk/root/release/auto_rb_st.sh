#!/bin/sh
if [ ! -e /dev/dvb/adapter0/frontend0 ]; then
	cp /etc/init.d/initmodules /var/initmodules
	file="/etc/init.d/initmodules"
	text="demod="
	str=`cat $file | grep $text`
	demod=${str:26:7}
	if [ $demod == stb0899 ]; then
		sed 's/modprobe fe-core demod=stb0899 tuner=stb6100/modprobe fe-core demod=stv090x tuner=stv6110x/' /var/initmodules >/etc/init.d/initmodules
	else
		sed 's/modprobe fe-core demod=stv090x tuner=stv6110x/modprobe fe-core demod=stb0899 tuner=stb6100/' /var/initmodules >/etc/init.d/initmodules
	fi
	rm /var/initmodules
	reboot -f
fi