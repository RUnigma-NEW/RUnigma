#!/bin/sh
#Mount things needed by this script
/sbin/mount -t proc proc /proc
/sbin/mount -t sysfs sysfs /sys

#Create all the symlinks to /bin/busybox
echo "-------------- Загружаю RAMFS -------------------"
echo "-------------------------------------------------"
echo "Устанавливаю Busybox"
busybox --install -s

rm -f /etc/mtab
grep ' / ' /proc/mounts >/etc/mtab

#Create device nodes
mknod /dev/tty c 5 0
mdev -s

#Function for parsing command line options with "=" in them
# get_opt("init=/sbin/init") will return "/sbin/init"
get_opt() {
	echo "$@" | cut -d "=" -f 2
}

#Defaults
init="/sbin/init"
root="/dev/sda1"

echo "Активирую дисплей"
insmod /drvko/proton.ko

if [ `cat /proc/cmdline | grep -c STB=` -eq 0 ] ; then
	reboot -f
fi

echo "Ожидание" > /dev/vfd
while [ -e `fdisk -l | grep -i "Disk" | awk '{ print $1 }'` ]
do
	echo "Ожидание usb"
	sleep 1
done
echo "RAMFS"

#Process command line options
for i in $(cat /proc/cmdline); do
	case $i in
		root\=*)
			root=$(get_opt $i)
			;;
		init\=*)
			init=$(get_opt $i)
			;;
	esac
done

if [ -x `fdisk -l | grep -i "FAT" | awk '{ print $1 }'` ]; then
	if [ `tune2fs -l /dev/sda1 | grep -i "Filesystem state" | awk '{ print $3 }'` == "clean" ]; then
		echo "SDA1 OK"
	else
		echo "FSCK SDA1 run"
		fsck.ext2 -y "${root}"
		tune2fs -l "${root}" | grep -i "Filesystem state"
		sleep 1
	fi
fi

echo "Монтирую загрузочный раздел /dev/sda1"
mount "${root}" /rootfs

if [ -e /rootfs/service/install ]; then
	cd /install
	./install.sh
elif [ -e /rootfs/service/update ]; then
	cd /install
	./update.sh
fi

if [[ -x "/rootfs/${init}" ]] ; then
	rmmod proton
	umount /sys /proc
	rm -rf /dev/*
	echo "Перехожу в системный раздел и начинаю загрузку сборки"
	exec switch_root /rootfs "${init}"
fi

echo "Не могу переключиться в новый системный раздел"
exec sh
