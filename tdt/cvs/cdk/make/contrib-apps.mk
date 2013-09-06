#
# bzip2
#
BEGIN[[
bzip2
  1.0.6
  {PN}-{PV}
  extract:http://www.bzip.org/{PV}/{PN}-{PV}.tar.gz
  patch:file://{PN}.diff
  make:install:PREFIX=PKDIR/usr
;
]]END
DESCRIPTION_bzip2 = "bzip2"

FILES_bzip2 = \
/usr/bin/* \
/usr/lib/*

$(DEPDIR)/bzip2: bootstrap $(DEPENDS_bzip2)
	$(PREPARE_bzip2)
	$(start_build)
	cd $(DIR_bzip2); \
		mv Makefile-libbz2_so Makefile; \
		$(MAKE) all CC=$(target)-gcc; \
		$(INSTALL_bzip2)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_bzip2)
	touch $@

#
# MODULE-INIT-TOOLS
#
BEGIN[[
module_init_tools
  3.16
  {PN}-{PV}
  extract:http://ftp.osuosl.org/pub/linux/utils/kernel/module-init-tools/{PN}-{PV}.tar.bz2
  patch:file://module-init-tools-no-man.patch
  make:INSTALL=install:install:sbin_PROGRAMS="depmod modinfo":bin_PROGRAMS=:mandir=/usr/share/man:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/module_init_tools: bootstrap $(DEPDIR)/lsb $(MODULE_INIT_TOOLS_ADAPTED_ETC_FILES:%=root/etc/%) $(DEPENDS_module_init_tools)
	$(PREPARE_module_init_tools)
	cd $(DIR_module_init_tools); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--enable-zlib \
			--prefix=; \
		$(MAKE); \
		$(INSTALL_module_init_tools)
	$(call adapted-etc-files,$(MODULE_INIT_TOOLS_ADAPTED_ETC_FILES))
	$(call initdconfig,module-init-tools)
	$(DISTCLEANUP_module_init_tools)
	touch $@

#
# GREP
#
BEGIN[[
grep
  2.14
  {PN}-{PV}
  extract:ftp://mirrors.kernel.org/gnu/{PN}/{PN}-{PV}.tar.xz
  nothing:http://64studio.hivelocity.net/apt/pool/main/g/{PN}/{PN}_2.5.1.ds2-6.diff.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_grep = "grep"

FILES_grep = \
/usr/bin/grep

$(DEPDIR)/grep: bootstrap $(DEPENDS_grep)
	$(PREPARE_grep)
	$(start_build)
	cd $(DIR_grep); \
		gunzip -cd $(lastword $^) | cat > debian.patch; \
		patch -p1 <debian.patch; \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--disable-nls \
			--disable-perl-regexp \
			--libdir=$(targetprefix)/usr/lib \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_grep)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_grep)
	touch $@

#
# USB MODESWITCH
#
BEGIN[[
usb_modeswitch
  1.2.5
  {PN}-{PV}
  extract:http://www.draisberghof.de/usb_modeswitch/{PN}-{PV}.tar.bz2
  patch:file://{PN}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_usb_modeswitch = usb_modeswitch
RDEPENDS_usb_modeswitch = libusb usb_modeswitch_data
FILES_usb_modeswitch = \
/etc/* \
/lib/udev/* \
/usr/sbin/*

$(DEPDIR)/usb_modeswitch: $(DEPENDS_usb_modeswitch) $(RDEPENDS_usb_modeswitch)
	$(PREPARE_usb_modeswitch)
	$(start_build)
	cd $(DIR_usb_modeswitch) ; \
		$(BUILDENV) \
		DESTDIR=$(PKDIR) \
		PREFIX=$(PKDIR)/usr \
	  $(MAKE) $(MAKE_OPTS) install
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_usb_modeswitch)
	touch $@
	

#
# USB MODESWITCH DATA
#
BEGIN[[
usb_modeswitch_data
  20121109
  {PN}-{PV}
  extract:http://www.draisberghof.de/usb_modeswitch/{PN}-{PV}.tar.bz2
  patch:file://{PN}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_usb_modeswitch_data = usb_modeswitch_data

FILES_usb_modeswitch_data = \
/usr/* \
/etc/* \
/lib/udev/rules.d

$(DEPDIR)/usb_modeswitch_data: $(DEPENDS_usb_modeswitch_data)
	$(PREPARE_usb_modeswitch_data)
	$(start_build)
	cd $(DIR_usb_modeswitch_data) ; \
		$(BUILDENV) \
		DESTDIR=$(PKDIR) \
		$(MAKE) install
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_usb_modeswitch_data)
	touch $@
	
#
# NTFS-3G
#
BEGIN[[
ntfs_3g
  2013.1.13
  ntfs-3g_ntfsprogs-{PV}
  extract:http://tuxera.com/opensource/ntfs-3g_ntfsprogs-{PV}.tgz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_ntfs_3g = ntfs-3g
#RDEPENDS_ntfs_3g = fuse
FILES_ntfs_3g = \
/bin/ntfs-3g \
/sbin/mount.ntfs-3g \
/usr/lib/* \
/lib/*

$(DEPDIR)/ntfs_3g: $(DEPENDS_ntfs_3g)
	$(PREPARE_ntfs_3g)
	$(start_build)
	export PATH=$(hostprefix)/bin:$(PATH); \
	LDCONFIG=$(prefix)/cdkroot/sbin/ldconfig \
	cd $(DIR_ntfs_3g) ; \
		$(BUILDENV) \
		PKG_CONFIG=$(hostprefix)/bin/pkg-config \
		./configure \
			--build=$(build) \
			--disable-ldconfig \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) $(MAKE_OPTS); \
		$(INSTALL_ntfs_3g)
	$(tocdk_build)	
	$(toflash_build)
	$(DISTCLEANUP_ntfs_3g)
	touch $@
	

#
# LSB
#
BEGIN[[
lsb
  3.2-28
  {PN}-3.2
  extract:http://www.emdebian.org/locale/pool/main/l/lsb/{PN}_{PV}.tar.gz
  install:-d:PKDIR/lib/{PN}
  install:-m644:init-functions:PKDIR/lib/{PN}
;
]]END

DESCRIPTION_lsb = "lsb"
FILES_lsb = \
/lib/lsb/*

$(DEPDIR)/lsb: bootstrap $(DEPENDS_lsb)
	$(PREPARE_lsb)
	$(start_build)
	cd $(DIR_lsb); \
		$(INSTALL_lsb)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_lsb)
	touch $@

#
# PORTMAP
#
BEGIN[[
portmap
  6.0
  {PN}_{PV}
  extract:http://fossies.org/linux/misc/{PN}-{PV}.tgz
  patch:file://{PN}_{PV}.diff
  patch:http://debian.osuosl.org/debian/pool/main/p/{PN}/{PN}_{PV}.0-2.diff.gz
  make:install:BASEDIR=PKDIR
  install:-m755:debian/init.d:PKDIR/etc/init.d/{PN}
;
]]END

DESCRIPTION_portmap = "the program supports access control in the style of the tcp wrapper (log_tcp) packag"
FILES_portmap = \
/sbin/* \
/etc/init.d/

$(DEPDIR)/portmap: bootstrap $(DEPENDS_portmap)
	$(PREPARE_portmap)
	$(start_build)
	mkdir -p $(PKDIR)/sbin/
	mkdir -p $(PKDIR)/etc/init.d/
	mkdir -p $(PKDIR)/usr/share/man/man8
	cd $(DIR_portmap); \
		gunzip -cd $(lastword $^) | cat > debian.patch; \
		patch -p1 <debian.patch; \
		sed -e 's/### BEGIN INIT INFO/# chkconfig: S 41 10\n### BEGIN INIT INFO/g' -i debian/init.d; \
		$(BUILDENV) \
		$(MAKE); \
		$(INSTALL_portmap)
	$(call adapted-etc-files,$(PORTMAP_ADAPTED_ETC_FILES))
	$(call initdconfig,portmap)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_portmap)
	touch $@

#
# OPENRDATE
#
BEGIN[[
openrdate
  1.2
  {PN}-{PV}
  extract:http://prdownloads.sourceforge.net/sourceforge/{PN}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_openrdate = openrdate
FILES_openrdate = \
/usr/bin/* \
/etc/init.d/*

$(DEPDIR)/openrdate: bootstrap $(DEPENDS_openrdate)
	$(PREPARE_openrdate)
	$(start_build)
	cd $(DIR_openrdate); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_openrdate)
	$(INSTALL_DIR) $(PKDIR)/etc/init.d/; \
	( cd root/etc && for i in $(OPENRDATE_ADAPTED_ETC_FILES); do \
		[ -f $$i ] && $(INSTALL) -m644 $$i $(PKDIR)/etc/$$i || true; \
		[ "$${i%%/*}" = "init.d" ] && chmod 755 $(PKDIR)/etc/$$i || true; done ); \
	( export HHL_CROSS_TARGET_DIR=$(prefix)/release && cd $(prefix)/release/etc/init.d; \
		for s in rdate ; do \
			$(hostprefix)/bin/target-initdconfig --add $$s || \
			echo "Unable to enable initd service: $$s" ; done && rm *rpmsave 2>/dev/null || true )
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_openrdate)
	touch $@

#
# E2FSPROGS
#
BEGIN[[
e2fsprogs
  1.42.8
  {PN}-{PV}
  extract:http://sourceforge.net/projects/e2fsprogs/files/e2fsprogs/v{PV}/{PN}-{PV}.tar.gz
  patch:file://{PN}-{PV}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_e2fsprogs = "e2fsprogs"
FILES_e2fsprogs = \
/sbin/e2fsck \
/sbin/fsck \
/sbin/fsck* \
/sbin/mkfs* \
/sbin/mke2fs \
/sbin/tune2fs \
/usr/lib/e2initrd_helper \
/lib/*.so* \
/usr/lib/*.so

$(DEPDIR)/e2fsprogs: bootstrap $(DEPENDS_e2fsprogs) | $(UTIL_LINUX)
	$(PREPARE_e2fsprogs)
	cd $(DIR_e2fsprogs); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		cc=$(target)-gcc \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--with-linker=$(target)-ld \
			--enable-e2initrd-helper \
			--enable-compression \
			--disable-uuidd \
			--disable-rpath \
			--disable-quota \
			--disable-defrag \
			--disable-nls \
			--disable-libuuid \
			--disable-libblkid \
			--enable-elf-shlibs \
			--enable-verbose-makecmds \
			--enable-symlink-install \
			--without-libintl-prefix \
			--without-libiconv-prefix \
			--with-root-prefix=; \
		$(MAKE) all; \
		$(MAKE) -C e2fsck e2fsck.static
		$(start_build)
		( cd $(DIR_e2fsprogs); \
		$(BUILDENV) \
		$(MAKE) install install-libs \
			LDCONFIG=true \
			DESTDIR=$(PKDIR); \
		$(INSTALL) e2fsck/e2fsck.static $(PKDIR)/sbin) || true
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_e2fsprogs)
	touch $@

#
# XFSPROGS
#
BEGIN[[
xfsprogs
  3.1.8
  {PN}-{PV}
  extract:ftp://oss.sgi.com/projects/xfs/cmd_tars/{PN}-{PV}.tar.gz
  patch:file://{PN}.diff
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_xfsprogs = "xfsprogs"

FILES_xfsprogs = \
/bin/*

$(DEPDIR)/xfsprogs: bootstrap $(DEPDIR)/e2fsprogs $(DEPDIR)/libreadline $(DEPENDS_xfsprogs)
	$(PREPARE_xfsprogs)
	$(start_build)
	export PATH=$(hostprefix)/bin:$(PATH); \
	cd $(DIR_xfsprogs); \
		export DEBUG=-DNDEBUG && export OPTIMIZER=-O2; \
		mv -f aclocal.m4 aclocal.m4.orig && mv Makefile Makefile.sgi || true && chmod 644 Makefile.sgi; \
		aclocal -I m4 -I $(hostprefix)/share/aclocal; \
		autoconf; \
		libtoolize; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--prefix= \
			--enable-shared=yes \
			--enable-gettext=yes \
			--enable-readline=yes \
			--enable-editline=no \
			--enable-termcap=yes; \
		cp -p Makefile.sgi Makefile && export top_builddir=`pwd`; \
		$(MAKE) $(MAKE_OPTS); \
		$(INSTALL_xfsprogs)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_xfsprogs)
	touch $@

#
# MC
#
BEGIN[[
mc
  4.8.4
  {PN}-{PV}
  extract:http://www.midnight-commander.org/downloads/{PN}-{PV}.tar.bz2
#nothing:file://{PN}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_mc = "Midnight Commander"

FILES_mc = \
/usr/bin/* \
/usr/etc/mc/* \
/usr/libexec/mc/extfs.d/* \
/usr/libexec/mc/fish/*

$(DEPDIR)/mc: bootstrap glib2 $(DEPENDS_mc) | $(NCURSES_DEV)
	$(PREPARE_mc)
	$(start_build)
	cd $(DIR_mc); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--without-gpm-mouse \
			--with-screen=ncurses \
			--without-x; \
		$(MAKE) all; \
		$(INSTALL_mc)
	$(tocdk_build)
	$(toflash_build)
#		export top_builddir=`pwd`; \
#		$(MAKE) install DESTDIR=$(prefix)/$*cdkroot
	$(DISTCLEANUP_mc)
	touch $@

#
# SDPARM
#
BEGIN[[
sdparm
  1.08
  {PN}-{PV}
  extract:http://sg.danny.cz/sg/p/{PN}-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_sdparm = "sdparm"

FILES_sdparm = \
/sbin/sdparm

$(DEPDIR)/sdparm: bootstrap $(DEPENDS_sdparm)
	$(PREPARE_sdparm)
	$(start_build)
	cd $(DIR_sdparm); \
		export PATH=$(MAKE_PATH); \
		$(MAKE) clean || true; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix= \
			--exec-prefix=/usr \
			--mandir=/usr/share/man; \
		$(MAKE) $(MAKE_OPTS); \
		$(INSTALL_sdparm)
	$(tocdk_build)
	mv -f $(PKDIR)/usr/bin/sdparm $(PKDIR)/sbin
	$(toflash_build)
	$(DISTCLEANUP_sdparm)
	touch $@

#
# SG3_UTILS
#
BEGIN[[
sg3_utils
  1.33
  sg3_utils-{PV}
  extract:http://sg.torque.net/sg/p/sg3_utils-{PV}.tgz
  patch:file://sg3_utils.diff
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/sg3_utils: bootstrap $(DEPENDS_sg3_utils)
	$(PREPARE_sg3_utils)
	export PATH=$(hostprefix)/bin:$(PATH); \
	cd $(DIR_sg3_utils); \
		$(MAKE) clean || true; \
		aclocal -I $(hostprefix)/share/aclocal; \
		autoconf; \
		libtoolize; \
		automake --add-missing --foreign; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=; \
		$(MAKE) $(MAKE_OPTS); \
		$(INSTALL_sg3_utils)
	$(INSTALL) -d $(prefix)/$*cdkroot/etc/default; \
	$(INSTALL) -d $(prefix)/$*cdkroot/etc/init.d; \
	$(INSTALL) -d $(prefix)/$*cdkroot/usr/sbin; \
	( cd root/etc && for i in $(SG3_UTILS_ADAPTED_ETC_FILES); do \
		[ -f $$i ] && $(INSTALL) -m644 $$i $(prefix)/$*cdkroot/etc/$$i || true; \
		[ "$${i%%/*}" = "init.d" ] && chmod 755 $(prefix)/$*cdkroot/etc/$$i || true; done ); \
	$(INSTALL) -m755 root/usr/sbin/sg_down.sh $(prefix)/$*cdkroot/usr/sbin
	$(DISTCLEANUP_sg3_utils)
	touch $@

#
# IPKG
#
BEGIN[[
ipkg
  0.99.163
  {PN}-{PV}
  extract:ftp.gwdg.de/linux/handhelds/packages/{PN}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/ipkg: bootstrap $(DEPENDS_ipkg)
	$(PREPARE_ipkg)
	cd $(DIR_ipkg); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_ipkg)
	ln -sf ipkg-cl $(prefix)/$*cdkroot/usr/bin/ipkg; \
	$(INSTALL) -d $(prefix)/$*cdkroot/etc && $(INSTALL) -m 644 root/etc/ipkg.conf $(prefix)/$*cdkroot/etc; \
	$(INSTALL) -d $(prefix)/$*cdkroot/etc/ipkg
	$(INSTALL) -d $(prefix)/$*cdkroot/usr/lib/ipkg
	$(INSTALL) -m 644 root/usr/lib/ipkg/status.initial $(prefix)/$*cdkroot/usr/lib/ipkg/status
	$(DISTCLEANUP_ipkg)
	touch $@

#
# ZD1211
#
BEGIN[[
zd1211
  2_15_0_0
  ZD1211LnxDrv_2_15_0_0
  extract:http://www.lutec.eu/treiber/{PN}lnxdrv_2_15_0_0.tar.gz
  patch:file://{PN}.diff
;
]]END

CONFIG_ZD1211B :=
$(DEPDIR)/zd1211: bootstrap $(DEPENDS_zd1211)
	$(PREPARE_zd1211)
	cd $(DIR_zd1211); \
		$(MAKE) KERNEL_LOCATION=$(buildprefix)/linux-sh4 \
			ZD1211B=$(ZD1211B) \
			CROSS_COMPILE=$(target)- ARCH=sh \
			BIN_DEST=$(targetprefix)/bin \
			INSTALL_MOD_PATH=$(targetprefix) \
			install; \
	$(DEPMOD) -ae -b $(targetprefix) -r $(KERNELVERSION)
	$(DISTCLEANUP_zd1211)
	touch $@

#
# NANO
#
BEGIN[[
nano
  2.0.9
  {PN}-{PV}
  extract:http://www.{PN}-editor.org/dist/v2.0/{PN}-{PV}.tar.gz
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/nano: bootstrap ncurses ncurses-dev $(DEPENDS_nano)
	$(PREPARE_nano)
	cd $(DIR_nano); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-nls \
			--enable-tiny \
			--enable-color; \
		$(MAKE); \
		$(INSTALL_nano)
	$(DISTCLEANUP_nano)
	touch $@

#
# RSYNC
#
BEGIN[[
rsync
  3.0.9
  {PN}-{PV}
  extract:http://samba.anu.edu.au/ftp/{PN}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/rsync: bootstrap $(DEPENDS_rsync)
	$(PREPARE_rsync)
	cd $(DIR_rsync); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-debug \
			--disable-locale; \
		$(MAKE); \
		$(INSTALL_rsync)
	$(DISTCLEANUP_rsync)
	touch $@

#
# RFKILL
#
BEGIN[[
rfkill
  git
  {PN}-{PV}
  nothing:git://git.sipsolutions.net/rfkill.git
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_rfkill = rfkill is a small tool to query the state of the rfkill switches, buttons and subsystem interfaces
PKGR_rfkill = r1
FILES_rfkill = \
/usr/sbin/*

$(DEPDIR)/rfkill: bootstrap $(DEPENDS_rfkill)
	$(PREPARE_rfkill)
	$(start_build)
	cd $(DIR_rfkill); \
		$(MAKE) $(MAKE_OPTS); \
		$(INSTALL_rfkill)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_rfkill)
	touch $@

#
# LM_SENSORS
#
BEGIN[[
lm_sensors
  3.3.2
  lm_sensors-{PV}
  extract:http://dl.{PN}.org/{PN}/releases/lm_sensors-{PV}.tar.gz
  make:user_install:MACHINE=sh:PREFIX=/usr:MANDIR=/usr/share/man:DESTDIR=PKDIR
;
]]END

DESCRIPTION_lm_sensors = "lm_sensors"

FILES_lm_sensors = \
/usr/bin/sensors \
/etc/sensors.conf \
/usr/lib/*.so* \
/usr/sbin/*

$(DEPDIR)/lm_sensors: bootstrap $(DEPENDS_lm_sensors)
	$(PREPARE_lm_sensors)
	$(start_build)
	cd $(DIR_lm_sensors); \
		$(MAKE) $(MAKE_OPTS) MACHINE=sh PREFIX=/usr user; \
		$(INSTALL_lm_sensors); \
		rm $(PKDIR)/usr/bin/*.pl; \
		rm $(PKDIR)/usr/sbin/*.pl; \
		rm $(PKDIR)/usr/sbin/sensors-detect; \
		rm $(PKDIR)/usr/share/man/man8/sensors-detect.8; \
		rm $(PKDIR)/usr/include/linux/i2c-dev.h; \
		rm $(PKDIR)/usr/bin/ddcmon
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_lm_sensors)
	touch $@

#
# FUSE
#
BEGIN[[
fuse
  2.9.2
  {PN}-{PV}
  extract:http://sourceforge.net/projects/{PN}/files/{PN}-2.X/{PV}/{PN}-{PV}.tar.gz
  patch:file://{PN}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_fuse = With FUSE it is possible to implement a fully functional filesystem in a userspace program.  Features include

FILES_fuse = \
/usr/lib/*.so* \
/etc/init.d/* \
/etc/udev/* \
/usr/bin/*

$(DEPDIR)/fuse: bootstrap curl glib2 $(DEPENDS_fuse)
	$(PREPARE_fuse)
	$(start_build)
	cd $(DIR_fuse); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -I$(buildprefix)/linux/arch/sh" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_fuse)
	rm -R $(PKDIR)/dev
	$(LN_SF) sh4-linux-fusermount $(PKDIR)/usr/bin/fusermount
	$(LN_SF) sh4-linux-ulockmgr_server $(PKDIR)/usr/bin/ulockmgr_server
	( export HHL_CROSS_TARGET_DIR=$(prefix)/release && $(prefix)/release/etc/init.d; \
		for s in fuse ; do \
			$(hostprefix)/bin/target-initdconfig --add $$s || \
			echo "Unable to enable initd service: $$s" ; done && rm *rpmsave 2>/dev/null || true )
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_fuse)
	touch $@

#
# CURLFTPFS
#
BEGIN[[
curlftpfs
  0.9.2
  {PN}-{PV}
  extract:http://sourceforge.net/projects/{PN}/files/latest/download/{PN}-{PV}.tar.gz
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/curlftpfs: bootstrap fuse $(DEPENDS_curlftpfs)
	$(PREPARE_curlftpfs)
	cd $(DIR_curlftpfs); \
		export ac_cv_func_malloc_0_nonnull=yes; \
		export ac_cv_func_realloc_0_nonnull=yes; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_curlftpfs)
	$(DISTCLEANUP_curlftpfs)
	touch $@

#
# FBSET
#
BEGIN[[
fbset
  2.1
  {PN}-{PV}
  extract:http://ftp.de.debian.org/debian/pool/main/f/{PN}/{PN}_{PV}.orig.tar.gz
  patch:http://archive.debian.org/debian/dists/potato/main/source/admin/{PN}_{PV}-6.diff.gz
  patch:file://{PN}_{PV}-fb.modes-ST.patch
  install:-d:-m755:TARGETS/{usr/sbin,etc}
  install:-m755:{PN}:TARGETS/usr/sbin
  install:-m755:con2fbmap:TARGETS/usr/sbin
  install:-m644:etc/fb.modes.ATI:TARGETS/etc/fb.modes
;
]]END

$(DEPDIR)/fbset: bootstrap $(DEPENDS_fbset)
	$(PREPARE_fbset)
	cd $(DIR_fbset); \
		make CC="$(target)-gcc -Wall -O2 -I."; \
		$(INSTALL_fbset)
	$(DISTCLEANUP_fbset)
	touch $@

#
# PNGQUANT
#
BEGIN[[
pngquant
  1.1
  {PN}-{PV}
  extract:ftp://ftp.simplesystems.org/pub/libpng/png/applications/{PN}/{PN}-{PV}-src.tgz
  install:-m755:{PN}:TARGETS/usr/bin
;
]]END

$(DEPDIR)/pngquant: bootstrap libz libpng $(DEPENDS_pngquant)
	$(PREPARE_pngquant)
	cd $(DIR_pngquant); \
		$(target)-gcc -O3 -Wall -I. -funroll-loops -fomit-frame-pointer -o pngquant pngquant.c rwpng.c -lpng -lz -lm; \
		$(INSTALL_pngquant)
	$(DISTCLEANUP_pngquant)
	touch $@

#
# MPLAYER
#
BEGIN[[
mplayer
  1.0
  {PN}-export-*
  extract:ftp://ftp.{PN}hq.hu/MPlayer/releases/{PN}-export-snapshot.tar.bz2
  make:install INSTALLSTRIP="":DESTDIR=TARGETS
;
]]END

$(DEPDIR)/mplayer: bootstrap $(DEPENDS_mplayer)
	$(PREPARE_mplayer)
	cd $(DIR_mplayer); \
		$(BUILDENV) \
		./configure \
			--cc=$(target)-gcc \
			--target=$(target) \
			--host-cc=gcc \
			--prefix=/usr \
			--disable-mencoder; \
		$(MAKE) CC="$(target)-gcc"; \
		$(INSTALL_mplayer)
	$(DISTCLEANUP_mplayer)
	touch $@

#
# MENCODER
#
BEGIN[[
mencoder
  1.0
  mplayer-export-*
  extract:ftp://ftp.mplayerhq.hu/MPlayer/releases/mplayer-export-snapshot.tar.bz2
  make:install INSTALLSTRIP="":DESTDIR=TARGETS
;
]]END

$(DEPDIR)/mencoder: bootstrap $(DEPENDS_mencoder)
	$(PREPARE_mencoder)
	cd $(DIR_mencoder); \
		$(BUILDENV) \
		./configure \
			--cc=$(target)-gcc \
			--target=$(target) \
			--host-cc=gcc \
			--prefix=/usr \
			--disable-dvdnav \
			--disable-dvdread \
			--disable-dvdread-internal \
			--disable-libdvdcss-internal \
			--disable-libvorbis \
			--disable-mp3lib \
			--disable-liba52 \
			--disable-mad \
			--disable-vcd \
			--disable-ftp \
			--disable-pvr \
			--disable-tv-v4l2 \
			--disable-tv-v4l1 \
			--disable-tv \
			--disable-network \
			--disable-real \
			--disable-xanim \
			--disable-faad-internal \
			--disable-tremor-internal \
			--disable-pnm \
			--disable-ossaudio \
			--disable-tga \
			--disable-v4l2 \
			--disable-fbdev \
			--disable-dvb \
			--disable-mplayer; \
		$(MAKE) CC="$(target)-gcc"; \
		$(INSTALL_mencoder)
	$(DISTCLEANUP_mencoder)
	touch $@

#
# jfsutils
#
BEGIN[[
jfsutils
  1.1.15
  {PN}-{PV}
  extract:http://jfs.sourceforge.net/project/pub/{PN}-{PV}.tar.gz
  make:install:mandir=/usr/share/man:DESTDIR=PKDIR
;
]]END

DESCRIPTION_jfsutils = "jfsutils"
FILES_jfsutils = \
/sbin/*

$(DEPDIR)/jfsutils: bootstrap e2fsprogs $(DEPENDS_jfsutils)
	$(PREPARE_jfsutils)
	$(start_build)
	cd $(DIR_jfsutils); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--disable-dependency-tracking \
			--prefix=; \
		$(MAKE); \
		$(INSTALL_jfsutils)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_jfsutils)
	touch $@

#
# opkg
#
BEGIN[[
opkg
  0.1.8
  {PN}-{PV}
  extract:http://{PN}.googlecode.com/files/{PN}-{PV}.tar.gz
  patch:file://opkg-0.1.8-dont-segfault.diff
  make:install:DESTDIR=PKDIR
  link:/usr/bin/{PN}-cl:PKDIR/usr/bin/{PN}
;
]]END


DESCRIPTION_opkg = "lightweight package management system"
FILES_opkg = \
/usr/bin \
/usr/lib

$(DEPDIR)/opkg: bootstrap $(DEPENDS_opkg)
	$(PREPARE_opkg)
	$(start_build)
	cd $(DIR_opkg); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-curl \
			--disable-gpg \
			--with-opkglibdir=/usr/lib; \
		$(MAKE) all; \
		$(INSTALL_opkg)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_opkg)
	touch $@

#
# ntpclient
#
BEGIN[[
ntpclient
  #second param is version
  2007_365
  #third is buid dir
  {PN}-2007
  #sources goes below
  http://doolittle.icarus.com/ntpclient/{PN}_{PV}.tar.gz
  nothing:file://{PN}-init.file
;
]]END


# PARENT_PK defined as per rule variable below is main postfix
# at first split_packages.py searches for variable PACKAGES_ + $(PARENT_PK)
# PACKAGES_ntpclient = ntpclient
# this is the default.
# PACKAGES_ntpclient = $(PARENT_PK)
# secondly for each package in the list it looks for control fields.
# the default control field is PARENT_PK one.

DESCRIPTION_ntpclient := time sync over ntp protocol
#this is default
#MAINTAINER_ntpclient := Ar-P team
#Source: are handled by smart-rules
#SRC_URI_ntpclient =
#PACKAGE_ARCH_ntpclient := sh4
#the Package: field in control file
#NAME_ntpclient := ntpclient
#mask for files to package
FILES_ntpclient := /sbin /etc
#version is handled by smart-rules
#PKGV_ntpclient =
PKGR_ntpclient = r1
# comment symbol '#' in define goes directly to split_packages.py. You do not need to escape it!
# moreover line breaks are also correctly exported to python, enjoy!
define postinst_ntpclient
#!/bin/sh
initdconfig --add ntpclient
endef
define postrm_ntpclient
#!/bin/sh
initdconfig --del ntpclient
endef

$(DEPDIR)/ntpclient: $(DEPENDS_ntpclient)
	$(PREPARE_ntpclient)
	$(start_build)
	cd $(DIR_ntpclient) ; \
		export CC=sh4-linux-gcc CFLAGS="$(TARGET_CFLAGS)"; \
		$(MAKE) ntpclient; \
		$(MAKE) adjtimex; \
		install -D -m 0755 ntpclient $(PKDIR)/sbin/ntpclient; \
		install -D -m 0755 adjtimex $(PKDIR)/sbin/adjtimex; \
		install -D -m 0755 rate.awk $(PKDIR)/sbin/ntpclient-drift-rate.awk
	install -D -m 0755 Patches/ntpclient-init.file $(PKDIR)/etc/init.d/ntpclient
	$(extra_build)
	touch $@

#
# udpxy
#
BEGIN[[
udpxy
  1.0.23-0
  {PN}-{PV}
  http://sourceforge.net/projects/udpxy/files/udpxy/Chipmunk-1.0/udpxy.{PV}-prod.tar.gz
  #for patch -p0 use the following
  patch-0:file://udpxy-makefile.patch
;
]]END

# You can use it as example of building and making package for new utility.
# First of all take a look at smart-rules file. Read the documentation at the beginning.
#
# At the first stage let's build one single package. For example udpxy. Be careful, each package name should be unique.
# First of all you should define some necessary info about your package.
# Such as 'Description:' field in control file

DESCRIPTION_udpxy := udp to http stream proxy

# Next set package release number and increase it each time you change something here in make scripts.
# Release number is part of the package version, updating it tells others that they can upgrade their system now.

PKGR_udpxy = r0

# Other variables are optional and have default values and another are taken from smart-rules (full list below)
# Usually each utility is split into three make-targets. Target name and package name 'udpxy' should be the same.
# Write
#  $(DEPDIR)/udpxy.do_prepare:
# But not
#  $(DEPDIR)/udpxy_proxy.do_prepare:
# *exceptions of this rule discussed later.

# Also target should contain only A-z characters and underscore "_".

# Firstly, downloading and patching. Use $(DEPENDS_udpxy) from smart rules as target-depends.
# In the body use $(PREPARE_udpxy) generated by smart-rules
# You can add your special commands too.

$(DEPDIR)/udpxy.do_prepare: $(DEPENDS_udpxy)
	$(PREPARE_udpxy)
	touch $@

# Secondly, the configure and compilation stage
# Each target should ends with 'touch $@'

$(DEPDIR)/udpxy.do_compile: $(DEPDIR)/udpxy.do_prepare
	cd $(DIR_udpxy); \
		export CC=sh4-linux-gcc; \
		$(MAKE)
	touch $@

# Finally, install and packaging!
# How does it works:
#  start with line $(start_build) to prepare temporary directories and determine package name by the target name.
#  At first all files should go to temporary directory $(PKDIR) which is cdk/packagingtmpdir.
#  If you fill $(PKDIR) correctly then our scripts could proceed.
#  You could call one of the following:
#    $(tocdk_build) - copy all $(PKDIR) contents to tufsbox/cdkroot to use them later if something depends on them.
#    $(extra_build) - perform strip and cleanup, then make package ready to install on your box. You can find ipk in tufsbox/ipkbox
#    $(toflash_build) - At first do exactly that $(extra_build) does. After install package to pkgroot to include it in image.
#    $(e2extra_build) - same as $(extra_build) but copies ipk to tufsbox/ipkextras
#  Tip: $(tocdk_build) and $(toflash_build) could be used simultaneously.

$(DEPDIR)/udpxy: $(DEPDIR)/udpxy.do_compile
	$(start_build)
	cd $(DIR_udpxy) ; \
		export INSTALLROOT=$(PKDIR)/usr; \
		$(MAKE) install
	$(extra_build)
	touch $@

# Note: all above defined variables has suffix 'udpxy' same as make-target name '$(DEPDIR)/udpxy'
# If you want to change name of make-target for some reason add $(call parent_pk,udpxy) before $(start_build) line.
# Of course place your variables suffix instead of udpxy.

# Some words about git and svn.
# It is available to automatically determine version from git and svn
# If there is git/svn rule in smart-rules and the version equals git/svn then the version will be automatically evaluated during $(start_build)
# Note: it is assumed that there is only one repo for the utility.
# If you use your own git/svn fetch mechanism we provide you with $(get_git_version) or $(get_svn_version), but make sure that DIR_foo is git/svn repo.

# FILES variable
# FILES variable is the filter for your $(PKDIR), by default it equals "/" so all files from $(PKDIR) are built into the package. It is list of files and directories separated by space. Wildcards are supported.
# Wildcards used in the FILES variables are processed via the python function fnmatch. The following items are of note about this function:
#   /<dir>/*: This will match all files and directories in the dir - it will not match other directories.
#   /<dir>/a*: This will only match files, and not directories.
#   /dir: will include the directory dir in the package, which in turn will include all files in the directory and all subdirectories.

# Info about some additional variables
# PKGV_foo
#  Taken from smart rules version. Set if you don't use smart-rules
# SRC_URI_foo
#  Sources from which package is built, taken from smart-rules file://, http://, git://, svn:// rules.
# NAME_foo
#  If real package name is too long put it in this variable. By default it is like in varible names.
# Next variables has default values and influence CONTROL file fields only:
# MAINTAINER_foo := Ar-P team
# PACKAGE_ARCH_foo := sh4
# SECTION_foo := base
# PRIORITY_foo := optional
# LICENSE_foo := unknown
# HOMEPAGE_foo := unknown
# You set package dependencies in CONTROL file with:
# RDEPENDS_foo :=
# RREPLACES :=
# RCONFLICTS :=

# post/pre inst/rm Scripts
# For these sripts use make define as following:

define postinst_foo
#!/bin/sh
initdconfig --add foo
endef

# This is all about scripts
# Note: init.d script starting and stopping is handled by initdconfig

# Multi-Packaging
# When you whant to split files from one target to different packages you should set PACKAGES_parentfoo value.
# By default parentfoo is equals make target name. Place subpackages names to PACKAGES_parentfoo variable,
# parentfoo could be also in the list. Example:
## PACKAGES_megaprog = megaprog_extra megaprog
# Then set FILES for each subpackage
## FILES_megaprog = /bin/prog /lib/*.so*
## FILES_megaprog_extra = /lib/megaprog-addon.so
# NOTE: files are moving to pacakges in same order they are listed in PACKAGES variable.

# Optional install to flash
# When you call $(tocdk_build)/$(toflash_build) all packages are installed to image.
# If you want to select some non-installing packages from the same target (multi-packaging case)
# just list them in EXTRA_parentfoo variable
# DIST_parentfoo variable works vice-versa

#
# sysstat
#
BEGIN[[
sysstat
  10.1.1
  {PN}-{PV}
  extract:http://pagesperso-orange.fr/sebastien.godard/{PN}-{PV}.tar.gz
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/sysstat: bootstrap $(DEPENDS_sysstat)
	$(PREPARE_sysstat)
	export PATH=$(hostprefix)/bin:$(PATH); \
	cd $(DIR_sysstat); \
	$(BUILDENV) \
	./configure \
		--build=$(build) \
		--host=$(target) \
		--prefix=/usr \
		--disable-documentation; \
		$(MAKE); \
		$(INSTALL_sysstat)
	$(DISTCLEANUP_sysstat)
	touch $@

#
# hotplug-e2
#
BEGIN[[
hotplug_e2
  git
  {PN}-helper
  git://openpli.git.sourceforge.net/gitroot/openpli/hotplug-e2-helper
  patch:file://hotplug-e2-helper-support_fw_upload.patch
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_hotplug_e2 = "hotplug_e2"
PKGR_hotplug_e2 = r1
FILES_hotplug_e2 = \
/sbin/bdpoll \
/usr/bin/hotplug_e2_helper

$(DEPDIR)/hotplug_e2: bootstrap $(DEPENDS_hotplug_e2)
	$(PREPARE_hotplug_e2)
	$(start_build)
	cd $(DIR_hotplug_e2); \
		./autogen.sh &&\
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_hotplug_e2)
	$(tocdk_build)
	mkdir $(PKDIR)/sbin
	cp -f $(PKDIR)/usr/bin/* $(PKDIR)/sbin
	$(toflash_build)
	$(DISTCLEANUP_hotplug_e2)
	touch $@

#
# autofs
#
BEGIN[[
autofs
  4.1.4
  {PN}-{PV}
  extract:http://kernel.org/pub/linux/daemons/{PN}/v4/{PN}-{PV}.tar.gz
  patch:file://{PN}-{PV}-misc-fixes.patch
  patch:file://{PN}-{PV}-multi-parse-fix.patch
  patch:file://{PN}-{PV}-non-replicated-ping.patch
  patch:file://{PN}-{PV}-locking-fix-1.patch
  patch:file://{PN}-{PV}-cross.patch
  patch:file://{PN}-{PV}-Makefile.rules-cross.patch
  patch:file://{PN}-{PV}-install.patch
  patch:file://{PN}-{PV}-auto.net-sort-option-fix.patch
  patch:file://{PN}-{PV}-{PN}-additional-distros.patch
  patch:file://{PN}-{PV}-no-bash.patch
  patch:file://{PN}-{PV}-{PN}-add-hotplug.patch
  patch:file://{PN}-{PV}-no_man.patch
  make:install:INSTALLROOT=PKDIR
;
]]END

DESCRIPTION_autofs = "autofs"
FILES_autofs = \
/usr/*

$(DEPDIR)/autofs: bootstrap $(DEPENDS_autofs)
	$(PREPARE_autofs)
	$(start_build)
	cd $(DIR_autofs); \
		cp aclocal.m4 acinclude.m4; \
		autoconf; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all CC=$(target)-gcc STRIP=$(target)-strip; \
		$(INSTALL_autofs)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_autofs)
	touch $@

#
# imagemagick
#
BEGIN[[
imagemagick
  6.8.6.8-7
  ImageMagick-{PV}
  extract:ftp://ftp.fifi.org/pub/ImageMagick/ImageMagick-{PV}.tar.xz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_imagemagick = "imagemagick"
FILES_imagemagick = \
/usr/*
$(DEPDIR)/imagemagick: bootstrap $(DEPENDS_imagemagick)
	$(PREPARE_imagemagick)
	$(start_build)
	cd $(DIR_imagemagick); \
		$(BUILDENV) \
		CFLAGS="-O1" \
		PKG_CONFIG=$(hostprefix)/bin/pkg-config \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--without-dps \
			--without-fpx \
			--without-gslib \
			--without-jbig \
			--without-jp2 \
			--without-lcms \
			--without-tiff \
			--without-xml \
			--without-perl \
			--disable-openmp \
			--disable-opencl \
			--without-zlib \
			--enable-shared \
			--enable-static \
			--without-x; \
		$(MAKE) all; \
		$(INSTALL_imagemagick)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_imagemagick)
	touch $@

#
# grab
#
BEGIN[[
grab
  git
  {PN}-{PV}
  git://git.code.sf.net/p/openpli/aio-grab
  patch:file://aio-grab-ADD_ST_SUPPORT.patch
  patch:file://aio-grab-ADD_ST_FRAMESYNC_SUPPORT.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_grab = make enigma2 screenshots
PKGR_grab = r1
RDEPENDS_grab = libpng libjpeg

$(DEPDIR)/grab: bootstrap $(RDEPENDS_grab) $(DEPENDS_grab)
	$(PREPARE_grab)
	$(start_build)
	cd $(DIR_grab); \
		autoreconf -i; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_grab)
	$(toflash_build)
	touch $@


#
# enigma2-plugin-cams-oscam
#
BEGIN[[
enigma2_plugin_cams_oscam
  svn
  {PN}-{PV}
  svn://www.streamboard.tv/svn/oscam/trunk/
  make:install:DESTDIR=PKDIR:OSCAM_BIN = OSCAM_BIN
;
]]END

DESCRIPTION_enigma2_plugin_cams_oscam = Open Source Conditional Access Module software
SRC_URI_enigma2_plugin_cams_oscam = http://www.streamboard.tv/oscam/
FILES_enigma2_plugin_cams_oscam = \
/usr/bin/cam/oscam

$(DEPDIR)/enigma2_plugin_cams_oscam: bootstrap $(DEPENDS_enigma2_plugin_cams_oscam)
	$(PREPARE_enigma2_plugin_cams_oscam)
	$(start_build)
	cd $(DIR_enigma2_plugin_cams_oscam); \
	$(BUILDENV) \
	$(MAKE) CROSS=$(prefix)/devkit/sh4/bin/$(target)-  CONF_DIR=/var/keys; \
		$(INSTALL_DIR) $(PKDIR)/usr/bin/cam; \
		$(INSTALL_BIN) Distribution/oscam*-sh4-linux $(PKDIR)/usr/bin/cam/oscam
	$(tocdk_build)
	$(toflash_build)
	touch $@

#
# enigma2-plugin-cams-oscam-config
#
BEGIN[[
enigma2_plugin_cams_oscam_config
  0.1
  {PN}-{PV}
  nothing:file://../root/var/keys/oscam.conf
  nothing:file://../root/var/keys/oscam.dvbapi
  nothing:file://../root/var/keys/oscam.services
  nothing:file://../root/var/keys/oscam.srvid
  nothing:file://../root/var/keys/oscam.user
  nothing:file://../root/var/keys/oscam.server2
  nothing:file://../root/var/keys/oscam.server
  nothing:file://../root/var/keys/oscam.guess
;
]]END

DESCRIPTION_enigma2_plugin_cams_oscam_config = Example configs for Open Source Conditional Access Module software
SRC_URI_enigma2_plugin_cams_oscam_config = http://www.streamboard.tv/oscam/
FILES_enigma2_plugin_cams_oscam_config = \
/var/keys/oscam.*

$(DEPDIR)/enigma2-plugin-cams-oscam-config: $(DEPENDS_enigma2_plugin_cams_oscam_config)
	$(PREPARE_enigma2_plugin_cams_oscam_config)
	$(start_build)
		$(INSTALL_DIR) $(PKDIR)/var/keys
		$(INSTALL_FILE) $(buildprefix)/root/var/keys/oscam.conf     $(PKDIR)/var/keys/oscam.conf
		$(INSTALL_FILE) $(buildprefix)/root/var/keys/oscam.dvbapi   $(PKDIR)/var/keys/oscam.dvbapi
		$(INSTALL_FILE) $(buildprefix)/root/var/keys/oscam.services $(PKDIR)/var/keys/oscam.services
		$(INSTALL_FILE) $(buildprefix)/root/var/keys/oscam.srvid    $(PKDIR)/var/keys/oscam.srvid
		$(INSTALL_FILE) $(buildprefix)/root/var/keys/oscam.user     $(PKDIR)/var/keys/oscam.user
		$(INSTALL_FILE) $(buildprefix)/root/var/keys/oscam.server   $(PKDIR)/var/keys/oscam.server
		$(INSTALL_FILE) $(buildprefix)/root/var/keys/oscam.guess    $(PKDIR)/var/keys/oscam.guess
	$(e2extra_build)
	touch $@


#
# parted
#
BEGIN[[
parted
  3.1
  {PN}-{PV}
  extract:http://ftp.gnu.org/gnu/{PN}/{PN}-{PV}.tar.xz
  patch:file://{PN}_{PV}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_parted = "parted"
FILES_parted = \
/usr/lib/libparted-fs-resize.s* \
/usr/lib/libparted.s* \
/usr/sbin/parted

$(DEPDIR)/parted: bootstrap $(DEPENDS_parted)
	$(PREPARE_parted)
	$(start_build)
	cd $(DIR_parted); \
		cp aclocal.m4 acinclude.m4; \
		autoconf; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-Werror \
			--disable-device-mapper; \
		$(MAKE) all CC=$(target)-gcc STRIP=$(target)-strip; \
		$(INSTALL_parted)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_parted)
	touch $@

#
# gettext
#
BEGIN[[
gettext
  0.18
  {PN}-{PV}
  extract:ftp://ftp.gnu.org/gnu/{PN}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gettext = "gettext"
FILES_gettext = \
*

$(DEPDIR)/gettext: bootstrap $(DEPENDS_gettext)
	$(PREPARE_gettext)
	$(start_build)
	cd $(DIR_gettext); \
		cp aclocal.m4 acinclude.m4; \
		autoconf; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--without-emacs \
			--without-cvs \
			--disable-java; \
		$(MAKE) all; \
		$(INSTALL_gettext)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gettext)
	touch $@
