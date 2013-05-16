#
# busybox
#
$(DEPDIR)/busybox.do_prepare: $(DEPENDS_busybox)
	$(PREPARE_busybox)
	touch $@

$(DEPDIR)/busybox.do_compile: bootstrap $(DEPDIR)/busybox.do_prepare Patches/busybox.config | $(DEPDIR)/$(GLIBC_DEV)
	cd $(DIR_busybox) && \
		export CROSS_COMPILE=$(target)- && \
		$(MAKE) mrproper && \
		$(INSTALL) -m644 ../$(lastword $^) .config && \
		$(MAKE) all \
			CROSS_COMPILE=$(target)- \
			CFLAGS_EXTRA="$(TARGET_CFLAGS)"
	touch $@

DESCRIPTION_busybox = "Utilities for embedded systems"

$(DEPDIR)/min-busybox $(DEPDIR)/std-busybox $(DEPDIR)/max-busybox \
$(DEPDIR)/busybox: \
$(DEPDIR)/%busybox: $(DEPDIR)/busybox.do_compile
	$(start_build)
	cd $(DIR_busybox) && \
		export CROSS_COMPILE=$(target)- && \
		$(INSTALL_busybox)
#		@CLEANUP_busybox@
	install -m644 -D /dev/null $(PKDIR)/etc/shells
	export HHL_CROSS_TARGET_DIR=$(PKDIR) && $(hostprefix)/bin/target-shellconfig --add /bin/ash 5
	$(tocdk_build)
	$(toflash_build)
	[ "x$*" = "x" ] && touch $@ || true

$(eval $(call guiconfig,busybox))
