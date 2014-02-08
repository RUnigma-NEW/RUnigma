#
# busybox
#

PKGR_busybox = r3
BEGIN[[
busybox
  1.22.1
  {PN}-{PV}
  extract:http://www.{PN}.net/downloads/{PN}-{PV}.tar.bz2
  nothing:file://{PN}-{PV}.config
  pmove:{PN}-{PV}/{PN}-{PV}.config:{PN}-{PV}/.config
  patch:file://{PN}-{PV}-ash.patch
  patch:file://{PN}-{PV}-date.patch
  patch:file://{PN}-{PV}-iplink.patch
  make:install:CONFIG_PREFIX=PKDIR
;
]]END

DESCRIPTION_busybox = "Utilities for embedded systems"

$(DEPDIR)/busybox: bootstrap $(DEPENDS_busybox)
	$(PREPARE_busybox)
	$(start_build)
	cd $(DIR_busybox) && \
		export CROSS_COMPILE=$(target)- && \
		$(MAKE) all \
			CROSS_COMPILE=$(target)- \
			CONFIG_EXTRA_CFLAGS="$(TARGET_CFLAGS)" && \
		$(INSTALL_busybox)
	install -m644 -D /dev/null $(PKDIR)/etc/shells
	export HHL_CROSS_TARGET_DIR=$(PKDIR) && $(hostprefix)/bin/target-shellconfig --add /bin/ash 5
	$(tocdk_build)
	$(toflash_build)
#	$(CLEANUP_busybox)
	touch $@

$(eval $(call guiconfig,busybox))
