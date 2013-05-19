#
# INIT-SCRIPTS customized
#
BEGIN[[
init_scripts_xbmc
  0.1
  {PN}-{PV}
  pdircreate:{PN}-{PV}
  nothing:file://../root/etc/inittab_xbmc
  nothing:file://../root/release/hostname
  nothing:file://../root/release/inetd
# for 'nothing:' only 'cp' is executed so '*' is ok.
  nothing:file://../root/release/initmodules_*
  nothing:file://../root/release/halt_*
  nothing:file://../root/release/mountall
  nothing:file://../root/release/mountsysfs
  nothing:file://../root/release/networking
  nothing:file://../root/release/rc
  nothing:file://../root/release/reboot
  nothing:file://../root/release/sendsigs
  nothing:file://../root/release/telnetd
  nothing:file://../root/release/syslogd
  nothing:file://../root/release/umountfs
  nothing:file://../root/release/lircd
;
]]END

DESCRIPTION_init_scripts_xbmc = init scripts and rules for system start
init_scripts_initd_xbmc_files = \
halt \
hostname \
inetd \
initmodules \
mountall \
mountsysfs \
networking \
reboot \
sendsigs \
telnetd \
syslogd \
lircd \
umountfs

define postinst_init_scripts_xbmc
#!/bin/sh
$(foreach f,$(init_scripts_initd_xbmc_files), initdconfig --add $f
)
endef

define prerm_init_scripts_xbmc
#!/bin/sh
$(foreach f,$(init_scripts_initd_xbmc_files), initdconfig --del $f
)
endef

$(DEPDIR)/init-scripts-xbmc: $(DEPENDS_init_scripts_xbmc)
	$(PREPARE_init_scripts_xbmc)
	$(start_build)
	$(INSTALL_DIR) $(PKDIR)/etc/init.d

# select initmodules
	cd $(DIR_init_scripts_xbmc) && \
	mv initmodules$(if $(HL101),_xbmc_$(HL101)) initmodules
# select halt
	cd $(DIR_init_scripts_xbmc) && \
	mv halt$(if $(HL101),_$(HL101)) halt
# init.d scripts
	cd $(DIR_init_scripts_xbmc) && \
		$(INSTALL) inittab_xbmc $(PKDIR)/etc/inittab && \
		$(INSTALL) -m 755 rc $(PKDIR)/etc/init.d/rc && \
		$(foreach f,$(init_scripts_initd_xbmc_files), $(INSTALL) -m 755 $f $(PKDIR)/etc/init.d && ) true
	$(toflash_build)
	touch $@

release_xbmc_spark:
	echo "src/gz AR-P http://alien.sat-universum.de" | cat - $(prefix)/release/etc/opkg/official-feed.conf > $(prefix)/release/etc/opkg/official-feed && \
	mv $(prefix)/release/etc/opkg/official-feed $(prefix)/release/etc/opkg/official-feed.conf

release_xbmc_spark7162:
	echo "src/gz AR-P http://alien2.sat-universum.de" | cat - $(prefix)/release/etc/opkg/official-feed.conf > $(prefix)/release/etc/opkg/official-feed && \
	mv -f $(prefix)/release/etc/opkg/official-feed $(prefix)/release/etc/opkg/official-feed.conf


# The main target depends on the model.
# IMPORTANT: it is assumed that only one variable is set. Otherwise the target name won't be resolved.
#
$(DEPDIR)/release_xbmc: $(DEPDIR)/%release_xbmc:release_base release_common_utils release_$(HL101)release_xbmc_$(HL101)
# Post tweaks
	depmod -b $(prefix)/release $(KERNELVERSION)
	touch $@

release-xbmc_clean:
	rm -f $(DEPDIR)/release_xbmc
	rm -f $(DEPDIR)/release_xbmc_base
	rm -f $(DEPDIR)/release_$(HL101)
	rm -f $(DEPDIR)/release_xbmc_$(HL101)
	rm -f $(DEPDIR)/release_xbmc_common_utils
