#######################################      #########################################

export CFLAGS
export CXXFLAGS

export DRPM
export DRPMBUILD

AUTOMAKE_OPTIONS = -Wno-portability

#######################################      #########################################

KERNEL_DIR = $(DIR_linux)
KERNEL_DEPENDS = $(DEPENDS_linux24)
KERNEL_PREPARE = $(PREPARE_linux24)
KERNELVERSION = $(VERSION_linux)
KERNELSTMLABEL = _$(word 2,$(subst _, ,$(KERNELVERSION)))_$(word 3,$(subst _, ,$(KERNELVERSION)))
KERNELLABEL = $(shell x=$(KERNELVERSION); echo $${x: -3})

#######################################      #########################################

STLINUX := stlinux24
STM_SRC := $(STLINUX)
STM_RELOCATE := /opt/STM/STLinux-2.4

#######################################      #########################################
#
# CCACHE
#
if ENABLE_CCACHE
PATH := $(hostprefix)/ccache-bin:$(crossprefix)/bin:$(PATH):/usr/sbin
else
PATH := $(crossprefix)/bin:$(PATH):/usr/sbin
endif

DEPMOD = $(hostprefix)/bin/depmod
SOCKSIFY=
CMD_CVS=$(SOCKSIFY) $(shell which cvs)
WGET=$(SOCKSIFY) wget

INSTALL_DIR=$(INSTALL) -d
INSTALL_BIN=$(INSTALL) -m 755
INSTALL_FILE=$(INSTALL) -m 644
LN_SF=$(shell which ln) -sf
CP_D=$(shell which cp) -d
CP_P=$(shell which cp) -p
CP_RD=$(shell which cp) -rd
SED=$(shell which sed)

MAKE_PATH := $(hostprefix)/bin:$(crossprefix)/bin:$(PATH)

ADAPTED_ETC_FILES =
ETC_RW_FILES =

# rpm helper-"functions":
TARGETLIB = $(targetprefix)/usr/lib
PKG_CONFIG_PATH = $(targetprefix)/usr/lib/pkgconfig
REWRITE_LIBDIR = sed -i "s,^libdir=.*,libdir='$(TARGETLIB)'," $(TARGETLIB)
REWRITE_LIBDEP = sed -i -e "s,\(^dependency_libs='\| \|-L\|^dependency_libs='\)/usr/lib,\$(TARGETLIB)," $(TARGETLIB)
REWRITE_PKGCONF = sed -i "s,^prefix=.*,prefix=$(targetprefix)/usr,"

BUILDENV := \
	CC=$(target)-gcc \
	CXX=$(target)-g++ \
	LD=$(target)-ld \
	NM=$(target)-nm \
	AR=$(target)-ar \
	AS=$(target)-as \
	RANLIB=$(target)-ranlib \
	STRIP=$(target)-strip \
	OBJCOPY=$(target)-objcopy \
	OBJDUMP=$(target)-objdump \
	LN_S="ln -s" \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS) -Wl,-rpath-link,$(packagingtmpdir)/usr/lib" \
	PKG_CONFIG_SYSROOT_DIR="$(targetprefix)" \
	PKG_CONFIG_LIBDIR="$(targetprefix)/usr/lib/pkgconfig"

MAKE_OPTS := \
	CC=$(target)-gcc \
	CXX=$(target)-g++ \
	LD=$(target)-ld \
	NM=$(target)-nm \
	AR=$(target)-ar \
	AS=$(target)-as \
	RANLIB=$(target)-ranlib \
	STRIP=$(target)-strip \
	OBJCOPY=$(target)-objcopy \
	OBJDUMP=$(target)-objdump \
	LN_S="ln -s" \
	ARCH=sh \
	CROSS_COMPILE=$(target)-

MAKE_ARGS := \
	CC=$(target)-gcc \
	CXX=$(target)-g++ \
	LD=$(target)-ld \
	NM=$(target)-nm \
	AR=$(target)-ar \
	AS=$(target)-as \
	RANLIB=$(target)-ranlib \
	STRIP=$(target)-strip \
	OBJCOPY=$(target)-objcopy \
	OBJDUMP=$(target)-objdump \
	LN_S="ln -s"

PLATFORM_CPPFLAGS := \
	$(if $(HL101),CPPFLAGS="$(CPPFLAGS) -DPLATFORM_HL101 -I$(driverdir)/include -I $(buildprefix)/$(KERNEL_DIR)/include" --enable-hl101) \
	$(if $(SPARK),CPPFLAGS="$(CPPFLAGS) -DPLATFORM_SPARK -I$(driverdir)/include -I $(buildprefix)/$(KERNEL_DIR)/include") \
	$(if $(SPARK7162),CPPFLAGS="$(CPPFLAGS) -DPLATFORM_SPARK7162 -I$(driverdir)/include -I $(buildprefix)/$(KERNEL_DIR)/include")

DEPDIR = .deps

VPATH = $(DEPDIR)

CONFIGURE_OPTS = \
	--build=$(build) \
	--host=$(target) \
	--prefix=$(targetprefix)/usr \
	--with-driver=$(driverdir) \
	--with-dvbincludes=$(driverdir)/include \
	--with-target=cdk

if ENABLE_CCACHE
CONFIGURE_OPTS += \
	--enable-ccache
endif

if MAINTAINER_MODE
CONFIGURE_OPTS += --enable-maintainer-mode
endif

CONFIGURE = \
	./autogen.sh && \
	CC=$(target)-gcc \
	CXX=$(target)-g++ \
	CFLAGS="-Wall $(TARGET_CFLAGS)" \
	CXXFLAGS="-Wall $(TARGET_CXXFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	./configure $(CONFIGURE_OPTS)

PYTHON_VERSION = $(word 1,$(subst ., ,$(VERSION_python))).$(word 2,$(subst ., ,$(VERSION_python)))
PYTHON_DIR = /usr/lib/python$(PYTHON_VERSION)

ACLOCAL_AMFLAGS = -I .

CONFIG_STATUS_DEPENDENCIES = \
	$(top_srcdir)/smart-rules.pl
#	$(top_srcdir)/smart-rules.am

min-query std-query max-query query: \
%query:
	rpm --dbpath $(prefix)/$*cdkroot-rpmdb $(DRPM) -qa

query-%:
	@for i in sh4 noarch ${host_arch} ; do \
		FOUND=`ls RPMS/$$i | grep $*` || true && \
		( for j in $$FOUND ; do \
			echo "RPMS/$$i/$$j:" && \
			rpm $(DRPM) -qplv --scripts RPMS/$$i/$$j || true; echo;done ) || true ; done


# -----------------------------------------
# Config gui
# Usage:
#   palce $(eval $(call guiconfig,util_name)) somewhere in *mk file
#   then call make util_name.xcofig
#
define guiconfig

$(1).menuconfig $(1).xconfig: \
$(1).%:
	$(MAKE) -C $(DIR_$(1)) ARCH=sh CROSS_COMPILE=sh4-linux- $$*
	@echo
	@echo "You have to edit m a n u a l l y Patches/...$(1)...*config to make changes permanent !!!"
	@echo ""
	diff $(DIR_$(1))/.config.old $(DIR_$(1))/.config
	@echo ""

endef
