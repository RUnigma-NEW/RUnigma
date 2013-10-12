#!/bin/sh

#Mount things needed by this script
/sbin/mount -t proc proc /proc
/sbin/mount -t sysfs sysfs /sys

#Create all the symlinks to /bin/busybox
echo "-------------- Загружаю RAMFS -------------------"
echo "-------------------------------------------------"
echo "Устанавливаю Busybox"
busybox --install -s

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
rootfs="/dev/sda2"
record="/dev/sda3"

echo "Активирую дисплей"
insmod /drvko/proton.ko

while [ -e `fdisk -l | grep -i "Disk" | awk '{ print $1 }'` ]
do
	echo "Ожидание usb"  > /dev/vfd
	echo "Ожидание usb"
	sleep 1
done
echo "usb-ok"  > /dev/vfd
echo "usb-ok"

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
if [ `tune2fs -l /dev/sda1 | grep -i "Filesystem state" | awk '{ print $3 }'` == "clean" ]; then
    echo "SDA1 OK"
else
    echo "Проверяю раздел SDA1" > /fsck.log
    echo "ПРОВЕРКА" > /dev/vfd
    fsck.ext3 -f -y "${root}" >> /fsck.log
    tune2fs -l "${root}" | grep -i "Filesystem state" >> /fsck.log
fi
#if [ `tune2fs -l /dev/sda2 | grep -i "Filesystem state" | awk '{ print $3 }'` == "clean" ]; then
#    echo "SDA2 OK"
#else
#    echo "Проверяю раздел SDA2" > /fsck.log
#    echo "ПРОВЕРКА" > /dev/vfd
#    fsck.ext4 -f -y "${rootfs}" >> /fsck.log
#    tune2fs -l "${rootfs}" | grep -i "Filesystem state" >> /fsck.log
#fi
#if [ `tune2fs -l /dev/sda3 | grep -i "Filesystem state" | awk '{ print $3 }'` == "clean" ]; then
#    echo "SDA3 OK"
#else
#    echo "Проверяю раздел SDA3" > /fsck.log
#    echo "ПРОВЕРКА" > /dev/vfd
#    fsck.ext4 -f -y "${record}" >> /fsck.log
#    tune2fs -l "${record}" | grep -i "Filesystem state" >> /fsck.log
#fi
#Mount the root device
echo "Монтирую загрузочный раздел /dev/sda1"
mount "${root}" /rootfs

# Check auf installing System Files
if [ -e /rootfs/service/install ]; then
	cd /install
	./install.sh
elif [ -e /rootfs/service/update ]; then
	cd /install
	./update.sh
fi
# Wenn kein install mounte rootfs to start
# Switch from /dev/sda1 to /dev/sda2 (ROOTFS)
#if [ ! -e /rootfs/install ]; then
#echo "Демонтирую /dev/sda1"
#umount "${root}"
#fi
#echo "Монтирую /dev/sda2"
#mount "${rootfs}" /rootfs

# erst hier ist sda2 mounted
if [ -e /fsck.log ]; then
    mkdir /rootfs/var/config 
	cp /fsck.log /rootfs/var/config/fsck.log
	rm /fsck.log
fi
#Check if $init exists and is executable
if [[ -x "/rootfs/${init}" ]] ; then
	#Unmount all other mounts so that the ram used by
	#the initramfs can be cleared after switch_root
	umount /sys /proc
	
	#Switch to the new root and execute init
	echo "Перехожу в новый системный раздел и начинаю загрузку сборки"
	exec switch_root /rootfs "${init}"
fi

#This will only be run if the exec above failed
echo "Не могу переключиться в новый системный раздел"
exec sh
