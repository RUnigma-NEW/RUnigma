#
# BOOTSTRAP
#
$(DEPDIR)/bootstrap: \
	build.env \
	$(FILESYSTEM) \
	| $(GLIBC_DEV) \
	$(CROSS_LIBGCC) \
	libz \
	$(BINUTILS) \
	$(BINUTILS_DEV) \
	$(GMP) \
	$(MPFR) \
	$(MPC) \
	$(LIBSTDC) \
	$(LIBSTDC_DEV)
	touch $@

#
# BARE-OS
#
bare-os: \
	bootstrap \
	$(LIBTERMCAP) \
	$(NCURSES_BASE) \
	$(NCURSES) \
	$(NCURSES_DEV) \
	$(BASE_PASSWD) \
	$(MAKEDEV) \
	$(BASE_FILES) \
	module_init_tools \
	busybox \
	grep \
	$(INITSCRIPTS) \
	$(NETBASE) \
	$(BC) \
	$(SYSVINIT) \
	$(SYSVINITTOOLS) \
	$(DISTRIBUTIONUTILS) \
	u-boot-utils

net-utils: \
	$(NETKIT_FTP) \
	portmap \
	nfs_utils \
	vsftpd \
	ethtool \
	opkg \
	grep \
	$(CIFS)
	touch $@

disk-utils: \
	e2fsprogs \
	$(XFSPROGS) \
	util-linux \
	jfsutils \
	$(SG3)
	touch $@
#
# YAUD
#
yaud-stock: yaud-none stock

yaud-enigma2-nightly: yaud-none \
		host_python \
		lirc \
		stslave \
		boot-elf \
		init-scripts \
		enigma2-nightly \
		release

yaud-enigma2-pli-nightly-base: yaud-none \
		host_python \
		lirc \
		stslave \
		boot-elf \
		init-scripts \
		enigma2-pli-nightly

yaud-enigma2-pli-nightly: yaud-enigma2-pli-nightly-base release

yaud-enigma2-pli-nightly-full: yaud-enigma2-pli-nightly-base min-extras release

yaud-xbmc-nightly: yaud-none host_python boot-elf xbmc-nightly init-scripts-xbmc release_xbmc

yaud-none: \
		bare-os \
		opkg-host \
		libdvdcss \
		libdvdread \
		libdvdnav \
		linux-kernel \
		net-utils \
		disk-utils \
		driver \
		udev \
		fp_control \
		evremote2 \
		devinit \
		ustslave \
		stfbcontrol \
		showiframe \
		streamproxy
#
# EXTRAS
#
min-extras: \
	usb_modeswitch \
	modem-scripts \
	ntfs_3g \
	enigma2_openwebif \
	enigma2-plugins-sh4-networkbrowser \
	enigma2-plugins-sh4-libgisclubskin \
	$(addsuffix -openpli,$(openpli_plugin_distlist)) \
	wireless_tools \
	enigma2-plugins-sh4
	
all-extras: \
	usb_modeswitch \
	modem-scripts \
	enigma2-plugins \
	xupnpd \
	ntfs_3g \
	enigma2-plugins-sh4 \
	wireless_tools \
	enigma2-skins-sh4 \
	ntpclient \
	udpxy \
	package-index
