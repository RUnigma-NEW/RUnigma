#
# libboost
#
BEGIN[[
libboost
  boost-1.54.0
  boost_1_54_0
  extract:http://sourceforge.net/projects/boost/files/boost/1.54.0/boost_1_54_0.tar.bz2
  patch:file://{PN}.diff
  remove:TARGETS/include/boost
  move:boost:TARGETS/usr/include/boost
;
]]END

$(DEPDIR)/libboost: bootstrap $(DEPENDS_libboost)
	$(PREPARE_libboost)
	cd $(DIR_libboost); \
		$(INSTALL_libboost)
	$(DISTCLEANUP_libboost)
	touch $@

#
# libz
#
BEGIN[[
libz
  1.2.8
  zlib-{PV}
  extract:http://zlib.net/zlib-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libz = "Compression library implementing the deflate compression method found in gzip and PKZIP"
FILES_libz = \
/usr/lib/*

LIBZ_ORDER = binutils-dev

$(DEPDIR)/libz: bootstrap $(DEPENDS_libz) $(if $(LIBZ_ORDER),| $(LIBZ_ORDER))
	$(PREPARE_libz)
	$(start_build)
	cd $(DIR_libz); \
		ln -sf /bin/true ./ldconfig; \
		$(BUILDENV) \
		./configure \
			--prefix=/usr \
			--shared; \
		$(MAKE) all libz.a AR="$(target)-ar " CFLAGS="-fpic -O2"; \
		$(INSTALL_libz)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libz)
	touch $@

#
# libreadline
#
BEGIN[[
libreadline
  6.2
  readline-{PV}
  extract:ftp://ftp.cwru.edu/pub/bash/readline-{PV}.tar.gz
  #patch:file://readline62.patch
  patch:file://readline62-001.patch
  patch:file://readline62-002.patch
  patch:file://readline62-003.patch
  patch:file://readline62-004.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libreadline = GNU readline library
FILES_libreadline = \
/usr/lib

$(DEPDIR)/libreadline: bootstrap ncurses-dev $(DEPENDS_libreadline)
	$(PREPARE_libreadline)
	$(start_build)
	cd $(DIR_libreadline); \
		autoconf; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			bash_cv_must_reinstall_sighandlers=no \
			bash_cv_func_sigsetjmp=present \
			bash_cv_func_strcoll_broken=no \
			bash_cv_have_mbstate_t=yes \
			--prefix=/usr \
		$(MAKE) all \
		$(INSTALL_libreadline)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libreadline)
	touch $@

#
# libfreetype
#
BEGIN[[
libfreetype
  2.5.0.1
  freetype-{PV}
  extract:http://download.savannah.gnu.org/releases/freetype/freetype-{PV}.tar.bz2
  patch:file://libfreetype-{PV}.patch
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libfreetype = "libfreetype"

FILES_libfreetype = \
/usr/lib/*.so* \
/usr/bin/freetype-config

$(DEPDIR)/libfreetype: bootstrap libpng12 $(DEPENDS_libfreetype)
	$(PREPARE_libfreetype)
	$(start_build)
	cd $(DIR_libfreetype); \
		sed -i '/#define FT_CONFIG_OPTION_OLD_INTERNALS/d' include/freetype/config/ftoption.h; \
		sed -i '/^FONT_MODULES += \(type1\|cid\|pfr\|type42\|pcf\|bdf\)/d' modules.cfg; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		sed -e "s,^prefix=,prefix=$(targetprefix)," < builds/unix/freetype-config > $(crossprefix)/bin/freetype-config; \
		chmod 755 $(crossprefix)/bin/freetype-config; \
		ln -sf $(crossprefix)/bin/freetype-config $(crossprefix)/bin/$(target)-freetype-config; \
		ln -sf $(targetprefix)/usr/include/freetype2/freetype $(targetprefix)/usr/include/freetype; \
		$(INSTALL_libfreetype)
		rm -f $(targetprefix)/usr/bin/freetype-config
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libfreetype)
	touch $@

#
# lirc
#
BEGIN[[
lirc
  0.9.0
  {PN}-{PV}
  extract:http://prdownloads.sourceforge.net/{PN}/{PN}-{PV}.tar.gz
  patch:file://{PN}-{PV}-try_first_last_remote.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_lirc ="lirc"
PKGR_lirc = r3
FILES_lirc = \
/usr/bin/lircd \
/usr/lib/*.so* \
/etc/lircd*

$(DEPDIR)/lirc: bootstrap $(DEPENDS_lirc)
	$(PREPARE_lirc)
	$(start_build)
	cd $(DIR_lirc); \
		$(BUILDENV) \
		ac_cv_path_LIBUSB_CONFIG= \
		CFLAGS="$(TARGET_CFLAGS) -Os -D__KERNEL_STRICT_NAMES" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--sbindir=\$${exec_prefix}/bin \
			--mandir=\$${prefix}/share/man \
			--with-kerneldir=$(buildprefix)/$(KERNEL_DIR) \
			--without-x \
			--with-devdir=/dev \
			--with-moduledir=/lib/modules \
			--with-major=61 \
			--with-driver=userspace \
			--enable-debug \
			--with-syslog=LOG_DAEMON \
			--enable-sandboxed; \
		$(MAKE) all; \
		$(INSTALL_lirc)
	$(tocdk_build)
	$(INSTALL_DIR) $(PKDIR)/etc
	$(INSTALL_DIR) $(PKDIR)/var/run/lirc/
	$(INSTALL_FILE) $(buildprefix)/root/etc/lircd$(if $(HL101),_$(HL101)).conf $(PKDIR)/etc/lircd.conf
	$(toflash_build)
	$(DISTCLEANUP_lirc)
	touch $@

#
# libjpeg
#
BEGIN[[
libjpeg
  8d
  jpeg-{PV}
  extract:http://www.ijg.org/files/jpegsrc.v{PV}.tar.gz
  patch:file://jpeg.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libjpeg = "libjpeg"

FILES_libjpeg = \
/usr/lib/*.so* 

$(DEPDIR)/libjpeg: bootstrap $(DEPENDS_libjpeg)
	$(PREPARE_libjpeg)
	$(start_build)
	cd $(DIR_libjpeg); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--enable-shared \
			--enable-static \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libjpeg)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libjpeg)
	touch $@

#
# libjpeg_turbo
#
BEGIN[[
libjpeg_turbo
  1.2.1
  libjpeg-turbo-{PV}
  extract:http://sourceforge.net/projects/libjpeg-turbo/files/1.2.1/libjpeg-turbo-1.2.1.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libjpeg_turbo = "libjpeg_turbo"

FILES_libjpeg_turbo = \
/usr/lib/*.so* 


$(DEPDIR)/libjpeg_turbo: bootstrap $(DEPENDS_libjpeg_turbo)
	$(PREPARE_libjpeg_turbo)
	$(start_build)
	cd $(DIR_libjpeg_turbo); \
		export CC=$(target)-gcc; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--enable-shared \
			--disable-static \
			--with-jpeg8 \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libjpeg_turbo)
	cd $(DIR_libjpeg_turbo); \
		make clean; \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--enable-shared \
			--disable-static \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libjpeg_turbo)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libjpeg_turbo)
	touch $@

#
# libpng12
#
BEGIN[[
libpng12
  1.2.49
  libpng-{PV}
  extract:http://ftp.de.debian.org/debian/pool/main/libp/libpng/libpng_{PV}.orig.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libpng12 = "libpng12"

FILES_libpng12 = \
/usr/lib/libpng12.so*

$(DEPDIR)/libpng12: bootstrap $(DEPENDS_libpng12)
	$(PREPARE_libpng12)
	$(start_build)
	cd $(DIR_libpng12); \
		./autogen.sh; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		export ECHO="echo"; \
		echo "Echo cmd =" $(ECHO); \
		$(MAKE) all; \
		sed -e "s,^prefix=,prefix=$(PKDIR)," < libpng-config > $(crossprefix)/bin/libpng-config; \
		chmod 755 $(crossprefix)/bin/libpng-config; \
		$(INSTALL_libpng12)
		rm -f $(PKDIR)/usr/bin/libpng*-config
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libpng12)
	touch $@

#
# libpng
#
BEGIN[[
libpng
  1.5.16
  {PN}-{PV}
  extract:http://prdownloads.sourceforge.net/libpng/{PN}/{PN}-{PV}.tar.xz
  nothing:file://{PN}.diff
  patch:file://{PN}-{PV}-workaround_for_stmfb_alpha_error.patch
  make:install:prefix=PKDIR/usr
;
]]END

DESCRIPTION_libpng = "libpng"

FILES_libpng = \
/usr/lib/*.so*

$(DEPDIR)/libpng: bootstrap $(DEPENDS_libpng)
	$(PREPARE_libpng)
	$(start_build)
	cd $(DIR_libpng); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--enable-maintainer-mode \
			--prefix=/usr; \
		export ECHO="echo"; \
		echo "Echo cmd =" $(ECHO); \
		$(MAKE) all; \
		sed -e "s,^prefix=,prefix=$(PKDIR)," < libpng-config > $(crossprefix)/bin/libpng-config; \
		chmod 755 $(crossprefix)/bin/libpng-config; \
		$(INSTALL_libpng)
		rm -f $(PKDIR)/usr/bin/libpng*-config
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libpng)
	touch $@

#
# libungif
#
BEGIN[[
libungif
  4.1.4
  {PN}-{PV}
  extract:http://heanet.dl.sourceforge.net/sourceforge/giflib/{PN}-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libungif = "libungif"

FILES_libungif = \
/usr/lib/*.so*

$(DEPDIR)/libungif: bootstrap $(DEPENDS_libungif)
	$(PREPARE_libungif)
	$(start_build)
	cd $(DIR_libungif); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--without-x; \
		$(MAKE); \
		$(INSTALL_libungif)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libungif)
	touch $@

#
# libgif
#
BEGIN[[
libgif
  5.0.4
  giflib-{PV}
  extract:http://heanet.dl.sourceforge.net/sourceforge/giflib/giflib-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libgif = "libgif"

FILES_libgif = \
/usr/lib/*.so*

$(DEPDIR)/libgif: bootstrap $(DEPENDS_libgif)
	$(PREPARE_libgif)
	$(start_build)
	cd $(DIR_libgif); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--without-x; \
		$(MAKE); \
		$(INSTALL_libgif)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libgif)
	touch $@

#
# libcurl
#
BEGIN[[
curl
  7.30.0
  {PN}-{PV}
  extract:http://{PN}.haxx.se/download/{PN}-{PV}.tar.bz2
  patch:file://lib{PN}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_curl = "Curl is a command line tool for transferring data specified with URL syntax"

FILES_curl = \
/usr/lib/*.so* \
/usr/bin/curl

$(DEPDIR)/curl: bootstrap openssl rtmpdump $(DEPENDS_curl)
	$(PREPARE_curl)
	$(start_build)
	cd $(DIR_curl); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--with-ssl \
			--disable-debug \
			--disable-verbose \
			--disable-manual \
			--mandir=/usr/share/man \
			--with-random; \
		$(MAKE) all; \
		sed -e "s,^prefix=,prefix=$(targetprefix)," < curl-config > $(crossprefix)/bin/curl-config; \
		chmod 755 $(crossprefix)/bin/curl-config; \
		$(INSTALL_curl)
		rm -f $(PKDIR)/usr/bin/curl-config
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_curl)
	touch $@

#
# libfribidi
#
BEGIN[[
libfribidi
  0.19.5
  fribidi-{PV}
  extract:http://fribidi.org/download/fribidi-{PV}.tar.bz2
  patch:file://glib.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libfribidi = "libfribidi"

FILES_libfribidi = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/libfribidi: bootstrap $(DEPENDS_libfribidi)
	$(PREPARE_libfribidi)
	$(start_build)
	cd $(DIR_libfribidi); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
		$(MAKE) all; \
		$(INSTALL_libfribidi)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libfribidi)
	touch $@

#
# libsigc
#
BEGIN[[
libsigc
  1.2.7
  {PN}++-{PV}
  extract:http://ftp.gnome.org/pub/GNOME/sources/{PN}++/1.2/{PN}++-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libsigc = "libsigc"

FILES_libsigc = \
/usr/lib/*.so*

$(DEPDIR)/libsigc: bootstrap libstdc++-dev $(DEPENDS_libsigc)
	$(PREPARE_libsigc)
	$(start_build)
	cd $(DIR_libsigc); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-checks; \
		$(MAKE) all; \
		$(INSTALL_libsigc)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libsigc)
	touch $@

#
# libmad
#
BEGIN[[
libmad
  0.15.1b
  {PN}-{PV}
  extract:ftp://ftp.mars.org/pub/mpeg/{PN}-{PV}.tar.gz
  patch:file://{PN}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libmad = "libmad - MPEG audio decoder library"

FILES_libmad = \
/usr/lib/*.so*

$(DEPDIR)/libmad: bootstrap $(DEPENDS_libmad)
	$(PREPARE_libmad)
	$(start_build)
	cd $(DIR_libmad); \
		aclocal -I $(hostprefix)/share/aclocal; \
		autoconf; \
		autoheader; \
		automake --foreign; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-debugging \
			--enable-shared=yes \
			--enable-speed \
			--enable-sso; \
		$(MAKE) all; \
		$(INSTALL_libmad)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libmad)
	touch $@

#
# libid3tag
#
BEGIN[[
libid3tag
  0.15.1b
  {PN}-{PV}
  extract:ftp://ftp.mars.org/pub/mpeg/{PN}-{PV}.tar.gz
  patch:file://{PN}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libid3tag = "libid3tag"

FILES_libid3tag = \
/usr/lib/*.so*

$(DEPDIR)/libid3tag: bootstrap $(DEPENDS_libid3tag)
	$(PREPARE_libid3tag)
	$(start_build)
	cd $(DIR_libid3tag); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--enable-shared=yes; \
		$(MAKE) all; \
		$(INSTALL_libid3tag)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libid3tag)
	touch $@

#
# libvorbisidec
#
BEGIN[[
libvorbisidec
  1.0.2+svn16259
  {PN}-{PV}
  extract:http://ftp.debian.org/debian/pool/main/libv/{PN}/{PN}_{PV}.orig.tar.gz
  patch:file://tremor.diff
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libvorbisidec = "libvorbisidec"

FILES_libvorbisidec = \
/usr/lib/*.so*

$(DEPDIR)/libvorbisidec: bootstrap $(DEPENDS_libvorbisidec)
	$(PREPARE_libvorbisidec)
	$(start_build)
	cd $(DIR_libvorbisidec); \
		$(BUILDENV) \
		./autogen.sh \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libvorbisidec)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libvorbisidec)
	touch $@

#
# libffi
#
BEGIN[[
libffi
  3.0.13
  {PN}-{PV}
  extract:ftp://sourceware.org/pub/{PN}/{PN}-{PV}.tar.gz
  patch:file://libffi-3.0.11.patch
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libffi = libffi

FILES_libffi = \
/usr/lib/*.so*

$(DEPDIR)/libffi: bootstrap libjpeg lcms $(DEPENDS_libffi)
	$(PREPARE_libffi)
	$(start_build)
	cd $(DIR_libffi); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--disable-static \
			--enable-builddir=libffi \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libffi)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libffi)
	touch $@

#
# libglib2
# You need libglib2.0-dev on host system
#
BEGIN[[
glib2
  2.37.1
  glib-{PV}
  extract:http://ftp.acc.umu.se/pub/GNOME/sources/glib/2.37/glib-{PV}.tar.xz
  patch:file://glib-{PV}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_glib2 = "libglib2"

FILES_glib2 = \
/usr/lib/*.so*

DESCRIPTION_glib2 = "libglib2"

FILES_glib2 = \
/usr/lib/*.so*

$(DEPDIR)/glib2: bootstrap libffi $(DEPENDS_glib2)
	$(PREPARE_glib2)
	$(start_build)
	echo "glib_cv_va_copy=no" > $(DIR_glib2)/config.cache
	echo "glib_cv___va_copy=yes" >> $(DIR_glib2)/config.cache
	echo "glib_cv_va_val_copy=yes" >> $(DIR_glib2)/config.cache
	echo "ac_cv_func_posix_getpwuid_r=yes" >> $(DIR_glib2)/config.cache
	echo "ac_cv_func_posix_getgrgid_r=yes" >> $(DIR_glib2)/config.cache
	echo "glib_cv_stack_grows=no" >> $(DIR_glib2)/config.cache
	echo "glib_cv_uscore=no" >> $(DIR_glib2)/config.cache
	cd $(DIR_glib2); \
		$(BUILDENV) \
		PKG_CONFIG=$(hostprefix)/bin/pkg-config \
		./configure \
			--cache-file=config.cache \
			--disable-gtk-doc \
			--with-threads="posix" \
			--enable-static \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--mandir=/usr/share/man; \
		$(MAKE) all; \
		$(INSTALL_glib2)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_glib2)
	touch $@

#
# libiconv
#
BEGIN[[
libiconv
  1.14
  {PN}-{PV}
  extract:http://ftp.gnu.org/gnu/{PN}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libiconv = "libiconv"

FILES_libiconv = \
/usr/lib/*.so* \
/usr/bin/iconv

$(DEPDIR)/libiconv: bootstrap $(DEPENDS_libiconv)
	$(PREPARE_libiconv)
	$(start_build)
	cd $(DIR_libiconv); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		cp ./srcm4/* $(hostprefix)/share/aclocal/; \
		$(INSTALL_libiconv)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libiconv)
	touch $@

#
# libmng
#
BEGIN[[
libmng
  1.0.10
  {PN}-{PV}
  extract:http://dfn.dl.sourceforge.net/sourceforge/{PN}/{PN}-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libmng = "libmng - Multiple-image Network Graphics"

FILES_libmng = \
/usr/lib/*.so*

$(DEPDIR)/libmng: bootstrap libjpeg lcms $(DEPENDS_libmng)
	$(PREPARE_libmng)
	$(start_build)
	cd $(DIR_libmng); \
		cat unmaintained/autogen.sh | tr -d \\r > autogen.sh && chmod 755 autogen.sh; \
		[ ! -x ./configure ] && ./autogen.sh --help || true; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--enable-shared \
			--enable-static \
			--with-zlib \
			--with-jpeg \
			--with-gnu-ld \
			--with-lcms; \
		$(MAKE); \
		$(INSTALL_libmng)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libmng)
	touch $@
#
# lcms
#
BEGIN[[
lcms
  2.5
  lcms2-{PV}
  extract:http://sourceforge.net/projects/lcms/files/lcms/{PV}/lcms2-{PV}.tar.gz
  patch:file://{PN}2.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_lcms = "lcms"

FILES_lcms = \
/usr/lib/*

$(DEPDIR)/lcms: bootstrap libjpeg $(DEPENDS_lcms)
	$(PREPARE_lcms)
	$(start_build)
	cd $(DIR_lcms); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--enable-shared \
			--enable-static; \
		$(MAKE); \
		$(INSTALL_lcms)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_lcms)
	touch $@
#
# directfb
#
BEGIN[[
directfb
  1.4.11
  DirectFB-{PV}
  extract:http://{PN}.org/downloads/Core/DirectFB-1.4/DirectFB-{PV}.tar.gz
  patch:file://{PN}-{PV}+STM2010.12.15-4.diff
  patch:file://{PN}-{PV}+STM2010.12.15-4.no-vt.diff
  patch:file://{PN}-libpng.diff
  patch:file://{PN}-{PV}+STM2010.12.15-4.enigma2remote.diff
  make:install:DESTDIR=PKDIR:LD=sh4-linux-ld
;
]]END

DESCRIPTION_directfb = "directfb"

FILES_directfb = \
/usr/lib/*.so* \
/usr/lib/directfb-1.4-5/gfxdrivers/*.so* \
/usr/lib/directfb-1.4-5/inputdrivers/*.so* \
/usr/lib/directfb-1.4-5/interfaces/*.so* \
/usr/lib/directfb-1.4-5/systems/libdirectfb_stmfbdev.so \
/usr/lib/directfb-1.4-5/wm/*.so* \
/usr/bin/*

$(DEPDIR)/directfb: bootstrap libfreetype $(DEPENDS_directfb)
	$(PREPARE_directfb)
	$(start_build)
	cd $(DIR_directfb); \
		cp $(hostprefix)/share/libtool/config/ltmain.sh .; \
		cp $(hostprefix)/share/libtool/config/ltmain.sh ..; \
		libtoolize -f -c; \
		autoreconf --verbose --force --install -I$(hostprefix)/share/aclocal; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--enable-static \
			--disable-sdl \
			--disable-x11 \
			--disable-devmem \
			--disable-multi \
			--with-gfxdrivers=stgfx \
			--with-inputdrivers=linuxinput,enigma2remote \
			--without-software \
			--enable-stmfbdev \
			--disable-fbdev \
			--enable-mme=yes; \
			export top_builddir=`pwd`; \
		$(MAKE); \
		$(INSTALL_directfb)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_directfb)
	touch $@

#
# DFB++
#
BEGIN[[
dfbpp
  1.2.0
  DFB++-{PV}
  extract:http://www.directfb.org/downloads/Extras/DFB++-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_dfbpp = ""

FILES_dfbpp = \
/usr/lib/*.so*

$(DEPDIR)/dfbpp: bootstrap libjpeg directfb $(DEPENDS_dfbpp)
	$(PREPARE_dfbpp)
	$(start_build)
	cd $(DIR_dfbpp); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_dfbpp)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_dfbpp)
	touch $@

#
# LIBSTGLES
#
BEGIN[[
libstgles
  git
  {PN}-{PV}
  plink:../apps/misc/tools/{PN}:{PN}-{PV}
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libstgles = "libstgles"
SRC_URI_libstgles = "https://code.google.com/p/tdt-amiko/"
PKGR_libstgles =r1
FILES_libstgles = \
/usr/lib/*

$(DEPDIR)/libstgles: bootstrap directfb $(DEPENDS_libstgles)
	$(PREPARE_libstgles)
	$(start_build)
	cd $(DIR_libstgles); \
		cp --remove-destination $(hostprefix)/share/libtool/config/ltmain.sh .; \
		aclocal -I $(hostprefix)/share/aclocal; \
		autoconf; \
		automake --foreign --add-missing; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) $(MAKE_OPTS); \
		$(INSTALL_libstgles)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libstgles)
	touch $@

#
# libexpat
#
BEGIN[[
libexpat
  2.1.0
  expat-{PV}
  extract:http://prdownloads.sourceforge.net/sourceforge/expat/expat-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libexpat = "Expat is an XML parser library written in C. It is a stream-oriented parser in which an application registers handlers for things the parser might find in the XML document"

FILES_libexpat = \
/usr/lib/libexpat.so* \
/usr/bin/xmlwf

$(DEPDIR)/libexpat: bootstrap $(DEPENDS_libexpat)
	$(PREPARE_libexpat)
	$(start_build)
	cd $(DIR_libexpat); \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libexpat)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libexpat)
	touch $@

#
# fontconfig
#
BEGIN[[
fontconfig
  2.10.95
  {PN}-{PV}
  extract:http://{PN}.org/release/{PN}-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_fontconfig = "Fontconfig is a library for configuring and customizing font access."

FILES_fontconfig = \
/etc \
/usr/lib/*

$(DEPDIR)/fontconfig: bootstrap libexpat libfreetype $(DEPENDS_fontconfig)
	$(PREPARE_fontconfig)
	$(start_build)
	cd $(DIR_fontconfig); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--with-arch=sh4 \
			--with-freetype-config=$(crossprefix)/bin/freetype-config \
			--with-expat-includes=$(targetprefix)/usr/include \
			--with-expat-lib=$(targetprefix)/usr/lib \
			--sysconfdir=/etc \
			--localstatedir=/var \
			--disable-docs \
			--without-add-fonts; \
		$(MAKE); \
		$(INSTALL_fontconfig)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_fontconfig)
	touch $@

#
# libxmlccwrap
#
BEGIN[[
libxmlccwrap
  0.0.12
  {PN}-{PV}
  extract:http://www.ant.uni-bremen.de/whomes/rinas/{PN}/download/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libxmlccwrap = "libxmlccwrap is a small C++ wrapper around libxml2 and libxslt "

FILES_libxmlccwrap = \
/usr/lib/*.so*

$(DEPDIR)/libxmlccwrap: bootstrap libxslt $(DEPENDS_libxmlccwrap)
	$(PREPARE_libxmlccwrap)
	$(start_build)
	cd $(DIR_libxmlccwrap); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libxmlccwrap); \
		sed -e "/^dependency_libs/ s,-L/usr/lib,-L$(PKDIR)/usr/lib,g" -i $(PKDIR)/usr/lib/libxmlccwrap.la; \
		sed -e "/^dependency_libs/ s, /usr/lib, $(PKDIR)/usr/lib,g" -i $(PKDIR)/usr/lib/libxmlccwrap.la
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libxmlccwrap)
	touch $@

#
# a52dec
#
BEGIN[[
a52dec
  0.7.4
  {PN}-{PV}
  extract:http://liba52.sourceforge.net/files/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_a52dec = "liba52 is a free library for decoding ATSC A/52 streams. It is released under the terms of the GPL license"

FILES_a52dec = \
/usr/lib/*

$(DEPDIR)/a52dec: bootstrap $(DEPENDS_a52dec)
	$(PREPARE_a52dec)
	$(start_build)
	cd $(DIR_a52dec); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_a52dec)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_a52dec)
	touch $@

#
# libdvdcss
#
BEGIN[[
libdvdcss
  1.2.13
  {PN}-{PV}
  extract:http://download.videolan.org/pub/{PN}/{PV}/{PN}-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libdvdcss = "libdvdcss"

FILES_libdvdcss = \
/usr/lib/libdvdcss.so*

$(DEPDIR)/libdvdcss: bootstrap $(DEPENDS_libdvdcss)
	$(PREPARE_libdvdcss)
	$(start_build)
	cd $(DIR_libdvdcss); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--disable-doc \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libdvdcss)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdvdcss)
	touch $@

#
# libdvdnav
#
BEGIN[[
libdvdnav
  4.2.0
  {PN}-{PV}
  extract:http://dvdnav.mplayerhq.hu/releases/{PN}-{PV}.tar.bz2
  patch:file://{PN}_{PV}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libdvdnav = "libdvdnav"

FILES_libdvdnav = \
/usr/lib/*.so* \
/usr/bin/dvdnav-config

$(DEPDIR)/libdvdnav: bootstrap libdvdread $(DEPENDS_libdvdnav)
	$(PREPARE_libdvdnav)
	$(start_build)
	cd $(DIR_libdvdnav); \
		$(BUILDENV) \
		cp $(hostprefix)/share/libtool/config/ltmain.sh .; \
		autoreconf --verbose --force --install -I$(hostprefix)/share/aclocal; \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--enable-static \
			--enable-shared \
			--with-dvdread-config=$(crossprefix)/bin/dvdread-config; \
		$(MAKE) all; \
		sed -e "s,^prefix=,prefix=$(targetprefix)," < misc/dvdnav-config > $(crossprefix)/bin/dvdnav-config; \
		chmod 755 $(crossprefix)/bin/dvdnav-config; \
		$(INSTALL_libdvdnav)
		rm -f $(targetprefix)/usr/bin/dvdnav-config
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdvdnav)
	touch $@

#
# libdvdread
#
BEGIN[[
libdvdread
  4.2.0
  {PN}-{PV}
  extract:http://dvdnav.mplayerhq.hu/releases/{PN}-{PV}.tar.bz2
  patch:file://{PN}_{PV}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libdvdread = "libdvdread"

FILES_libdvdread = \
/usr/lib/*.so* \
/usr/bin/dvdread-config

$(DEPDIR)/libdvdread: bootstrap $(DEPENDS_libdvdread)
	$(PREPARE_libdvdread)
	$(start_build)
	cd $(DIR_libdvdread); \
		cp $(hostprefix)/share/libtool/config/ltmain.sh .; \
		cp $(hostprefix)/share/libtool/config/ltmain.sh ..; \
		autoreconf -f -i -I$(hostprefix)/share/aclocal; \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--enable-static \
			--enable-shared \
			--prefix=/usr; \
		$(MAKE) all; \
		sed -e "s,^prefix=,prefix=$(targetprefix)," < misc/dvdread-config > $(crossprefix)/bin/dvdread-config; \
		chmod 755 $(crossprefix)/bin/dvdread-config; \
		$(INSTALL_libdvdread)
		rm -f $(targetprefix)/usr/bin/dvdread-config
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdvdread)
	touch $@

#
# ffmpeg
#
BEGIN[[
ffmpeg
  2.0.1
  {PN}-{PV}
  extract:http://{PN}.org/releases/{PN}-{PV}.tar.bz2
  patch:file://{PN}-{PV}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_ffmpeg = "ffmpeg"

FILES_ffmpeg = \
/usr/lib/*.so* \
/sbin/ffmpeg

$(DEPDIR)/ffmpeg: bootstrap libass libaacplus libfaac rtmpdump libx264 $(DEPENDS_ffmpeg)
	$(PREPARE_ffmpeg)
	$(start_build)
	cd $(DIR_ffmpeg); \
		PKG_CONFIG_PATH=$(targetprefix)/usr/lib/pkgconfig \
		./configure \
			--disable-vfp \
			--disable-runtime-cpudetect \
			--disable-static \
			--disable-doc \
			--disable-htmlpages \
			--disable-manpages \
			--disable-podpages \
			--disable-txtpages \
			--disable-fast-unaligned \
			--disable-bsfs \
			--enable-libaacplus \
			--enable-libfaac \
			--enable-nonfree \
			--enable-libass \
			--enable-libx264 \
			--enable-gpl \
			--enable-version3 \
			--enable-shared \
			--enable-cross-compile \
			--enable-librtmp \
			--enable-openssl \
			--disable-ffserver \
			--disable-ffplay \
			--disable-ffprobe \
			--disable-debug \
			--disable-asm \
			--disable-altivec \
			--disable-amd3dnow \
			--disable-amd3dnowext \
			--disable-mmx \
			--disable-mmxext \
			--disable-sse \
			--disable-sse2 \
			--disable-sse3 \
			--disable-ssse3 \
			--disable-sse4 \
			--disable-sse42 \
			--disable-avx \
			--disable-fma4 \
			--disable-armv5te \
			--disable-armv6 \
			--disable-armv6t2 \
			--disable-neon \
			--disable-vis \
			--disable-inline-asm \
			--disable-yasm \
			--disable-mips32r2 \
			--disable-mipsdspr1 \
			--disable-mipsdspr2 \
			--disable-mipsfpu \
			--disable-indevs \
			--disable-outdevs \
			--disable-muxers \
			--enable-muxer=ogg \
			--enable-muxer=flac \
			--enable-muxer=mp3 \
			--enable-muxer=h261 \
			--enable-muxer=h263 \
			--enable-muxer=h264 \
			--enable-muxer=mpeg1video \
			--enable-muxer=mpeg2video \
			--enable-muxer=image2 \
			--disable-encoders \
			--enable-encoder=aac \
			--enable-encoder=h261 \
			--enable-encoder=h263 \
			--enable-encoder=h263p \
			--enable-encoder=ljpeg \
			--enable-encoder=mjpeg \
			--enable-encoder=png \
			--enable-encoder=mpeg4 \
			--enable-encoder=mpeg1video \
			--enable-encoder=mpeg2video \
			--disable-decoders \
			--enable-decoder=aac \
			--enable-decoder=mp3 \
			--enable-decoder=theora \
			--enable-decoder=h261 \
			--enable-decoder=h263 \
			--enable-decoder=h263i \
			--enable-decoder=h264 \
			--enable-decoder=mpeg1video \
			--enable-decoder=mpeg2video \
			--enable-decoder=mpeg4 \
			--enable-decoder=png \
			--enable-decoder=mjpeg \
			--enable-decoder=vorbis \
			--enable-demuxer=wav \
			--enable-decoder=wmv3 \
			--enable-decoder=pcm_s16le \
			--enable-decoder=flac \
			--enable-parser=h264 \
			--enable-parser=mjpeg \
			--enable-demuxer=mjpeg \
			--enable-demuxer=rtsp \
			--enable-decoder=dvbsub \
			--enable-decoder=iff_byterun1 \
			--enable-small \
			--enable-avresample \
			--enable-pthreads \
			--enable-bzlib \
			--enable-zlib \
			--pkg-config="pkg-config" \
			--cross-prefix=$(target)- \
			--target-os=linux \
			--arch=sh4 \
			--extra-cflags="-fno-strict-aliasing" \
			--enable-stripping \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_ffmpeg)
	$(tocdk_build)
	mv $(PKDIR)/usr/bin $(PKDIR)/sbin
	$(toflash_build)
	$(DISTCLEANUP_ffmpeg)
	touch $@

#
# libass
#
BEGIN[[
libass
  0.10.1
  {PN}-{PV}
  extract:http://{PN}.googlecode.com/files/{PN}-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libass = "libass"

FILES_libass = \
/usr/lib/*.so*

$(DEPDIR)/libass: bootstrap libfreetype libfribidi $(DEPENDS_libass)
	$(PREPARE_libass)
	$(start_build)
	cd $(DIR_libass); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--disable-fontconfig \
			--disable-enca \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libass)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libass)
	touch $@

#
# WebKitDFB
#
BEGIN[[
webkitdfb
  2010-11-18
  {PN}_{PV}
  extract:http://www.duckbox.info/files/packages/{PN}_{PV}.tar.gz
  patch:file://{PN}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_webkitdfb = "webkitdfb"
RDEPENDS_webkitdfb = lite enchant fontconfig sqlite cairo enchant
FILES_webkitdfb = \
/usr/lib*

$(DEPDIR)/webkitdfb: bootstrap glib2 icu4c libxml2 enchant lite curl fontconfig sqlite libsoup cairo libjpeg $(DEPENDS_webkitdfb)
	$(PREPARE_webkitdfb)
	$(start_build)
	export PATH=$(buildprefix)/$(DIR_icu4c)/host/config:$(PATH); \
	cd $(DIR_webkitdfb); \
		$(BUILDENV) \
		./autogen.sh \
			--with-target=directfb \
			--without-gtkplus \
			--host=$(target) \
			--prefix=/usr \
			--with-cairo-directfb \
			--disable-shared-workers \
			--enable-optimizations \
			--disable-channel-messaging \
			--disable-javascript-debugger \
			--enable-offline-web-applications \
			--enable-dom-storage \
			--enable-database \
			--disable-eventsource \
			--enable-icon-database \
			--enable-datalist \
			--disable-video \
			--enable-svg \
			--enable-xpath \
			--disable-xslt \
			--disable-dashboard-support \
			--disable-geolocation \
			--disable-workers \
			--disable-web-sockets \
			--with-networking-backend=soup; \
		$(MAKE); \
		$(INSTALL_webkitdfb)
	$(tocdk_build)
	$(e2extra_build)
	$(DISTCLEANUP_webkitdfb)
	touch $@

#
# icu4c
#
BEGIN[[
icu4c
  4_4_1
  icu/source
  extract:http://download.icu-project.org/files/{PN}/4.4.1/{PN}-4_4_1-src.tgz
  nothing:file://{PN}-4_4_1_locales.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_icu4c = "icu4c"

FILES_icu4c = \
/usr/lib/*.so* \
/usr/bin/* \
/usr/sbin/*

$(DEPDIR)/icu4c: bootstrap $(DEPENDS_icu4c)
	$(PREPARE_icu4c)
	$(start_build)
	cd $(DIR_icu4c); \
		rm data/mappings/ucm*.mk; \
		patch -p1 < $(buildprefix)/Patches/icu4c-4_4_1_locales.patch;
		echo "Building host icu"
		mkdir -p $(DIR_icu4c)/host; \
		cd $(DIR_icu4c)/host; \
		sh ../configure --disable-samples --disable-tests; \
		unset TARGET; \
		make
		echo "Building cross icu"
		cd $(DIR_icu4c); \
		$(BUILDENV) \
		./configure \
			--with-cross-build=$(buildprefix)/$(DIR_icu4c)/host \
			--host=$(target) \
			--prefix=/usr \
			--disable-extras \
			--disable-layout \
			--disable-tests \
			--disable-samples; \
		unset TARGET; \
		$(INSTALL_icu4c)
	$(tocdk_build)
	$(e2extra_build)
	$(DISTCLEANUP_icu4c)
	touch $@

#
# enchant
#
BEGIN[[
enchant
  1.5.0
  {PN}-{PV}
  extract:http://www.abisource.com/downloads/{PN}/{PV}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_enchant = "libenchant -- Generic spell checking library"

FILES_enchant = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/enchant: bootstrap $(DEPENDS_enchant)
	$(PREPARE_enchant)
	$(start_build)
	cd $(DIR_enchant); \
		libtoolize -f -c; \
		autoreconf --verbose --force --install -I$(hostprefix)/share/aclocal; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--disable-aspell \
			--disable-ispell \
			--disable-myspell \
			--disable-zemberek \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) LD=$(target)-ld; \
		$(INSTALL_enchant)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_enchant)
	touch $@

#
# lite
#
BEGIN[[
lite
  0.9.0
  {PN}-{PV}+git0.7982ccc
  extract:http://www.duckbox.info/files/packages/{PN}-{PV}+git0.7982ccc.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_lite = "LiTE is a Toolkit Engine"

FILES_lite = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/lite: bootstrap directfb $(DEPENDS_lite)
	$(PREPARE_lite)
	$(start_build)
	cd $(DIR_lite); \
		cp $(hostprefix)/share/libtool/config/ltmain.sh ..; \
		libtoolize -f -c; \
		autoreconf --verbose --force --install -I$(hostprefix)/share/aclocal; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--disable-debug; \
		$(MAKE); \
		$(INSTALL_lite)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_lite)
	touch $@

#
# sqlite
#
BEGIN[[
sqlite
  3.8.0
  {PN}-autoconf-3080002
  extract:http://www.sqlite.org/2013/sqlite-autoconf-3080002.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_sqlite = "sqlite"

FILES_sqlite = \
/usr/lib/*.so* \
/usr/bin/sqlite3

$(DEPDIR)/sqlite: bootstrap $(DEPENDS_sqlite)
	$(PREPARE_sqlite)
	$(start_build)
	cd $(DIR_sqlite); \
		$(BUILDENV) \
		libtoolize -f -c; \
		autoreconf --verbose --force --install -I$(hostprefix)/share/aclocal; \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--disable-tcl \
			--disable-debug \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_sqlite)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_sqlite)
	touch $@

#
# libsoup
#
BEGIN[[
libsoup
  2.43.90
  {PN}-{PV}
  extract:http://ftp.acc.umu.se/pub/GNOME/sources/libsoup/2.43/{PN}-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libsoup = "libsoup is an HTTP client/server library"

FILES_libsoup = \
/usr/lib/*.so*

$(DEPDIR)/libsoup: bootstrap $(DEPENDS_libsoup)
	$(PREPARE_libsoup)
	$(start_build)
	cd $(DIR_libsoup); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--disable-more-warnings \
			--without-gnome; \
		$(MAKE); \
		$(INSTALL_libsoup)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libsoup)
	touch $@

#
# pixman
#
BEGIN[[
pixman
  0.30.2
  {PN}-{PV}
  extract:http://cairographics.org/releases/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_pixman = "pixman is a library that provides low-level pixel manipulation"

FILES_pixman = \
/usr/lib/*.so*

$(DEPDIR)/pixman: bootstrap $(DEPENDS_pixman)
	$(PREPARE_pixman)
	$(start_build)
	cd $(DIR_pixman); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_pixman)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_pixman)
	touch $@

#
# cairo
#
BEGIN[[
cairo
  1.12.16
  {PN}-{PV}
  extract:http://cairographics.org/releases/{PN}-{PV}.tar.xz
  patch:file://{PN}-{PV}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_cairo = "Cairo - Multi-platform 2D graphics library"

FILES_cairo = \
/usr/lib/*.so*

$(DEPDIR)/cairo: bootstrap libpng pixman $(DEPENDS_cairo)
	$(PREPARE_cairo)
	$(start_build)
	cd $(DIR_cairo); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--disable-gtk-doc \
			--enable-ft=yes \
			--enable-png=yes \
			--enable-ps=no \
			--enable-pdf=no \
			--enable-svg=no \
			--disable-glitz \
			--disable-xcb \
			--disable-xlib \
			--enable-directfb \
			--program-suffix=-directfb; \
		$(MAKE); \
		$(INSTALL_cairo)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_cairo)
	touch $@

#
# libogg
#
BEGIN[[
libogg
  1.3.1
  {PN}-{PV}
  extract:http://downloads.xiph.org/releases/ogg/{PN}-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libogg = "distribution includes libogg and nothing else"

FILES_libogg = \
/usr/lib/*.so*

$(DEPDIR)/libogg: bootstrap $(DEPENDS_libogg)
	$(PREPARE_libogg)
	$(start_build)
	cd $(DIR_libogg); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libogg)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libogg)
	touch $@

#
# libflac
#
BEGIN[[
libflac
  1.2.1
  flac-{PV}
  extract:http://downloads.sourceforge.net/flac/flac-{PV}.tar.gz
  patch:file://flac-{PV}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libflac = "libflac is Open Source lossless audio codec"

FILES_libflac = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/libflac: bootstrap $(DEPENDS_libflac)
	$(PREPARE_libflac)
	$(start_build)
	cd $(DIR_libflac); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--disable-ogg \
			--disable-oggtest \
			--disable-id3libtest \
			--disable-asm-optimizations \
			--disable-doxygen-docs \
			--disable-xmms-plugin \
			--without-xmms-prefix \
			--without-xmms-exec-prefix \
			--without-libiconv-prefix \
			--without-id3lib \
			--with-ogg-includes=. \
			--disable-cpplibs; \
		$(MAKE); \
		$(INSTALL_libflac)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libflac)
	touch $@


##############################   PYTHON   #####################################

#
# elementtree
#
BEGIN[[
elementtree
  1.2.6-20050316
  {PN}-{PV}
  extract:http://effbot.org/media/downloads/{PN}-{PV}.tar.gz
  patch:file://elementtree3.patch
;
]]END

DESCRIPTION_elementtree = "Provides light-weight components for working with XML"
FILES_elementtree = \
$(PYTHON_DIR)

$(DEPDIR)/elementtree: bootstrap $(DEPENDS_elementtree)
	$(PREPARE_elementtree)
	$(start_build)
	cd $(DIR_elementtree); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_elementtree)
	touch $@

#
# libxml2
#
BEGIN[[
libxml2
  2.9.1
  {PN}-{PV}
  extract:http://xmlsoft.org/sources/{PN}-{PV}.tar.gz
  patch:file://{PN}-{PV}.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libxml2 = "XML parsing library, version 2"
FILES_libxml2 = \
/usr/lib/libxml2* \
$(PYTHON_DIR)/site-packages/*libxml2.py

$(DEPDIR)/libxml2: bootstrap $(DEPENDS_libxml2)
	$(PREPARE_libxml2)
	$(start_build)
	cd $(DIR_libxml2); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--with-python=$(hostprefix) \
			--without-c14n \
			--without-debug \
			--without-mem-debug; \
		$(MAKE) all; \
		$(INSTALL_libxml2); \
		sed -e "s,^prefix=,prefix=$(targetprefix)," < xml2-config > $(crossprefix)/bin/xml2-config; \
		chmod 755 $(crossprefix)/bin/xml2-config
	$(tocdk_build_start)
		sed -e "/^XML2_LIBDIR/ s,/usr/lib,$(targetprefix)/usr/lib,g" -i $(ipkgbuilddir)/libxml2/usr/lib/xml2Conf.sh; \
		sed -e "/^XML2_INCLUDEDIR/ s,/usr/include,$(targetprefix)/usr/include,g" -i $(ipkgbuilddir)/libxml2/usr/lib/xml2Conf.sh
	$(call do_build_pkg,install,cdk)
	$(toflash_build)
	$(DISTCLEANUP_libxml2)
	touch $@

#
# libxslt
#
BEGIN[[
libxslt
  1.1.28
  {PN}-{PV}
  extract:http://xmlsoft.org/sources/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libxslt = "XML stylesheet transformation library"
FILES_libxslt = \
/usr/lib/libxslt* \
/usr/lib/libexslt* \
$(PYTHON_DIR)/site-packages/libxslt.py

$(DEPDIR)/libxslt: bootstrap libxml2 $(DEPENDS_libxslt)
	$(PREPARE_libxslt)
	$(start_build)
	cd $(DIR_libxslt); \
		$(BUILDENV) \
		CPPFLAGS="$(CPPFLAGS) -I$(targetprefix)/usr/include/libxml2" \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--with-libxml-prefix="$(crossprefix)" \
			--with-libxml-include-prefix="$(targetprefix)/usr/include" \
			--with-libxml-libs-prefix="$(targetprefix)/usr/lib" \
			--with-python=$(hostprefix) \
			--without-crypto \
			--without-debug \
			--without-mem-debug; \
		$(MAKE) all; \
		$(INSTALL_libxslt); \
		sed -e "s,^prefix=,prefix=$(targetprefix)," < xslt-config > $(crossprefix)/bin/xslt-config; \
		chmod 755 $(crossprefix)/bin/xslt-config
	$(tocdk_build_start)
	sed -e "/^dependency_libs/ s,/usr/lib/libxslt.la,$(targetprefix)/usr/lib/libxslt.la,g" -i $(ipkgbuilddir)/usr/lib/libexslt.la; \
	sed -e "/^XML2_LIBDIR/ s,/usr/lib,$(targetprefix)/usr/lib,g" -i $(ipkgbuilddir)/libxslt/usr/lib/xsltConf.sh; \
	sed -e "/^XML2_INCLUDEDIR/ s,/usr/include,$(targetprefix)/usr/include,g" -i $(ipkgbuilddir)/libxslt/usr/lib/xsltConf.sh
	$(call do_build_pkg,install,cdk)
	$(toflash_build)
	$(DISTCLEANUP_libxslt)
	touch $@

#
# lxml
#
BEGIN[[
lxml
  3.2.3
  {PN}-{PV}
  extract:https://pypi.python.org/packages/source/l/lxml/lxml-3.2.3.tar.gz
;
]]END

DESCRIPTION_lxml = "Python binding for the libxml2 and libxslt libraries"
FILES_lxml = \
$(PYTHON_DIR)

$(DEPDIR)/lxml: bootstrap python $(DEPENDS_lxml)
	$(PREPARE_lxml)
	$(start_build)
	cd $(DIR_lxml); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(extra_build)
	$(DISTCLEANUP_lxml)
	touch $@

#
# setuptools
#
BEGIN[[
ifdef ENABLE_PY332
setuptools
  1.1.4
  {PN}-{PV}
  extract:http://pypi.python.org/packages/source/s/{PN}/{PN}-{PV}.tar.gz
;
else
setuptools
  0.6c11
  {PN}-{PV}
  extract:http://pypi.python.org/packages/source/s/{PN}/{PN}-{PV}.tar.gz
;
endif
]]END

DESCRIPTION_setuptools = "setuptools"

FILES_setuptools = \
$(PYTHON_DIR)/site-packages/*.py \
$(PYTHON_DIR)/site-packages/*.pyo \
$(PYTHON_DIR)/site-packages/setuptools/*.py \
$(PYTHON_DIR)/site-packages/setuptools/*.pyo \
$(PYTHON_DIR)/site-packages/setuptools/command/*.py \
$(PYTHON_DIR)/site-packages/setuptools/command/*.pyo

$(DEPDIR)/setuptools: bootstrap $(DEPENDS_setuptools)
	$(PREPARE_setuptools)
	$(start_build)
	cd $(DIR_setuptools); \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(DISTCLEANUP_setuptools)
	touch $@

#
# gdata
#
BEGIN[[
gdata
  2.0.18
  gdata-{PV}
  extract:http://gdata-python-client.googlecode.com/files/gdata-{PV}.tar.gz
;
]]END

DESCRIPTION_gdata = "The Google Data APIs (Google Data) provide a simple protocol for reading and writing data on the web. Though it is possible to use these services with a simple HTTP client, this library provides helpful tools to streamline your code and keep up with server-side changes. "
FILES_gdata = \
$(PYTHON_DIR)/site-packages/atom/*.py \
$(PYTHON_DIR)/site-packages/atom/*.pyo \
$(PYTHON_DIR)/site-packages/gdata/*.py \
$(PYTHON_DIR)/site-packages/gdata/*.pyo \
$(PYTHON_DIR)/site-packages/gdata/*.pyo \
$(PYTHON_DIR)/site-packages/gdata/youtube/*.py \
$(PYTHON_DIR)/site-packages/gdata/youtube/*.pyo \
$(PYTHON_DIR)/site-packages/gdata/geo/*.py \
$(PYTHON_DIR)/site-packages/gdata/geo/*.pyo \
$(PYTHON_DIR)/site-packages/gdata/media/*.py \
$(PYTHON_DIR)/site-packages/gdata/media/*.pyo \
$(PYTHON_DIR)/site-packages/gdata/oauth/*.py \
$(PYTHON_DIR)/site-packages/gdata/oauth/*.pyo \
$(PYTHON_DIR)/site-packages/gdata/tlslite/*.py \
$(PYTHON_DIR)/site-packages/gdata/tlslite/*.pyo

$(DEPDIR)/gdata: bootstrap setuptools $(DEPENDS_gdata)
	$(PREPARE_gdata)
	$(start_build)
	cd $(DIR_gdata); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) -c "import setuptools; execfile('setup.py')" install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(e2extra_build)
	$(DISTCLEANUP_gdata)
	touch $@
#
# twisted
#
BEGIN[[
twisted
  13.1.0
  Twisted-{PV}
  extract:https://pypi.python.org/packages/source/T/Twisted/Twisted-{PV}.tar.bz2
;
]]END

DESCRIPTION_twisted = "Asynchronous networking framework written in Python"
FILES_twisted = \
$(PYTHON_DIR)/site-packages/twisted/copyright.* \
$(PYTHON_DIR)/site-packages/twisted/cred \
$(PYTHON_DIR)/site-packages/twisted/im.* \
$(PYTHON_DIR)/site-packages/twisted/__init__.* \
$(PYTHON_DIR)/site-packages/twisted/internet \
$(PYTHON_DIR)/site-packages/twisted/persisted \
$(PYTHON_DIR)/site-packages/twisted/plugin.* \
$(PYTHON_DIR)/site-packages/twisted/plugins \
$(PYTHON_DIR)/site-packages/twisted/protocols \
$(PYTHON_DIR)/site-packages/twisted/python \
$(PYTHON_DIR)/site-packages/twisted/spread \
$(PYTHON_DIR)/site-packages/twisted/_version.py \
$(PYTHON_DIR)/site-packages/twisted/_version.pyo \
$(PYTHON_DIR)/site-packages/twisted/web

ifdef ENABLE_PY332
$(DEPDIR)/twisted: bootstrap setuptools $(DEPENDS_twisted)
	$(PREPARE_twisted)
	$(start_build)
	cd $(DIR_twisted); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) -c "import setuptools; exec(open('setup3.py').read())" install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_twisted)
	touch $@
else
$(DEPDIR)/twisted: bootstrap setuptools $(DEPENDS_twisted)
	$(PREPARE_twisted)
	$(start_build)
	cd $(DIR_twisted); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) -c "import setuptools; execfile('setup.py')" install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_twisted)
	touch $@
endif

#
# twistedweb2
#
BEGIN[[
twistedweb2
  8.1.0
  TwistedWeb2-{PV}
  extract:http://twistedmatrix.com/Releases/Web2/8.1/TwistedWeb2-{PV}.tar.bz2
;
]]END

DESCRIPTION_twistedweb2 = "twistedweb2"

FILES_twistedweb2 = \
$(PYTHON_DIR)/site-packages/twisted/*.py \
$(PYTHON_DIR)/site-packages/twisted/*.pyo \
$(PYTHON_DIR)/site-packages/twisted/web2 \
$(PYTHON_DIR)/site-packages/twisted/plugins

$(DEPDIR)/twistedweb2: bootstrap setuptools twisted $(DEPENDS_twistedweb2)
	$(PREPARE_twistedweb2)
	$(start_build)
	cd $(DIR_twistedweb2); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) -c "import setuptools; execfile('setup.py')" install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_twistedweb2)
	touch $@

#
# twistedweb
#
BEGIN[[
twistedweb
  13.1.0
  TwistedWeb-{PV}
  extract:http://twistedmatrix.com/Releases/Web/13.1/TwistedWeb-{PV}.tar.bz2
;
]]END

DESCRIPTION_twistedweb = "twistedweb"

FILES_twistedweb = \
$(PYTHON_DIR)/site-packages/twisted/*.py \
$(PYTHON_DIR)/site-packages/twisted/*.pyo \
$(PYTHON_DIR)/site-packages/twisted/web \
$(PYTHON_DIR)/site-packages/twisted/plugins

ifdef ENABLE_PY332
$(DEPDIR)/twistedweb: bootstrap setuptools $(DEPENDS_twistedweb)
	$(PREPARE_twistedweb)
	$(start_build)
	cd $(DIR_twistedweb); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) -c "import setuptools; exec(open('setup.py').read())" install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_twistedweb)
	touch $@
else
$(DEPDIR)/twistedweb: bootstrap setuptools $(DEPENDS_twistedweb)
	$(PREPARE_twistedweb)
	$(start_build)
	cd $(DIR_twistedweb); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) -c "import setuptools; execfile('setup.py')" install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_twistedweb)
	touch $@
endif

#
# twistedmail
#
BEGIN[[
twistedmail
  13.1.0
  TwistedMail-{PV}
  extract:http://twistedmatrix.com/Releases/Mail/13.1/TwistedMail-{PV}.tar.bz2
;
]]END
DESCRIPTION_twistedmail = "twistedmail"

FILES_twistedmail = \
$(PYTHON_DIR)/site-packages/twisted/*.py \
$(PYTHON_DIR)/site-packages/twisted/*.pyo \
$(PYTHON_DIR)/site-packages/twisted/mail/* \
$(PYTHON_DIR)/site-packages/twisted/plugins/*

$(DEPDIR)/twistedmail: bootstrap setuptools $(DEPENDS_twistedmail)
	$(PREPARE_twistedmail)
	$(start_build)
	cd $(DIR_twistedmail); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) -c "import setuptools; execfile('setup.py')" install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_twistedmail)
	touch $@

#
# pilimaging
#
BEGIN[[
pilimaging
  1.1.7
  Imaging-{PV}
  extract:http://effbot.org/downloads/Imaging-{PV}.tar.gz
  patch:file://pilimaging-fix-search-paths.patch
;
]]END

DESCRIPTION_pilimaging = "pilimaging"
FILES_pilimaging = \
$(PYTHON_DIR)/site-packages \
/usr/bin/*

$(DEPDIR)/pilimaging: bootstrap python $(DEPENDS_pilimaging)
	$(PREPARE_pilimaging)
	$(start_build)
	cd $(DIR_pilimaging); \
		echo 'JPEG_ROOT = "$(targetprefix)/usr/lib", "$(targetprefix)/usr/include"' > setup_site.py; \
		echo 'ZLIB_ROOT = "$(targetprefixIR)/usr/lib", "$(targetprefix)/usr/include"' >> setup_site.py; \
		echo 'FREETYPE_ROOT = "$(targetprefix)/usr/lib", "$(targetprefix)/usr/include"' >> setup_site.py; \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr; \
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_pilimaging)
	touch $@

#
# Pillow
#
BEGIN[[
Pillow
  2.1.0
  {PN}-{PV}
  extract:https://pypi.python.org/packages/source/P/{PN}/{PN}-{PV}.zip
  patch:file://Pillow-fix-search-paths.patch
;
]]END

DESCRIPTION_Pillow = "Pillow"
FILES_Pillow = \
$(PYTHON_DIR)/site-packages \
/usr/bin/*

$(DEPDIR)/Pillow: bootstrap python $(DEPENDS_Pillow)
	$(PREPARE_Pillow)
	$(start_build)
	cd $(DIR_Pillow); \
		sed -ie "s|"darwin"|"darwinNot"|g" "setup.py"; \
		echo 'JPEG_ROOT = "$(targetprefix)/usr/lib", "$(targetprefix)/usr/include"' > setup_site.py; \
		echo 'ZLIB_ROOT = "$(targetprefixIR)/usr/lib", "$(targetprefix)/usr/include"' >> setup_site.py; \
		echo 'FREETYPE_ROOT = "$(targetprefix)/usr/lib", "$(targetprefix)/usr/include"' >> setup_site.py; \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_Pillow)
	touch $@

#
# pyusb
#
BEGIN[[
pyusb
  1.0.0a3
  {PN}-{PV}
  extract:http://pypi.python.org/packages/source/p/{PN}/{PN}-{PV}.tar.gz
;
]]END

DESCRIPTION_pyusb = pyusb
FILES_pyusb = \
$(PYTHON_DIR)/site-packages/usb/*

$(DEPDIR)/pyusb: bootstrap setuptools $(DEPENDS_pyusb)
	$(PREPARE_pyusb)
	$(start_build)
	cd $(DIR_pyusb); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_pyusb)
	touch $@

#
# pycrypto
#
BEGIN[[
pycrypto
  2.5
  {PN}-{PV}
  extract:http://ftp.dlitz.net/pub/dlitz/crypto/{PN}/{PN}-{PV}.tar.gz
  patch:file://python-{PN}-no-usr-include.patch
;
]]END

DESCRIPTION_pycrypto = pycrypto
FILES_pycrypto = \
$(PYTHON_DIR)/site-packages/Crypto/*

$(DEPDIR)/pycrypto: bootstrap setuptools $(DEPENDS_pycrypto)
	$(PREPARE_pycrypto)
	$(start_build)
	cd $(DIR_pycrypto); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_pycrypto)
	touch $@

#
# pyopenssl
#
BEGIN[[
pyopenssl
  0.13.1
  pyOpenSSL-{PV}
  extract:http://pypi.python.org/packages/source/p/pyOpenSSL/pyOpenSSL-{PV}.tar.gz
;
]]END

DESCRIPTION_pyopenssl = "Python wrapper module around the OpenSSL library"
FILES_pyopenssl = \
$(PYTHON_DIR)/site-packages/OpenSSL/*py \
$(PYTHON_DIR)/site-packages/OpenSSL/*so

$(DEPDIR)/pyopenssl: bootstrap setuptools $(DEPENDS_pyopenssl)
	$(PREPARE_pyopenssl)
	$(start_build)
	cd $(DIR_pyopenssl); \
		CPPFLAGS="$(CPPFLAGS) -I$(targetprefix)/usr/include/python$(PYTHON_VERSION)" \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_pyopenssl)
	touch $@

#
# python
#
ifdef ENABLE_PY273
BEGIN[[
python
  2.7.3
  {PN}-{PV}
  extract:http://www.{PN}.org/ftp/{PN}/{PV}/Python-{PV}.tar.bz2
  pmove:Python-{PV}:{PN}-{PV}
  patch:file://{PN}_{PV}.diff
  patch:file://{PN}_{PV}-ctypes-libffi-fix-configure.diff
  patch:file://{PN}_{PV}-pgettext.diff
  patch:file://{PN}_{PV}-build-module-zlib.patch
;
]]END

PACKAGES_python = python python_ctypes

DESCRIPTION_python = "A high-level scripting language"
FILES_python = \
/usr/bin/python* \
/usr/lib/libpython$(PYTHON_VERSION).* \
$(PYTHON_DIR)/*.py \
$(PYTHON_DIR)/encodings \
$(PYTHON_DIR)/hotshot \
$(PYTHON_DIR)/email \
$(PYTHON_DIR)/idlelib \
$(PYTHON_DIR)/json \
$(PYTHON_DIR)/config \
$(PYTHON_DIR)/lib-dynload \
$(PYTHON_DIR)/lib-tk \
$(PYTHON_DIR)/lib2to3 \
$(PYTHON_DIR)/logging \
$(PYTHON_DIR)/multiprocessing \
$(PYTHON_DIR)/plat-linux3 \
$(PYTHON_DIR)/plat-linux2 \
$(PYTHON_DIR)/sqlite3 \
$(PYTHON_DIR)/wsgiref \
/usr/include/python$(PYTHON_VERSION)/pyconfig.h \
$(PYTHON_DIR)/xml

DESCRIPTION_python_ctypes = python ctypes module
FILES_python_ctypes = \
$(PYTHON_DIR)/ctypes

$(DEPDIR)/python: bootstrap host_python openssl-dev sqlite libreadline bzip2 libexpat libdb libgdbm $(DEPENDS_python)
	$(PREPARE_python)
	$(start_build)
	( cd $(DIR_python); \
		CONFIG_SITE= \
		autoreconf --verbose --install --force Modules/_ctypes/libffi; \
		autoconf; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--prefix=/usr \
			--sysconfdir=/etc \
			--enable-shared \
			--disable-ipv6 \
			--without-cxx-main \
			--with-threads \
			--with-pymalloc \
			--with-system-expat \
			--with-system-ffi \
			--enable-unicode=ucs4 \
			--with-signal-module \
			--with-wctype-functions \
			HOSTPYTHON=$(hostprefix)/bin/python$(PYTHON_VERSION) \
			OPT="$(TARGET_CFLAGS)"; \
		$(MAKE) $(MAKE_ARGS) \
			TARGET_OS=$(target) \
			PYTHON_DISABLE_MODULES="_tkinter" \
			PYTHON_MODULES_INCLUDE="$(prefix)/$*cdkroot/usr/include" \
			PYTHON_MODULES_LIB="$(prefix)/$*cdkroot/usr/lib" \
			CROSS_COMPILE_TARGET=yes \
			CROSS_COMPILE=$(target) \
			HOSTARCH=sh4-linux \
			CFLAGS="$(TARGET_CFLAGS) -fno-inline" \
			LDFLAGS="$(TARGET_LDFLAGS)" \
			LD="$(target)-gcc" \
			HOSTPYTHON=$(hostprefix)/bin/python$(PYTHON_VERSION) \
			HOSTPGEN=$(hostprefix)/bin/pgen \
			all install DESTDIR=$(PKDIR)); \
	touch $@
	$(LN_SF) ../../libpython$(PYTHON_VERSION).so.1.0 $(PKDIR)$(PYTHON_DIR)/config/libpython$(PYTHON_VERSION).so; \
	$(LN_SF) $(PKDIR)$(PYTHON_INCLUDE_DIR) $(PKDIR)/usr/include/python
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_python)
	touch $@
endif
ifdef ENABLE_PY275
BEGIN[[
python
  2.7.5
  {PN}-{PV}
  extract:http://www.python.org/ftp/python/{PV}/Python-{PV}.tar.bz2
  pmove:Python-{PV}:{PN}-{PV}
  patch:file://python-{PV}/python_{PV}.diff
  patch:file://python-{PV}/python-{PV}-pgettext.diff
;
]]END

PACKAGES_python = python python_ctypes

DESCRIPTION_python = "A high-level scripting language"
FILES_python = \
/usr/bin/python* \
/usr/lib/libpython* \
$(PYTHON_DIR)/*.py \
$(PYTHON_DIR)/encodings \
$(PYTHON_DIR)/hotshot \
$(PYTHON_DIR)/email \
$(PYTHON_DIR)/idlelib \
$(PYTHON_DIR)/json \
$(PYTHON_DIR)/config \
$(PYTHON_DIR)/lib-dynload \
$(PYTHON_DIR)/lib-tk \
$(PYTHON_DIR)/lib2to3 \
$(PYTHON_DIR)/logging \
$(PYTHON_DIR)/multiprocessing \
$(PYTHON_DIR)/plat-linux3 \
$(PYTHON_DIR)/plat-linux2 \
$(PYTHON_DIR)/sqlite3 \
$(PYTHON_DIR)/wsgiref \
/usr/include/python$(PYTHON_VERSION)/pyconfig.h \
$(PYTHON_DIR)/xml

DESCRIPTION_python_ctypes = python ctypes module
FILES_python_ctypes = \
$(PYTHON_DIR)/ctypes

$(DEPDIR)/python: bootstrap host_python libffi openssl-dev libreadline sqlite bzip2 libexpat libdb libgdbm $(DEPENDS_python)
	$(PREPARE_python)
	$(start_build)
	( cd $(DIR_python); \
		CONFIG_SITE= \
		autoreconf --verbose --install --force Modules/_ctypes/libffi; \
		autoconf; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--target=$(target) \
			--prefix=/usr \
			--sysconfdir=/etc \
			--enable-shared \
			--disable-ipv6 \
			--without-cxx-main \
			--with-threads \
			--with-pymalloc \
			--with-system-expat \
			--with-system-ffi \
			--enable-unicode=ucs4 \
			--with-signal-module \
			--with-wctype-functions \
			ac_cv_have_chflags=no \
			ac_cv_have_lchflags=no \
			ac_cv_have_long_long_format=yes \
			ac_cv_buggy_getaddrinfo=no \
			ac_cv_file__dev_ptmx=yes \
			ac_cv_file__dev_ptc=no \
			HOSTPYTHON=$(hostprefix)/bin/python$(PYTHON_VERSION) \
			OPT="$(TARGET_CFLAGS)"; \
		$(MAKE) $(MAKE_ARGS) \
			TARGET_OS=$(target) \
			PYTHON_MODULES_INCLUDE="$(prefix)/$*cdkroot/usr/include" \
			PYTHON_MODULES_LIB="$(prefix)/$*cdkroot/usr/lib" \
			LDFLAGS="$(TARGET_LDFLAGS) -L$(prefix)/$*cdkroot/usr/lib -L$(DIR_python)" \
			CROSS_COMPILE_TARGET=yes \
			CROSS_COMPILE=$(target) \
			HOSTARCH=sh4-linux \
			CFLAGS="$(TARGET_CFLAGS) -fno-inline" \
			LD="$(target)-gcc" \
			HOSTPYTHON=$(hostprefix)/bin/python$(PYTHON_VERSION) \
			HOSTPGEN=$(hostprefix)/bin/pgen \
			all install DESTDIR=$(PKDIR)); \
	touch $@
	$(LN_SF) ../../libpython$(PYTHON_VERSION).so.1.0 $(PKDIR)$(PYTHON_DIR)/config/libpython$(PYTHON_VERSION).so; \
	$(LN_SF) $(PKDIR)$(PYTHON_INCLUDE_DIR) $(PKDIR)/usr/include/python
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_python)
	touch $@
endif
ifdef ENABLE_PY332
BEGIN[[
python
  3.3.2
  {PN}-{PV}
  extract:http://www.{PN}.org/ftp/{PN}/{PV}/Python-{PV}.tar.bz2
  pmove:Python-{PV}:{PN}-{PV}
  patch:file://{PN}-{PV}/{PN}_{PV}.diff
  patch:file://{PN}-{PV}/{PN}-{PV}-pgettext.diff
  patch:file://{PN}-{PV}/12-distutils-prefix-is-inside-staging-area.patch
  patch:file://{PN}-{PV}/080-distutils-dont_adjust_files.patch
  patch:file://{PN}-{PV}/130-readline-setup.patch
  patch:file://{PN}-{PV}/150-fix-setupterm.patch
  patch:file://{PN}-{PV}/03-fix-tkinter-detection.patch
  patch:file://{PN}-{PV}/04-default-is-optimized.patch
  patch:file://{PN}-{PV}/avoid_warning_about_tkinter.patch
  patch:file://{PN}-{PV}/06-ctypes-libffi-fix-configure.patch
  patch:file://{PN}-{PV}/remove_sqlite_rpath.patch
  patch:file://{PN}-{PV}/cgi_py.patch
  patch:file://{PN}-{PV}/host_include_contamination.patch
  patch:file://{PN}-{PV}/python-3.3-multilib.patch
  patch:file://{PN}-{PV}/shutil-follow-symlink-fix.patch
  patch:file://{PN}-{PV}/sysroot-include-headers.patch
  patch:file://{PN}-{PV}/unixccompiler.patch
;
]]END

PACKAGES_python = python python_ctypes

DESCRIPTION_python = "A high-level scripting language"
FILES_python = \
/usr/bin/python* \
/usr/lib/libpython* \
$(PYTHON_DIR)/*.py \
$(PYTHON_DIR)/encodings \
$(PYTHON_DIR)/hotshot \
$(PYTHON_DIR)/email \
$(PYTHON_DIR)/idlelib \
$(PYTHON_DIR)/json \
$(PYTHON_DIR)/config \
$(PYTHON_DIR)/lib-dynload \
$(PYTHON_DIR)/lib-tk \
$(PYTHON_DIR)/lib2to3 \
$(PYTHON_DIR)/logging \
$(PYTHON_DIR)/multiprocessing \
$(PYTHON_DIR)/plat-linux3 \
$(PYTHON_DIR)/plat-linux2 \
$(PYTHON_DIR)/sqlite3 \
$(PYTHON_DIR)/wsgiref \
/usr/include/python$(PYTHON_VERSION)/pyconfig.h \
$(PYTHON_DIR)/xml

DESCRIPTION_python_ctypes = python ctypes module
FILES_python_ctypes = \
$(PYTHON_DIR)/ctypes

$(DEPDIR)/python: bootstrap host_python libffi openssl-dev libreadline sqlite bzip2 libexpat libdb libgdbm $(DEPENDS_python)
	$(PREPARE_python)
	$(start_build)
	( cd $(DIR_python); \
		CONFIG_SITE= \
		autoreconf --verbose --install --force Modules/_ctypes/libffi; \
		autoconf; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--sysconfdir=/etc \
			--enable-shared \
			--disable-ipv6 \
			--without-cxx-main \
			--with-threads \
			--with-pymalloc \
			--with-system-expat \
			--with-system-ffi \
			--with-signal-module \
			ac_cv_have_chflags=no \
			ac_cv_have_lchflags=no \
			ac_cv_have_long_long_format=yes \
			ac_cv_buggy_getaddrinfo=no \
			ac_cv_file__dev_ptmx=yes \
			ac_cv_file__dev_ptc=no \
			HOSTPYTHON=$(hostprefix)/bin/python$(PYTHON_VERSION) \
			OPT="$(TARGET_CFLAGS)" libpython3.so; \
		$(MAKE) $(MAKE_ARGS) \
			TARGET_OS=$(target) \
			PYTHON_MODULES_INCLUDE="$(prefix)/$*cdkroot/usr/include" \
			PYTHON_MODULES_LIB="$(prefix)/$*cdkroot/usr/lib $(DIR_python)" \
			CPPFLAGS="-I$(prefix)/$*cdkroot/usr/include" \
			CFLAGS="$(TARGET_CFLAGS) -fno-inline" \
			LDFLAGS="$(TARGET_LDFLAGS) -L$(prefix)/$*cdkroot/usr/lib -L$(DIR_python)" \
			CROSS_COMPILE_TARGET=yes \
			CROSS_COMPILE=$(target)- \
			LD="$(target)-gcc" \
			HOSTARCH=sh4-linux \
			HOSTPYTHON=$(hostprefix)/bin/python$(PYTHON_VERSION) \
			HOSTPGEN=$(hostprefix)/bin/pgen \
			all install DESTDIR=$(PKDIR)); \
	touch $@
	$(LN_SF) ../../libpython$(PYTHON_VERSION)m.so.1.0 $(PKDIR)$(PYTHON_DIR)/config-$(PYTHON_VERSION)m/libpython$(PYTHON_VERSION).so
	$(LN_SF) $(PKDIR)$(PYTHON_INCLUDE_DIR) $(PKDIR)/usr/include/python
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_python)
	touch $@
endif

BEGIN[[
libgdbm
  1.10
  gdbm-{PV}
  extract:ftp://ftp.gnu.org/gnu/gdbm/gdbm-1.10.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libgdbm = "libgdbm"

FILES_libgdbm = \
*

$(DEPDIR)/libgdbm: bootstrap $(DEPENDS_libgdbm)
	$(PREPARE_libgdbm)
	$(start_build)
	cd $(DIR_libgdbm); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			-enable-libgdbm-compat \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libgdbm)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libgdbm)
	touch $@

BEGIN[[
libdb
  5.3.21
  db-{PV}
  extract:http://download.oracle.com/berkeley-db/db-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libdb = "libdb"

FILES_libdb = \
/usr/bin/* \
/usr/lib/*

$(DEPDIR)/libdb: bootstrap $(DEPENDS_libdb)
	$(PREPARE_libdb)
	$(start_build)
	cd $(DIR_libdb); \
		$(BUILDENV) \
		./dist/configure \
			--build=$(build) \
			--host=$(target) \
			--enable-o_direct --disable-cryptography --disable-queue --disable-replication --disable-verify --disable-compat185 --disable-sql \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libdb)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdb)
	touch $@

#
# pythonwifi
#
BEGIN[[
pythonwifi
  0.5.0
  python-wifi-{PV}
  extract:http://freefr.dl.sourceforge.net/project/{PN}.berlios/python-wifi-{PV}.tar.bz2
ifdef ENABLE_PY332
  patch:file://pythonwifi3.diff
endif
;
]]END

DESCRIPTION_pythonwifi = "pythonwifi"
FILES_pythonwifi =\
/usr/bin/* \
$(PYTHON_DIR)/site-packages/pythonwifi

$(DEPDIR)/pythonwifi: bootstrap setuptools $(DEPENDS_pythonwifi)
	$(PREPARE_pythonwifi)
	$(start_build)
	cd $(DIR_pythonwifi); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_pythonwifi)
	touch $@

#
# pythoncheetah
#
BEGIN[[
pythoncheetah
  2.4.4
  Cheetah-{PV}
  extract:http://pypi.python.org/packages/source/C/Cheetah/Cheetah-{PV}.tar.gz
ifdef ENABLE_PY332
  patch:file://Cheetah3.patch
endif
;
]]END

DESCRIPTION_pythoncheetah = "pythoncheetah"
FILES_pythoncheetah = \
$(PYTHON_DIR)/site-packages/Cheetah

$(DEPDIR)/pythoncheetah: bootstrap setuptools $(DEPENDS_pythoncheetah)
	$(PREPARE_pythoncheetah)
	$(start_build)
	cd $(DIR_pythoncheetah); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_pythoncheetah)
	touch $@

#
# zope interface
#
BEGIN[[
zope_interface
  4.0.5
  zope.interface-{PV}
  extract:http://pypi.python.org/packages/source/z/zope.interface/zope.interface-{PV}.zip
;
]]END

DESCRIPTION_zope_interface = "Zope Interfaces for Python2"
FILES_zope_interface = \
$(PYTHON_DIR)

$(DEPDIR)/zope_interface: bootstrap python setuptools $(DEPENDS_zope_interface)
	$(PREPARE_zope_interface)
	$(start_build)
	cd $(DIR_zope_interface); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_zope_interface)
	touch $@

#
# zope interface
#
BEGIN[[
zope_component
  4.1.0
  zope.component-{PV}
  extract:https://pypi.python.org/packages/source/z/zope.component/zope.component-4.1.0.zip
;
]]END

DESCRIPTION_zope_component = "Zope Component for Python2"
FILES_zope_component = \
$(PYTHON_DIR)

$(DEPDIR)/zope_component: bootstrap python setuptools $(DEPENDS_zope_component)
	$(PREPARE_zope_component)
	$(start_build)
	cd $(DIR_zope_component); \
		CC='$(target)-gcc' LDSHARED='$(target)-gcc -shared' \
		PYTHONPATH=$(targetprefix)$(PYTHON_DIR)/site-packages \
		$(hostprefix)/bin/python$(PYTHON_VERSION) ./setup.py install --root=$(PKDIR) --prefix=/usr
	$(tocdk_build)
	$(remove_pyo)
	$(toflash_build)
	$(DISTCLEANUP_zope_component)
	touch $@

##############################   GSTREAMER + PLUGINS   #########################

#
# GSTREAMER
#
BEGIN[[
gstreamer
  0.10.36
  {PN}-{PV}
  extract:http://{PN}.freedesktop.org/src/{PN}/{PN}-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gstreamer = "GStreamer Multimedia Framework"

FILES_gstreamer = \
/usr/bin/gst-* \
/usr/lib/libgst* \
/usr/lib/gstreamer-0.10/libgstcoreelements.so \
/usr/lib/gstreamer-0.10/libgstcoreindexers.so

$(DEPDIR)/gstreamer: bootstrap glib2 libxml2 $(DEPENDS_gstreamer)
	$(PREPARE_gstreamer)
	$(start_build)
	cd $(DIR_gstreamer); \
		autoreconf --verbose --force --install -I$(hostprefix)/share/aclocal; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--disable-docs-build \
			--disable-dependency-tracking \
			--disable-check \
			ac_cv_func_register_printf_function=no; \
		$(MAKE); \
		$(INSTALL_gstreamer)
	$(tocdk_build)
	sh4-linux-strip --strip-unneeded $(PKDIR)/usr/bin/gst-launch*
	$(toflash_build)
	$(DISTCLEANUP_gstreamer)
	touch $@

#
# GST-PLUGINS-BASE
#
BEGIN[[
gst_plugins_base
  0.10.36
  {PN}-{PV}
  extract:http://gstreamer.freedesktop.org/src/{PN}/{PN}-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_plugins_base = "GStreamer Multimedia Framework base plugins"

FILES_gst_plugins_base = \
/usr/lib/libgst* \
/usr/lib/gstreamer-0.10/libgstalsa.so \
/usr/lib/gstreamer-0.10/libgstapp.so \
/usr/lib/gstreamer-0.10/libgstaudioconvert.so \
/usr/lib/gstreamer-0.10/libgstaudioresample.so \
/usr/lib/gstreamer-0.10/libgstdecodebin.so \
/usr/lib/gstreamer-0.10/libgstdecodebin2.so \
/usr/lib/gstreamer-0.10/libgstogg.so \
/usr/lib/gstreamer-0.10/libgstplaybin.so \
/usr/lib/gstreamer-0.10/libgstsubparse.so \
/usr/lib/gstreamer-0.10/libgsttypefindfunctions.so

$(DEPDIR)/gst_plugins_base: bootstrap glib2 gstreamer libogg libalsa libvorbis $(DEPENDS_gst_plugins_base)
	$(PREPARE_gst_plugins_base)
	$(start_build)
	cd $(DIR_gst_plugins_base); \
		autoreconf --verbose --force --install -I$(hostprefix)/share/aclocal; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--disable-theora \
			--disable-gnome_vfs \
			--disable-pango \
			--disable-x \
			--disable-examples \
			--with-audioresample-format=int; \
		$(MAKE); \
		$(BUILDENV) \
		$(INSTALL_gst_plugins_base)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_plugins_base)
	touch $@

#
# GST-PLUGINS-GOOD
#
BEGIN[[
gst_plugins_good
  0.10.31
  {PN}-{PV}
  extract:http://gstreamer.freedesktop.org/src/{PN}/{PN}-{PV}.tar.xz
  patch:file://{PN}-0.10.29_avidemux_only_send_pts_on_keyframe.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_plugins_good = "GStreamer Multimedia Framework good plugins"

FILES_gst_plugins_good = \
/usr/lib/libgst* \
/usr/lib/gstreamer-0.10/libgstaudioparsers.so \
/usr/lib/gstreamer-0.10/libgstautodetect.so \
/usr/lib/gstreamer-0.10/libgstavi.so \
/usr/lib/gstreamer-0.10/libgstflac.so \
/usr/lib/gstreamer-0.10/libgstflv.so \
/usr/lib/gstreamer-0.10/libgsticydemux.so \
/usr/lib/gstreamer-0.10/libgstid3demux.so \
/usr/lib/gstreamer-0.10/libgstmatroska.so \
/usr/lib/gstreamer-0.10/libgstrtp.so \
/usr/lib/gstreamer-0.10/libgstrtpmanager.so \
/usr/lib/gstreamer-0.10/libgstrtsp.so \
/usr/lib/gstreamer-0.10/libgstsouphttpsrc.so \
/usr/lib/gstreamer-0.10/libgstisomp4.so \
/usr/lib/gstreamer-0.10/libgstudp.so \
/usr/lib/gstreamer-0.10/libgstapetag.so \
/usr/lib/gstreamer-0.10/libgstsouphttpsrc.so \
/usr/lib/gstreamer-0.10/libgstwavparse.so

$(DEPDIR)/gst_plugins_good: bootstrap gstreamer gst_plugins_base libsoup libflac $(DEPENDS_gst_plugins_good)
	$(PREPARE_gst_plugins_good)
	$(start_build)
	cd $(DIR_gst_plugins_good); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--disable-esd \
			--enable-experimental \
			--disable-esdtest \
			--disable-aalib \
			--disable-shout2 \
			--disable-shout2test \
			--disable-x; \
		$(MAKE)	$(start_build); \
		$(INSTALL_gst_plugins_good)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_plugins_good)
	touch $@

#
# GST-PLUGINS-BAD
#
BEGIN[[
gst_plugins_bad
  0.10.23
  {PN}-{PV}
  extract:http://gstreamer.freedesktop.org/src/{PN}/{PN}-{PV}.tar.xz
  patch:file://{PN}-0.10.22-mpegtsdemux_remove_bluray_pgs_detection.diff
  patch:file://{PN}-0.10.22-mpegtsdemux_speedup.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_plugins_bad = "GStreamer Multimedia Framework bad plugins"

FILES_gst_plugins_bad = \
/usr/lib/libgst* \
/usr/lib/gstreamer-0.10/libgstassrender.so \
/usr/lib/gstreamer-0.10/libgstcdxaparse.so \
/usr/lib/gstreamer-0.10/libgstfragmented.so \
/usr/lib/gstreamer-0.10/libgstmpegdemux.so \
/usr/lib/gstreamer-0.10/libgstvcdsrc.so \
/usr/lib/gstreamer-0.10/libgstmpeg4videoparse.so \
/usr/lib/gstreamer-0.10/libgsth264parse.so \
/usr/lib/gstreamer-0.10/libgstneonhttpsrc.so \
/usr/lib/gstreamer-0.10/libgstrtmp.so

$(DEPDIR)/gst_plugins_bad: bootstrap gstreamer gst_plugins_base libmodplug $(DEPENDS_gst_plugins_bad)
	$(PREPARE_gst_plugins_bad)
	$(start_build)
	cd $(DIR_gst_plugins_bad); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--with-check=no \
			--disable-sdl \
			--disable-modplug \
			ac_cv_openssldir=no; \
		$(MAKE); \
		$(INSTALL_gst_plugins_bad)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_plugins_bad)
	touch $@

#
# GST-PLUGINS-UGLY
#
BEGIN[[
gst_plugins_ugly
  0.10.19
  {PN}-{PV}
  extract:http://gstreamer.freedesktop.org/src/{PN}/{PN}-{PV}.tar.xz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_plugins_ugly = "GStreamer Multimedia Framework ugly plugins"

FILES_gst_plugins_ugly = \
/usr/lib/gstreamer-0.10/libgstasf.so \
/usr/lib/gstreamer-0.10/libgstdvdsub.so \
/usr/lib/gstreamer-0.10/libgstmad.so \
/usr/lib/gstreamer-0.10/libgstmpegaudioparse.so \
/usr/lib/gstreamer-0.10/libgstmpegstream.so

$(DEPDIR)/gst_plugins_ugly: bootstrap gstreamer gst_plugins_base $(DEPENDS_gst_plugins_ugly)
	$(PREPARE_gst_plugins_ugly)
	$(start_build)
	cd $(DIR_gst_plugins_ugly); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--disable-mpeg2dec; \
		$(MAKE); \
		$(INSTALL_gst_plugins_ugly)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_plugins_ugly)
	touch $@

#
# GST-FFMPEG
#
BEGIN[[
gst_ffmpeg
  0.10.13
  {PN}-{PV}
  extract:http://gstreamer.freedesktop.org/src/{PN}/{PN}-{PV}.tar.bz2
  patch:file://{PN}-0.10.12_lower_rank.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_ffmpeg = "GStreamer Multimedia Framework ffmpeg module"

FILES_gst_ffmpeg = \
/usr/lib/gstreamer-0.10/libgstffmpeg.so \
/usr/lib/gstreamer-0.10/libgstffmpegscale.so \
/usr/lib/gstreamer-0.10/libgstpostproc.so

$(DEPDIR)/gst_ffmpeg: bootstrap gstreamer gst_plugins_base $(DEPENDS_gst_ffmpeg)
	$(PREPARE_gst_ffmpeg)
	$(start_build)
	cd $(DIR_gst_ffmpeg); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			\
			--with-ffmpeg-extra-configure=" \
			--disable-ffserver \
			--disable-ffplay \
			--disable-ffmpeg \
			--disable-ffprobe \
			--enable-postproc \
			--enable-gpl \
			--enable-static \
			--enable-pic \
			--disable-protocols \
			--disable-devices \
			--disable-network \
			--disable-hwaccels \
			--disable-filters \
			--disable-doc \
			--enable-optimizations \
			--enable-cross-compile \
			--target-os=linux \
			--arch=sh4 \
			--cross-prefix=$(target)- \
			\
			--disable-muxers \
			--disable-encoders \
			--disable-decoders \
			--enable-decoder=ogg \
			--enable-decoder=vorbis \
			--enable-decoder=flac \
			\
			--disable-demuxers \
			--enable-demuxer=ogg \
			--enable-demuxer=vorbis \
			--enable-demuxer=flac \
			--enable-demuxer=mpegts \
			\
			--disable-bsfs \
			--enable-pthreads \
			--enable-bzlib"
		$(INSTALL_gst_ffmpeg)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_ffmpeg)
	touch $@

#
# GST-PLUGINS-FLUENDO-MPEGDEMUX
#
BEGIN[[
gst_plugins_fluendo_mpegdemux
  0.10.71
  gst-fluendo-mpegdemux-{PV}
  extract:http://core.fluendo.com/gstreamer/src/gst-fluendo-mpegdemux/gst-fluendo-mpegdemux-{PV}.tar.gz
  patch:file://{PN}-0.10.69-add_dts_hd_detection.diff
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_plugins_fluendo_mpegdemux = "GStreamer Multimedia Framework fluendo"
FILES_gst_plugins_fluendo_mpegdemux = \
/usr/lib/gstreamer-0.10/*.so


$(DEPDIR)/gst_plugins_fluendo_mpegdemux: bootstrap gstreamer gst_plugins_base $(DEPENDS_gst_plugins_fluendo_mpegdemux)
	$(PREPARE_gst_plugins_fluendo_mpegdemux)
	$(start_build)
	cd $(DIR_gst_plugins_fluendo_mpegdemux); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--with-check=no; \
		$(MAKE); \
		$(INSTALL_gst_plugins_fluendo_mpegdemux)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_plugins_fluendo_mpegdemux)
	touch $@

#
# GST-PLUGIN-SUBSINK
#
BEGIN[[
gst_plugin_subsink
  git
  {PN}
  nothing:git://openpli.git.sourceforge.net/gitroot/openpli/gstsubsink:r=8182abe751364f6eb1ed45377b0625102aeb68d5
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_plugin_subsink = GStreamer Multimedia Framework gstsubsink
PKGR_gst_plugin_subsink = r1
FILES_gst_plugin_subsink = \
/usr/lib/gstreamer-0.10/*.so

$(DEPDIR)/gst_plugin_subsink: bootstrap gstreamer gst_ffmpeg gst_plugins_base gst_plugins_good gst_plugins_bad gst_plugins_ugly gst_plugins_fluendo_mpegdemux $(DEPENDS_gst_plugin_subsink)
	$(PREPARE_gst_plugin_subsink)
	$(start_build)
	cd $(DIR_gst_plugin_subsink); \
		touch NEWS README AUTHORS ChangeLog; \
		aclocal -I $(hostprefix)/share/aclocal -I m4; \
		cp $(hostprefix)/share/libtool/config/ltmain.sh .; \
		autoheader; \
		autoconf; \
		automake --add-missing; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_gst_plugin_subsink)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_plugin_subsink)
	touch $@

#
# GST-PLUGINS-DVBMEDIASINK
#
BEGIN[[
gst_plugins_dvbmediasink
  git
  {PN}-{PV}
  plink:$(appsdir)/misc/tools/{PN}:{PN}-{PV}
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_gst_plugins_dvbmediasink = "GStreamer Multimedia Framework dvbmediasink"
SRC_URI_gst_plugins_dvbmediasink = "https://code.google.com/p/tdt-amiko/"

FILES_gst_plugins_dvbmediasink = \
/usr/lib/gstreamer-0.10/libgstdvbaudiosink.so \
/usr/lib/gstreamer-0.10/libgstdvbvideosink.so

$(DEPDIR)/gst_plugins_dvbmediasink: bootstrap gstreamer gst_plugins_base gst_plugins_good gst_plugins_bad gst_plugins_ugly gst_plugin_subsink libdca liborc $(DEPENDS_gst_plugins_dvbmediasink)
	$(PREPARE_gst_plugins_dvbmediasink)
	$(start_build)
	$(get_git_version)
	export PATH=$(hostprefix)/bin:$(PATH); \
	cd $(DIR_gst_plugins_dvbmediasink); \
		aclocal -I $(hostprefix)/share/aclocal -I m4; \
		autoheader; \
		autoconf; \
		automake --foreign --add-missing; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_gst_plugins_dvbmediasink)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_gst_plugins_dvbmediasink)
	touch $@

#
# libdca
#
BEGIN[[
libdca
  0.0.5
  {PN}-{PV}
  extract:http://download.videolan.org/pub/videolan/{PN}/0.0.5/{PN}-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libdca = "libdca"

FILES_libdca = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/libdca: $(DEPENDS_libdca)
	$(PREPARE_libdca)
	$(start_build)
	cd $(DIR_libdca); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libdca)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdca)
	touch $@

#
# liborc
#
BEGIN[[
liborc
  0.4.17
  orc-{PV}
  extract:http://code.entropywave.com/download/orc/orc-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_liborc = "liborc"
FILES_liborc = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/liborc: $(DEPENDS_liborc)
	$(PREPARE_liborc)
	$(start_build)
	cd $(DIR_liborc); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_liborc)
	$(tocdk_build)
	$(toflash_build)
	touch $@

#
# libao
#
BEGIN[[
libao
  1.1.0
  {PN}-{PV}
  extract:http://downloads.xiph.org/releases/ao/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libao = "libao"

$(DEPDIR)/libao: bootstrap $(DEPENDS_libao)
	$(PREPARE_libao)
	$(start_build)
	cd $(DIR_libao); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libao)
	$(tocdk_build)
	$(toflash_build)
	touch $@

##############################   EXTERNAL_LCD   ################################

#
# libusb
#
BEGIN[[
libusb
  0.1.12
  {PN}-{PV}
  extract:http://downloads.sourceforge.net/{PN}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libusb = "libusb is a library which allows userspace application access to USB devices."

FILES_libusb = \
/usr/lib/libusb* \
/usr/lib/libusbpp*

$(DEPDIR)/libusb: $(DEPENDS_libusb)
	$(PREPARE_libusb)
	$(start_build)
	cd $(DIR_libusb); \
	$(BUILDENV) \
	./configure \
		--host=$(target) \
		--disable-build-docs \
		--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libusb)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libusb)
	touch $@

#
# graphlcd
#
BEGIN[[
graphlcd
  git
  {PN}-{PV}
  nothing:git://projects.vdr-developer.org/{PN}-base.git:r=281feef328f8e3772f7a0dde0a90c3a5260c334d:b=touchcol
  patch:file://{PN}.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_graphlcd = "Driver and Tools for LCD4LINUX"
PKGR_graphlcd =r1
FILES_graphlcd = \
/usr/bin/* \
/usr/lib/libglcddrivers* \
/usr/lib/libglcdgraphics* \
/usr/lib/libglcdskin* \
/etc/graphlcd.conf

$(DEPDIR)/graphlcd: bootstrap libusb $(DEPENDS_graphlcd)
	$(PREPARE_graphlcd)
	$(start_build)
	cd $(DIR_graphlcd); \
	$(BUILDENV) \
		$(MAKE) all; \
		install -d $(PKDIR)/etc
		$(INSTALL_graphlcd)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_graphlcd)
	touch $@

##############################   LCD4LINUX   ###################################

#
#
# libgd2
#
BEGIN[[
libgd2
  2.0.35
  gd-{PV}
  extract:http://www.chipsnbytes.net/downloads/gd-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libgd2 = "A graphics library for fast image creation"

FILES_libgd2 = \
/usr/lib/libgd* \
/usr/bin/*

$(DEPDIR)/libgd2: bootstrap libpng libjpeg libiconv libfreetype $(DEPENDS_libgd2)
	$(PREPARE_libgd2)
	$(start_build)
	cd $(DIR_libgd2); \
		chmod +w configure; \
		libtoolize -f -c; \
		autoreconf --force --install -I$(hostprefix)/share/aclocal; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libgd2)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libgd2)
	touch $@

#
# libusb2
#
BEGIN[[
libusb2
  1.0.8
  libusb-{PV}
  extract:http://downloads.sourceforge.net/project/libusb/libusb-1.0/libusb-{PV}/libusb-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libusb2 = "libusb2"
FILES_libusb2 = \
/usr/lib/*.so*

$(DEPDIR)/libusb2: bootstrap $(DEPENDS_libusb2)
	$(PREPARE_libusb2)
	$(start_build)
	cd $(DIR_libusb2); \
	$(BUILDENV) \
	./configure \
		--build=$(build) \
		--host=$(target) \
		--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libusb2)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libusb2)
	touch $@

#
# libusbcompat
#
BEGIN[[
libusbcompat
  0.1.3
  libusb-compat-{PV}
  extract:http://downloads.sourceforge.net/project/libusb/libusb-compat-0.1/libusb-compat-{PV}/libusb-compat-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libusbcompat = "A compatibility layer allowing applications written for libusb-0.1 to work with libusb-1.0"
FILES_libusbcompat = \
/usr/lib/*.so*

$(DEPDIR)/libusbcompat: bootstrap libusb2 $(DEPENDS_libusbcompat)
	$(PREPARE_libusbcompat)
	$(start_build)
	cd $(DIR_libusbcompat); \
	$(BUILDENV) \
	./configure \
		--build=$(build) \
		--host=$(target) \
		--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libusbcompat)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libusbcompat)
	touch $@

##############################   END EXTERNAL_LCD   #############################


#
# eve-browser
#
BEGIN[[
evebrowser
  svn
  {PN}-{PV}
  svn://eve-browser.googlecode.com/svn/trunk/
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_evebrowser = evebrowser for HbbTv
#RDEPENDS_evebrowser = webkitdfb
FILES_evebrowser = \
/usr/lib/*.so* \
/usr/lib/enigma2/python/Plugins/SystemPlugins/HbbTv/bin/hbbtvscan-sh4 \
/usr/lib/enigma2/python/Plugins/SystemPlugins/HbbTv/*.py

$(DEPDIR)/evebrowser: bootstrap $(DEPENDS_evebrowser)
	$(PREPARE_evebrowser)
	$(start_build)
	cd $(DIR_evebrowser); \
		aclocal -I $(hostprefix)/share/aclocal -I m4; \
		autoheader; \
		autoconf; \
		automake --foreign; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		mkdir -p $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/; \
		$(INSTALL_evebrowser); \
		cp -ar enigma2/HbbTv $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/; \
		rm -r $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/HbbTv/bin/hbbtvscan-mipsel; \
		rm -r $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/HbbTv/bin/hbbtvscan-powerpc; \
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_evebrowser)
	touch $@

#
# brofs
#
BEGIN[[
brofs
  1.2
  BroFS{PV}
  extract:http://www.avalpa.com/assets/freesoft/other/BroFS{PV}.tgz
  make:install:prefix=/usr/bin:DESTDIR=PKDIR
;
]]END

DESCRIPTION_brofs = "BROFS (BroadcastReadOnlyFileSystem)"
FILES_brofs = \
/usr/bin/*

$(DEPDIR)/brofs: bootstrap $(DEPENDS_brofs)
	$(PREPARE_brofs)
	$(start_build)
	cd $(DIR_brofs); \
		$(BUILDENV) \
		$(MAKE) all; \
		$(INSTALL_brofs)
		mv -b $(PKDIR)/BroFS $(PKDIR)/usr/bin/; \
		mv -b $(PKDIR)/BroFSCommand $(PKDIR)/usr/bin/; \
		rm -r $(PKDIR)/BroFSd; \
		cd $(PKDIR)/usr/bin/; \
		ln -sf BroFS BroFSd; \
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_brofs)
	touch $@

#
# libcap
#
BEGIN[[
libcap
  2.22
  {PN}-{PV}
  extract:http://mirror.linux.org.au/linux/libs/security/linux-privs/{PN}2/{PN}-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libcap = "This is a library for getting and setting POSIX"
FILES_libcap = \
/usr/lib/*.so* \
/usr/sbin/*

$(DEPDIR)/libcap: bootstrap $(DEPENDS_libcap)
	$(PREPARE_libcap)
	$(start_build)
	cd $(DIR_libcap); \
		$(MAKE) \
		DESTDIR=$(PKDIR) \
		PREFIX=$(PKDIR)/usr \
		LIBDIR=$(PKDIR)/usr/lib \
		SBINDIR=$(PKDIR)/usr/sbin \
		INCDIR=$(PKDIR)/usr/include \
		BUILD_CC=gcc \
		PAM_CAP=no \
		LIBATTR=no \
		CC=$(target)-gcc
		$(INSTALL_libcap) \
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libcap)
	touch $@

	
#
# alsa-lib
#
BEGIN[[
libalsa
  1.0.26
  alsa-lib-{PV}
  extract:http://alsa.cybermirror.org/lib/alsa-lib-{PV}.tar.bz2
  #patch:file://alsa-lib-{PV}-soft_float.patch
  make:install:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libalsa = "ALSA library"

FILES_libalsa = \
/usr/lib/libasound*

$(DEPDIR)/libalsa: bootstrap $(DEPENDS_libalsa)
	$(PREPARE_libalsa)
	$(start_build)
	cd $(DIR_libalsa); \
		aclocal -I $(hostprefix)/share/aclocal -I m4; \
		autoheader; \
		autoconf; \
		automake --foreign; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--with-debug=no \
			--enable-shared=no \
			--enable-static \
			--disable-python; \
		$(MAKE) all; \
		$(INSTALL_libalsa)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libalsa)
	touch $@

#
# rtmpdump
#
BEGIN[[
rtmpdump
  2.4
  {PN}-{PV}
  extract:http://{PN}.mplayerhq.hu/download/{PN}-{PV}.tar.gz
  pmove:{PN}:{PN}-{PV}
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END
DESCRIPTION_rtmpdump = "rtmpdump is a tool for dumping media content streamed over RTMP."

FILES_rtmpdump = \
/usr/bin/rtmpdump \
/usr/lib/librtmp* \
/usr/sbin/rtmpgw

$(DEPDIR)/rtmpdump: bootstrap openssl openssl-dev $(DEPENDS_rtmpdump)
	$(PREPARE_rtmpdump)
	$(start_build)
	cd $(DIR_rtmpdump); \
	cp $(hostprefix)/share/libtool/config/ltmain.sh ..; \
		libtoolize -f -c; \
		$(BUILDENV) \
			make CROSS_COMPILE=$(target)-; \
		$(INSTALL_rtmpdump)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_rtmpdump)
	touch $@

#
# libdvbsi++
#
BEGIN[[
libdvbsipp
  0.3.6
  libdvbsi++-{PV}
  extract:http://www.saftware.de/libdvbsi++/libdvbsi++-{PV}.tar.bz2
  patch:file://libdvbsi++-{PV}.patch
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END
PKGR_libdvbsipp = r0

DESCRIPTION_libdvbsipp = "libdvbsi++ is a open source C++ library for parsing DVB Service Information and MPEG-2 Program Specific Information."

FILES_libdvbsipp = \
/usr/lib/libdvbsi++*

$(DEPDIR)/libdvbsipp: bootstrap $(DEPENDS_libdvbsipp)
	$(PREPARE_libdvbsipp)
	$(start_build)
	cd $(DIR_libdvbsipp); \
		aclocal -I $(hostprefix)/share/aclocal -I m4; \
		autoheader; \
		autoconf; \
		automake --foreign; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libdvbsipp)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdvbsipp)
	touch $@

#
# tuxtxtlib
#
BEGIN[[
tuxtxtlib
  1.0
  libtuxtxt
  nothing:git://git.code.sf.net/p/openpli/tuxtxt:r=4ff8fff:sub=libtuxtxt
  patch:file://libtuxtxt-{PV}-fix_dbox_headers.diff
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_tuxtxtlib = "tuxtxt library"
PKGR_tuxtxtlib = r1
FILES_tuxtxtlib = \
/usr/lib/libtuxtxt*

$(DEPDIR)/tuxtxtlib: bootstrap $(DEPENDS_tuxtxtlib)
	$(PREPARE_tuxtxtlib)
	$(start_build)
	cd $(DIR_tuxtxtlib); \
		aclocal -I $(hostprefix)/share/aclocal; \
		autoheader; \
		autoconf; \
		libtoolize --force; \
		automake --foreign --add-missing; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--with-boxtype=generic \
			--with-configdir=/etc \
			--with-datadir=/usr/share/tuxtxt \
			--with-fontdir=/usr/share/fonts; \
		$(MAKE) all; \
		$(INSTALL_tuxtxtlib)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_tuxtxtlib)
	touch $@

#
# tuxtxt32bpp
#
BEGIN[[
tuxtxt32bpp
  1.0
  tuxtxt
  nothing:git://git.code.sf.net/p/openpli/tuxtxt:r=4ff8fff:sub=tuxtxt
  patch:file://{PN}-{PV}-fix_dbox_headers.diff
  make:install:prefix=/usr:DESTDIR=PKDIR
# overwrite after make install
  install -m644 -D:file://../root/usr/tuxtxt/tuxtxt2.conf:PKDIR/etc/tuxtxt/tuxtxt2.conf
;
]]END

DESCRIPTION_tuxtxt32bpp = "tuxtxt plugin"
PKGR_tuxtxt32bpp = r2
FILES_tuxtxt32bpp = \
/usr/lib/libtuxtxt32bpp* \
/usr/lib/enigma2/python/Plugins/Extensions/Tuxtxt/* \
/etc/tuxtxt/tuxtxt2.conf

$(DEPDIR)/tuxtxt32bpp: tuxtxtlib $(DEPENDS_tuxtxt32bpp)
	$(PREPARE_tuxtxt32bpp)
	$(start_build)
	cd $(DIR_tuxtxt32bpp); \
		aclocal -I $(hostprefix)/share/aclocal; \
		autoheader; \
		autoconf; \
		libtoolize --force; \
		automake --foreign --add-missing; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--with-boxtype=generic \
			--with-configdir=/etc \
			--with-datadir=/usr/share/tuxtxt \
			--with-fontdir=/usr/share/fonts; \
		$(MAKE) all; \
		$(INSTALL_tuxtxt32bpp)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_tuxtxt32bpp)
	touch $@

#
# libdreamdvd
#
BEGIN[[
libdreamdvd
  git
  {PN}
  plink:../apps/misc/tools/{PN}:{PN}
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libdreamdvd = "libdreamdvd"
PKGR_libdreamdvd = r1
FILES_libdreamdvd = \
/usr/lib/libdreamdvd*

SRC_URI_libdreamdvd = "libdreamdvd"

$(DEPDIR)/libdreamdvd: bootstrap $(DEPENDS_libdreamdvd)
	$(PREPARE_libdreamdvd)
	$(start_build)
	cd $(DIR_libdreamdvd); \
		aclocal -I $(hostprefix)/share/aclocal; \
		autoheader; \
		autoconf; \
		automake --foreign; \
		libtoolize --force; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libdreamdvd)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdreamdvd)
	touch $@

#
# libdreamdvd2
#
BEGIN[[
libdreamdvd2
  git
  libdreamdvd
  nothing:git://github.com/mirakels/libdreamdvd.git:r=1bdc2c33f912b9e87cb7e204485a57c6a08a0e8c
  patch:file://libdreamdvd-1.0-support_sh4.patch
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libdreamdvd2 = ""
PKGR_libdreamdvd2 = r1
FILES_libdreamdvd2 = \
/usr/lib/*

$(DEPDIR)/libdreamdvd2: bootstrap libdvdnav $(DEPENDS_libdreamdvd2)
	$(PREPARE_libdreamdvd2)
	$(start_build)
	cd $(DIR_libdreamdvd2); \
		autoreconf -i; \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libdreamdvd2)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libdreamdvd2)
	touch $@

#
# libmpeg2
#
BEGIN[[
libmpeg2
  0.5.1
  {PN}-{PV}
  extract:http://{PN}.sourceforge.net/files/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libmpeg2 = "libmpeg2 is a free library for decoding mpeg-2 and mpeg-1 video streams. It is released under the terms of the GPL license."

FILES_libmpeg2 = \
/usr/lib/libmpeg2.* \
/usr/lib/libmpeg2convert.* \
/usr/bin/*

$(DEPDIR)/libmpeg2: bootstrap $(DEPENDS_libmpeg2)
	$(PREPARE_libmpeg2)
	$(start_build)
	cd $(DIR_libmpeg2); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--disable-sdl \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libmpeg2)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libmpeg2)
	touch $@

#
# libsamplerate
#
BEGIN[[
libsamplerate
  0.1.8
  {PN}-{PV}
  extract:http://www.mega-nerd.com/SRC/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libsamplerate = "libsamplerate (also known as Secret Rabbit Code) is a library for perfroming sample rate conversion of audio data."

FILES_libsamplerate = \
/usr/bin/sndfile-resample \
/usr/lib/libsamplerate.*

$(DEPDIR)/libsamplerate: bootstrap $(DEPENDS_libsamplerate)
	$(PREPARE_libsamplerate)
	$(start_build)
	cd $(DIR_libsamplerate); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libsamplerate)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libsamplerate)
	touch $@

#
# libvorbis
#
BEGIN[[
libvorbis
  1.3.2
  {PN}-{PV}
  extract:http://downloads.xiph.org/releases/vorbis/{PN}-{PV}.tar.bz2
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END
DESCRIPTION_libvorbis = "The libvorbis reference implementation provides both a standard encoder and decoder"

FILES_libvorbis = \
/usr/lib/libvorbis*

$(DEPDIR)/libvorbis: bootstrap $(DEPENDS_libvorbis)
	$(PREPARE_libvorbis)
	$(start_build)
	cd $(DIR_libvorbis); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libvorbis)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libvorbis)
	touch $@

#
# libmodplug
#
BEGIN[[
libmodplug
  0.8.8.4
  {PN}-{PV}
  extract:http://downloads.sourceforge.net/project/modplug-xmms/{PN}/{PV}/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libmodplug = "the library for decoding mod-like music formats"

FILES_libmodplug = \
/usr/lib/lib*

$(DEPDIR)/libmodplug: bootstrap $(DEPENDS_libmodplug)
	$(PREPARE_libmodplug)
	$(start_build)
	cd $(DIR_libmodplug); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libmodplug)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libmodplug)
	touch $@

#
# tiff
#
BEGIN[[
tiff
  4.0.1
  {PN}-{PV}
  extract:ftp://ftp.remotesensing.org/pub/lib{PN}/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_tiff = "TIFF Software Distribution"

FILES_tiff = \
/usr/lib/libtiff* \
/usr/bin/*

$(DEPDIR)/tiff: bootstrap $(DEPENDS_tiff)
	$(PREPARE_tiff)
	$(start_build)
	cd $(DIR_tiff); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_tiff)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_tiff)
	touch $@

#
# lzo
#
BEGIN[[
lzo
  2.06
  {PN}-{PV}
  extract:http://www.oberhumer.com/opensource/{PN}/download/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_lzo = "LZO -- a real-time data compression library"

FILES_lzo = \
/usr/lib/*

$(DEPDIR)/lzo: $(DEPENDS_lzo)
	$(PREPARE_lzo)
	$(start_build)
	cd $(DIR_lzo); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_lzo)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_lzo)
	touch $@

#
# yajl
#
BEGIN[[
yajl
  2.0.1
  {PN}-{PV}
  nothing:git://github.com/lloyd/{PN}:r=f4b2b1af87483caac60e50e5352fc783d9b2de2d
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_yajl = "Yet Another JSON Library"
PKGR_yajl = r1
FILES_yajl = \
/usr/lib/libyajl.* \
/usr/bin/json*

$(DEPDIR)/yajl: bootstrap $(DEPENDS_yajl)
	$(PREPARE_yajl)
	$(start_build)
	cd $(DIR_yajl); \
		$(BUILDENV) \
		./configure \
			--prefix=/usr; \
		sed -i "s/install: all/install: distro/g" Makefile; \
		$(MAKE) distro; \
		$(INSTALL_yajl)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_yajl)
	touch $@

#
# libpcre (shouldn't this be named pcre without the lib?)
#
BEGIN[[
libpcre
  8.31
  pcre-{PV}
  extract:ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-{PV}.tar.bz2
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libpcre = "Perl-compatible regular expression library"

FILES_libpcre = \
/usr/lib/* \
/usr/bin/pcre*

$(DEPDIR)/libpcre: bootstrap $(DEPENDS_libpcre)
	$(PREPARE_libpcre)
	$(start_build)
	cd $(DIR_libpcre); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--prefix=/usr \
			--enable-utf8 \
			--enable-unicode-properties; \
		$(MAKE) all; \
		sed -e "s,^prefix=,prefix=$(targetprefix)," < pcre-config > $(crossprefix)/bin/pcre-config; \
		chmod 755 $(crossprefix)/bin/pcre-config; \
		$(INSTALL_libpcre)
		rm -f $(targetprefix)/usr/bin/pcre-config
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libpcre)
	touch $@

#
# libcdio
#
BEGIN[[
libcdio
  0.83
  {PN}-{PV}
  extract:ftp://ftp.gnu.org/gnu/{PN}/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libcdio = "The libcdio package contains a library for CD-ROM and CD image access"

FILES_libcdio = \
/usr/lib/* \
/usr/bin/*

$(DEPDIR)/libcdio: bootstrap $(DEPENDS_libcdio)
	$(PREPARE_libcdio)
	$(start_build)
	cd $(DIR_libcdio); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libcdio)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libcdio)
	touch $@

#
# jasper
#
BEGIN[[
jasper
  1.900.1
  {PN}-{PV}
  extract:http://www.ece.uvic.ca/~frodo/{PN}/software/{PN}-{PV}.zip
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_jasper = "JasPer is a collection \
of software (i.e., a library and application programs) for the coding \
and manipulation of images.  This software can handle image data in a \
variety of formats"

FILES_jasper = \
/usr/bin/* 

$(DEPDIR)/jasper: bootstrap $(DEPENDS_jasper)
	$(PREPARE_jasper)
	$(start_build)
	cd $(DIR_jasper@/@DIR_jasper); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_jasper)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_jasper)
	touch $@

#
# mysql
#
BEGIN[[
mysql
  5.1.40
  {PN}-{PV}
  extract:http://downloads.{PN}.com/archives/{PN}-5.1/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_mysql = "MySQL"

FILES_mysql = \
/usr/bin/*

$(DEPDIR)/mysql: bootstrap $(DEPENDS_mysql)
	$(PREPARE_mysql)
	$(start_build)
	cd $(DIR_mysql); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--with-atomic-ops=up --with-embedded-server --prefix=/usr --sysconfdir=/etc/mysql --localstatedir=/var/mysql --disable-dependency-tracking --without-raid --without-debug --with-low-memory --without-query-cache --without-man --without-docs --without-innodb; \
		$(MAKE) all; \
		$(INSTALL_mysql)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_mysql)
	touch $@

#
# xupnpd
#
BEGIN[[
xupnpd
  svn
  {PN}-{PV}
  svn://tsdemuxer.googlecode.com/svn/trunk/xupnpd/src/
  patch-0:file://{PN}.diff
  make:install:DESTDIR=PKDIR
;
]]END


DESCRIPTION_xupnpd = eXtensible UPnP agent
FILES_xupnpd = \
/

$(DEPDIR)/xupnpd: bootstrap $(DEPENDS_xupnpd)
	$(PREPARE_xupnpd)
	$(start_build)
	cd $(DIR_xupnpd); \
		$(BUILDENV) \
	$(MAKE) embedded; \
	  install -d 0644  $(PKDIR)/{etc,usr/bin}; \
	  install -m 0755 xupnpd- $(PKDIR)/usr/bin/xupnpd; \
	  install -d 0644  $(PKDIR)/usr/share/xupnpd/{ui,www,plugins,config,playlists}; \
	  install -m 0644 *.lua $(PKDIR)/usr/share/xupnpd; \
	  install -m 0644 ui/* $(PKDIR)/usr/share/xupnpd/ui; \
	  install -m 0644 www/* $(PKDIR)/usr/share/xupnpd/www; \
	  install -m 0644 plugins/* $(PKDIR)/usr/share/xupnpd/plugins; \
	  cp -a playlists/*.m3u $(PKDIR)/usr/share/xupnpd/playlists; \
	  $(LN_SF)  /usr/share/xupnpd/xupnpd.lua $(PKDIR)/etc/xupnpd.lua
#	  install -D -m 0755 xupnpd-init.file $(PKDIR)/etc/init.d/xupnpd
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_xupnpd)
	touch $@
   
#
# libmicrohttpd
#
BEGIN[[
libmicrohttpd
  0.9.19
  {PN}-{PV}
  extract:http://ftp.halifax.rwth-aachen.de/gnu/{PN}/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libmicrohttpd = ""

FILES_libmicrohttpd = \
/usr/lib/libmicrohttpd.*

$(DEPDIR)/libmicrohttpd: bootstrap $(DEPENDS_libmicrohttpd)
	$(PREPARE_libmicrohttpd)
	$(start_build)
	cd $(DIR_libmicrohttpd); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libmicrohttpd)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libmicrohttpd)
	touch $@

#
# libexif
#
BEGIN[[
libexif
  0.6.20
  {PN}-{PV}
  extract:http://sourceforge.net/projects/{PN}/files/{PN}/{PV}/{PN}-{PV}.tar.gz
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libexif = "libexif is a library for parsing, editing, and saving EXIF data."

FILES_libexif = \
/usr/lib/libexif.*

$(DEPDIR)/libexif: bootstrap $(DEPENDS_libexif)
	$(PREPARE_libexif)
	$(start_build)
	cd $(DIR_libexif); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libexif)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libexif)
	touch $@

#
# minidlna
#
BEGIN[[
minidlna
  1.0.25
  {PN}-{PV}
  extract:http://netcologne.dl.sourceforge.net/project/{PN}/{PN}/{PV}/{PN}_{PV}_src.tar.gz
  patch:file://{PN}-{PV}.patch
  make:install:prefix=/usr:DESTDIR=PKDIR
;
]]END

DESCRIPTION_minidlna = "The MiniDLNA daemon is an UPnP-A/V and DLNA service which serves multimedia content to compatible clients on the network."

FILES_minidlna = \
/usr/lib/* \
/usr/sbin/*
$(DEPDIR)/minidlna: bootstrap ffmpeg libflac libogg libvorbis libid3tag sqlite libexif libjpeg $(DEPENDS_minidlna)
	$(PREPARE_minidlna)
	$(start_build)
	cd $(DIR_minidlna); \
		libtoolize -f -c; \
		$(BUILDENV) \
		DESTDIR=$(prefix)/cdkroot \
		$(MAKE) \
		PREFIX=$(prefix)/cdkroot/usr \
		LIBDIR=$(prefix)/cdkroot/usr/lib \
		SBINDIR=$(prefix)/cdkroot/usr/sbin \
		INCDIR=$(prefix)/cdkroot/usr/include \
		PAM_CAP=no \
		LIBATTR=no; \
		$(INSTALL_minidlna)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_minidlna)
	touch $@

#
# vlc
#
BEGIN[[
vlc
  2.0.3
  {PN}-{PV}
  extract:http://download.videolan.org/pub/videolan/{PN}/{PV}/{PN}-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_vlc = "VLC player"

FILES_vlc = \
/usr/bin/* \
/usr/lib/libvlc* \
/usr/lib/vlc/plugins/access/*.so \
/usr/lib/vlc/plugins/access_output/*.so \
/usr/lib/vlc/plugins/audio_filter/*.so \
/usr/lib/vlc/plugins/audio_mixer/*.so \
/usr/lib/vlc/plugins/audio_output/*.so \
/usr/lib/vlc/plugins/codec/*.so \
/usr/lib/vlc/plugins/control/*.so \
/usr/lib/vlc/plugins/demux/*.so \
/usr/lib/vlc/plugins/gui/*.so \
/usr/lib/vlc/plugins/meta_engine/*.so \
/usr/lib/vlc/plugins/misc/*.so \
/usr/lib/vlc/plugins/mux/*.so \
/usr/lib/vlc/plugins/packetizer/*.so \
/usr/lib/vlc/plugins/services_discovery/*.so \
/usr/lib/vlc/plugins/stream_filter/*.so \
/usr/lib/vlc/plugins/stream_out/*.so \
/usr/lib/vlc/plugins/video_chroma/*.so \
/usr/lib/vlc/plugins/video_filter/*.so \
/usr/lib/vlc/plugins/video_output/*.so \
/usr/lib/vlc/plugins/visualization/*.so

$(DEPDIR)/vlc: bootstrap libstdc++-dev libfribidi ffmpeg $(DEPENDS_vlc)
	$(PREPARE_vlc)
	$(start_build)
	cd $(DIR_vlc); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--disable-fontconfig \
			--prefix=/usr \
			--disable-xcb \
			--disable-glx \
			--disable-qt4 \
			--disable-mad \
			--disable-postproc \
			--disable-a52 \
			--disable-qt4 \
			--disable-skins2 \
			--disable-remoteosd \
			--disable-lua \
			--disable-libgcrypt \
			--disable-nls \
			--disable-mozilla \
			--disable-dbus \
			--disable-sdl \
			--enable-run-as-root; \
		$(MAKE); \
		$(INSTALL_vlc)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_vlc)
	touch $@

#
# djmount
#
BEGIN[[
djmount
  0.71
  {PN}-{PV}
  extract:http://sourceforge.net/projects/{PN}/files/{PN}/{PV}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_djmount = djmount is a UPnP AV client. It mounts as a Linux filesystem the media content of compatible UPnP AV devices.
RDEPENDS_djmount = fuse
FILES_djmount = \
/usr/bin/* \
/usr/lib/*

$(DEPDIR)/djmount: bootstrap fuse $(DEPENDS_djmount)
	$(PREPARE_djmount)
	$(start_build)
	cd $(DIR_djmount); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_djmount)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_djmount)
	touch $@

#
# libupnp
#
BEGIN[[
libupnp
  1.6.17
  {PN}-{PV}
  extract:http://sourceforge.net/projects/upnp/files/latest/download/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libupnp = "The portable SDK for UPnP™ Devices (libupnp) provides developers with an API and open source code for building control points"

FILES_libupnp = \
/usr/lib/*.so*

$(DEPDIR)/libupnp: bootstrap $(DEPENDS_libupnp)
	$(PREPARE_libupnp)
	$(start_build)
	cd $(DIR_libupnp); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libupnp)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libupnp)
	touch $@

#
# rarfs
#
BEGIN[[
rarfs
  0.1.1
  {PN}-{PV}
  extract:http://sourceforge.net/projects/{PN}/files/{PN}/{PV}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_rarfs = ""

FILES_rarfs = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/rarfs: bootstrap libstdc++-dev fuse $(DEPENDS_rarfs)
	$(PREPARE_rarfs)
	$(start_build)
	cd $(DIR_rarfs); \
		export PKG_CONFIG_PATH=$(targetprefix)/usr/lib/pkgconfig; \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os -D_FILE_OFFSET_BITS=64" \
		./configure \
			--host=$(target) \
			--disable-option-checking \
			--includedir=/usr/include/fuse \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_rarfs)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_rarfs)
	touch $@

#
# sshfs
#
BEGIN[[
sshfs
  2.4
  {PN}-fuse-{PV}
  extract:http://fossies.org/linux/misc/{PN}-fuse-{PV}.tar.bz2
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/sshfs: bootstrap fuse $(DEPENDS_sshfs)
	$(PREPARE_sshfs)
	$(start_build)
	cd $(DIR_sshfs); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_sshfs)
	$(DISTCLEANUP_sshfs)
	touch $@

#
# gmediarender
#
BEGIN[[
gmediarender
  0.0.6
  {PN}-{PV}
  extract:http://savannah.nongnu.org/download/gmrender/{PN}-{PV}.tar.bz2
  patch:file://{PN}.patch
  make:install:DESTDIR=TARGETS
;
]]END

$(DEPDIR)/gmediarender: bootstrap libstdc++-dev gst_plugins_dvbmediasink libupnp $(DEPENDS_gmediarender)
	$(PREPARE_gmediarender)
	$(start_build)
	cd $(DIR_gmediarender); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--with-libupnp=$(targetprefix)/usr; \
		$(MAKE) all; \
		$(INSTALL_gmediarender)
	$(DISTCLEANUP_gmediarender)
	touch $@
#
# mediatomb
#
BEGIN[[
mediatomb
  0.12.1
  {PN}-{PV}
  extract:http://downloads.sourceforge.net/{PN}/{PN}-{PV}.tar.gz
  patch:file://{PN}_metadata.patch
#  patch:file://{PN}_libav_support.patch
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_mediatomb = MediaTomb is an open source (GPL) UPnP MediaServer with a nice web user interfaces
FILES_mediatomb = \
/usr/bin/* \
/usr/share/mediatomb/*

$(DEPDIR)/mediatomb: bootstrap libstdc++-dev ffmpeg curl sqlite libexpat $(DEPENDS_mediatomb)
	$(PREPARE_mediatomb)
	$(start_build)
	cd $(DIR_mediatomb); \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--disable-ffmpegthumbnailer \
			--disable-libmagic \
			--disable-mysql \
			--disable-id3lib \
			--disable-taglib \
			--disable-lastfmlib \
			--disable-libexif \
			--disable-libmp4v2 \
			--disable-inotify \
			--with-avformat-h=$(targetprefix)/usr/include \
			--disable-rpl-malloc \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_mediatomb)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_mediatomb)
	touch $@

#
# tinyxml
#
BEGIN[[
tinyxml
  2.6.2
  {PN}-{PV}
  extract:http://ignum.dl.sourceforge.net/project/tinyxml/tinyxml/{PV}/tinyxml_2_6_2.tar.gz
  pmove:{PN}:{PN}-{PV}
  patch:file://{PN}{PV}.patch
  make:install:PREFIX=PKDIR/usr:LD=sh4-linux-ld
;
]]END

DESCRIPTION_tinyxml = tinyxml
FILES_tinyxml = \
/usr/lib/*

$(DEPDIR)/tinyxml: $(DEPENDS_tinyxml)
	$(PREPARE_tinyxml)
	$(start_build)
	cd $(DIR_tinyxml); \
		libtoolize -f -c; \
		$(BUILDENV) \
		$(MAKE); \
		$(INSTALL_tinyxml)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_tinyxml)
	touch $@

#
# libnfs
#
BEGIN[[
libnfs
  git
  {PN}
  git://github.com/sahlberg/libnfs.git:r=c0ebf57b212ffefe83e2a50358499f68e7289e93
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libnfs = nfs
PKGR_libnfs = r1
FILES_libnfs = \
/usr/lib/*

$(DEPDIR)/libnfs: bootstrap $(DEPENDS_libnfs)
	$(PREPARE_libnfs)
	$(start_build)
	cd $(DIR_libnfs); \
		aclocal -I $(hostprefix)/share/aclocal; \
		autoheader; \
		autoconf; \
		automake --foreign; \
		libtoolize --force; \
		$(BUILDENV) \
		CFLAGS="$(TARGET_CFLAGS) -Os" \
		./configure \
			--host=$(target) \
			--prefix=/usr; \
		$(MAKE) all; \
		$(INSTALL_libnfs)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libnfs)
	touch $@

#
# taglib
#
BEGIN[[
taglib
  1.8
  {PN}-{PV}
  extract:https://github.com/downloads/{PN}/{PN}/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_taglib = taglib
FILES_taglib = \
/usr/*

$(DEPDIR)/taglib: bootstrap $(DEPENDS_taglib)
	$(PREPARE_taglib)
	$(start_build)
	cd $(DIR_taglib); \
		$(BUILDENV) \
		cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_RELEASE_TYPE=Release .; \
		$(MAKE) all; \
		$(INSTALL_taglib)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_taglib)
	touch $@

#
# e2-rtmpgw
#
BEGIN[[
e2_rtmpgw
  git
  {PN}
  git://github.com/zakalibit/e2-rtmpgw.git:b=gw-e2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_e2_rtmpgw = A toolkit for RTMP streams
PKGR_e2_rtmpgw = r1
FILES_e2_rtmpgw = \
/usr/sbin/rtmpgw2

$(DEPDIR)/e2_rtmpgw: bootstrap openssl openssl-dev libz $(DEPENDS_e2_rtmpgw)
	$(PREPARE_e2_rtmpgw)
	$(start_build)
	cd $(DIR_e2_rtmpgw); \
		$(BUILDENV) \
		$(MAKE) all; \
		$(INSTALL_e2_rtmpgw)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_e2_rtmpgw)
	touch $@

#
# libx264
#
BEGIN[[
libx264
  x264
  {PV}-snapshot-20130608-2245
  extract:ftp://ftp.videolan.org/pub/x264/snapshots/last_{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libx264 = "libx264"

FILES_libx264 = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/libx264: bootstrap $(DEPENDS_libx264)
	$(PREPARE_libx264)
	$(start_build)
	cd $(DIR_libx264); \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--system-libx264 \
			--enable-shared \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libx264)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libx264)
	touch $@

#
# libaacplus
#
BEGIN[[
libaacplus
  2.0.2
  {PN}-{PV}
  extract:http://ffmpeg.gusari.org/uploads/{PN}-{PV}.tar.gz
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libaacplus = "libaacplus"

FILES_libaacplus = \
/usr/bin/*

$(DEPDIR)/libaacplus: bootstrap $(DEPENDS_libaacplus)
	$(PREPARE_libaacplus)
	$(start_build)
	cd $(DIR_libaacplus); \
		./autogen.sh; \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--with-parameter-expansion-string-replace-capable-shell=/bin/bash \
			--disable-shared \
			--enable-static \
			--without-fftw3 \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libaacplus)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libaacplus)
	touch $@

#
# libfaac
#
BEGIN[[
libfaac
  1.28
  faac-{PV}
  extract:http://downloads.sourceforge.net/faac/faac-{PV}.tar.bz2
  make:install:DESTDIR=PKDIR
;
]]END

DESCRIPTION_libfaac = "libfaac"

FILES_libfaac = \
/usr/lib/*.so* \
/usr/bin/*

$(DEPDIR)/libfaac: bootstrap $(DEPENDS_libfaac)
	$(PREPARE_libfaac)
	$(start_build)
	cd $(DIR_libfaac); \
		$(BUILDENV) \
		./configure \
			--build=$(build) \
			--host=$(target) \
			--without-mp4v2 \
			--prefix=/usr; \
		$(MAKE); \
		$(INSTALL_libfaac)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_libfaac)
	touch $@
