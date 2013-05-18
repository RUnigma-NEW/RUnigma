#
# BOOTSTRAP
#
$(DEPDIR)/bootstrap: \
$(DEPDIR)/%bootstrap: \
	$(FILESYSTEM) \
	| $(GLIBC) \
	$(CROSS_LIBGCC) \
	$(GLIBC) \
	$(GLIBC_DEV) \
	$(ZLIB) \
	$(ZLIB_DEV) \
	$(ZLIB_BIN) \
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
$(DEPDIR)/bare-os: \
$(DEPDIR)/%bare-os: \
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
	$(SYSVINIT) \
	$(SYSVINITTOOLS) \
	$(INITSCRIPTS) \
	$(NETBASE) \
	$(BC) \
	$(DISTRIBUTIONUTILS) \
	u-boot-utils

$(DEPDIR)/net-utils: \
$(DEPDIR)/%net-utils: \
	$(NETKIT_FTP) \
	portmap \
	$(NFSSERVER) \
	vsftpd \
	ethtool \
	opkg \
	grep \
	$(CIFS)

$(DEPDIR)/disk-utils: \
$(DEPDIR)/%disk-utils: \
	e2fsprogs \
	$(XFSPROGS) \
	util-linux \
	jfsutils \
	$(SG3)

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
		udev-rules \
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
	pppd \
	modem-scripts \
	ntfs_3g \
	enigma2_openwebif \
	enigma2-plugins-sh4-networkbrowser \
	enigma2-plugins-sh4-libgisclubskin \
	$(addsuffix -openpli,$(openpli_plugin_distlist)) \
	wireless_tools
	
all-extras: \
	usb_modeswitch \
	pppd \
	modem-scripts \
	evebrowser \
	enigma2-plugins \
	xupnpd \
	ntfs_3g \
	wireless_tools \
	enigma2-skins-sh4 \
	ntpclient \
	udpxy \
	package-index
