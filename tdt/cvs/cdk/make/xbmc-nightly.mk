# tuxbox/enigma2

$(DEPDIR)/xbmc-nightly.do_prepare: $(DEPENDS_xbmc_nightly)
	$(PREPARE_xbmc_nightly)
	touch $@

$(DIR_xbmc_nightly)/config.status: bootstrap libboost directfb libstgles libass libmpeg2 libmad jpeg libsamplerate libogg libvorbis libmodplug curl libflac bzip2 tiff lzo libz fontconfig libfribidi freetype sqlite libpng libpcre libcdio jasper yajl libmicrohttpd tinyxml python gstreamer gst_plugins_dvbmediasink expat sdparm lirc libnfs 
	cd $(DIR_xbmc_nightly) && \
		$(BUILDENV) \
		./bootstrap && \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			PKG_CONFIG_SYSROOT_DIR=$(targetprefix) \
			PKG_CONFIG=$(hostprefix)/bin/pkg-config \
			PKG_CONFIG_PATH=$(targetprefix)/usr/lib/pkgconfig \
			PYTHON_SITE_PKG=$(targetprefix)/usr/lib/python$(PYTHON_VERSION)/site-packages \
			PYTHON_CPPFLAGS=-I$(targetprefix)/usr/include/python$(PYTHON_VERSION) \
			PY_PATH=$(targetprefix)/usr \
			SWIG_EXE=none \
			JRE_EXE=none \
			--disable-gl \
			--enable-glesv1 \
			--disable-gles \
			--disable-sdl \
			--enable-webserver \
			--enable-nfs \
			--disable-x11 \
			--disable-samba \
			--disable-mysql \
			--disable-joystick \
			--disable-rsxs \
			--disable-projectm \
			--disable-goom \
			--disable-afpclient \
			--disable-airplay \
			--disable-airtunes \
			--disable-dvdcss \
			--disable-hal \
			--disable-avahi \
			--disable-optical-drive \
			--disable-libbluray \
			--disable-texturepacker \
			--disable-libcec \
			--enable-gstreamer \
			--disable-paplayer \
			--enable-gstplayer \
			--enable-dvdplayer \
			--disable-pulse \
			--disable-alsa \
			--disable-ssh

$(DEPDIR)/xbmc-nightly.do_compile: $(DIR_xbmc_nightly)/config.status
	cd $(DIR_xbmc_nightly) && \
		$(MAKE) all
	touch $@
DESCRIPTION_xbmc_nightly = xbmc
PKGR_xbmc_nightly =r1
SRC_URI_xbmc = git://github.com/xbmc/xbmc.git
FILES_xbmc_nightly = /usr/lib/xbmc/* \
			/usr/share/applications/* \
			/usr/share/icons/* \
			/usr/share/xbmc/addons/* \
			/usr/share/xbmc/language/Russian \
			/usr/share/xbmc/language/German \
			/usr/share/xbmc/language/English \
			/usr/share/xbmc/media/* \
			/usr/share/xbmc/sounds/* \
			/usr/share/xbmc/userdata/* \
			/usr/share/xbmc/system/* \
			/usr/share/xbmc/FEH.py

$(DEPDIR)/xbmc-nightly: xbmc-nightly.do_prepare xbmc-nightly.do_compile
	$(call parent_pk,xbmc_nightly)
	$(start_build)
	$(get_git_version)
	$(MAKE) -C $(DIR_xbmc_nightly) install DESTDIR=$(PKDIR)
	if [ -e $(PKDIR)/usr/lib/xbmc/xbmc.bin ]; then \
		$(target)-strip $(PKDIR)/usr/lib/xbmc/xbmc.bin; \
	fi
	$(tocdk_build)
	$(toflash_build)
	touch $@

xbmc-nightly-clean:
	rm -f $(DEPDIR)/xbmc-nightly.do_compile
	cd $(DIR_xbmc) && $(MAKE) clean
	
xbmc-nightly-distclean:
	rm -f $(DEPDIR)/xbmc-nightly
	rm -f $(DEPDIR)/xbmc-nightly.do_compile
	rm -f $(DEPDIR)/xbmc-nightly.do_prepare
	rm -rf $(DIR_xbmc_nightly)
