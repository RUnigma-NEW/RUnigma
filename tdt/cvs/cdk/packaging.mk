PARENT_PK = ''

PACKAGE_PARAMS_LIST := PKGV PKGR DESCRIPTION SECTION PRIORITY MAINTAINER LICENSE PACKAGE_ARCH HOMEPAGE RDEPENDS RREPLACES RCONFLICTS SRC_URI FILES NAME preinst postinst prerm postrm conffiles PACKAGES

EXPORT_ALLENV = $(shell echo '$(.VARIABLES) ' | awk -v RS=' ' '/^[a-zA-Z0-9_]+$$/')
EXPORT_ENV = $(filter PARENT_PK, $(EXPORT_ALLENV))
EXPORT_ENV += $(filter $(addsuffix _%, $(PACKAGE_PARAMS_LIST)), $(EXPORT_ALLENV))
export $(EXPORT_ENV)

export packagingtmpdir
export ipkgbuilddir
IPKGBUILDDIR := $(ipkgbuilddir)
export IPKGBUILDDIR

ipkcdk := $(prefix)/ipkcdk
ipkprefix := $(prefix)/ipkbox
ipkextras := $(prefix)/ipkextras
workprefix := $(prefix)/work

$(ipkcdk):
	$(INSTALL) -d $@

$(ipkprefix):
	$(INSTALL) -d $@

$(ipkextras):
	$(INSTALL) -d $@

$(workprefix):
	$(INSTALL) -d $@

define extra_build
	rm -rf $(ipkgbuilddir)/*
	$(flash_prebuild)
	python split_packages.py
	$(call do_build_pkg,none,flash)
endef

define e2extra_build
	rm -rf $(ipkgbuilddir)/*
	$(flash_prebuild)
	python split_packages.py
	$(call do_build_pkg,none,extra)
endef


define toflash_build
	rm -rf $(ipkgbuilddir)/*
	$(flash_prebuild)
	python split_packages.py
	$(call do_build_pkg,install,flash)
endef

define tocdk_build_start
	rm -rf $(ipkgbuilddir)/*
	export FILES_$(PARENT_PK)="/" && \
	python split_packages.py
	$(rewrite_libtool)
#	$(rewrite_pkgconfig)
	$(rewrite_dependency)
endef

define tocdk_build
	$(tocdk_build_start)
	$(call do_build_pkg,install,cdk)
endef

define fromrpm_build
	$(fromrpm_get)
	$(remove_includedir)
	$(toflash_build)
endef

flash_ipkg_args = -f $(crossprefix)/etc/opkg.conf -o $(prefix)/pkgroot
cdk_ipkg_args = -f $(crossprefix)/etc/opkg-cdk.conf -o $(targetprefix)

define do_build_pkg
	@echo
	@echo "====> do_build_pkg $(1) $(2)"
	for pkg in `ls $(ipkgbuilddir)`; do \
		ipkg-build -o root -g root $(ipkgbuilddir)/$$pkg \
		$(if $(filter cdk,$(2)),$(ipkcdk)) \
		$(if $(filter extra,$(2)),$(ipkextras)) \
		$(if $(filter flash,$(2)),$(ipkprefix)) |tee tmpname \
		&& \
			pkgn=`cat tmpname |perl -ne 'if (m/Packaged contents/) { print ((split / /)[-1])}'` && \
			(opkg --force-depends remove $(if $(filter cdk,$(2)),$(cdk_ipkg_args),$(flash_ipkg_args)) `echo $${pkg//_/-}| tr A-Z a-z` || true) && \
			in_extra="`echo $(EXTRA_$(PARENT_PK)) |tr ' ' '\n' |grep -x $$pkg ||true`" && \
			in_dist="`echo $(DIST_$(PARENT_PK)) |tr ' ' '\n' |grep -x $$pkg ||true`" && \
			if [ "install" = "$(1)" -a \( -z "$$in_extra" -o "$(2)" = "cdk" \) \
			   -o "$(2)" != "cdk" -a -n "$$in_dist" ]; then \
			opkg install $(if $(filter cdk,$(2)),$(cdk_ipkg_args),$(flash_ipkg_args)) $$pkgn \
			; else \
				echo "Not installing $$pkg" ; \
			fi \
		; done
endef


define start_build
	$(eval export $(EXPORT_ENV))$(warning MANUAL_EXPORT)
	@echo
	@echo "====> checking for PARENT_PK variable"
	$(eval $(if $(filter '',$(PARENT_PK)), $@: PARENT_PK = $(subst -,_,$(notdir $@))))
	$(eval export PARENT_PK)
	@echo "====> start_build $(PARENT_PK)"
	rm -rf $(PKDIR)
	mkdir $(PKDIR)
	$(if $(findstring $(PKGV_$(PARENT_PK)),git),
	@echo determine version from git
	$(AUTOPKGV_$(PARENT_PK))
	)
	$(if $(findstring $(PKGV_$(PARENT_PK)),svn),
	@echo determine version from svn
	$(AUTOPKGV_$(PARENT_PK))
	)
endef

define package_rpm_get
	$(shell rpm --dbpath $(prefix)/cdkroot-rpmdb --queryformat $1)
endef

define fromrpm_get
	$(eval export DESCRIPTION_$(PARENT_PK) = $(call package_rpm_get,'%{SUMMARY}' -qp $(lastword $^)))
	$(eval export PKGV_$(PARENT_PK) = $(call package_rpm_get,'%{VERSION}-%{RELEASE}' -qp $(lastword $^)))
	$(eval export SRC_URI_$(PARENT_PK) = "stlinux.com")
	@echo "rpm got descr $(DESCRIPTION_$(PARENT_PK))"
	@echo "rpm got version $(PKGV_$(PARENT_PK))"
	rm -rf rpmtmpdir
	mkdir rpmtmpdir
	bsdtar xf $(lastword $^) -C rpmtmpdir
	mv -v rpmtmpdir/$(targetprefix)/* $(PKDIR)
endef

define flash_prebuild
	$(remove_libs)
	$(remove_pkgconfigs)
	$(strip_libs)
	$(remove_docs)
endef

define remove_docs
	rm -rf $(PKDIR)/usr/share/doc
	rm -rf $(PKDIR)/usr/share/man
	rm -rf $(PKDIR)/usr/share/gtk-doc
endef

define strip_libs
	find $(PKDIR) -type f -regex '.*/lib/.*\.so\(\.[0-9]+\)*' \
		-exec echo strip {} \; \
		-exec sh4-linux-strip --strip-unneeded {} \;
endef

define remove_libs
	rm -f $(PKDIR)/lib/*.{a,la,o}
	rm -f $(PKDIR)/usr/lib/*.{a,la,o}
endef

define remove_pkgconfigs
	rm -rf $(PKDIR)/usr/lib/pkgconfig
endef

define remove_includedir
	rm -rf $(PKDIR)/usr/include
endef

define remove_pyo
	find $(PKDIR) -name "*.pyo" -type f -exec rm -f {} \;
	rm -rf $(PKDIR)/usr/lib/python$(PYTHON_VERSION)/site-packages/*-py$(PYTHON_VERSION).egg-info
endef

define remove_pyc
	find $(PKDIR) -name "*.pyc" -type f -exec rm -f {} \;
	rm -rf $(PKDIR)/usr/lib/python$(PYTHON_VERSION)/site-packages/*-py$(PYTHON_VERSION).egg-info
endef

define prepare_pkginfo_for_flash
	export OPKG_OFFLINE_ROOT=$(flashprefix)/root/ \
	for i in $(flashprefix)/root/usr/lib/opkg/info/*.preinst; do \
		if [ -f $i ] && ! sh $i; then \
			opkg-cl $(flash_ipkg_args) flag unpacked `basename $i .preinst` \
		fi \
	done \
	for i in $(flashprefix)/root/usr/lib/opkg/info/*.postinst; do \
		if [ -f $i ] && ! sh $i configure; then \
			opkg-cl $(flash_ipkg_args) flag unpacked `basename $i .postinst` \
		fi \
	done
endef

define rewrite_libtool
	 find $(ipkgbuilddir)/* -name "*.la" -type f -exec perl -pi -e "s,^libdir=.*$$,libdir='$(targetprefix)/usr/lib'," {} \;
endef

define rewrite_pkgconfig
	 find $(ipkgbuilddir)/* -name "*.pc" -type f -exec perl -pi -e "s,^prefix=.*$$,prefix=$(targetprefix)/usr," {} \;
endef

define rewrite_dependency
	find $(ipkgbuilddir)/* -name "*.la" -type f -exec \
		perl -pi -e "s, /usr/lib, $(targetprefix)/usr/lib,g if /^dependency_libs/" {} \;
endef

define parent_pk
	$(eval $@: PARENT_PK = $1)
endef

define git_fetch_prepare
	@echo git fetching $1
	$(eval DIR_$(1) ?= $(buildprefix)/$(1)-git)
	$(eval SRC_URI_$(1) += $(2))
	@echo 'setting dir DIR_$(1)=$(DIR_$(1))'
	@echo 'setting uri SRC_URI_$(1)=$(SRC_URI_$(1))'
	if [ -d $(DIR_$(1)) ]; then \
		cd $(DIR_$(1)) && git pull; \
	else \
		git clone $(2) $(DIR_$(1)); \
	fi
endef

opkgcdkl-%:
	opkg files $(cdk_ipkg_args) $*

opkgl-%:
	opkg files $(flash_ipkg_args) $*

opkgcdkl:
	opkg list-installed $(cdk_ipkg_args)

opkgl:
	opkg list-installed $(flash_ipkg_args)

opkg-sanitycheck:
	dup=`cat $(prefix)/pkgroot/usr/lib/opkg/info/*list |sort |uniq -d` && test -z $$dup \
		|| (echo "ERROR: opkg lists has duplicate files:"; echo $$dup; false)

.PHONY: package-index $(ipkprefix)/Packages package-index-extras $(ipkextras)/Packages
package-index: $(ipkprefix)/Packages
$(ipkprefix)/Packages: $(ipkprefix)
	cd $(ipkprefix) && \
		/usr/bin/python $(crossprefix)/bin/ipkg-make-index . > Packages && \
		cat Packages | gzip > Packages.gz

package-index-extras: $(ipkextras)/Packages
$(ipkextras)/Packages: $(ipkextras)
	cd $(ipkextras) && \
		/usr/bin/python $(crossprefix)/bin/ipkg-make-index . > Packages && \
		cat Packages | gzip > Packages.gz

svn_version := svn info | awk '/Revision:/ { print $$2 }'
get_svn_version = $(eval export PKGV_$(PARENT_PK) = $(shell cd $(DIR_$(PARENT_PK)) && $(svn_version)))

git_version := git log -1 --format=%cd --date=short -- . |sed s/-//g
get_git_version = $(eval export PKGV_$(PARENT_PK) = $(shell cd $(DIR_$(PARENT_PK)) && $(git_version)))
