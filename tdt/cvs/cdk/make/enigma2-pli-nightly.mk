# tuxbox/enigma2
E_CONFIG_OPTS =

if ENABLE_EXTERNALLCD
E_CONFIG_OPTS += --with-graphlcd
endif

if ENABLE_MEDIAFWGSTREAMER
E_CONFIG_OPTS += --enable-mediafwgstreamer
else
E_CONFIG_OPTS += --enable-libeplayer3
endif

$(DEPDIR)/enigma2-pli-nightly.do_prepare: $(DEPENDS_enigma2_pli)
	$(PREPARE_enigma2_pli)
	touch $@

$(DIR_enigma2_pli)/config.status: bootstrap libfreetype libexpat fontconfig libpng libjpeg libgif libmme_host libmmeimage libfribidi libid3tag libmad libsigc libreadline font-valis-enigma \
		enigma2-pli-nightly.do_prepare \
		libdvbsipp python libxml2 libxslt elementtree zope_interface twisted twistedweb2 twistedmail pycrypto pyusb pilimaging pyopenssl pythonwifi lxml libxmlccwrap \
		ncurses-dev libdreamdvd2 tuxtxt32bpp sdparm hotplug_e2 $(MEDIAFW_DEP) $(EXTERNALLCD_DEP)
	cd $(DIR_enigma2_pli) && \
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
			--with-boxtype=none \
			STAGING_INCDIR=$(hostprefix)/usr/include \
			STAGING_LIBDIR=$(hostprefix)/usr/lib \
			PY_PATH=$(targetprefix)/usr \
			$(PLATFORM_CPPFLAGS) $(E_CONFIG_OPTS)


$(DEPDIR)/enigma2-pli-nightly.do_compile: $(DIR_enigma2_pli)/config.status
	cd $(DIR_enigma2_pli) && \
		$(MAKE) all
	touch $@

# Select enigma2 keymap.xml
enigma2_keymap_file = keymap$(if $(HL101),_$(HL101))$(if $(SPARK)$(SPARK7162),_spark).xml

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
