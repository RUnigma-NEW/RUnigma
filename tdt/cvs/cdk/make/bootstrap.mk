##############################   BOOTSTRAP-HOST    #############################
#
# HOST-RPMCONFIG
#
HOST_RPMCONFIG = host-rpmconfig
HOST_RPMCONFIG_VERSION = 2.4-33
HOST_RPMCONFIG_SPEC = stm-$(HOST_RPMCONFIG).spec
HOST_RPMCONFIG_SPEC_PATCH = $(HOST_RPMCONFIG_SPEC).$(HOST_RPMCONFIG_VERSION).diff
HOST_RPMCONFIG_PATCHES = stm-$(HOST_RPMCONFIG)-$(HOST_RPMCONFIG_VERSION)-ignore-skip-cvs-errors.patch \
			 stm-$(HOST_RPMCONFIG)-$(HOST_RPMCONFIG_VERSION)-autoreconf-add-libtool-macros.patch

HOST_RPMCONFIG_RPM = RPMS/noarch/$(STLINUX)-$(HOST_RPMCONFIG)-$(HOST_RPMCONFIG_VERSION).noarch.rpm

$(HOST_RPMCONFIG_RPM): \
		$(addprefix Patches/,$(HOST_RPMCONFIG_SPEC_PATCH) $(HOST_RPMCONFIG_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_RPMCONFIG)-$(HOST_RPMCONFIG_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_RPMCONFIG_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_RPMCONFIG_SPEC) < $(buildprefix)/Patches/$(HOST_RPMCONFIG_SPEC_PATCH) ) &&) \
	$(if $(HOST_RPMCONFIG_PATCHES),cp $(addprefix Patches/,$(HOST_RPMCONFIG_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_RPMCONFIG_SPEC)

$(DEPDIR)/$(HOST_RPMCONFIG): $(HOST_RPMCONFIG_RPM)
	@rpm $(DRPM) --ignorearch --nodeps -Uhv --badreloc --relocate $(STM_RELOCATE)=$(prefix) $<
	touch $@

#
# HOST-LIBTOOL
#
HOST_LIBTOOL = host-libtool
HOST_LIBTOOL_VERSION = 2.2.8-4
HOST_LIBTOOL_SPEC = stm-$(HOST_LIBTOOL).spec
HOST_LIBTOOL_SPEC_PATCH =
HOST_LIBTOOL_PATCHES =

HOST_LIBTOOL_RPM = RPMS/sh4/$(STLINUX)-$(HOST_LIBTOOL)-$(HOST_LIBTOOL_VERSION).sh4.rpm

$(HOST_LIBTOOL_RPM): \
		$(addprefix Patches/,$(HOST_LIBTOOL_SPEC_PATCH) $(HOST_LIBTOOL_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_LIBTOOL)-$(HOST_LIBTOOL_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_LIBTOOL_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_LIBTOOL_SPEC) < $(buildprefix)/Patches/$(HOST_LIBTOOL_SPEC_PATCH) ) &&) \
	$(if $(HOST_LIBTOOL_PATCHES),cp $(addprefix Patches/,$(HOST_LIBTOOL_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_LIBTOOL_SPEC)

$(DEPDIR)/$(HOST_LIBTOOL): $(HOST_LIBTOOL_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $<
	touch $@

#
# HOST-BASE-PASSWD
#
HOST_BASE_PASSWD = host-base-passwd
HOST_BASE_PASSWD_VERSION = 3.5.9-7
HOST_BASE_PASSWD_SPEC = stm-$(HOST_BASE_PASSWD).spec
HOST_BASE_PASSWD_SPEC_PATCH =
HOST_BASE_PASSWD_PATCHES =

HOST_BASE_PASSWD_RPM = RPMS/sh4/$(STLINUX)-$(HOST_BASE_PASSWD)-$(HOST_BASE_PASSWD_VERSION).sh4.rpm

$(HOST_BASE_PASSWD_RPM): \
		$(addprefix Patches/,$(HOST_BASE_PASSWD_SPEC_PATCH) $(HOST_BASE_PASSWD_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_BASE_PASSWD)-$(HOST_BASE_PASSWD_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_BASE_PASSWD_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_BASE_PASSWD_SPEC) < $(buildprefix)/Patches/$(HOST_BASE_PASSWD_SPEC_PATCH) ) &&) \
	$(if $(HOST_BASE_PASSWD_PATCHES),cp $(addprefix Patches/,$(HOST_BASE_PASSWD_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_BASE_PASSWD_SPEC)

$(DEPDIR)/$(HOST_BASE_PASSWD): $(HOST_BASE_PASSWD_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# HOST-DISTRIBUTIONUTILS
#
HOST_DISTRIBUTIONUTILS = host-distributionutils
HOST_DISTRIBUTIONUTILS_VERSION = 2.8.4-6
HOST_DISTRIBUTIONUTILS_SPEC = stm-$(HOST_DISTRIBUTIONUTILS).spec
HOST_DISTRIBUTIONUTILS_SPEC_PATCH =
HOST_DISTRIBUTIONUTILS_PATCHES =

HOST_DISTRIBUTIONUTILS_RPM = RPMS/$(host_arch)/$(STLINUX)-$(HOST_DISTRIBUTIONUTILS)-$(HOST_DISTRIBUTIONUTILS_VERSION).$(host_arch).rpm

$(HOST_DISTRIBUTIONUTILS_RPM): \
		$(addprefix Patches/,$(HOST_DISTRIBUTIONUTILS_SPEC_PATCH) $(HOST_DISTRIBUTIONUTILS_PATCHES)) \
		$(archivedir)/$(STM_SRC)-$(HOST_DISTRIBUTIONUTILS)-$(HOST_DISTRIBUTIONUTILS_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_DISTRIBUTIONUTILS_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_DISTRIBUTIONUTILS_SPEC) < $(buildprefix)/Patches/$(HOST_DISTRIBUTIONUTILS_SPEC_PATCH) ) &&) \
	$(if $(HOST_DISTRIBUTIONUTILS_PATCHES),cp $(addprefix Patches/,$(HOST_DISTRIBUTIONUTILS_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_DISTRIBUTIONUTILS_SPEC)

$(DEPDIR)/$(HOST_DISTRIBUTIONUTILS): $(HOST_DISTRIBUTIONUTILS_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# HOST AUTOTOOLS
#
HOST_AUTOTOOLS = host-autotools
HOST_AUTOTOOLS_VERSION = dev-20091012-3
HOST_AUTOTOOLS_SPEC = stm-$(HOST_AUTOTOOLS)-dev.spec
HOST_AUTOTOOLS_SPEC_PATCH =
HOST_AUTOTOOLS_PATCHES =

HOST_AUTOTOOLS_RPM = RPMS/sh4/$(STLINUX)-$(HOST_AUTOTOOLS)-$(HOST_AUTOTOOLS_VERSION).sh4.rpm

$(HOST_AUTOTOOLS_RPM): \
		$(addprefix Patches/,$(HOST_AUTOTOOLS_SPEC_PATCH) $(HOST_AUTOTOOLS_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_AUTOTOOLS)-$(HOST_AUTOTOOLS_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_AUTOTOOLS_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_AUTOTOOLS_SPEC) < $(buildprefix)/Patches/$(HOST_AUTOTOOLS_SPEC_PATCH) ) &&) \
	$(if $(HOST_AUTOTOOLS_PATCHES),cp $(addprefix Patches/,$(HOST_AUTOTOOLS_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_AUTOTOOLS_SPEC)

$(DEPDIR)/$(HOST_AUTOTOOLS): $(HOST_AUTOTOOLS_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# HOST AUTOMAKE
#
#HOST_AUTOMAKE = host-automake
#HOST_AUTOMAKE_VERSION = 1.11.1-3
#HOST_AUTOMAKE_SPEC = stm-$(HOST_AUTOMAKE).spec
#HOST_AUTOMAKE_SPEC_PATCH =
#HOST_AUTOMAKE_PATCHES =
#
#HOST_AUTOMAKE_RPM = RPMS/sh4/$(STLINUX)-$(HOST_AUTOMAKE)-$(HOST_AUTOMAKE_VERSION).sh4.rpm
#
#$(HOST_AUTOMAKE_RPM): \
#		$(addprefix Patches/,$(HOST_AUTOMAKE_SPEC_PATCH) $(HOST_AUTOMAKE_PATCHES)) \
#		$(archivedir)/$(STLINUX)-$(HOST_AUTOMAKE)-$(HOST_AUTOMAKE_VERSION).src.rpm
#	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
#	$(if $(HOST_AUTOMAKE_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_AUTOMAKE_SPEC) < $(buildprefix)/Patches/$(HOST_AUTOMAKE_SPEC_PATCH) ) &&) \
#	$(if $(HOST_AUTOMAKE_PATCHES),cp $(addprefix Patches/,$(HOST_AUTOMAKE_PATCHES)) SOURCES/ &&) \
#	export PATH=$(hostprefix)/bin:$(PATH) && \
#	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_AUTOMAKE_SPEC)
#
#$(DEPDIR)/$(HOST_AUTOMAKE): $(HOST_AUTOMAKE_RPM)
#	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $<
#	touch $@

#
# HOST AUTOCONF
#
#HOST_AUTOCONF = host-autoconf
#HOST_AUTOCONF_VERSION = 2.64-6
#HOST_AUTOCONF_SPEC = stm-$(HOST_AUTOCONF).spec
#HOST_AUTOCONF_SPEC_PATCH = stm-$(HOST_AUTOCONF).$(HOST_AUTOCONF_VERSION).spec.diff
#HOST_AUTOCONF_PATCHES = stm-$(HOST_AUTOCONF).$(HOST_AUTOCONF_VERSION).diff
#
#HOST_AUTOCONF_RPM = RPMS/sh4/$(STLINUX)-$(HOST_AUTOCONF)-$(HOST_AUTOCONF_VERSION).sh4.rpm
#
#$(HOST_AUTOCONF_RPM): \
#		$(addprefix Patches/,$(HOST_AUTOCONF_SPEC_PATCH) $(HOST_AUTOCONF_PATCHES)) \
#		$(archivedir)/$(STLINUX)-$(HOST_AUTOCONF)-$(HOST_AUTOCONF_VERSION).src.rpm
#	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
#	$(if $(HOST_AUTOCONF_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_AUTOCONF_SPEC) < $(buildprefix)/Patches/$(HOST_AUTOCONF_SPEC_PATCH) ) &&) \
#	$(if $(HOST_AUTOCONF_PATCHES),cp $(addprefix Patches/,$(HOST_AUTOCONF_PATCHES)) SOURCES/ &&) \
#	export PATH=$(hostprefix)/bin:$(PATH) && \
#	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_AUTOCONF_SPEC)
#
#$(DEPDIR)/$(HOST_AUTOCONF): $(HOST_AUTOCONF_RPM)
#	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $<
#	touch $@

#
# HOST MODULE INIT TOOLS
#
HOST_MODINIT = host-module-init-tools
HOST_MODINIT_VERSION = 3.16-3
HOST_MODINIT_SPEC = stm-$(HOST_MODINIT).spec
HOST_MODINIT_SPEC_PATCH = $(HOST_MODINIT_SPEC).$(HOST_MODINIT_VERSION).diff
HOST_MODINIT_PATCHES = stm-$(HOST_MODINIT).$(HOST_MODINIT_VERSION)-no-man.diff

HOST_MODINIT_RPM = RPMS/sh4/$(STLINUX)-$(HOST_MODINIT)-$(HOST_MODINIT_VERSION).sh4.rpm

$(HOST_MODINIT_RPM): \
		$(addprefix Patches/,$(HOST_MODINIT_SPEC_PATCH) $(HOST_MODINIT_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_MODINIT)-$(HOST_MODINIT_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_MODINIT_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_MODINIT_SPEC) < $(buildprefix)/Patches/$(HOST_MODINIT_SPEC_PATCH) ) &&) \
	$(if $(HOST_MODINIT_PATCHES),cp $(addprefix Patches/,$(HOST_MODINIT_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_MODINIT_SPEC)

$(DEPDIR)/$(HOST_MODINIT): $(HOST_MODINIT_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# HOST PKGCONFIG
#
HOST_PKGCONFIG = host-pkg-config
HOST_PKGCONFIG_VERSION = 0.23-3
HOST_PKGCONFIG_SPEC = stm-$(HOST_PKGCONFIG).spec
HOST_PKGCONFIG_SPEC_PATCH =
HOST_PKGCONFIG_PATCHES =

HOST_PKGCONFIG_RPM = RPMS/sh4/$(STLINUX)-$(HOST_PKGCONFIG)-$(HOST_PKGCONFIG_VERSION).sh4.rpm

$(HOST_PKGCONFIG_RPM): \
		$(addprefix Patches/,$(HOST_PKGCONFIG_SPEC_PATCH) $(HOST_PKGCONFIG_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_PKGCONFIG)-$(HOST_PKGCONFIG_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_PKGCONFIG_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_PKGCONFIG_SPEC) < $(buildprefix)/Patches/$(HOST_PKGCONFIG_SPEC_PATCH) ) &&) \
	$(if $(HOST_PKGCONFIG_PATCHES),cp $(addprefix Patches/,$(HOST_PKGCONFIG_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_PKGCONFIG_SPEC)

$(DEPDIR)/$(HOST_PKGCONFIG): $(HOST_PKGCONFIG_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# HOST-MTD-UTILS
#
HOST_MTD_UTILS = host-mtd-utils
HOST_MTD_UTILS_VERSION = 1.0.1-8
HOST_MTD_UTILS_SPEC = stm-$(HOST_MTD_UTILS).spec
HOST_MTD_UTILS_SPEC_PATCH =
HOST_MTD_UTILS_PATCHES =

HOST_MTD_UTILS_RPM = RPMS/sh4/$(STLINUX)-$(HOST_MTD_UTILS)-$(HOST_MTD_UTILS_VERSION).sh4.rpm

$(HOST_MTD_UTILS_RPM): \
		$(addprefix Patches/,$(HOST_MTD_UTILS_SPEC_PATCH) $(HOST_MTD_UTILS_PATCHES)) \
		$(archivedir)/stlinux23-$(HOST_MTD_UTILS)-$(HOST_MTD_UTILS_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_MTD_UTILS_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_MTD_UTILS_SPEC) < $(buildprefix)/Patches/$(HOST_MTD_UTILS_SPEC_PATCH) ) &&) \
	$(if $(HOST_MTD_UTILS_PATCHES),cp $(addprefix Patches/,$(HOST_MTD_UTILS_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_MTD_UTILS_SPEC)

$(DEPDIR)/$(HOST_MTD_UTILS): $(HOST_MTD_UTILS_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# HOST FLEX
#
HOST_FLEX = host-flex
HOST_FLEX_VERSION = 2.5.35-2
HOST_FLEX_SPEC = stm-$(HOST_FLEX).spec
HOST_FLEX_SPEC_PATCH =
HOST_FLEX_PATCHES =

HOST_FLEX_RPM = RPMS/sh4/$(STLINUX)-$(HOST_FLEX)-$(HOST_FLEX_VERSION).sh4.rpm

$(HOST_FLEX_RPM): \
		$(addprefix Patches/,$(HOST_FLEX_SPEC_PATCH) $(HOST_FLEX_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_FLEX)-$(HOST_FLEX_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_FLEX_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_FLEX_SPEC) < $(buildprefix)/Patches/$(HOST_FLEX_SPEC_PATCH) ) &&) \
	$(if $(HOST_FLEX_PATCHES),cp $(addprefix Patches/,$(HOST_FLEX_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_FLEX_SPEC)

$(DEPDIR)/$(HOST_FLEX): $(HOST_FLEX_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# HOST LIBFFI
#
HOST_LIBFFI = host-libffi-dev
HOST_LIBFFI_VERSION = 3.0.10-2
HOST_LIBFFI_SPEC = stm-$(HOST_LIBFFI).spec
HOST_LIBFFI_SPEC_PATCH = #$(HOST_LIBFFI_SPEC).$(HOST_LIBFFI_VERSION).diff
HOST_LIBFFI_PATCHES =

HOST_LIBFFI_RPM = RPMS/sh4/$(STLINUX)-$(HOST_LIBFFI)-$(HOST_LIBFFI_VERSION).sh4.rpm

$(HOST_LIBFFI_RPM): \
		$(addprefix Patches/,$(HOST_LIBFFI_SPEC_PATCH) $(HOST_LIBFFI_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_LIBFFI)-$(HOST_LIBFFI_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_LIBFFI_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_LIBFFI_SPEC) < $(buildprefix)/Patches/$(HOST_LIBFFI_SPEC_PATCH) ) &&) \
	$(if $(HOST_LIBFFI_PATCHES),cp $(addprefix Patches/,$(HOST_LIBFFI_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_LIBFFI_SPEC)

$(DEPDIR)/$(HOST_LIBFFI): $(HOST_LIBFFI_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $<
	touch $@

#
# HOST GLIB2
#
HOST_GLIB2 = host-glib2
HOST_GLIB2_VERSION = 2.32.1-26
HOST_GLIB2_SPEC = stm-$(HOST_GLIB2).spec
HOST_GLIB2_SPEC_PATCH = $(HOST_GLIB2_SPEC).$(HOST_GLIB2_VERSION).diff
HOST_GLIB2_PATCHES =

HOST_GLIB2_RPM = RPMS/sh4/$(STLINUX)-$(HOST_GLIB2)-$(HOST_GLIB2_VERSION).sh4.rpm

$(HOST_GLIB2_RPM): \
		$(addprefix Patches/,$(HOST_GLIB2_SPEC_PATCH) $(HOST_GLIB2_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(HOST_GLIB2)-$(HOST_GLIB2_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(HOST_GLIB2_SPEC_PATCH),( cd SPECS && patch -p1 $(HOST_GLIB2_SPEC) < $(buildprefix)/Patches/$(HOST_GLIB2_SPEC_PATCH) ) &&) \
	$(if $(HOST_GLIB2_PATCHES),cp $(addprefix Patches/,$(HOST_GLIB2_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(HOST_GLIB2_SPEC)

$(DEPDIR)/$(HOST_GLIB2): $(HOST_LIBFFI) $(HOST_GLIB2_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $<
	touch $@

########################################   BOOTSTRAP-CROSS   ########################################
#
# CROSS_DISTRIBUTIONUTILS
#
CROSS_DISTRIBUTIONUTILS = cross-sh4-distributionutils
CROSS_DISTRIBUTIONUTILS_VERSION = 1.14-6
CROSS_DISTRIBUTIONUTILS_SPEC = stm-$(subst cross-sh4,cross,$(CROSS_DISTRIBUTIONUTILS)).spec
CROSS_DISTRIBUTIONUTILS_SPEC_PATCH =
CROSS_DISTRIBUTIONUTILS_PATCHES =

CROSS_DISTRIBUTIONUTILS_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_DISTRIBUTIONUTILS)-$(CROSS_DISTRIBUTIONUTILS_VERSION).$(host_arch).rpm

$(CROSS_DISTRIBUTIONUTILS_RPM): \
		$(addprefix Patches/,$(CROSS_DISTRIBUTIONUTILS_SPEC_PATCH) $(CROSS_DISTRIBUTIONUTILS_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(subst cross-sh4,cross,$(CROSS_DISTRIBUTIONUTILS))-$(CROSS_DISTRIBUTIONUTILS_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(CROSS_DISTRIBUTIONUTILS_SPEC_PATCH),( cd SPECS && patch -p1 $(CROSS_DISTRIBUTIONUTILS_SPEC) < $(buildprefix)/Patches/$(CROSS_DISTRIBUTIONUTILS_SPEC_PATCH) ) &&) \
	$(if $(CROSS_DISTRIBUTIONUTILS_PATCHES),cp $(addprefix Patches/,$(CROSS_DISTRIBUTIONUTILS_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(CROSS_DISTRIBUTIONUTILS_SPEC)

$(DEPDIR)/$(CROSS_DISTRIBUTIONUTILS): $(CROSS_DISTRIBUTIONUTILS_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

#
# CROSS BINUTILS
#
CROSS_BINUTILS = cross-sh4-binutils
CROSS_BINUTILS_DEV = cross-sh4-binutils-dev
CROSS_BINUTILS_VERSION = 2.23.2-71
CROSS_BINUTILS_SPEC = stm-$(subst cross-sh4,cross,$(CROSS_BINUTILS)).spec
CROSS_BINUTILS_SPEC_PATCH = $(CROSS_BINUTILS_SPEC).$(CROSS_BINUTILS_VERSION).diff
CROSS_BINUTILS_PATCHES =

CROSS_BINUTILS_RPM = RPMS/${host_arch}/$(STLINUX)-$(CROSS_BINUTILS)-$(CROSS_BINUTILS_VERSION).${host_arch}.rpm
CROSS_BINUTILS_DEV_RPM = RPMS/${host_arch}/$(STLINUX)-$(CROSS_BINUTILS_DEV)-$(CROSS_BINUTILS_VERSION).${host_arch}.rpm

$(CROSS_BINUTILS_RPM) $(CROSS_BINUTILS_DEV_RPM) : \
		$(addprefix Patches/,$(CROSS_BINUTILS_SPEC_PATCH) $(CROSS_BINUTILS_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(subst cross-sh4-,cross-,$(CROSS_BINUTILS))-$(CROSS_BINUTILS_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(CROSS_BINUTILS_SPEC_PATCH),( cd SPECS && patch -p1 $(CROSS_BINUTILS_SPEC) < $(buildprefix)/Patches/$(CROSS_BINUTILS_SPEC_PATCH) ) &&) \
	$(if $(CROSS_BINUTILS_PATCHES),cp $(addprefix Patches/,$(CROSS_BINUTILS_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --nodeps --target=sh4-linux SPECS/$(CROSS_BINUTILS_SPEC)

$(DEPDIR)/$(CROSS_BINUTILS): $(CROSS_BINUTILS_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

$(DEPDIR)/$(CROSS_BINUTILS_DEV): $(CROSS_BINUTILS_DEV_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps --noscripts -Uhv $< && \
	touch $@

#
# CROSS GMP
#
CROSS_GMP = cross-sh4-gmp
CROSS_GMP_VERSION = 5.1.0-12
CROSS_GMP_SPEC = stm-$(subst cross-sh4,cross,$(CROSS_GMP)).spec
CROSS_GMP_SPEC_PATCH = #$(CROSS_GMP_SPEC).$(CROSS_GMP_VERSION).diff
CROSS_GMP_PATCHES =

CROSS_GMP_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_GMP)-$(CROSS_GMP_VERSION).$(host_arch).rpm

$(CROSS_GMP_RPM): \
		$(addprefix Patches/,$(CROSS_GMP_SPEC_PATCH) $(CROSS_GMP_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(subst cross-sh4-,cross-,$(CROSS_GMP))-$(CROSS_GMP_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(CROSS_GMP_SPEC_PATCH),( cd SPECS && patch -p1 $(CROSS_GMP_SPEC) < $(buildprefix)/Patches/$(CROSS_GMP_SPEC_PATCH) ) &&) \
	$(if $(CROSS_GMP_PATCHES),cp $(addprefix Patches/,$(CROSS_GMP_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --target=sh4-linux SPECS/$(CROSS_GMP_SPEC)

$(DEPDIR)/$(CROSS_GMP): $(CROSS_GMP_RPM)
	@rpm $(DRPM) --nodeps -Uhv $(lastword $^) && \
	touch $@
#
# CROSS MPFR
#
CROSS_MPFR = cross-sh4-mpfr
CROSS_MPFR_VERSION = 3.1.2-13
CROSS_MPFR_SPEC = stm-$(subst cross-sh4,cross,$(CROSS_MPFR)).spec
CROSS_MPFR_SPEC_PATCH =
CROSS_MPFR_PATCHES =

CROSS_MPFR_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_MPFR)-$(CROSS_MPFR_VERSION).$(host_arch).rpm

$(CROSS_MPFR_RPM): \
		$(addprefix Patches/,$(CROSS_MPFR_SPEC_PATCH) $(CROSS_MPFR_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(subst cross-sh4-,cross-,$(CROSS_MPFR))-$(CROSS_MPFR_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(CROSS_MPFR_SPEC_PATCH),( cd SPECS && patch -p1 $(CROSS_MPFR_SPEC) < $(buildprefix)/Patches/$(CROSS_MPFR_SPEC_PATCH) ) &&) \
	$(if $(CROSS_MPFR_PATCHES),cp $(addprefix Patches/,$(CROSS_MPFR_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(CROSS_MPFR_SPEC)

$(DEPDIR)/$(CROSS_MPFR): $(CROSS_MPFR_RPM)
	@rpm $(DRPM) --nodeps -Uhv $(lastword $^) && \
	touch $@

#
# CROSS MPC
#
CROSS_MPC = cross-sh4-mpc
CROSS_MPC_VERSION = 1.0.1-6
CROSS_MPC_SPEC = stm-$(subst cross-sh4,cross,$(CROSS_MPC)).spec
CROSS_MPC_SPEC_PATCH =
CROSS_MPC_PATCHES =

CROSS_MPC_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_MPC)-$(CROSS_MPC_VERSION).$(host_arch).rpm

$(CROSS_MPC_RPM): \
		$(addprefix Patches/,$(CROSS_MPC_SPEC_PATCH) $(CROSS_MPC_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(subst cross-sh4-,cross-,$(CROSS_MPC))-$(CROSS_MPC_VERSION).src.rpm
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(CROSS_MPC_SPEC_PATCH),( cd SPECS && patch -p1 $(CROSS_MPC_SPEC) < $(buildprefix)/Patches/$(CROSS_MPC_SPEC_PATCH) ) &&) \
	$(if $(CROSS_MPC_PATCHES),cp $(addprefix Patches/,$(CROSS_MPC_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(CROSS_MPC_SPEC)

$(DEPDIR)/$(CROSS_MPC): $(CROSS_MPC_RPM)
	@rpm $(DRPM) --nodeps -Uhv $(lastword $^) && \
	touch $@

#
# CROSS LIBELF
#
CROSS_LIBELF = cross-sh4-libelf
CROSS_LIBELF_VERSION = 0.8.13-2
CROSS_LIBELF_SPEC = stm-$(subst cross-sh4,cross,$(CROSS_LIBELF)).spec
CROSS_LIBELF_SPEC_PATCH =
CROSS_LIBELF_PATCHES =

CROSS_LIBELF_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_LIBELF)-$(CROSS_LIBELF_VERSION).$(host_arch).rpm

$(CROSS_LIBELF_RPM): \
		$(addprefix Patches/,$(CROSS_LIBELF_SPEC_PATCH) $(CROSS_LIBELF_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(subst cross-sh4-,cross-,$(CROSS_LIBELF))-$(CROSS_LIBELF_VERSION).src.rpm
	rpm  $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(CROSS_LIBELF_SPEC_PATCH),( cd SPECS && patch -p1 $(CROSS_LIBELF_SPEC) < $(buildprefix)/Patches/$(CROSS_LIBELF_SPEC_PATCH) ) &&) \
	$(if $(CROSS_LIBELF_PATCHES),cp $(addprefix Patches/,$(CROSS_LIBELF_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(CROSS_LIBELF_SPEC)

$(DEPDIR)/$(CROSS_LIBELF): $(CROSS_LIBELF_RPM)
	@rpm $(DRPM) --nodeps -Uhv $(lastword $^) && \
	touch $@

#
# CROSS GCC
#
CROSS_GCC = cross-sh4-gcc
CROSS_CPP = cross-sh4-cpp
CROSS_G++ = cross-sh4-g++
CROSS_PROTOIZE = cross-sh4-protoize
CROSS_LIBGCC = cross-sh4-libgcc
CROSS_GCC_VERSION = 4.8.2-126
CROSS_GCC_RAWVERSION = $(firstword $(subst -, ,$(CROSS_GCC_VERSION)))
CROSS_GCC_SPEC = stm-$(subst cross-sh4-,cross-,$(CROSS_GCC)).spec
CROSS_GCC_SPEC_PATCH = $(CROSS_GCC_SPEC).$(CROSS_GCC_VERSION).diff
CROSS_GCC_PATCHES = stm-cross-$(GCC).$(CROSS_GCC_VERSION).diff
CROSS_GCC_KERNELHEADERS = linux-kernel-headers
CROSS_GCC_INVALIDATE =
CROSS_GCC_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_GCC)-$(CROSS_GCC_VERSION).$(host_arch).rpm
CROSS_CPP_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_CPP)-$(CROSS_GCC_VERSION).$(host_arch).rpm
CROSS_G++_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_G++)-$(CROSS_GCC_VERSION).$(host_arch).rpm
CROSS_PROTOIZE_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_PROTOIZE)-$(CROSS_GCC_VERSION).$(host_arch).rpm
CROSS_LIBGCC_RPM = RPMS/$(host_arch)/$(STLINUX)-$(CROSS_LIBGCC)-$(CROSS_GCC_VERSION).$(host_arch).rpm

$(CROSS_GCC_RPM) $(CROSS_CPP_RPM) $(CROSS_G++_RPM) $(CROSS_PROTOIZE_RPM) $(CROSS_LIBGCC_RPM): \
		$(addprefix Patches/,$(CROSS_GCC_SPEC_PATCH) $(CROSS_GCC_PATCHES)) \
		$(archivedir)/$(STLINUX)-$(subst cross-sh4-,cross-,$(CROSS_GCC))-$(CROSS_GCC_VERSION).src.rpm \
		| $(archivedir)/$(STLINUX)-sh4-$(GLIBC)-$(GLIBC_VERSION).sh4.rpm \
		$(archivedir)/$(STLINUX)-sh4-$(GLIBC_DEV)-$(GLIBC_VERSION).sh4.rpm \
		$(if $(KERNELHEADERS),$(KERNELHEADERS)) \
		kernel-headers
	rpm $(DRPM) --nosignature --ignorearch --nodeps --force -Uhv \
		--badreloc --relocate $(STM_RELOCATE)/devkit/sh4/target=$(targetprefix) $(word 1,$|) && \
	rpm $(DRPM) --nosignature --ignorearch --nodeps --force -Uhv \
		--badreloc --relocate $(STM_RELOCATE)/devkit/sh4/target=$(targetprefix) $(word 2,$|)
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(CROSS_GCC_SPEC_PATCH),( cd SPECS && patch -p1 $(CROSS_GCC_SPEC) < $(buildprefix)/Patches/$(CROSS_GCC_SPEC_PATCH) ) &&) \
	$(if $(CROSS_GCC_PATCHES),cp $(addprefix Patches/,$(CROSS_GCC_PATCHES)) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild  $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(CROSS_GCC_SPEC)
	rpm $(DRPM) --nodeps -ev $(STLINUX)-sh4-$(GLIBC_DEV) && \
	rpm $(DRPM) --nodeps -ev $(STLINUX)-sh4-$(GLIBC)

$(DEPDIR)/$(CROSS_GCC): $(CROSS_GCC_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	$(if $(CROSS_GCC_INVALIDATE),sh4-linux-objcopy -v --redefine-sym __ic_invalidate_syscall=__ic_invalidate $(prefix)/devkit/sh4/lib/gcc/sh4-linux/$(CROSS_GCC_RAWVERSION)/libgcc.a &&) \
	touch $@

$(DEPDIR)/$(CROSS_CPP): $(CROSS_CPP_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

$(DEPDIR)/$(CROSS_G++): $(CROSS_G++_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $< && \
	touch $@

$(DEPDIR)/$(CROSS_LIBGCC): \
$(DEPDIR)/%$(CROSS_LIBGCC): $(CROSS_LIBGCC_RPM) | $(DEPDIR)/%$(GLIBC)
	@rpm --dbpath $(prefix)/$*cdkroot-rpmdb  $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^) && \
	touch $@
	$(start_build)
	$(fromrpm_build)

$(DEPDIR)/$(CROSS_PROTOIZE): $(CROSS_PROTOIZE_RPM)
	@rpm  $(DRPM) --ignorearch --nodeps -Uhv $<
	touch $@

##############################                     #############################

#
# HOST-FILESYSTEM
#
host-filesystem:
	$(INSTALL) -d $(prefix)
	$(INSTALL) -d $(prefix)/work
	$(INSTALL) -d $(configprefix)
	$(INSTALL) -d $(devkitprefix)
	$(INSTALL) -d $(hostprefix)
	$(INSTALL) -d $(hostprefix)/{bin,doc,etc,include,info,lib,man,share,var}
#	ln -sf $(hostprefix)/lib $(hostprefix)/lib64
	$(INSTALL) -d $(hostprefix)/man/man{1..9}
	touch .deps/$@

#
# CROSS_FILESYSTEM
#
cross-sh4-filesystem:
	$(INSTALL) -d $(targetprefix)
	$(INSTALL) -d $(crossprefix)
	$(INSTALL) -d $(crossprefix)/{bin,doc,etc,include,lib,man,sh4-linux,share,var}
#	ln -s /$(crossprefix)/lib $(crossprefix)/lib64
	$(INSTALL) -d $(crossprefix)/man/man{1..9}
	$(INSTALL) -d $(crossprefix)/sh4-linux/{bin,include,lib}
	touch .deps/$@

#
# BOOTSTRAP-HOST
#
bootstrap-host: | \
	host-filesystem \
	cross-sh4-filesystem \
	$(CCACHE) \
	host_libtool \
	host-rpmconfig \
	host_autoconf \
	host_pkgconfig \
	host-autotools \
	host_automake \
	host-base-passwd \
	host-distributionutils \
	host-module-init-tools \
	host-flex
	touch .deps/$@

#
# BOOTSTRAP-CROSS
#
$(DEPDIR)/bootstrap-cross: | \
	bootstrap-host \
	cross-sh4-distributionutils \
	cross-sh4-gmp \
	cross-sh4-mpfr \
	cross-sh4-mpc \
	cross-sh4-binutils \
	cross-sh4-binutils-dev \
	cross-sh4-cpp \
	cross-sh4-gcc \
	cross-sh4-g++ \
	ipkg-utils \
	opkg-host \
	cross-sh4-libgcc
	touch -r $(CROSS_G++_RPM) $@

$(DEPDIR)/setup-cross-doc: \
	cross-binutils-doc \
	cross-sh4-cpp-doc \
	cross-sh4-gcc-doc \
	cross-sh4-g++-doc

$(DEPDIR)/setup-cross-optional: \
	cross-sh4-protoize

#
# host_libtool
#
BEGIN[[
host_libtool
  2.4.2
  libtool-{PV}
  extract:http://ftp.gnu.org/gnu/libtool/libtool-{PV}.tar.xz
  patch:file://libtool-{PV}-duckbox-branding.patch
  make:install
;
]]END

$(DEPDIR)/host_libtool: $(DEPENDS_libtool)
	$(PREPARE_host_libtool)
	cd $(DIR_host_libtool) && \
		./configure \
			lt_cv_sys_dlsearch_path="" \
			--prefix=$(hostprefix) && \
		$(MAKE) && \
		$(INSTALL_host_libtool)
	$(DISTCLEANUP_host_libtool)
	touch $@

#
# host_autoconf
#
BEGIN[[
host_autoconf
  2.64
  autoconf-{PV}
  extract:http://ftp.gnu.org/gnu/autoconf/autoconf-{PV}.tar.xz
  patch:file://autoconf-remove-usr-local-lib-from-m4.patch
  make:install
;
]]END

$(DEPDIR)/host_autoconf: $(DEPENDS_host_autoconf)
	$(PREPARE_host_autoconf)
	cd $(DIR_host_autoconf) && \
		./configure \
			--prefix=$(hostprefix) && \
		$(MAKE) && \
		$(INSTALL_host_autoconf)
	$(DISTCLEANUP_host_autoconf)
	touch $@

#
# host_automake
#
BEGIN[[
host_automake
  1.11.6
  automake-{PV}
  extract:http://ftp.gnu.org/gnu/automake/automake-{PV}.tar.xz
  patch:file://automake-py-compile-compile-only-optimized-byte-code.patch
  make:install
;
]]END

$(DEPDIR)/host_automake: $(DEPENDS_host_automake)
	$(PREPARE_host_automake)
	cd $(DIR_host_automake) && \
		./configure \
			--prefix=$(hostprefix) && \
		$(MAKE) && \
		$(INSTALL_host_automake)
	$(DISTCLEANUP_host_automake)
	touch $@

#
# host_pkgconfig
#			--with-system-include-path=$(TARGETINCLUDE)
#			--with-system-library-path=$(TARGETLIB)
BEGIN[[
host_pkgconfig
  0.28
  pkg-config-{PV}
  extract:http://pkgconfig.freedesktop.org/releases/pkg-config-{PV}.tar.gz
  make:install
;
]]END

$(DEPDIR)/host_pkgconfig: $(DEPENDS_host_pkgconfig)
	$(PREPARE_host_pkgconfig)
	cd $(DIR_host_pkgconfig) && \
		./configure \
			--prefix=$(hostprefix) \
			--with-pc_path=$(PKG_CONFIG_PATH) && \
		$(MAKE) && \
		$(INSTALL_host_pkgconfig)
		ln -sf $(hostprefix)/bin/pkg-config $(crossprefix)/bin/$(target)-pkg-config
	$(DISTCLEANUP_host_pkgconfig)
	touch $@
