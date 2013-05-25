#
# NFS-UTILS
#
BEGIN[[
nfs_utils
  1.2.3
  {PN}-{PV}
  extract:http://downloads.sourceforge.net/project/nfs/nfs-utils/1.2.3/nfs-utils-1.2.3.tar.bz2
  patch:file://nfs-utils-1.2.3.patch
  make:install:DESTDIR=PKDIR
  remove:PKDIR/sbin/mount.nfs4:PKDIR/sbin/umount.nfs4
;
]]END

DESCRIPTION_nfs_utils = "nfs_utils"
FILES_nfs_utils = \
/usr/sbin/* \
sbin/*

$(DEPDIR)/nfs_utils: bootstrap e2fsprogs $(DEPENDS_nfs_utils)
	$(PREPARE_nfs_utils)
	$(start_build)
	cd $(DIR_nfs_utils) && \
		$(BUILDENV) \
		./configure \
			CC_FOR_BUILD=$(target)-gcc \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-gss \
			--enable-ipv6=no \
			--disable-tirpc \
			--disable-nfsv4 \
			--without-tcp-wrappers && \
		$(MAKE) && \
		$(INSTALL_nfs_utils)
	( cd $(buildprefix)/root/etc && for i in $(NFS_UTILS_ADAPTED_ETC_FILES); do \
		[ -f $$i ] && $(INSTALL) -m644 $$i $(PKDIR)/etc/$$i || true; \
		[ "$${i%%/*}" = "init.d" ] && chmod 755 $(PKDIR)/etc/$$i || true; done )
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_nfs_utils)
	touch $@

#
# vsftpd
#
BEGIN[[
vsftpd
  3.0.2
  {PN}-{PV}
  extract:http://fossies.org/unix/misc/{PN}-{PV}.tar.gz
  patch:file://{PN}_{PV}.diff
  nothing:file://../root/release/vsftpd
  nothing:file://../root/etc/vsftpd.conf
  pmove:{PN}-{PV}/vsftpd:{PN}-{PV}/vsftpd.initscript
  make:install:PREFIX=PKDIR
  install:-m644:vsftpd.conf:PKDIR/etc
  install:-m755 -D:vsftpd.initscript:PKDIR/etc/init.d/vsftpd
;
]]END

DESCRIPTION_vsftpd = "vsftpd"
PKGR_vsftpd = r0
FILES_vsftpd = \
/etc/* \
/usr/bin/*

define postinst_vsftpd
#!/bin/sh
initdconfig --add vsftpd
endef

define prerm_vsftpd
#!/bin/sh
initdconfig --del vsftpd
endef

$(DEPDIR)/vsftpd: $(DEPENDS_vsftpd)
	$(PREPARE_vsftpd)
	$(start_build)
	mkdir -p $(PKDIR)/etc/
	mkdir -p $(PKDIR)/usr/bin/
	mkdir -p $(PKDIR)/usr/share/man/man8/
	mkdir -p $(PKDIR)/usr/share/man/man5/
	cd $(DIR_vsftpd) && \
		$(MAKE) clean && \
		$(MAKE) $(MAKE_OPTS) CFLAGS="-pipe -Os -g0" && \
		$(INSTALL_vsftpd)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_vsftpd)
	touch $@

#
# ETHTOOL
#
BEGIN[[
ethtool
  6
  {PN}-{PV}
  extract:http://downloads.openwrt.org/sources/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_ethtool = "ethtool"
FILES_ethtool = \
/usr/sbin/*

$(DEPDIR)/ethtool: $(DEPENDS_ethtool)
	$(PREPARE_ethtool)
	$(start_build)
	cd $(DIR_ethtool)  && \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--libdir=$(targetprefix)/usr/lib \
			--prefix=/usr && \
		$(MAKE) && \
		$(INSTALL_ethtool)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_ethtool)
	touch $@

#
# SAMBA
#
BEGIN[[
samba
  3.6.12
  {PN}-{PV}
  extract:http://www.{PN}.org/{PN}/ftp/stable/{PN}-{PV}.tar.gz
  patch:file://{PN}-{PV}.diff
  make:install bin/smbd bin/nmbd:DESTDIR=PKDIR:prefix=./.
;
]]END

DESCRIPTION_samba = "samba"
FILES_samba = \
/usr/sbin/* \
/usr/lib/*.so \
/etc/init.d/* \
/etc/samba/smb.conf \
/usr/lib/vfs/*.so

$(DEPDIR)/samba.do_prepare: bootstrap $(DEPENDS_samba)
	$(PREPARE_samba)
	touch $@

$(DEPDIR)/samba.do_compile: $(DEPDIR)/samba.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_samba) && \
		cd source3 && \
		./autogen.sh && \
		$(BUILDENV) \
		libreplace_cv_HAVE_GETADDRINFO=no \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix= \
			--exec-prefix=/usr \
			--disable-pie \
			--disable-avahi \
			--disable-cups \
			--disable-relro \
			--disable-swat \
			--disable-shared-libs \
			--disable-socket-wrapper \
			--disable-nss-wrapper \
			--disable-smbtorture4 \
			--disable-fam \
			--disable-iprint \
			--disable-dnssd \
			--disable-pthreadpool \
			--disable-dmalloc \
			--with-included-iniparser \
			--with-included-popt \
			--with-sendfile-support \
			--without-aio-support \
			--without-cluster-support \
			--without-ads \
			--without-krb5 \
			--without-dnsupdate \
			--without-automount \
			--without-ldap \
			--without-pam \
			--without-pam_smbpass \
			--without-winbind \
			--without-wbclient \
			--without-syslog \
			--without-nisplus-home \
			--without-quotas \
			--without-sys-quotas \
			--without-utmp \
			--without-acl-support \
			--with-configdir=/etc/samba \
			--with-privatedir=/etc/samba \
			--with-mandir=/usr/share/man \
			--with-piddir=/var/run \
			--with-logfilebase=/var/log \
			--with-lockdir=/var/lock \
			--with-swatdir=/usr/share/swat \
			--disable-cups && \
		$(MAKE) $(MAKE_OPTS) && \
		$(target)-strip -s bin/smbd && $(target)-strip -s bin/nmbd
	touch $@

$(DEPDIR)/samba: \
$(DEPDIR)/%samba: $(DEPDIR)/samba.do_compile
	$(start_build)
	cd $(DIR_samba) && \
		cd source3 && \
		$(INSTALL) -d $(PKDIR)/etc/samba && \
		$(INSTALL) -c -m644 ../examples/smb.conf.spark $(PKDIR)/etc/samba/smb.conf && \
		$(INSTALL) -d $(PKDIR)/etc/init.d && \
		$(INSTALL) -c -m755 ../examples/samba.spark $(PKDIR)/etc/init.d/samba && \
		$(INSTALL_samba)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_samba)
	touch $@ || true

#
# NETIO
#
BEGIN[[
netio
  1.26
  {PN}126
  extract:http://bnsmb.de/files/public/windows/{PN}126.zip
  install:-m755:{PN}:PKDIR/usr/bin
  install:-m755:bin/linux-i386:HOST/bin/{PN}
;
]]END

DESCRIPTION_netio = "netio"
FILES_netio = \
/usr/bin/*

$(DEPDIR)/netio.do_prepare: $(DEPENDS_netio)
	$(PREPARE_netio)
	touch $@

$(DEPDIR)/netio.do_compile: bootstrap $(DEPDIR)/netio.do_prepare
	cd $(DIR_netio) && \
		$(MAKE_OPTS) \
		$(MAKE) all O=.o X= CFLAGS="-DUNIX" LIBS="$(LDFLAGS) -lpthread" OUT=-o
	touch $@

$(DEPDIR)/netio: \
$(DEPDIR)/%netio: $(DEPDIR)/netio.do_compile
	$(start_build)
	cd $(DIR_netio) && \
		$(INSTALL) -d $(PKDIR)/usr/bin && \
		$(INSTALL_netio)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_netio)
	touch $@ || true

#
# LIGHTTPD
#
BEGIN[[
lighttpd
  1.4.15
  {PN}-{PV}
  extract:http://www.{PN}.net/download/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_lighttpd = "lighttpd"
FILES_lighttpd = \
/usr/bin/* \
/usr/sbin/* \
/usr/lib/*.so \
/etc/init.d/* \
/etc/lighttpd/*.conf 

$(DEPDIR)/lighttpd: bootstrap $(DEPENDS_lighttpd)
	$(PREPARE_lighttpd)
	$(start_build)
	cd $(DIR_lighttpd) && \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix= \
			--exec-prefix=/usr \
			--datarootdir=/usr/share && \
		$(MAKE) && \
		$(INSTALL_lighttpd)
	cd $(DIR_lighttpd) && \
		$(INSTALL) -d $(PKDIR)/etc/lighttpd && \
		$(INSTALL) -c -m644 doc/lighttpd.conf $(PKDIR)/etc/lighttpd && \
		$(INSTALL) -d $(PKDIR)/etc/init.d && \
		$(INSTALL) -c -m644 doc/rc.lighttpd.redhat $(PKDIR)/etc/init.d/lighttpd
	$(INSTALL) -d $(PKDIR)/etc/lighttpd && $(INSTALL) -m755 root/etc/lighttpd/lighttpd.conf $(PKDIR)/etc/lighttpd
	$(INSTALL) -d $(PKDIR)/etc/init.d && $(INSTALL) -m755 root/etc/init.d/lighttpd $(PKDIR)/etc/init.d
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_lighttpd)
	touch $@

#
# NETKIT_FTP
#
BEGIN[[
netkit_ftp
  0.17
  {PN}-{PV}
  extract:http://ibiblio.org/pub/linux/system/network/netkit//{PN}-{PV}.tar.gz
#patch:file://{PN}.diff
  make:install:MANDIR=/usr/share/man:INSTALLROOT=TARGETS
;
]]END

DESCRIPTION_netkit_ftp = "netkit_ftp"
FILES_netkit_ftp = \
/usr/bin/*

$(DEPDIR)/netkit_ftp: bootstrap ncurses libreadline $(DEPENDS_netkit_ftp)
	$(PREPARE_netkit_ftp)
	$(start_build)
	cd $(DIR_netkit_ftp)  && \
		$(BUILDENV) \
		./configure \
			--with-c-compiler=$(target)-gcc \
			--prefix=/usr \
			--installroot=$(PKDIR) && \
		$(MAKE) && \
		$(INSTALL_netkit_ftp)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_netkit_ftp)
	touch $@

#
# WIRELESS_TOOLS
#
BEGIN[[
wireless_tools
  29
  wireless_tools.{PV}
  extract:http://www.hpl.hp.com/personal/Jean_Tourrilhes/Linux/wireless_tools.{PV}.tar.gz
  make:install:INSTALL_MAN=PKDIR/usr/share/man:PREFIX=PKDIR/usr
;
]]END

DESCRIPTION_wireless_tools = wireless-tools
RDEPENDS_wireless_tools = rfkill wpa-supplicant
FILES_wireless_tools = \
/usr/sbin/* \
/usr/lib/*.so*

$(DEPDIR)/wireless_tools: bootstrap wpa_supplicant rfkill $(DEPENDS_wireless_tools)
	$(PREPARE_wireless_tools)
	$(start_build)
	cd $(DIR_wireless_tools)  && \
		$(MAKE) $(MAKE_OPTS) && \
		$(INSTALL_wireless_tools)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_wireless_tools)
	touch $@ || true

#
# WPA_SUPPLICANT
#
BEGIN[[
wpa_supplicant
  1.0
  wpa_supplicant-{PV}
  extract:http://hostap.epitest.fi/releases/wpa_supplicant-{PV}.tar.gz
  nothing:file://wpa_supplicant.config
  make:install:DESTDIR=PKDIR:LIBDIR=/usr/lib:BINDIR=/usr/sbin
;
]]END

DESCRIPTION_wpa_supplicant = "wpa-supplicant"
PKGR_wpa_supplicant = r0
FILES_wpa_supplicant = \
/usr/sbin/*

$(DEPDIR)/wpa_supplicant: bootstrap $(DEPENDS_wpa_supplicant)
	$(PREPARE_wpa_supplicant)
	$(start_build)
	cd $(DIR_wpa_supplicant)/wpa_supplicant  && \
		mv ../wpa_supplicant.config .config && \
		$(MAKE) $(MAKE_OPTS) && \
		$(INSTALL_wpa_supplicant)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_wpa_supplicant)
	touch $@


#
# TRANSMISSION
#

BEGIN[[
transmission
  2.77
  {PN}-{PV}
  extract:http://mirrors.m0k.org/transmission/files/{PN}-{PV}.tar.bz2
  nothing:file://../root/etc/init.d/transmission.init
  nothing:file://../root/etc/transmission.json
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_transmission = "A free, lightweight BitTorrent client"
PKGR_transmission = r3
RDEPENDS_transmission = curl openssl libevent
FILES_transmission = \
/usr/bin/* \
/usr/share/transmission/*

define postinst_transmission
#!/bin/sh

initdconfig --add transmission
endef
define postrm_transmission
#!/bin/sh

initdconfig --del transmission
endef

$(DEPDIR)/transmission: bootstrap libevent-dev curl $(DEPENDS_transmission)
	$(PREPARE_transmission)
	$(start_build)
	cd $(DIR_transmission) && \
		$(BUILDENV) \
		./configure \
			--prefix=/usr \
			--disable-nls \
			--disable-mac \
			--disable-libappindicator \
			--disable-libcanberra \
			--with-gnu-ld \
			--enable-daemon \
			--enable-cli \
			--disable-gtk \
			--enable-largefile \
			--enable-lightweight \
			--build=$(build) \
			--host=$(target) && \
		$(MAKE) && \
		$(INSTALL_transmission) && \
		$(INSTALL_DIR) $(PKDIR)/etc && \
		$(INSTALL_DIR) $(PKDIR)/etc/transmission && \
		$(INSTALL_FILE) transmission.json $(PKDIR)/etc/transmission/settings.json && \
		$(INSTALL_DIR) $(PKDIR)/etc/init.d && \
		$(INSTALL_BIN) transmission.init $(PKDIR)/etc/init.d/transmission
	$(extra_build)
	$(DISTCLEANUP_transmission)
	touch $@
