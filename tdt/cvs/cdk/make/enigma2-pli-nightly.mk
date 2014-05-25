# tuxbox/enigma2
BEGIN[[
enigma2_pli
  git
  {PN}-nightly

ifdef ENABLE_E2PD0
  git://openpli.git.sourceforge.net/gitroot/openpli/enigma2:r=945aeb939308b3652b56bc6c577853369d54a537
  patch:file://enigma2-pli-nightly.0.diff
endif

ifdef ENABLE_E2PD1
  git://github.com/technic/amiko-e2-pli.git:b=last
  patch:file://enigma2-pli-nightly-last.diff
  patch:file://python_m4.diff
ifdef ENABLE_PY332
  patch:file://enigma2-pli-nightly-last-python3.diff
endif
endif

ifdef ENABLE_E2PD2
  git://github.com/technic/amiko-e2-pli.git:b=master
endif

;
]]END

DESCRIPTION_enigma2_pli := a framebuffer-based zapping application (GUI) for linux
PKGR_enigma2_pli = r3
SRC_URI_enigma2_pli := git://openpli.git.sourceforge.net/gitroot/openpli/enigma2
FILES_enigma2_pli := /usr/lib/ /etc/enigma2 /usr/share /usr/bin
RDEPENDS_enigma2_pli = fp_control \
evremote2 \
devinit \
ustslave \
stfbcontrol \
showiframe

# Select enigma2 keymap.xml
enigma2_keymap_file = keymap$(if $(HL101),_$(HL101)).xml

E_CONFIG_OPTS =

ifdef ENABLE_EXTERNALLCD
E_CONFIG_OPTS += --with-graphlcd
endif

ifdef ENABLE_MEDIAFWGSTREAMER
E_CONFIG_OPTS += --enable-mediafwgstreamer
else
E_CONFIG_OPTS += --enable-libeplayer3 LIBEPLAYER3_CPPFLAGS="-I $(appsdir)/misc/tools/libeplayer3/include"
endif

$(DEPDIR)/enigma2-pli-nightly.do_prepare: $(DEPENDS_enigma2_pli)
	$(PREPARE_enigma2_pli)
	touch $@

ifdef ENABLE_PY332
$(DIR_enigma2_pli)/config.status: bootstrap opkg ethtool libfreetype libexpat fontconfig libpng libjpeg libgif libmme_host libmmeimage libfribidi libid3tag libmad libsigc libreadline font-valis-enigma \
		enigma2-pli-nightly.do_prepare \
		libdvbsipp python libxml2 libxslt elementtree zope_component zope_interface service_identity six pip twisted pycrypto pyusb Pillow pyopenssl pythonwifi lxml libxmlccwrap \
		ncurses-dev libdreamdvd2 tuxtxt32bpp sdparm hotplug_e2 $(MEDIAFW_DEP) $(EXTERNALLCD_DEP)
	cd $(DIR_enigma2_pli) && \
		$(BUILDENV) \
		./autogen.sh && \
		sed -e 's|#!/usr/bin/python|#!$(hostprefix)/bin/python$(PYTHON_VERSION)|' -i po/xml2po.py && \
		./configure \
			--host=$(target) \
			--with-libsdl=no \
			--datadir=/usr/share \
			--libdir=/usr/lib \
			--bindir=/usr/bin \
			--prefix=/usr \
			--sysconfdir=/etc \
			--with-boxtype=none \
			STAGING_INCDIR=$(hostprefix)/usr/include \
			STAGING_LIBDIR=$(hostprefix)/usr/lib \
			PKG_CONFIG=$(hostprefix)/bin/pkg-config \
			PY_PATH=$(targetprefix)/usr \
			$(PLATFORM_CPPFLAGS) $(E_CONFIG_OPTS)
else
$(DIR_enigma2_pli)/config.status: bootstrap opkg ethtool libfreetype libexpat fontconfig libpng libjpeg libgif libmme_host libmmeimage libfribidi libid3tag libmad libsigc libreadline font-valis-enigma \
		enigma2-pli-nightly.do_prepare \
		libdvbsipp python libxml2 libxslt elementtree zope_interface service_identity six twisted twistedweb2 twistedweb twistedmail pycrypto pyusb Pillow pyopenssl pythonwifi lxml libxmlccwrap \
		ncurses-dev libdreamdvd2 tuxtxt32bpp sdparm hotplug_e2 $(MEDIAFW_DEP) $(EXTERNALLCD_DEP)
	cd $(DIR_enigma2_pli) && \
		$(BUILDENV) \
		./autogen.sh && \
		sed -e 's|#!/usr/bin/python$(PYTHON_VERSION)|#!$(hostprefix)/bin/python$(PYTHON_VERSION)|' -i po/xml2po.py && \
		./configure \
			--host=$(target) \
			--with-libsdl=no \
			--datadir=/usr/share \
			--libdir=/usr/lib \
			--bindir=/usr/bin \
			--prefix=/usr \
			--sysconfdir=/etc \
			--with-boxtype=none \
			STAGING_INCDIR=$(hostprefix)/usr/include \
			STAGING_LIBDIR=$(hostprefix)/usr/lib \
			PKG_CONFIG=$(hostprefix)/bin/pkg-config \
			PY_PATH=$(targetprefix)/usr \
			$(PLATFORM_CPPFLAGS) $(E_CONFIG_OPTS)
endif

$(DEPDIR)/enigma2-pli-nightly.do_compile: $(DIR_enigma2_pli)/config.status
	cd $(DIR_enigma2_pli) && \
		$(MAKE) all
	touch $@

$(DEPDIR)/enigma2-pli-nightly: enigma2-pli-nightly.do_compile
	$(call parent_pk,enigma2_pli)
	$(start_build)
	$(get_git_version)
	cd $(DIR_enigma2_pli) && \
		$(MAKE) install DESTDIR=$(PKDIR)
	$(target)-strip $(PKDIR)/usr/bin/enigma2
	cp -f $(buildprefix)/root/usr/local/share/enigma2/$(enigma2_keymap_file) $(PKDIR)/usr/share/enigma2/keymap.xml
	$(tocdk_build)
	$(toflash_build)
	touch $@

enigma2-pli-nightly-clean:
	rm -f $(DEPDIR)/enigma2-pli-nightly.do_compile
	cd $(DIR_enigma2_pli) && $(MAKE) clean

enigma2-pli-nightly-distclean:
	rm -f $(DEPDIR)/enigma2-pli-nightly
	rm -f $(DEPDIR)/enigma2-pli-nightly.do_compile
	rm -f $(DEPDIR)/enigma2-pli-nightly.do_prepare
	rm -rf $(DIR_enigma2_pli)
