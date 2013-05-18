#
# DIVERSE STUFF / TOOLS
#
$(DEPDIR)/diverse-tools: $(DEPDIR)/%diverse-tools: $(DIVERSE_TOOLS_ADAPTED_ETC_FILES:%=root/etc/%)
	( cd root/etc && for i in $(DIVERSE_TOOLS_ADAPTED_ETC_FILES); do \
		[ -f $$i ] && $(INSTALL) -m644 $$i $(prefix)/$*cdkroot/etc/$$i || true; \
		[ "$${i%%/*}" = "init.d" ] && chmod 755 $(prefix)/$*cdkroot/etc/$$i || true; done ) && \
	( export HHL_CROSS_TARGET_DIR=$(prefix)/$*cdkroot && cd $(prefix)/$*cdkroot/etc/init.d && \
		for s in $(DIVERSE_TOOLS_ADAPTED_ETC_FILES); do \
			[ "$${s%%/*}" = "init.d" ] && ( \
			$(hostprefix)/bin/target-initdconfig --add $${s#init.d/} || \
			echo "Unable to enable initd service: $${s#init.d/}" ) ; done && rm *rpmsave 2>/dev/null || true ) && \
	ln -sf /usr/share/zoneinfo/Europe/Berlin $(prefix)/$*cdkroot/etc/localtime
	$(INSTALL_BIN) root/usr/sbin/mountro $(prefix)/$*cdkroot/usr/sbin/
	$(INSTALL_BIN) root/usr/sbin/mountrw $(prefix)/$*cdkroot/usr/sbin/
	touch $@
