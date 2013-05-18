#
# NFS-UTILS
#
DESCRIPTION_nfs_utils = "nfs_utils"
FILES_nfs_utils = \
/usr/bin/*

$(DEPDIR)/nfs_utils.do_prepare: $(DEPENDS_nfs_utils)
	$(PREPARE_nfs_utils)
	chmod +x $(DIR_nfs_utils)/autogen.sh
	cd $(DIR_nfs_utils) && \
		gunzip -cd ../$(lastword $^) | cat > debian.patch && \
		patch -p1 <debian.patch && \
		sed -e 's/### BEGIN INIT INFO/# chkconfig: 2345 19 81\n### BEGIN INIT INFO/g' -i debian/nfs-common.init && \
		sed -e 's/### BEGIN INIT INFO/# chkconfig: 2345 20 80\n### BEGIN INIT INFO/g' -i debian/nfs-kernel-server.init && \
		sed -e 's/do_modprobe nfsd/# do_modprobe nfsd/g' -i debian/nfs-kernel-server.init && \
		sed -e 's/RPCNFSDCOUNT=8/RPCNFSDCOUNT=3/g' -i debian/nfs-kernel-server.default
	touch $@

$(DEPDIR)/nfs_utils.do_compile: bootstrap e2fsprogs $(DEPDIR)/nfs_utils.do_prepare
	cd $(DIR_nfs_utils) && \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			CC_FOR_BUILD=$(target)-gcc \
			--disable-gss \
			--disable-nfsv4 \
			--without-tcp-wrappers && \
		$(MAKE)
	touch $@

$(DEPDIR)/nfs_utils: \
$(DEPDIR)/%nfs_utils: $(NFS_UTILS_ADAPTED_ETC_FILES:%=root/etc/%) \
		$(DEPDIR)/nfs_utils.do_compile
	$(start_build)
	cd $(DIR_nfs_utils) && \
		$(INSTALL_nfs_utils)
	( cd root/etc && for i in $(NFS_UTILS_ADAPTED_ETC_FILES); do \
		[ -f $$i ] && $(INSTALL) -m644 $$i $(PKDIR)/etc/$$i || true; \
		[ "$${i%%/*}" = "init.d" ] && chmod 755 $(PKDIR)/etc/$$i || true; done )
	$(tocdk_build)
	$(toflash_build)
#	$(DISTCLEANUP_nfs_utils)
	[ "x$*" = "x" ] && touch $@ || true

#
# vsftpd
#
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

$(DEPDIR)/vsftpd.do_prepare: $(DEPENDS_vsftpd)
	$(PREPARE_vsftpd)
	touch $@

$(DEPDIR)/vsftpd.do_compile: bootstrap $(DEPDIR)/vsftpd.do_prepare
	cd $(DIR_vsftpd) && \
		$(MAKE) clean && \
		$(MAKE) $(MAKE_OPTS)
	touch $@

$(DEPDIR)/vsftpd: \
$(DEPDIR)/%vsftpd: $(DEPDIR)/vsftpd.do_compile
	$(start_build)
	mkdir -p $(PKDIR)/etc/
	mkdir -p $(PKDIR)/usr/bin/
	mkdir -p $(PKDIR)/usr/share/man/man8/
	mkdir -p $(PKDIR)/usr/share/man/man5/
	cd $(DIR_vsftpd) && \
		$(INSTALL_vsftpd)
	$(tocdk_build)
	$(toflash_build)
#	$(DISTCLEANUP_vsftpd)
	[ "x$*" = "x" ] && touch $@ || true

#
# ETHTOOL
#
DESCRIPTION_ethtool = "ethtool"
FILES_ethtool = \
/usr/sbin/*

$(DEPDIR)/ethtool.do_prepare: $(DEPENDS_ethtool)
	$(PREPARE_ethtool)
	touch $@

$(DEPDIR)/ethtool.do_compile: bootstrap $(DEPDIR)/ethtool.do_prepare
	cd $(DIR_ethtool)  && \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--libdir=$(targetprefix)/usr/lib \
			--prefix=/usr && \
		$(MAKE)
	touch $@

$(DEPDIR)/ethtool: \
$(DEPDIR)/%ethtool: $(DEPDIR)/ethtool.do_compile
	$(start_build)
	cd $(DIR_ethtool)  && \
		$(INSTALL_ethtool)
	$(tocdk_build)
	$(toflash_build)
#	$(DISTCLEANUP_ethtool)
	[ "x$*" = "x" ] && touch $@ || true

#
# SAMBA
#
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
#	$(DISTCLEANUP_samba)
	)[ "x$*" = "x" ] && touch $@ || true

#
# NETIO
#
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
#	$(DISTCLEANUP_netio)
	[ "x$*" = "x" ] && touch $@ || true

#
# LIGHTTPD
#
DESCRIPTION_lighttpd = "lighttpd"
FILES_lighttpd = \
/usr/bin/* \
/usr/sbin/* \
/usr/lib/*.so \
/etc/init.d/* \
/etc/lighttpd/*.conf 

$(DEPDIR)/lighttpd.do_prepare: $(DEPENDS_lighttpd)
	$(PREPARE_lighttpd)
	touch $@

$(DEPDIR)/lighttpd.do_compile: bootstrap $(DEPDIR)/lighttpd.do_prepare
	cd $(DIR_lighttpd) && \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix= \
			--exec-prefix=/usr \
			--datarootdir=/usr/share && \
		$(MAKE)
	touch $@

$(DEPDIR)/lighttpd: \
$(DEPDIR)/%lighttpd: $(DEPDIR)/lighttpd.do_compile
	$(start_build)
	cd $(DIR_lighttpd) && \
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
#	$(DISTCLEANUP_lighttpd)
	[ "x$*" = "x" ] && touch $@ || true

#
# NETKIT_FTP
#
DESCRIPTION_netkit_ftp = "netkit_ftp"
FILES_netkit_ftp = \
/usr/bin/*

$(DEPDIR)/netkit_ftp.do_prepare: $(DEPENDS_netkit_ftp)
	$(PREPARE_netkit_ftp)
	touch $@

$(DEPDIR)/netkit_ftp.do_compile: bootstrap ncurses libreadline $(DEPDIR)/netkit_ftp.do_prepare
	cd $(DIR_netkit_ftp)  && \
		$(BUILDENV) \
		./configure \
			--with-c-compiler=$(target)-gcc \
			--prefix=/usr \
			--installroot=$(PKDIR) && \
		$(MAKE)
	touch $@

$(DEPDIR)/netkit_ftp: \
$(DEPDIR)/%netkit_ftp: $(DEPDIR)/netkit_ftp.do_compile
	$(start_build)
	cd $(DIR_netkit_ftp)  && \
		$(INSTALL_netkit_ftp)
	$(tocdk_build)
	$(toflash_build)
#	$(DISTCLEANUP_netkit_ftp)
	[ "x$*" = "x" ] && touch $@ || true

#
# WIRELESS_TOOLS
#
DESCRIPTION_wireless_tools = wireless-tools
RDEPENDS_wireless_tools = rfkill wpa-supplicant
FILES_wireless_tools = \
/usr/sbin/* \
/usr/lib/*.so*

$(DEPDIR)/wireless_tools.do_prepare: wpa_supplicant rfkill $(DEPENDS_wireless_tools)
	$(PREPARE_wireless_tools)
	touch $@

$(DEPDIR)/wireless_tools.do_compile: bootstrap $(DEPDIR)/wireless_tools.do_prepare
	cd $(DIR_wireless_tools)  && \
		$(MAKE) $(MAKE_OPTS)
	touch $@

$(DEPDIR)/wireless_tools: \
$(DEPDIR)/%wireless_tools: $(DEPDIR)/wireless_tools.do_compile
	$(start_build)
	cd $(DIR_wireless_tools)  && \
		$(INSTALL_wireless_tools)
	$(tocdk_build)
	$(toflash_build)
#	$(DISTCLEANUP_wireless_tools)
	[ "x$*" = "x" ] && touch $@ || true

#
# WPA_SUPPLICANT
#
DESCRIPTION_wpa_supplicant = "wpa-supplicant"
PKGR_wpa_supplicant = r0
FILES_wpa_supplicant = \
/usr/sbin/*

$(DEPDIR)/wpa_supplicant.do_prepare: $(DEPENDS_wpa_supplicant)
	$(PREPARE_wpa_supplicant)
	touch $@

$(DEPDIR)/wpa_supplicant.do_compile: bootstrap $(DEPDIR)/wpa_supplicant.do_prepare
	cd $(DIR_wpa_supplicant)/wpa_supplicant  && \
		mv ../wpa_supplicant.config .config && \
		$(MAKE) $(MAKE_OPTS)
	touch $@

$(DEPDIR)/wpa_supplicant: \
$(DEPDIR)/%wpa_supplicant: $(DEPDIR)/wpa_supplicant.do_compile
	$(start_build)
	cd $(DIR_wpa_supplicant)/wpa_supplicant && \
		$(INSTALL_wpa_supplicant)
	$(tocdk_build)
	$(toflash_build)
#	$(DISTCLEANUP_wpa_supplicant)
	[ "x$*" = "x" ] && touch $@ || true
