# misc/tools

#DESCRIPTION_misc_tools = "misc tools for box"
#SRC_URI_misc_tools = "git://gitorious.org/~schpuntik/open-duckbox-project-sh4/tdt-amiko.git"
#DIR_misc_tools = $(appsdir)/misc/tools


##$(appsdir)/misc/tools/config.status: bootstrap libpng
#$(appsdir)/misc/tools/config.status: bootstrap
#	export PATH=$(hostprefix)/bin:$(PATH) && \
#	cd $(appsdir)/misc/tools && \
#	libtoolize -f -c && \
#	$(CONFIGURE) --prefix= \
#	$(if $(MULTICOM324), --enable-multicom324)

#$(DEPDIR)/misc-tools: \
#$(DEPDIR)/%misc-tools: driver libstdc++-dev libdvdnav libdvdcss libpng jpeg ffmpeg expat fontconfig bzip2 $(appsdir)/misc/tools/config.status
#	$(start_build)
#	$(get_git_version)
#	$(MAKE) -C $(appsdir)/misc/tools all install DESTDIR=$(PKDIR) \
#	CPPFLAGS="\
#	$(if $(SPARK), -DPLATFORM_SPARK) \
#	$(if $(SPARK7162), -DPLATFORM_SPARK7162) \
#	$(if $(HL101), -DPLATFORM_HL101) \
#	$(if $(PLAYER179), -DPLAYER179) \
#	$(if $(PLAYER191), -DPLAYER191) \
#	$(if $(VDR1722), -DVDR1722) \
#	$(if $(VDR1727), -DVDR1727)"
#	$(tocdk_build)
#	$(toflash_build)
#	[ "x$*" = "x" ] && touch $@ || true

#misc-tools-clean:
#	-$(MAKE) -C $(appsdir)/misc/tools distclean

#
# DEVINIT
#
DESCRIPTION_devinit = "devinit"
SRC_URI_devinit = "https://code.google.com/p/tdt-amiko/"

FILES_devinit = \
/bin/devinit

$(DEPDIR)/devinit.do_prepare: bootstrap $(DEPENDS_devinit)
	$(PREPARE_devinit)
	touch $@

$(DEPDIR)/devinit.do_compile: $(DEPDIR)/devinit.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_devinit) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/devinit: \
$(DEPDIR)/%devinit: $(DEPDIR)/devinit.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_devinit) && \
		$(INSTALL_devinit)
#	$(DISTCLEANUP_devinit)
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# EVREMOTE2
#
DESCRIPTION_evremote2 = "evremote2"
SRC_URI_evremote2 = "https://code.google.com/p/tdt-amiko/"

FILES_evremote2 = \
/bin/evremote2 \
/bin/evtest

$(DEPDIR)/evremote2.do_prepare: bootstrap $(DEPENDS_evremote2)
	$(PREPARE_evremote2)
	touch $@

$(DEPDIR)/evremote2.do_compile: $(DEPDIR)/evremote2.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_evremote2) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/evremote2: \
$(DEPDIR)/%evremote2: $(DEPDIR)/evremote2.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_evremote2) && \
		$(INSTALL_evremote2)
#	$(DISTCLEANUP_evremote2)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# FP-CONTROL
#
DESCRIPTION_fp_control = "fp_control"
SRC_URI_fp_control = "https://code.google.com/p/tdt-amiko/"

FILES_fp_control = \
/bin/fp_control

$(DEPDIR)/fp_control.do_prepare: bootstrap $(DEPENDS_fp_control)
	$(PREPARE_fp_control)
	touch $@

$(DEPDIR)/fp_control.do_compile: $(DEPDIR)/fp_control.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_fp_control) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/fp_control: \
$(DEPDIR)/%fp_control: $(DEPDIR)/fp_control.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_fp_control) && \
		$(INSTALL_fp_control)
#	$(DISTCLEANUP_fp_control)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# gitVCInfo
#
DESCRIPTION_gitVCInfo = "gitVCInfo"
SRC_URI_gitVCInfo = "https://code.google.com/p/tdt-amiko/"

FILES_gitVCInfo = \
/bin/gitVCInfo

$(DEPDIR)/gitVCInfo.do_prepare: bootstrap $(DEPENDS_gitVCInfo)
	$(PREPARE_gitVCInfo)
	touch $@

$(DEPDIR)/gitVCInfo.do_compile: $(DEPDIR)/gitVCInfo.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_gitVCInfo) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/gitVCInfo: \
$(DEPDIR)/%gitVCInfo: $(DEPDIR)/gitVCInfo.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_gitVCInfo) && \
		$(INSTALL_gitVCInfo)
#	$(DISTCLEANUP_gitVCInfo)
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# HOTPLUG
#
DESCRIPTION_hotplug = "hotplug"
SRC_URI_hotplug = "https://code.google.com/p/tdt-amiko/"

FILES_hotplug = \
/bin/hotplug

$(DEPDIR)/hotplug.do_prepare: bootstrap $(DEPENDS_hotplug)
	$(PREPARE_hotplug)
	touch $@

$(DEPDIR)/hotplug.do_compile: $(DEPDIR)/hotplug.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_hotplug) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/hotplug: \
$(DEPDIR)/%hotplug: $(DEPDIR)/hotplug.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_hotplug) && \
		$(INSTALL_hotplug)
#	$(DISTCLEANUP_hotplug)
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true#

#
# LIBEPLAYER3
#
DESCRIPTION_libeplayer3 = "libeplayer3"
SRC_URI_libeplayer3 = "https://code.google.com/p/tdt-amiko/"

FILES_libeplayer3 = \
/bin/eplayer3 \
/bin/meta \
/lib/libeplayer3.*

$(DEPDIR)/libeplayer3.do_prepare: bootstrap libstdc++-dev libdvdnav libdvdcss libpng libjpeg ffmpeg libexpat fontconfig bzip2 $(DEPENDS_libeplayer3)
	$(PREPARE_libeplayer3)
	touch $@

$(DEPDIR)/libeplayer3.do_compile: $(DEPDIR)/libeplayer3.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_libeplayer3) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/libeplayer3: \
$(DEPDIR)/%libeplayer3: $(DEPDIR)/libeplayer3.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_libeplayer3) && \
		$(INSTALL_libeplayer3)
#	$(DISTCLEANUP_libeplayer3)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# LIBMME-HOST
#
DESCRIPTION_libmme_host = "libmme-host"
SRC_URI_libmme_host = "https://code.google.com/p/tdt-amiko/"

FILES_libmme_host = \
/lib/libmme_host.*

$(DEPDIR)/libmme_host.do_prepare: bootstrap $(DEPENDS_libmme_host)
	$(PREPARE_libmme_host)
	touch $@

$(DEPDIR)/libmme_host.do_compile: $(DEPDIR)/libmme_host.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_libmme_host) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/libmme_host: \
$(DEPDIR)/%libmme_host: $(DEPDIR)/libmme_host.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_libmme_host) && \
		$(INSTALL_libmme_host)
#	$(DISTCLEANUP_libmme_host)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# LIBMMEIMAGE
#
DESCRIPTION_libmmeimage = "libmmeimage"
SRC_URI_libmmeimage = "https://code.google.com/p/tdt-amiko/"

FILES_libmmeimage = \
/lib/libmmeimage.*

$(DEPDIR)/libmmeimage.do_prepare: bootstrap $(DEPENDS_libmmeimage)
	$(PREPARE_libmmeimage)
	touch $@

$(DEPDIR)/libmmeimage.do_compile: $(DEPDIR)/libmmeimage.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_libmmeimage) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/libmmeimage: \
$(DEPDIR)/%libmmeimage: $(DEPDIR)/libmmeimage.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_libmmeimage) && \
		$(INSTALL_libmmeimage)
#	$(DISTCLEANUP_libmmeimage)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# SHOWIFRAME
#
DESCRIPTION_showiframe = "showiframe"
SRC_URI_showiframe = "https://code.google.com/p/tdt-amiko/"

FILES_showiframe = \
/bin/showiframe

$(DEPDIR)/showiframe.do_prepare: bootstrap $(DEPENDS_showiframe)
	$(PREPARE_showiframe)
	touch $@

$(DEPDIR)/showiframe.do_compile: $(DEPDIR)/showiframe.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_showiframe) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/showiframe: \
$(DEPDIR)/%showiframe: $(DEPDIR)/showiframe.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_showiframe) && \
		$(INSTALL_showiframe)
#	$(DISTCLEANUP_showiframe)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
#STFBCONTROL
#
DESCRIPTION_stfbcontrol = "stfbcontrol"
SRC_URI_stfbcontrol = "https://code.google.com/p/tdt-amiko/"

FILES_stfbcontrol = \
/bin/stfbcontrol

$(DEPDIR)/stfbcontrol.do_prepare: bootstrap $(DEPENDS_stfbcontrol)
	$(PREPARE_stfbcontrol)
	touch $@

$(DEPDIR)/stfbcontrol.do_compile: $(DEPDIR)/stfbcontrol.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_stfbcontrol) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/stfbcontrol: \
$(DEPDIR)/%stfbcontrol: $(DEPDIR)/stfbcontrol.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_stfbcontrol) && \
		$(INSTALL_stfbcontrol)
#	$(DISTCLEANUP_stfbcontrol)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# STREAMPROXY
#
DESCRIPTION_streamproxy = "streamproxy"
SRC_URI_streamproxy = "https://code.google.com/p/tdt-amiko/"

FILES_streamproxy = \
/bin/streamproxy

$(DEPDIR)/streamproxy.do_prepare: bootstrap $(DEPENDS_streamproxy)
	$(PREPARE_streamproxy)
	touch $@

$(DEPDIR)/streamproxy.do_compile: $(DEPDIR)/streamproxy.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_streamproxy) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/streamproxy: \
$(DEPDIR)/%streamproxy: $(DEPDIR)/streamproxy.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_streamproxy) && \
		$(INSTALL_streamproxy)
#	$(DISTCLEANUP_streamproxy)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# USTSLAVE
#
DESCRIPTION_ustslave = "ustslave"
SRC_URI_ustslave = "https://code.google.com/p/tdt-amiko/"

FILES_ustslave = \
/bin/ustslave

$(DEPDIR)/ustslave.do_prepare: bootstrap $(DEPENDS_ustslave)
	$(PREPARE_ustslave)
	touch $@

$(DEPDIR)/ustslave.do_compile: $(DEPDIR)/ustslave.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_ustslave) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/ustslave: \
$(DEPDIR)/%ustslave: $(DEPDIR)/ustslave.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_ustslave) && \
		$(INSTALL_ustslave)
#	$(DISTCLEANUP_ustslave)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

#
# EPLAYER4
#
DESCRIPTION_eplayer4 = "eplayer4"
SRC_URI_eplayer4 = "https://code.google.com/p/tdt-amiko/"

FILES_eplayer4 = \
/bin/eplayer4

$(DEPDIR)/eplayer4.do_prepare: bootstrap $(DEPENDS_eplayer4)
	$(PREPARE_eplayer4)
	touch $@

$(DEPDIR)/eplayer4.do_compile: $(DEPDIR)/eplayer4.do_prepare
	export PATH=$(hostprefix)/bin:$(PATH) && \
	cd $(DIR_eplayer4) && \
	libtoolize -f -c && \
	$(CONFIGURE) --prefix= \
	$(if $(MULTICOM324), --enable-multicom324) \
	$(if $(MULTICOM406), --enable-multicom406) \
	$(MAKE)
	touch $@

$(DEPDIR)/eplayer4: \
$(DEPDIR)/%eplayer4: $(DEPDIR)/eplayer4.do_compile
	$(start_build)
	$(get_git_version)
	cd $(DIR_eplayer4) && \
		$(INSTALL_eplayer4)
#	$(DISTCLEANUP_eplayer4)
	CPPFLAGS="\
	$(if $(HL101), -DPLATFORM_HL101) \
	$(if $(PLAYER191), -DPLAYER191)"
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true
