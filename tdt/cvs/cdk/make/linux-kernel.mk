############ Patches Kernel 24 ###############

ifdef ENABLE_P0207
PATCH_STR=_0207
endif

ifdef ENABLE_P0209
PATCH_STR=_0209
endif

ifdef ENABLE_P0210
PATCH_STR=_0210
endif

ifdef ENABLE_P0211
PATCH_STR=_0211
endif

STM24_DVB_PATCH = linux-sh4-linuxdvb_stm24$(PATCH_STR).patch

COMMONPATCHES_24 = \
		$(STM24_DVB_PATCH) \
		linux-sh4-sound_stm24$(PATCH_STR).patch \
		linux-sh4-time_stm24$(PATCH_STR).patch \
		linux-sh4-init_mm_stm24$(PATCH_STR).patch \
		linux-sh4-copro_stm24$(PATCH_STR).patch \
		linux-sh4-strcpy_stm24$(PATCH_STR).patch \
		linux-squashfs-lzma_stm24$(PATCH_STR).patch \
		linux-sh4-ext23_as_ext4_stm24$(PATCH_STR).patch \
		bpa2_procfs_stm24$(PATCH_STR).patch \
		$(if $(P0207),xchg_fix_stm24$(PATCH_STR).patch) \
		$(if $(P0207),mm_cache_update_stm24$(PATCH_STR).patch) \
		$(if $(P0207),linux-sh4-ehci_stm24$(PATCH_STR).patch) \
		linux-ftdi_sio.c_stm24$(PATCH_STR).patch \
		linux-sh4-lzma-fix_stm24$(PATCH_STR).patch \
		linux-tune_stm24.patch \
		$(if $(P0210)$(P0211),linux-sh4-mmap_stm24.patch) \
		$(if $(P0210)$(P0211),linux-sh4-remove_pcm_reader_stm24.patch) \
		$(if $(P0207),linux-sh4-sti7100_missing_clk_alias_stm24$(PATCH_STR).patch) \
		$(if $(P0211),linux-sh4-directfb_stm24$(PATCH_STR).patch)

HL101_PATCHES_24 = $(COMMONPATCHES_24) \
		linux-sh4-hl101_setup_stm24$(PATCH_STR).patch \
		linux-usbwait123_stm24.patch \
		linux-sh4-stmmac_stm24$(PATCH_STR).patch \
		linux-sh4-i2c-st40-pio_stm24$(PATCH_STR).patch \
		linux-sh4-hl101_i2c_st40_stm24.patch

KERNELPATCHES_24 =  \
		$(if $(HL101),$(HL101_PATCHES_24))

############ Patches Kernel 24 End ###############

#
# KERNEL-HEADERS
#
$(DEPDIR)/kernel-headers: linux-kernel.do_prepare
	cd $(KERNEL_DIR) && \
		$(INSTALL) -d $(targetprefix)/usr/include && \
		cp -a include/linux $(targetprefix)/usr/include && \
		cp -a include/asm-sh $(targetprefix)/usr/include/asm && \
		cp -a include/asm-generic $(targetprefix)/usr/include && \
		cp -a include/mtd $(targetprefix)/usr/include
	touch $@

KERNELHEADERS := linux-kernel-headers
ifdef ENABLE_P0207
KERNELHEADERS_VERSION := 2.6.32.16-44
else
ifdef ENABLE_P0209
KERNELHEADERS_VERSION := 2.6.32.46-45
else
ifdef ENABLE_P0210
KERNELHEADERS_VERSION := 2.6.32.46-45
else
ifdef ENABLE_P0211
KERNELHEADERS_VERSION := 2.6.32.46-45
endif
endif
endif
endif

KERNELHEADERS_SPEC := stm-target-kernel-headers-kbuild.spec
KERNELHEADERS_SPEC_PATCH :=
KERNELHEADERS_PATCHES :=
KERNELHEADERS_RPM := RPMS/noarch/$(STLINUX)-sh4-$(KERNELHEADERS)-$(KERNELHEADERS_VERSION).noarch.rpm

$(KERNELHEADERS_RPM): \
		$(if $(KERNELHEADERS_SPEC_PATCH),Patches/$(KERNELHEADERS_SPEC_PATCH)) \
		$(if $(KERNELHEADERS_PATCHES),$(KERNELHEADERS_PATCHES:%=Patches/%)) \
		$(archivedir)/$(STLINUX)-target-$(KERNELHEADERS)-$(KERNELHEADERS_VERSION).src.rpm \
		| linux-kernel.do_prepare
	rpm $(DRPM) --nosignature -Uhv $(lastword $^) && \
	$(if $(KERNELHEADERS_SPEC_PATCH),( cd SPECS && patch -p1 $(KERNELHEADERS_SPEC) < $(buildprefix)/Patches/$(KERNELHEADERS_SPEC_PATCH) ) &&) \
	$(if $(KERNELHEADERS_PATCHES),cp $(KERNELHEADERS_PATCHES:%=Patches/%) SOURCES/ &&) \
	export PATH=$(hostprefix)/bin:$(PATH) && \
	rpmbuild $(DRPMBUILD) -bb -v --clean --target=sh4-linux SPECS/$(KERNELHEADERS_SPEC)

$(DEPDIR)/max-$(KERNELHEADERS) \
$(DEPDIR)/$(KERNELHEADERS): \
$(DEPDIR)/%$(KERNELHEADERS): $(KERNELHEADERS_RPM)
	@rpm $(DRPM) --ignorearch --nodeps -Uhv \
		--badreloc --relocate $(targetprefix)=$(prefix)/$*cdkroot $(lastword $^)
	touch $@

#
# HOST-KERNEL
#
# IMPORTANT: it is expected that only one define is set
MODNAME = $(HL101)

ifdef DEBUG
DEBUG_STR=.debug
else
DEBUG_STR=
endif

HOST_KERNEL := host-kernel

ifdef ENABLE_P0207
HOST_KERNEL_VERSION = 2.6.32.28$(KERNELSTMLABEL)-$(KERNELLABEL)
else
ifdef ENABLE_P0209
HOST_KERNEL_VERSION = 2.6.32.46$(KERNELSTMLABEL)-$(KERNELLABEL)
else
ifdef ENABLE_P0210
HOST_KERNEL_VERSION = 2.6.32.57$(KERNELSTMLABEL)-$(KERNELLABEL)
else
ifdef ENABLE_P0211
HOST_KERNEL_VERSION = 2.6.32.59$(KERNELSTMLABEL)-$(KERNELLABEL)
endif
endif
endif
endif

HOST_KERNEL_SPEC = stm-$(HOST_KERNEL)-sh4.spec
HOST_KERNEL_SPEC_PATCH =
HOST_KERNEL_PATCHES = $(KERNELPATCHES_24)
HOST_KERNEL_CONFIG = linux-sh4-$(subst _stm24_,-,$(KERNELVERSION))_$(MODNAME).config$(DEBUG_STR)
HOST_KERNEL_SRC_RPM = $(STLINUX)-$(HOST_KERNEL)-source-sh4-$(HOST_KERNEL_VERSION).src.rpm
HOST_KERNEL_RPM = RPMS/noarch/$(STLINUX)-$(HOST_KERNEL)-source-sh4-$(HOST_KERNEL_VERSION).noarch.rpm

$(HOST_KERNEL_RPM): \
		$(if $(HOST_KERNEL_SPEC_PATCH),Patches/$(HOST_KERNEL_SPEC_PATCH)) \
		$(archivedir)/$(HOST_KERNEL_SRC_RPM)
	rpm $(DRPM) --nosignature --nodeps -Uhv $(lastword $^) && \
	$(if $(HOST_KERNEL_SPEC_PATCH),( cd SPECS; patch -p1 $(HOST_KERNEL_SPEC) < $(buildprefix)/Patches/$(HOST_KERNEL_SPEC_PATCH) ) &&) \
	rpmbuild $(DRPMBUILD) -ba -v --clean --target=sh4-linux SPECS/$(HOST_KERNEL_SPEC)

$(DEPDIR)/linux-kernel.do_prepare: \
		$(if $(HOST_KERNEL_PATCHES),$(HOST_KERNEL_PATCHES:%=Patches/%)) \
		$(HOST_KERNEL_RPM)
	@rpm $(DRPM) -ev $(HOST_KERNEL_SRC_RPM:%.src.rpm=%) || true
	rm -rf $(KERNEL_DIR)
	rm -rf linux{,-sh4}
	rpm $(DRPM) --ignorearch --nodeps -Uhv $(lastword $^)
	$(if $(HOST_KERNEL_PATCHES),cd $(KERNEL_DIR) && cat $(HOST_KERNEL_PATCHES:%=$(buildprefix)/Patches/%) | patch -p1)
	$(INSTALL) -m644 Patches/$(HOST_KERNEL_CONFIG) $(KERNEL_DIR)/.config
	-rm $(KERNEL_DIR)/localversion*
	echo "$(KERNELSTMLABEL)" > $(KERNEL_DIR)/localversion-stm
	if [ `grep -c "CONFIG_BPA2_DIRECTFBOPTIMIZED" $(KERNEL_DIR)/.config` -eq 0 ]; then echo "# CONFIG_BPA2_DIRECTFBOPTIMIZED is not set" >> $(KERNEL_DIR)/.config; fi
	$(MAKE) -C $(KERNEL_DIR) ARCH=sh oldconfig
	$(MAKE) -C $(KERNEL_DIR) ARCH=sh include/asm
	$(MAKE) -C $(KERNEL_DIR) ARCH=sh include/linux/version.h
	rm $(KERNEL_DIR)/.config
	touch $@

ifdef ENABLE_GRAPHICFWDIRECTFB
GRAPHICFWDIRECTFB_SED_CONF=-i s"/^\# CONFIG_BPA2_DIRECTFBOPTIMIZED is not set/CONFIG_BPA2_DIRECTFBOPTIMIZED=y/"
else
GRAPHICFWDIRECTFB_SED_CONF=-i s"/^CONFIG_BPA2_DIRECTFBOPTIMIZED=y/\# CONFIG_BPA2_DIRECTFBOPTIMIZED is not set/"
endif

#dagobert: without stboard ->not sure if we need this
$(DEPDIR)/linux-kernel.do_compile: \
		bootstrap-cross \
		linux-kernel.do_prepare \
		Patches/$(HOST_KERNEL_CONFIG) \
		| $(HOST_U_BOOT_TOOLS)
	-rm $(DEPDIR)/linux-kernel*.do_compile
	cd $(KERNEL_DIR) && \
		export PATH=$(hostprefix)/bin:$(PATH) && \
		$(MAKE) ARCH=sh CROSS_COMPILE=$(target)- mrproper && \
		@M4@ $(buildprefix)/Patches/$(HOST_KERNEL_CONFIG) > .config && \
	if [ `grep -c "CONFIG_BPA2_DIRECTFBOPTIMIZED" .config` -eq 0 ]; then echo "# CONFIG_BPA2_DIRECTFBOPTIMIZED is not set" >> .config; fi && \
		sed $(GRAPHICFWDIRECTFB_SED_CONF) .config && \
		$(MAKE) ARCH=sh CROSS_COMPILE=$(target)- uImage modules
	touch $@

NFS_FLASH_SED_CONF=$(foreach param,XCONFIG_NFS_FS XCONFIG_LOCKD XCONFIG_SUNRPC,-e s"/^.*$(param)[= ].*/$(param)=m/")

ifdef ENABLE_XFS
XFS_SED_CONF=$(foreach param,CONFIG_XFS_FS,-e s"/^.*$(param)[= ].*/$(param)=m/")
else
XFS_SED_CONF=-e ""
endif

ifdef ENABLE_NFSSERVER
#NFSSERVER_SED_CONF=$(foreach param,CONFIG_NFSD CONFIG_NFSD_V3 CONFIG_NFSD_TCP,-e s"/^.*$(param)[= ].*/$(param)=y/")
NFSSERVER_SED_CONF=$(foreach param,CONFIG_NFSD,-e s"/^.*$(param)[= ].*/$(param)=m\nCONFIG_NFSD_V3=y\n\# CONFIG_NFSD_V3_ACL is not set\n\# CONFIG_NFSD_V4 is not set\nCONFIG_NFSD_TCP=y/")
else
NFSSERVER_SED_CONF=-e ""
endif

ifdef ENABLE_NTFS
NTFS_SED_CONF=$(foreach param,CONFIG_NTFS_FS,-e s"/^.*$(param)[= ].*/$(param)=m/")
else
NTFS_SED_CONF=-e ""
endif

$(DEPDIR)/linux-kernel.cramfs.do_compile $(DEPDIR)/linux-kernel.squashfs.do_compile \
$(DEPDIR)/linux-kernel.focramfs.do_compile $(DEPDIR)/linux-kernel.fosquashfs.do_compile:
$(DEPDIR)/linux-kernel.%.do_compile: \
		bootstrap-cross \
		linux-kernel.do_prepare \
		Patches/linux-sh4-$(KERNELVERSION).stboards.c.m4 \
		Patches/$(HOST_KERNEL_CONFIG) \
		| $(DEPDIR)/$(HOST_U_BOOT_TOOLS)
	-rm $(DEPDIR)/linux-kernel*.do_compile
	cd $(KERNEL_DIR) && \
		export PATH=$(hostprefix)/bin:$(PATH) && \
		$(MAKE) ARCH=sh CROSS_COMPILE=$(target)- mrproper && \
		@M4@ -Drootfs=$* --define=rootsize=$(ROOT_PARTITION_SIZE) --define=datasize=$(DATA_PARTITION_SIZE) ../$(word 3,$^) \
					> drivers/mtd/maps/stboards.c && \
		@M4@ --define=btldrdef=$* $(buildprefix)/Patches/$(HOST_KERNEL_CONFIG) \
					> .config && \
		sed $(NFS_FLASH_SED_CONF) -i .config && \
		sed $(XFS_SED_CONF) $(NFSSERVER_SED_CONF) $(NTFS_SED_CONF) -i .config && \
		$(MAKE) ARCH=sh CROSS_COMPILE=$(target)- uImage modules
	touch $@

DESCRIPTION_linux_kernel = "The Linux Kernel and modules"
PKGV_linux_kernel = $(KERNELVERSION)
PKGR_linux_kernel = r4
SRC_URI_linux_kernel = stlinux.com
FILES_linux_kernel = \
/lib/modules/$(KERNELVERSION)/kernel \
/lib/modules/$(KERNELVERSION)/modules.* \
/boot/uImage

define postinst_linux_kernel
#!/bin/sh
flash_eraseall /dev/mtd5
nandwrite -p /dev/mtd5 /boot/uImage
rm /boot/uImage
depmod
endef

$(DEPDIR)/linux-kernel: bootstrap $(DEPDIR)/linux-kernel.do_compile
	$(start_build)
	@$(INSTALL) -d $(PKDIR)/boot && \
	$(INSTALL) -d $(prefix)/$*$(notdir $(bootprefix)) && \
	$(INSTALL) -m644 $(KERNEL_DIR)/arch/sh/boot/uImage $(prefix)/$*$(notdir $(bootprefix))/vmlinux.ub && \
	$(INSTALL) -m644 $(KERNEL_DIR)/vmlinux $(PKDIR)/boot/vmlinux-sh4-$(KERNELVERSION) && \
	$(INSTALL) -m644 $(KERNEL_DIR)/System.map $(PKDIR)/boot/System.map-sh4-$(KERNELVERSION) && \
	$(INSTALL) -m644 $(KERNEL_DIR)/COPYING $(PKDIR)/boot/LICENSE
	cp $(KERNEL_DIR)/arch/sh/boot/uImage $(PKDIR)/boot/
	echo -e "ST Linux Distribution - Binary Kernel\n \
	CPU: sh4\n \
	PLATFORM: stb7109ref\n \
	KERNEL VERSION: $(KERNELVERSION)\n" > $(PKDIR)/README.ST && \
	$(MAKE) -C $(KERNEL_DIR) ARCH=sh INSTALL_MOD_PATH=$(PKDIR) modules_install && \
	rm $(PKDIR)/lib/modules/$(KERNELVERSION)/build || true && \
	rm $(PKDIR)/lib/modules/$(KERNELVERSION)/source || true
	$(tocdk_build)
	$(toflash_build)
	touch $@

linux-kernel-distclean: $(KERNELHEADERS)-distclean

BEGIN[[
driver
  git
  {PN}-{PV}
  plink:$(driverdir):{PN}-{PV}
;
]]END
DESCRIPTION_driver = Drivers for stm box
PKGR_driver = r3
PACKAGES_driver = driver_pti driver
FILES_driver = /lib/modules/$(KERNELVERSION)/extra
SRC_URI_driver = "http://gitorious.org/~schpuntik/open-duckbox-project-sh4/tdt-amiko"
DESCRIPTION_driver_pti = open source st-pti kernel module
RCONFLICTS_driver_pti = driver_ptinp
FILES_driver_pti = /lib/modules/$(KERNELVERSION)/extra/pti
EXTRA_driver = driver_pti

define postinst_driver
#!/bin/sh
depmod
endef

$(DEPDIR)/driver: $(DEPENDS_driver) $(driverdir)/Makefile glibc-dev linux-kernel.do_compile
	$(PREPARE_driver)
#	$(MAKE) -C $(KERNEL_DIR) $(MAKE_OPTS) ARCH=sh modules_prepare
	$(start_build)
	$(get_git_version)
	$(eval export PKGV_driver = $(PKGV_driver)$(KERNELSTMLABEL))
	$(if $(PLAYER191),cp $(driverdir)/stgfb/stmfb/linux/drivers/video/stmfb.h $(targetprefix)/usr/include/linux)
	cp $(driverdir)/player2/linux/include/linux/dvb/stm_ioctls.h $(targetprefix)/usr/include/linux/dvb
	$(LN_SF) $(driverdir)/wireless/rtl8192cu/autoconf_rtl8192c_usb_linux.h $(buildprefix)/
	$(MAKE) -C $(driverdir) ARCH=sh \
		CONFIG_MODULES_PATH=$(targetprefix) \
		KERNEL_LOCATION=$(buildprefix)/$(KERNEL_DIR) \
		DRIVER_TOPDIR=$(driverdir) \
		HL101=$(HL101) \
		PLAYER191=$(PLAYER191) \
		CROSS_COMPILE=$(target)-
	$(MAKE) -C $(driverdir) ARCH=sh \
		CONFIG_MODULES_PATH=$(targetprefix) \
		KERNEL_LOCATION=$(buildprefix)/$(KERNEL_DIR) \
		DRIVER_TOPDIR=$(driverdir) \
		BIN_DEST=$(PKDIR)/bin \
		INSTALL_MOD_PATH=$(PKDIR) \
		DEPMOD=$(DEPMOD) \
		HL101=$(HL101) \
		PLAYER191=$(PLAYER191) \
		install
	$(DEPMOD) -ae -b $(PKDIR) -F $(buildprefix)/$(KERNEL_DIR)/System.map -r $(KERNELVERSION)
	$(tocdk_build)
	$(toflash_build)
	touch $@

# overwrite make driver-distclean
define DISTCLEANUP_driver
	rm -f $(DEPDIR)/driver
	rm -f $(buildprefix)/autoconf_rtl8192c_usb_linux.h
	$(MAKE) -C $(driverdir) ARCH=sh \
		KERNEL_LOCATION=$(buildprefix)/$(KERNEL_DIR) \
		distclean
endef
define DEPSCLEANUP_driver
	rm -f $(DEPDIR)/driver
	rm -f $(buildprefix)/autoconf_rtl8192c_usb_linux.h
	$(MAKE) -C $(driverdir) ARCH=sh \
		KERNEL_LOCATION=$(buildprefix)/$(KERNEL_DIR) \
		distclean
endef

#------------------- Helper

linux-kernel.menuconfig linux-kernel.xconfig: \
linux-kernel.%:
	$(MAKE) -C $(KERNEL_DIR) ARCH=sh CROSS_COMPILE=sh4-linux- $*
	@echo
	@echo "You have to edit m a n u a l l y Patches/linux-$(KERNELVERSION).config to make changes permanent !!!"
	@echo ""
	diff $(KERNEL_DIR)/.config.old $(KERNEL_DIR)/.config
	@echo ""
#-------------------
