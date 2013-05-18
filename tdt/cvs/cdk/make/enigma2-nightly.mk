# tuxbox/enigma2
E_CONFIG_OPTS_e =

if ENABLE_EXTERNALLCD
E_CONFIG_OPTS_e += --with-graphlcd
endif

if ENABLE_MEDIAFWGSTREAMER
E_CONFIG_OPTS_e += --enable-mediafwgstreamer
else
E_CONFIG_OPTS_e += --enable-libeplayer3
endif

$(DEPDIR)/enigma2-nightly.do_prepare:$(DEPENDS_enigma2)
	$(PREPARE_enigma2)
	touch $@

$(appsdir)/enigma2-nightly/config.status: bootstrap libfreetype libexpat fontconfig libpng libjpeg libgif libfribidi libid3tag libmad libsigc libreadline \
		libdvbsipp python libxml2 libxslt elementtree zope_interface twisted twisted twistedweb2 pyopenssl lxml libxmlccwrap ncurses-dev libdreamdvd sdparm opkg-host ipkg-utils $(MEDIAFW_DEP) $(EXTERNALLCD_DEP)
	cd $(DIR_enigma2) && \
		./autogen.sh && \
		sed -e 's|#!/usr/bin/python|#!$(hostprefix)/bin/python|' -i po/xml2po.py && \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--with-libsdl=no \
			--datadir=/usr/share \
			--libdir=/usr/lib \
			--bindir=/usr/bin \
			--prefix=/usr \
			--sysconfdir=/etc \
			STAGING_INCDIR=$(hostprefix)/usr/include \
			STAGING_LIBDIR=$(hostprefix)/usr/lib \
			PY_PATH=$(targetprefix)/usr \
			$(PLATFORM_CPPFLAGS) $(E_CONFIG_OPTS_e)


$(DEPDIR)/enigma2-nightly.do_compile: $(DIR_enigma2)/config.status
	cd $(DIR_enigma2) && \
		$(MAKE) all
	touch $@

DESCRIPTION_enigma2 := a framebuffer-based zapping application (GUI) for linux
SRC_URI_enigma2 := https://code.google.com/p/enigma2-dmm/
FILES_enigma2 := /usr/bin /usr/lib/ /etc/enigma2 /usr/share
$(DEPDIR)/enigma2-nightly: enigma2-nightly.do_prepare enigma2-nightly.do_compile
#alternate you can do the following: (see packaging.mk)
	$(call parent_pk,enigma2)
	$(start_build)
	$(get_git_version)
	$(MAKE) -C $(DIR_enigma2) install DESTDIR=$(PKDIR)
	$(target)-strip $(PKDIR)/usr/bin/enigma2
	cp -f $(buildprefix)/root/usr/local/share/enigma2/$(enigma2_keymap_file) $(PKDIR)/usr/share/enigma2/keymap.xml
	$(tocdk_build)
	$(toflash_build)
	touch $@

enigma2-nightly-clean:
	rm -f $(DEPDIR)/enigma2-nightly.do_compile
	cd $(DIR_enigma2) && \
		$(MAKE) clean
	rm -f $(DEPDIR)/enigma2-nightly.do_compile
	
enigma2-nightly-distclean:
	rm -f $(DEPDIR)/enigma2-nightly
	rm -f $(DEPDIR)/enigma2-nightly.do_compile
	rm -f $(DEPDIR)/enigma2-nightly.do_prepare
	rm -rf $(DIR_enigma2)

#
# dvb/libdvbsi++
#
$(appsdir)/dvb/libdvbsi++/config.status: bootstrap
	cd $(appsdir)/dvb/libdvbsi++ && $(CONFIGURE) CPPFLAGS="$(CPPFLAGS) -I$(driverdir)/dvb/include"

$(DEPDIR)/libdvbsi++: $(appsdir)/dvb/libdvbsi++/config.status
	$(MAKE) -C $(appsdir)/dvb/libdvbsi++ all install
	touch $@
