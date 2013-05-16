#!/bin/sh
#
# ---> CONFIGURATION AT LINE 83 <---
#
# MANUAL:
# -------
# 1.
# If you run VDR as user, you need in /etc/fstab the option "user" for your HDD mountpoint.
#
# Example:
# /dev/sdf /media/hdd auto defaults,ro,user,noauto 0 0
#
# 2.
# If you use a VFAT partition for your video-data, you must configure sudo to allow the
# VDR-user to execute mount and umount as root. To edit sudoers run 'visudo' command as root.
#
# Example:
# # Cmnd alias specification
# Cmnd_Alias SYSTEM = /bin/mount, /bin/umount
# # User privilege specification
# vdr	ALL=(root) NOPASSWD: SYSTEM
#
# With a vfat partition some things doesn't work: resume, marks
#
# 3.
# Tools needed:   mount, awk, find, test, stat, sed
#
# 4.
# If you have some Archive-HDDs with the index.vdr only on Archive-HDD, you don't use vfat
# and you want to see the recording length in the menu, you can switch GETLENGTH to 1 and
# the script will create a length.vdr for you.
#
# 5.
# If something went wrong, set DEBUG=1 and send me the file which is defined in $DEBUGLOG.
# Use VDR-Portal or EMail.
#
# 6.
# Exitcodes:
#
# exit 0 - no error
# exit 1 - mount/umount error
# exit 3 - wrong hdd / recording not found
# exit 4 - error while linking [0-9]*.vdr
# exit 5 - sudo or mount --bind / umount error (vfat system)
#
# CONFIGURATION
# -------------
#<Configuration>

# Mountpoint, the same as in fstab
MOUNTPOINT="/mnt/archive-hdd" # no trailing '/'
# Log warnings/errors in syslog. 1 = yes, 0 = no.
SYSLOG=1

# Create a length.vdr after mounting the Archive-HDD for the played recording. 1 = yes, 0 = no.
# Only for non-vfat and with index.vdr only on Archive-HDD.
GETLENGTH=1
# Put debug infos in file $DEBUGLOG. Only if $DEBUG=1.
DEBUG=0
DEBUGLOG="/tmp/hddarchive.sh-debug.log"
VDRSETTINGSFILE=/etc/conf.d/vdr

#</Configuration>
# No changes needed after this mark

# read config file
if [ -f /etc/vdr/hddarchive.conf ]; then
    . /etc/vdr/hddarchive.conf
fi

# get VIDEO path
if [ -f ${VDRSETTINGSFILE} ]; then
    source ${VDRSETTINGSFILE}
fi

# Remove trailing slash
MOUNTPOINT="$(echo "${MOUNTPOINT}" | sed -e 's/\/$//')"
if [ -L "$MOUNTPOINT" ]; then
	MOUNTPOINTT="$(find "$MOUNTPOINT" -printf "%l")"
else
	MOUNTPOINTT="$MOUNTPOINT"
fi
# determine hdd-device
DEVICE="$(awk '( $1 !~ /^#/ ) && ( $2 == "'$MOUNTPOINT'" ) { printf("%s", $1); exit; }' /etc/fstab)"
if [ -L "$DEVICE" ]; then
	DEVICET="$(find "$DEVICE" -printf "%l")"
else
	DEVICET="$DEVICE"
fi

ACTION="$1"
REC="$2"
NAME="$3"
if [ "$ACTION" = "mount" -a -z "$NAME" ]; then
	NAME="basename ${REC})"
fi

# function to print help
call() {
	echo -e "\nScript $0 needs three parameters for mount and two for umount. The first must be mount or umount, the second is the full path.\n"
	echo -e "Only for mounting the script needs a third parameter, the last part of the recording path.\n"
	echo -e "Example: hddarchive.sh mount '/video0/Music/%Riverdance/2004-06-06.00:10.50.99.rec' '2004-06-06.00:10.50.99.rec'\n"
	echo -e "Example: hddarchive.sh umount '/video0/Music/%Riverdance/2004-06-06.00:10.50.99.rec'\n"
	echo -e "For more information read the MANUAL part inside this script.\n"
}

# function to log messages
log() {
	case "$1" in
		info)
			echo -e "INFO: $2"
			[ $SYSLOG -eq 1 ] && logger -t "$0" "INFO: $2"
			;;
		warning)
			echo -e "WARNING: $2"
			[ $SYSLOG -eq 1 ] && logger -t "$0" "WARNING: $2"
			;;
		error)
			echo -e "ERROR: $2"
			[ $SYSLOG -eq 1 ] && logger -t "$0" "ERROR: $2"
			if [ $DEBUG -eq 1 ]; then
				echo "-------" >> $DEBUGLOG
				echo -e "Parameters: $ACTION $REC $NAME\n" >> $DEBUGLOG
				echo -e "ERROR: $2\n\n" >> $DEBUGLOG
				echo -e "Mountpoint: $MOUNTPOINT\nDevice: $DEVICE\n" >> $DEBUGLOG
				echo -e "MountpointT: $MOUNTPOINTT\nDeviceT: $DEVICET\n" >> $DEBUGLOG
				FSTAB="$(awk '( $1 !~ /^#/ ) && ( $2 == "'$MOUNTPOINT'" || $2 == "'$MOUNTPOINTT'" ) { printf("%s", $0); }' /etc/fstab)"
				echo -e "fstab: ${FSTAB}\n" >>$DEBUGLOG
				echo -e "Filesystem: $(stat -f -c %T "$REC")\n" >> $DEBUGLOG
				mount >> $DEBUGLOG
				echo >> $DEBUGLOG
				cat /proc/mounts >> $DEBUGLOG
				echo >> $DEBUGLOG
				sudo -l >> $DEBUGLOG
			fi
			;;
	esac
}

# Some checks before doing something
[ "$ACTION" = "mount" -o "$ACTION" = "umount" ] || { call; exit 10; }
[ -z "$REC" -o ! -d "$REC" ] && { call; exit 10; }
[ "$ACTION" = "mount" -a -z "$NAME" ] && { call; exit 10; }
[ ! -d "$MOUNTPOINT" ] && { log error "Mountpoint $MOUNTPOINT doesn't exist"; exit 10; }
[ ! -e "$DEVICE" ] && { log error "Device $DEVICE doesn't exist"; exit 10; }

case "$ACTION" in
mount)
	# check if not mounted
	if mount | egrep -q " $MOUNTPOINTT "; then
		# check if hdd is in use
		if mount | egrep -q "^$DEVICET"; then
			log warning "hdd in use (at: check if hdd is in use)"
		fi

		# if already mounted, try to umount
		log warning "hdd already mounted, try to umount"
		umount "$MOUNTPOINT" || { log error "hdd umount error (at: hdd already mounted, try to umount)"; exit 1; }

		# unlink broken existing links
		for LINK in "${REC}/"*.vdr; do
			if [ -L "$LINK" -a ! -s "$LINK" ]; then
				rm "$LINK"
			fi
		done
		for LINK in "${REC}/"*.ts; do
			if [ -L "$LINK" -a ! -s "$LINK" ]; then
				rm "$LINK"
			fi
		done
		for LINK in "${REC}/"index*; do
			if [ -L "$LINK" -a ! -s "$LINK" ]; then
				rm "$LINK"
			fi
		done
	fi

	# mount hdd
	mount "$MOUNTPOINT" || { log error "hdd mount error (at: mount hdd)"; exit 1; }
	# mount was OK. Now, find recording on hdd

	# search recording in the subtree (Movies/Action/)
	RELPATH1=${REC#$VIDEO}
	RELPATH=${RELPATH1%$NAME}
	DIR="$(find "${MOUNTPOINT}/${RELPATH}" -name "$NAME")"
	# if not found, seach recording on the whole disk
	if [ -z "$DIR" ]; then
		DIR="$(find "${MOUNTPOINT}/" -name "$NAME")"
	fi
	# if not found, umount
	if [ -z "$DIR" ]; then
		log error "wrong hdd / recording not found on hdd"
		umount "$MOUNTPOINT" || { log error "hdd umount error (at: wrong hdd / recording not found on hdd)"; exit 1; }
		exit 3
	fi

	# check if video partition is vfat
	if [ "$(stat -f -c %T "$REC")" != "vfat" ]; then

		# link index.vdr if not exist
		if [ -e "${DIR}/index.vdr"  -a ! -e "${REC}/index.vdr" ]; then
			ln -s "${DIR}/index.vdr" "${REC}/index.vdr" || { log error "could not link index.vdr (at: link index.vdr from hdd to disk)"; }
		fi
		if [ -e "${DIR}/index"      -a ! -e "${REC}/index" ]; then
			ln -s "${DIR}/index" "${REC}/index" || { log error "could not link index (at: link index from hdd to disk)"; }
		fi

		# link [0-9]*.vdr files
		for FILE in "${DIR}/"[0-9]*.vdr; do
			if [ -e "${FILE}" ]; then
				ln -s "$FILE" "${REC}/$(basename "$FILE")"
			fi
		done
		# error while linking [0-9]*.vdr files?
		if [ $? -ne 0 ]; then
			log error "error while linking [0-9]*.vdr"
			# umount hdd bevor unlinking
			umount "$MOUNTPOINT" || { log error "hdd umount error (at: error while linking)"; exit 1; }
			# unlink broken links
			for LINK in "${REC}/"*.vdr; do
				if [ -L "$LINK" -a ! -s "$LINK" ]; then
					rm "$LINK"
				fi
			done
			exit 4
		fi
		for FILE in "${DIR}/"[0-9]*.ts; do
			if [ -e "${FILE}" ]; then
				ln -s "$FILE" "${REC}/$(basename "$FILE")"
			fi
		done
		# error while linking [0-9]*.ts files?
		if [ $? -ne 0 ]; then
			log error "error while linking [0-9]*.ts"
			# umount hdd bevor unlinking
			umount "$MOUNTPOINT" || { log error "hdd umount error (at: error while linking)"; exit 1; }
			# unlink broken links
			for LINK in "${REC}/"*.ts; do
				if [ -L "$LINK" -a ! -s "$LINK" ]; then
					rm "$LINK"
				fi
			done
			exit 4
		fi
		
		# If wanted, create length.vdr
		if [ $GETLENGTH -eq 1 -a ! -s "${REC}/length.vdr" -a -L "${REC}/index.vdr" ]; then
			 echo $(( $(stat -L -c %s "${REC}/index.vdr")/12000 )) > "${REC}/length.vdr"
		fi
		if [ $GETLENGTH -eq 1 -a ! -s "${REC}/length.vdr" -a -L "${REC}/index" ]; then
			 echo $(( $(stat -L -c %s "${REC}/index")/12000 )) > "${REC}/length.vdr"
		fi
	else
		if [ ! "$(sudo -l | egrep "\(root\) NOPASSWD: /bin/mount")" -o ! "$(sudo -l | egrep "\(root\) NOPASSWD: /bin/umount")" ]; then
			log error "you must configure sudo and allow $(whoami) to use mount/umount!"
			log info "$(sudo -l)"
			umount "$MOUNTPOINT" || { log error "hdd umount error (at: you must configure sudo)"; exit 1; }
			exit 5
		fi
		# mount recording
		sudo mount --bind "$DIR" "$REC"
		if [ $? -ne 0 ]; then
			log error "sudo mount --bind $DIR $REC"
			umount "$MOUNTPOINT" || { log error "hdd umount error (at: sudo mount --bind)"; exit 1; }
			exit 5
		fi
	fi
	;;
umount)
	# check if hdd is mounted
	mount | egrep -q " $MOUNTPOINTT " || { log error "hdd not mounted (at: check if hdd is mounted)"; exit 1; }
	# check if video partition is vfat
	if [ "$(stat -f -c %T "$REC")" != "vfat" ]; then
		# is mounted, umount hdd bevor unlinking
		umount "$MOUNTPOINT" || { log error "hdd umount error (at: is mounted, umount hdd bevor unlinking)"; exit 1; }
		# unlink broken links
		for LINK in "${REC}/"index*; do
			if [ -L "$LINK" -a ! -s "$LINK" ]; then
				rm "$LINK"
			fi
		done
		for LINK in "${REC}/"*.vdr; do
			if [ -L "$LINK" -a ! -s "$LINK" ]; then
				rm "$LINK"
			fi
		done
		for LINK in "${REC}/"*.ts; do
			if [ -L "$LINK" -a ! -s "$LINK" ]; then
				rm "$LINK"
			fi
		done
	else
		# umount recording
		sudo umount "$REC" || { log error "sudo umount $REC"; exit 5; }
		# umount Archive-HDD at umount
		umount "$MOUNTPOINT" || { log error "hdd umount error (at: umount hdd at umount)"; exit 1; }
	fi
	;;
*)
# Output help
	log error "\nWrong action $ACTION."
	call
	;;
esac

exit 0
