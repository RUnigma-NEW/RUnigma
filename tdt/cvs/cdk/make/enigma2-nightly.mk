# tuxbox/enigma2
BEGIN[[
enigma2
  git
  $(appsdir)/{PN}-nightly
ifdef ENABLE_E2D0
  git://gitorious.org/open-duckbox-project-sh4/guigit.git:r=945aeb939308b3652b56bc6c577853369d54a537
  patch:file://enigma2-nightly.0.diff
  patch:file://enigma2-nightly.0.eplayer3.diff
  patch:file://enigma2-nightly.0.gstreamer.diff
  patch:file://enigma2-nightly.0.graphlcd.diff
endif

ifdef ENABLE_E2D1
  git://gitorious.org/open-duckbox-project-sh4/guigit.git:r=945aeb939308b3652b56bc6c577853369d54a537
  patch:file://enigma2-nightly.1.diff
endif

ifdef ENABLE_E2D2
  git://gitorious.org/open-duckbox-project-sh4/guigit.git:r=839e96b79600aba73f743fd39628f32bc1628f4c
  patch:file://enigma2-nightly.2.diff
  patch:file://enigma2-nightly.2.gstreamer.diff
  patch:file://enigma2-nightly.2.graphlcd.diff
endif

ifdef ENABLE_E2D3
  git://gitorious.org/open-duckbox-project-sh4/guigit.git:r=51a7b9349070830b5c75feddc52e97a1109e381e
  patch:file://enigma2-nightly.3.diff
  patch:file://enigma2-nightly.3.gstreamer.diff
  patch:file://enigma2-nightly.3.graphlcd.diff
endif

ifdef ENABLE_E2D4
  git://gitorious.org/open-duckbox-project-sh4/guigit.git:r=002b85aa8350e9d8e88f75af48c3eb8a6cdfb880
  patch:file://enigma2-pli-nightly.4.diff
endif

ifdef ENABLE_E2D5
  git://openpli.git.sourceforge.net/gitroot/openpli/enigma2:r=a869076762f6e24305d6a58f95c3918e02a1442a
  patch:file://enigma2-pli-nightly.5.diff
endif

ifdef ENABLE_E2D6
  git://openpli.git.sourceforge.net/gitroot/openpli/enigma2:r=388dcd814d4e99720cb9a6c769611be4951e4ad4
  patch:file://enigma2-pli-nightly.6.diff
endif

ifdef ENABLE_E2D7
  git://code.google.com/p/enigma2-dmm:protocol=https
endif

ifdef ENABLE_E2D8
  git://gitorious.org/~schpuntik/open-duckbox-project-sh4/amiko-guigit.git:b=arp-no_gst
endif
;
]]END

DESCRIPTION_enigma2 := a framebuffer-based zapping application (GUI) for linux
PKGR_enigma2 = r1
SRC_URI_enigma2 := https://code.google.com/p/enigma2-dmm/
FILES_enigma2 := /usr/bin /usr/lib/ /etc/enigma2 /usr/share

$(DEPDIR)/enigma2-nightly.do_prepare:$(DEPENDS_enigma2)
	$(PREPARE_enigma2)
	touch $@


$(appsdir)/enigma2-nightly/config.status: bootstrap libfreetype libexpat fontconfig libpng libjpeg libgif libfribidi libid3tag libmad libsigc libreadline \
		libdvbsipp python libxml2 libxslt elementtree zope_interface twisted twistedweb2 pyopenssl lxml libxmlccwrap ncurses-dev libdreamdvd sdparm opkg-host ipkg-utils $(MEDIAFW_DEP) $(EXTERNALLCD_DEP)
	cd $(DIR_enigma2) && \
		./autogen.sh && \
		sed -e 's|#!/usr/bin/python|#!$(hostprefix)/bin/python$(PYTHON_VERSION)|' -i po/xml2po.py && \
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
			$(PLATFORM_CPPFLAGS) $(E_CONFIG_OPTS)


$(DEPDIR)/enigma2-nightly.do_compile: $(DIR_enigma2)/config.status
	cd $(DIR_enigma2) && \
		$(MAKE) all
	touch $@

$(DEPDIR)/enigma2-nightly: enigma2-nightly.do_prepare enigma2-nightly.do_compile
#alternate you can do the following: (see packaging.mk)
	$(call parent_pk,enigma2)
	$(start_build)
	$(get_git_version)
	$(MAKE) -C $(DIR_enigma2) install DESTDIR=$(PKDIR)
	$(target)-strip $(PKDIR)/usr/bin/enigma2
	cp -f $(buildprefix)/root/usr/local/share/enigma2/$(enigma2_keymap_file) $(PKDIR)/usr/share/enigma2/keymap.xml
	cp -f $(buildprefix)/root/usr/local/share/enigma2/keymap_amiko.xml $(PKDIR)/usr/share/enigma2/
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
