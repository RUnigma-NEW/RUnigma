LIRCD_CONF := lircd_hl101.conf

$(DEPDIR)/misc-cp:
	cp $(buildprefix)/root/sbin/hotplug $(targetprefix)/sbin
	cp $(buildprefix)/root/etc/$(LIRCD_CONF) $(targetprefix)/etc/lircd.conf
	cp -rd $(buildprefix)/root/etc/hotplug $(targetprefix)/etc
	cp -rd $(buildprefix)/root/etc/hotplug.d $(targetprefix)/etc
	touch $@

$(DEPDIR)/misc-e2:
	$(INSTALL_DIR) $(targetprefix)/media/hdd
	$(INSTALL_DIR) $(targetprefix)/media/dvd
	$(INSTALL_DIR) $(targetprefix)/hdd
	$(INSTALL_DIR) $(targetprefix)/hdd/music
	$(INSTALL_DIR) $(targetprefix)/hdd/picture
	$(INSTALL_DIR) $(targetprefix)/hdd/movie
	touch $@

#
# SPLASHUTILS
#
SPLASHUTILS := splashutils
SPLASHUTILS_VERSION := 1.5.4.3-9
SPLASHUTILS_SPEC := stm-target-$(SPLASHUTILS).spec
SPLASHUTILS_SPEC_PATCH :=
SPLASHUTILS_PATCHES :=

SPLASHUTILS_RPM := RPMS/sh4/$(STLINUX)-sh4-$(SPLASHUTILS)-$(SPLASHUTILS_VERSION).sh4.rpm

$(SPLASHUTILS_RPM): \
		$(if $(SPLASHUTILS_SPEC_PATCH),Patches/$(SPLASHUTILS_SPEC_PATCH)) \
		$(if $(SPLASHUTILS_PATCHES),$(SPLASHUTILS_PATCHES:%=Patches/%)) \
		libjpeg libmng libfreetype libpng \
		$(archivedir)/$(STLINUX)-target-$(SPLASHUTILS)-$(SPLASHUTILS_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(SPLASHUTILS_SPEC_PATCH),( cd SPECS && patch -p1 $(SPLASHUTILS_SPEC) < $(buildprefix)/Patches/$(SPLASHUTILS_SPEC_PATCH) ) &&) \
	$(if $(SPLASHUTILS_PATCHES),cp $(SPLASHUTILS_PATCHES:%=Patches/%) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	export PKG_CONFIG_PATH=$(targetprefix)/usr/lib/pkgconfig && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --nodeps --target=sh4-linux SPECS/$(SPLASHUTILS_SPEC)

$(DEPDIR)/$(SPLASHUTILS): $(SPLASHUTILS_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^) && \
	cp root/etc/splash/luxisri.ttf $(targetprefix)/etc/splash/ && \
	cp -rd root/etc/splash/{vdr,liquid,together}_theme $(targetprefix)/etc/splash/ && \
	$(LN_SF) liquid_theme $(targetprefix)/etc/splash/default && \
	$(INSTALL_DIR) $(targetprefix)/lib/lsb && \
	cp root/lib/lsb/splash-functions $(targetprefix)/lib/lsb/ && \
	touch -r $(lastword $^) $@

#
# STSLAVE
#
STSLAVE := stslave
STSLAVE_VERSION := 0.7-27
STSLAVE_SPEC := stm-target-$(STSLAVE).spec
STSLAVE_SPEC_PATCH :=
STSLAVE_PATCHES :=

STSLAVE_RPM := RPMS/sh4/$(STLINUX)-sh4-$(STSLAVE)-$(STSLAVE_VERSION).sh4.rpm

$(STSLAVE_RPM): \
		$(addprefix Patches/,$(STSLAVE_SPEC_PATCH) $(STSLAVE_PATCHES)) \
		$(archivedir)/$(STLINUX)-target-$(STSLAVE)-$(STSLAVE_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(STSLAVE_SPEC_PATCH),( cd SPECS && patch -p1 $(STSLAVE_SPEC) < $(buildprefix)/Patches/$(STSLAVE_SPEC_PATCH) ) &&) \
	$(if $(STSLAVE_PATCHES),cp $(addprefix Patches/,$(STSLAVE_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(STSLAVE_SPEC)

$(DEPDIR)/$(STSLAVE): $(STSLAVE_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch -r $(lastword $^) $@
	$(start_build)
	$(fromrpm_build)

#
# OPENSSL
#
OPENSSL := openssl
OPENSSL_DEV := openssl-dev
FILES_openssl_dev = \
/usr/lib
FILES_openssl = \
/usr/lib/*.so*

OPENSSL_VERSION := 1.0.1e-29
OPENSSL_SPEC := stm-target-$(OPENSSL).spec
OPENSSL_SPEC_PATCH :=
OPENSSL_PATCHES :=

OPENSSL_RPM := RPMS/sh4/$(STLINUX)-sh4-$(OPENSSL)-$(OPENSSL_VERSION).sh4.rpm
OPENSSL_DEV_RPM := RPMS/sh4/$(STLINUX)-sh4-$(OPENSSL_DEV)-$(OPENSSL_VERSION).sh4.rpm

$(OPENSSL_RPM) $(OPENSSL_DEV_RPM): \
		$(addprefix Patches/,$(OPENSSL_SPEC_PATCH) $(OPENSSL_PATCHES)) \
		$(archivedir)/$(STLINUX)-target-$(OPENSSL)-$(OPENSSL_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(OPENSSL_SPEC_PATCH),( cd SPECS && patch -p1 $(OPENSSL_SPEC) < $(buildprefix)/Patches/$(OPENSSL_SPEC_PATCH) ) &&) \
	$(if $(OPENSSL_PATCHES),cp $(addprefix Patches/,$(OPENSSL_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(OPENSSL_SPEC)

$(DEPDIR)/$(OPENSSL): $(OPENSSL_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@
	$(start_build)
	$(fromrpm_build)

$(DEPDIR)/$(OPENSSL_DEV): $(OPENSSL) $(OPENSSL_DEV_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@
	$(start_build)
	$(fromrpm_build)

#
# ALSALIB
#
ALSALIB := alsa-lib
ALSALIB_DEV := alsa-lib-dev
ALSALIB_VERSION := 1.0.21a-24
ALSALIB_SPEC := stm-target-$(ALSALIB).spec
ALSALIB_SPEC_PATCH :=
ALSALIB_PATCHES :=

ALSALIB_RPM := RPMS/sh4/$(STLINUX)-sh4-$(ALSALIB)-$(ALSALIB_VERSION).sh4.rpm
ALSALIB_DEV_RPM := RPMS/sh4/$(STLINUX)-sh4-$(ALSALIB_DEV)-$(ALSALIB_VERSION).sh4.rpm

$(ALSALIB_RPM) $(ALSALIB_DEV_RPM): \
		$(addprefix Patches/,$(ALSALIB_SPEC_PATCH) $(ALSALIB_PATCHES)) \
		$(archivedir)/$(STLINUX)-target-$(ALSALIB)-$(ALSALIB_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(ALSALIB_SPEC_PATCH),( cd SPECS && patch -p1 $(ALSALIB_SPEC) < $(buildprefix)/Patches/$(ALSALIB_SPEC_PATCH) ) &&) \
	$(if $(ALSALIB_PATCHES),cp $(addprefix Patches/,$(ALSALIB_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(ALSALIB_SPEC)

$(DEPDIR)/$(ALSALIB): $(ALSALIB_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@

$(DEPDIR)/$(ALSALIB_DEV): $(ALSALIB) $(ALSALIB_DEV_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@

#
# ALSAUTILS
#
ALSAUTILS := alsa-utils
ALSAUTILS_VERSION := 1.0.21-17
ALSAUTILS_SPEC := stm-target-$(ALSAUTILS).spec
ALSAUTILS_SPEC_PATCH :=
ALSAUTILS_PATCHES :=

ALSAUTILS_RPM := RPMS/sh4/$(STLINUX)-sh4-$(ALSAUTILS)-$(ALSAUTILS_VERSION).sh4.rpm

$(ALSAUTILS_RPM): \
		$(addprefix Patches/,$(ALSAUTILS_SPEC_PATCH) $(ALSAUTILS_PATCHES)) \
		$(NCURSES_DEV) $(ALSALIB_DEV) \
		$(archivedir)/$(STLINUX)-target-$(ALSAUTILS)-$(ALSAUTILS_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(ALSAUTILS_SPEC_PATCH),( cd SPECS && patch -p1 $(ALSAUTILS_SPEC) < $(buildprefix)/Patches/$(ALSAUTILS_SPEC_PATCH) ) &&) \
	$(if $(ALSAUTILS_PATCHES),cp $(addprefix Patches/,$(ALSAUTILS_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --nodeps --target=sh4-linux SPECS/$(ALSAUTILS_SPEC)

$(DEPDIR)/$(ALSAUTILS): $(ALSAUTILS_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@

#
# ALSAPLAYER
#
ALSAPLAYER := alsaplayer
ALSAPLAYER_DEV := alsaplayer-dev
ALSAPLAYER_VERSION := 0.99.77-20
ALSAPLAYER_SPEC := stm-target-$(ALSAPLAYER).spec
ALSAPLAYER_SPEC_PATCH :=
ALSAPLAYER_PATCHES :=

ALSAPLAYER_RPM := RPMS/sh4/$(STLINUX)-sh4-$(ALSAPLAYER)-$(ALSAPLAYER_VERSION).sh4.rpm
ALSAPLAYER_DEV_RPM := RPMS/sh4/$(STLINUX)-sh4-$(ALSAPLAYER_DEV)-$(ALSAPLAYER_VERSION).sh4.rpm

$(ALSAPLAYER_RPM) $(ALSAPLAYER_DEV_RPM): \
		$(addprefix Patches/,$(ALSAPLAYER_SPEC_PATCH) $(ALSAPLAYER_PATCHES)) \
		libmad libid3tag \
		$(archivedir)/$(STLINUX)-target-$(ALSAPLAYER)-$(ALSAPLAYER_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(ALSAPLAYER_SPEC_PATCH),( cd SPECS && patch -p1 $(ALSAPLAYER_SPEC) < $(buildprefix)/Patches/$(ALSAPLAYER_SPEC_PATCH) ) &&) \
	$(if $(ALSAPLAYER_PATCHES),cp $(addprefix Patches/,$(ALSAPLAYER_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	export PKG_CONFIG_PATH=$(targetprefix)/usr/include/pkgconfig && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(ALSAPLAYER_SPEC)

$(DEPDIR)/$(ALSAPLAYER): $(ALSAPLAYER_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@

$(DEPDIR)/$(ALSAPLAYER_DEV): $(ALSAPLAYER_DEV_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@


#
# LIBEVENT
#

LIBEVENT := libevent
LIBEVENT_DEV := libevent-dev
FILES_libevent_dev = \
/usr/lib
FILES_libevent = \
/usr/lib/*.so*

LIBEVENT_VERSION := 2.0.19-4
LIBEVENT_SPEC := stm-target-$(LIBEVENT).spec
LIBEVENT_SPEC_PATCH := stm-target-$(LIBEVENT).spec.diff
LIBEVENT_PATCHES :=
LIBEVENT_RPM := RPMS/sh4/$(STLINUX)-sh4-$(LIBEVENT)-$(LIBEVENT_VERSION).sh4.rpm
LIBEVENT_DEV_RPM := RPMS/sh4/$(STLINUX)-sh4-$(LIBEVENT_DEV)-$(LIBEVENT_VERSION).sh4.rpm

$(LIBEVENT_RPM) $(LIBEVENT_DEV_RPM): \
		$(if $(LIBEVENT_SPEC_PATCH),Patches/$(LIBEVENT_SPEC_PATCH)) \
		$(if $(LIBEVENT_PATCHES),$(LIBEVENT_PATCHES:%=Patches/%)) \
		$(archivedir)/$(STLINUX)-target-$(LIBEVENT)-$(LIBEVENT_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(LIBEVENT_SPEC_PATCH),( cd SPECS && patch -p1 $(LIBEVENT_SPEC) < $(buildprefix)/Patches/$(LIBEVENT_SPEC_PATCH) ) &&) \
	$(if $(LIBEVENT_PATCHES),cp $(LIBEVENT_PATCHES:%=Patches/%) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(LIBEVENT_SPEC)

$(DEPDIR)/$(LIBEVENT): $(LIBEVENT_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^) && \
	touch $@ || true
	$(start_build)
	$(fromrpm_build)

$(DEPDIR)/$(LIBEVENT_DEV): $(LIBEVENT) $(LIBEVENT_DEV_RPM)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^) && \
	touch $@ || true
	$(start_build)
	$(fromrpm_build)
