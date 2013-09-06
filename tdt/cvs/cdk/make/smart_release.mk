#
# INIT-SCRIPTS customized
#
BEGIN[[
init_scripts
  0.10
  {PN}-{PV}
  pdircreate:{PN}-{PV}
  nothing:file://../root/etc/inittab
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
  nothing:file://../root/release/crond
  nothing:file://../root/release/umountfs
  nothing:file://../root/release/lircd
  nothing:file://../root/etc/init.d/avahi-daemon
  nothing:file://../root/etc/init.d/rdate
;
]]END

DESCRIPTION_init_scripts = init scripts and rules for system start
init_scripts_initd_files = \
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
crond \
lircd \
umountfs \
avahi-daemon

define postinst_init_scripts
#!/bin/sh
$(foreach f,$(init_scripts_initd_files), initdconfig --add $f
)
endef

define prerm_init_scripts
#!/bin/sh
$(foreach f,$(init_scripts_initd_files), initdconfig --del $f
)
endef

$(DEPDIR)/init-scripts: $(DEPENDS_init_scripts)
	$(PREPARE_init_scripts)
	$(start_build)
	$(INSTALL_DIR) $(PKDIR)/etc/init.d

# select initmodules
	cd $(DIR_init_scripts) && \
	mv initmodules_hl101 initmodules
# select halt
	cd $(DIR_init_scripts) && \
	mv halt_hl101 halt
# init.d scripts
	cd $(DIR_init_scripts) && \
		$(INSTALL) inittab $(PKDIR)/etc/ && \
		$(INSTALL) -m 755 rc $(PKDIR)/etc/init.d/rc && \
		$(foreach f,$(init_scripts_initd_files), $(INSTALL) -m 755 $f $(PKDIR)/etc/init.d && ) true
	$(toflash_build)
	touch $@

#
# EXTRA FONTS
#

DESCRIPTION_fonts_extra = Extra fonts to beautify your box
SRC_URI_fonts_extra = git://gitorious.org/~schpuntik/open-duckbox-project-sh4/tdt-amiko.git
PKGV_fonts_extra = 0.1
SRC_URI_font_valis_enigma = $(SRC_URI_fonts_extra)
PKGV_font_valis_enigma = $(PKGV_fonts_extra)

fonts_extra_file_list = $(subst .ttf,,$(shell ls root/usr/share/fonts/))

# This can be used as multipackaging example
# Remember to replace '-' with '_' in variables and package names

PACKAGES_fonts_extra = $(subst -,_,$(addprefix font_,$(fonts_extra_file_list)))
$(foreach f,$(fonts_extra_file_list), \
 $(eval DESCRIPTION_font_$(subst -,_,$f) = font $f ) \
 $(eval FILES_font_$(subst -,_,$f) = /usr/share/fonts/$f*) \
)

$(DEPDIR)/font-valis-enigma: fonts-extra
	$(start_build)
	$(INSTALL) -d $(PKDIR)/usr/share/fonts
	$(INSTALL) -m 644 root/usr/share/fonts/valis_enigma.ttf $(PKDIR)/usr/share/fonts
	$(toflash_build)
	touch $@

$(DEPDIR)/fonts-extra: $(addsuffix .ttf, $(addprefix root/usr/share/fonts/,$(fonts_extra_file_list)))
	$(start_build)
	$(INSTALL) -d $(PKDIR)/usr/share/fonts
	$(INSTALL) -m 644 $^ $(PKDIR)/usr/share/fonts
	$(extra_build)
	touch $@

#
# 3G MODEMS
#
BEGIN[[
modem_scripts
  0.4
  {PN}-{PV}
  pdircreate:{PN}-{PV}
  nothing:file://../root/etc/ppp/ip-*
  nothing:file://../root/usr/bin/modem.sh
  nothing:file://../root/usr/bin/modemctrl.sh
  nothing:file://../root/etc/modem.conf
  nothing:file://../root/etc/modem.list
  nothing:file://../root/etc/55-modem.rules
  nothing:file://../root/etc/30-modemswitcher.rules
;
]]END

DESCRIPTION_modem_scripts = utils to setup 3G modems
RDEPENDS_modem_scripts = usb_modeswitch iptables iptables-dev

$(DEPDIR)/modem-scripts: $(DEPENDS_modem_scripts) $(RDEPENDS_modem_scripts)
	$(PREPARE_modem_scripts)
	$(start_build)
	cd $(DIR_modem_scripts) && \
	$(INSTALL_DIR) $(PKDIR)/etc/ppp/peers && \
	$(INSTALL_DIR) $(PKDIR)/etc/udev/rules.d/ && \
	$(INSTALL_DIR) $(PKDIR)/usr/bin/ && \
	$(INSTALL_BIN) ip-* $(PKDIR)/etc/ppp/ && \
	$(INSTALL_BIN) modem.sh $(PKDIR)/usr/bin/ && \
	$(INSTALL_BIN) modemctrl.sh $(PKDIR)/usr/bin/ && \
	$(INSTALL_FILE) modem.conf $(PKDIR)/etc/ && \
	$(INSTALL_FILE) modem.list $(PKDIR)/etc/ && \
	$(INSTALL_FILE) 55-modem.rules $(PKDIR)/etc/udev/rules.d/ && \
	$(INSTALL_FILE) 30-modemswitcher.rules $(PKDIR)/etc/udev/rules.d/
	$(toflash_build)
	touch $@

DESCRIPTION_driver_ptinp = pti non public
PKGV_driver_ptinp = 0.1
$(eval export PKGV_driver_ptinp = $(PKGV_driver_ptinp)$(KERNELSTMLABEL))
RCONFLICTS_driver_ptinp = driver-pti
SRC_URI_driver_ptinp = unknown

$(DEPDIR)/driver-ptinp:
	$(start_build)
	mkdir -p $(PKDIR)/lib/modules/$(KERNELVERSION)/extra/pti
	$(if $(P0207),cp -dp $(archivedir)/ptinp/pti_207.ko $(PKDIR)/lib/modules/$(KERNELVERSION)/extra/pti/pti.ko) \
	$(if $(P0209),cp -dp $(archivedir)/ptinp/pti_209.ko $(PKDIR)/lib/modules/$(KERNELVERSION)/extra/pti/pti.ko) \
	$(if $(P0210),cp -dp $(archivedir)/ptinp/pti_210.ko $(PKDIR)/lib/modules/$(KERNELVERSION)/extra/pti/pti.ko) \
	$(if $(P0211),cp -dp $(archivedir)/ptinp/pti_211.ko $(PKDIR)/lib/modules/$(KERNELVERSION)/extra/pti/pti.ko)
	$(toflash_build)
	touch $@

DESCRIPTION_driver_encrypt = driver encrypt
PKGV_driver_encrypt = 0.1
$(eval export PKGV_driver_encrypt = $(PKGV_driver_encrypt)$(KERNELSTMLABEL))
SRC_URI_driver_encrypt = unknown

$(DEPDIR)/driver-encrypt:
	$(start_build)
	mkdir -p $(PKDIR)/lib/modules/$(KERNELVERSION)/extra/encrypt
	$(toflash_build)
	touch $@

#
# boot-elf
#

DESCRIPTION_boot_elf = firmware non public
SRC_URI_boot_elf = unknown
PKGV_boot_elf = r2
RDEPENDS_boot_elf :=


$(DEPDIR)/boot-elf: firmware $(RDEPENDS_boot_elf)
	$(start_build)
	$(INSTALL_DIR) $(PKDIR)/lib/firmware/
	$(INSTALL_FILE) $(archivedir)/boot/video_7109.elf $(PKDIR)/lib/firmware/video.elf
	$(INSTALL_FILE) $(archivedir)/boot/audio_7109.elf $(PKDIR)/lib/firmware/audio.elf
	$(toflash_build)
	touch $@

#
# opkg plugin nonfree feed
#

DESCRIPTION_enigma2_plugin_nonfree_feed = Download non free plugins and skins
SRC_URI_enigma2_plugin_nonfree_feed = unknown
PKGV_enigma2_plugin_nonfree_feed = r3
RDEPENDS_enigma2_plugin_nonfree_feed :=

define postinst_enigma2_plugin_nonfree_feed
#!/bin/sh
# postinst script for nonfree-feed
(sleep 10; opkg update) &
exit 0
endef

$(DEPDIR)/enigma2-plugin-nonfree-feed: $(RDEPENDS_enigma2_plugin_nonfree_feed)
	$(start_build)
	$(INSTALL_DIR) $(PKDIR)/etc/opkg/ && \
	$(INSTALL_FILE) $(buildprefix)/root/etc/ipkg/non-free-feed.conf $(PKDIR)/etc/opkg/non-free-feed.conf
	$(e2extra_build)
	touch $@
	
#
# firmware
#

DESCRIPTION_firmware = firmware non public
SRC_URI_firmware = unknown
PKGV_firmware = r1
RDEPENDS_firmware :=

$(DEPDIR)/firmware:  $(RDEPENDS_firmware)
	$(start_build)
	$(INSTALL_DIR) $(PKDIR)/lib/firmware/
	$(toflash_build)
	touch $@

# auxiliary targets for model-specific builds
release_common_utils:
# opkg config
	mkdir -p $(prefix)/release/etc/opkg
	mkdir -p $(prefix)/release/usr/lib/locale
	cp -f $(buildprefix)/root/release/official-feed.conf $(prefix)/release/etc/opkg/
	cp -f $(buildprefix)/root/release/opkg.conf $(prefix)/release/etc/
	$(call initdconfig,$(shell ls $(prefix)/release/etc/init.d))
	
release_base: driver-ptinp
	rm -rf $(prefix)/release || true
	$(INSTALL_DIR) $(prefix)/release && \
	cp -rp $(prefix)/pkgroot/* $(prefix)/release
# filesystem
	$(INSTALL_DIR) $(prefix)/release/bin && \
	$(INSTALL_DIR) $(prefix)/release/sbin && \
	$(INSTALL_DIR) $(prefix)/release/boot && \
	$(INSTALL_DIR) $(prefix)/release/dev && \
	$(INSTALL_DIR) $(prefix)/release/dev.static && \
	$(INSTALL_DIR) $(prefix)/release/etc && \
	$(INSTALL_DIR) $(prefix)/release/etc/fonts && \
	$(INSTALL_DIR) $(prefix)/release/etc/init.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/modprobe.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-down.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-post-up.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-post-down.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-pre-up.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-pre-down.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/network/if-up.d && \
	$(INSTALL_DIR) $(prefix)/release/etc/tuxbox && \
	$(INSTALL_DIR) $(prefix)/release/etc/enigma2 && \
	$(INSTALL_DIR) $(prefix)/release/media && \
	$(INSTALL_DIR) $(prefix)/release/media/dvd && \
	$(INSTALL_DIR) $(prefix)/release/media/net && \
	$(INSTALL_DIR) $(prefix)/release/mnt && \
	$(INSTALL_DIR) $(prefix)/release/mnt/usb && \
	$(INSTALL_DIR) $(prefix)/release/mnt/hdd && \
	$(INSTALL_DIR) $(prefix)/release/mnt/nfs && \
	$(INSTALL_DIR) $(prefix)/release/root && \
	$(INSTALL_DIR) $(prefix)/release/proc && \
	$(INSTALL_DIR) $(prefix)/release/sys && \
	$(INSTALL_DIR) $(prefix)/release/tmp && \
	$(INSTALL_DIR) $(prefix)/release/usr && \
	$(INSTALL_DIR) $(prefix)/release/usr/bin && \
	$(INSTALL_DIR) $(prefix)/release/media/hdd && \
	$(INSTALL_DIR) $(prefix)/release/media/hdd/music && \
	$(INSTALL_DIR) $(prefix)/release/media/hdd/picture && \
	$(INSTALL_DIR) $(prefix)/release/lib && \
	$(INSTALL_DIR) $(prefix)/release/lib/modules && \
	$(INSTALL_DIR) $(prefix)/release/lib/firmware && \
	$(INSTALL_DIR) $(prefix)/release/ram && \
	$(INSTALL_DIR) $(prefix)/release/var && \
	$(INSTALL_DIR) $(prefix)/release/var/etc && \
	mkdir -p $(prefix)/release/var/run/lirc && \
	$(INSTALL_DIR) $(prefix)/release/usr/lib/opkg && \
	ln -sf /usr/bin/opkg-cl  $(prefix)/release/usr/bin/ipkg-cl && \
	ln -sf /usr/bin/opkg-cl  $(prefix)/release/usr/bin/opkg && \
	ln -sf /usr/bin/opkg-cl  $(prefix)/release/usr/bin/ipkg
# rc.d directories
	mkdir -p $(prefix)/release/etc/rc.d/rc{0,1,2,3,4,5,6,S}.d
	ln -sf ../init.d $(prefix)/release/etc/rc.d

# zoneinfo
	$(INSTALL_DIR) $(prefix)/release/usr/share/zoneinfo && \
	cp -aR $(buildprefix)/root/usr/share/zoneinfo/* $(prefix)/release/usr/share/zoneinfo/
# udhcpc
	$(INSTALL_DIR) $(prefix)/release/usr/share/udhcpc && \
	cp -aR $(buildprefix)/root/usr/share/udhcpc/* $(prefix)/release/usr/share/udhcpc/ && \
	cp -dp $(buildprefix)/root/etc/init.d/udhcpc $(prefix)/release/etc/init.d/ && \
	cp $(buildprefix)/root/etc/timezone.xml $(prefix)/release/etc/ && \
	cp -a $(buildprefix)/root/etc/Wireless $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/firmware/*.bin $(prefix)/release/lib/firmware/ && \
	cp -dp $(targetprefix)/etc/network/options $(prefix)/release/etc/network/ && \
	ln -sf /etc/timezone.xml $(prefix)/release/etc/tuxbox/timezone.xml && \
	ln -sf /usr/local/share/keymaps $(prefix)/release/usr/share/keymaps
	if [ -e $(targetprefix)/usr/share/alsa ]; then \
	mkdir -p $(prefix)/release/usr/share/alsa/; \
	mkdir -p $(prefix)/release/usr/share/alsa/cards/; \
	mkdir -p $(prefix)/release/usr/share/alsa/pcm/; \
	cp $(targetprefix)/etc/tuxbox/satellites.xml $(prefix)/release/etc/tuxbox/ && \
	cp $(targetprefix)/etc/tuxbox/cables.xml $(prefix)/release/etc/tuxbox/ && \
	cp $(targetprefix)/etc/tuxbox/terrestrial.xml $(prefix)/release/etc/tuxbox/ && \
	cp $(kernelprefix)/linux-sh4/arch/sh/boot/uImage $(prefix)/release/boot/ && \
	cp $(targetprefix)/usr/share/alsa/alsa.conf          $(prefix)/release/usr/share/alsa/alsa.conf; \
	cp $(targetprefix)/usr/share/alsa/cards/aliases.conf $(prefix)/release/usr/share/alsa/cards/; \
	cp $(targetprefix)/usr/share/alsa/pcm/default.conf   $(prefix)/release/usr/share/alsa/pcm/; \
	cp $(targetprefix)/usr/share/alsa/pcm/dmix.conf      $(prefix)/release/usr/share/alsa/pcm/; fi
# AUTOFS
	if [ -d $(prefix)/release/usr/lib/autofs ]; then \
		cp -f $(buildprefix)/root/release/auto.hotplug $(prefix)/release/etc/; \
		cp -f $(buildprefix)/root/release/auto.usb $(prefix)/release/etc/; \
		cp -f $(buildprefix)/root/release/auto.network $(prefix)/release/etc/; \
		cp -f $(buildprefix)/root/release/autofs $(prefix)/release/etc/init.d/; \
	fi

# Copy lircd.conf
	touch $(prefix)/release/var/etc/.firstboot && \
	cp -f $(buildprefix)/root/bootscreen/bootlogo.mvi $(prefix)/release/boot/ && \
	cp -f $(buildprefix)/root/bin/autologin $(prefix)/release/bin/ && \
	cp -f $(buildprefix)/root/bin/vdstandby $(prefix)/release/bin/ && \
	cp -f $(buildprefix)/root/etc/vdstandby.cfg $(prefix)/release/etc/ && \
	cp -f $(buildprefix)/root/etc/network/interfaces $(prefix)/release/etc/network/ && \
	cp -f $(buildprefix)/root/sbin/flash_* $(prefix)/release/sbin/ && \
	cp -f $(buildprefix)/root/sbin/nand* $(prefix)/release/sbin/ && \
	cp -f $(buildprefix)/root/etc/image-version $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/etc/fstab $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/group $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/host.conf $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/etc/hostname $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/hosts $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/etc/inetd.conf $(prefix)/release/etc/ && \
	ln -s /usr/share/zoneinfo/Europe/Berlin $(prefix)/release/etc/localtime && \
	cp -dp $(targetprefix)/etc/mtab $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/etc/motd $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/passwd $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/etc/profile $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/protocols $(prefix)/release/etc/ && \
	cp -dp $(buildprefix)/root/etc/resolv.conf $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/services $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/shells $(prefix)/release/etc/ && \
	cp -dp $(targetprefix)/etc/shells.conf $(prefix)/release/etc/ && \
	$(INSTALL_DIR) $(prefix)/release/etc/tuxbox && \
	$(INSTALL_FILE) root/etc/tuxbox/satellites.xml $(prefix)/release/etc/tuxbox/ && \
	$(INSTALL_FILE) root/etc/tuxbox/cables.xml $(prefix)/release/etc/tuxbox/ && \
	$(INSTALL_FILE) root/etc/tuxbox/terrestrial.xml $(prefix)/release/etc/tuxbox/ && \
	$(INSTALL_FILE) root/etc/tuxbox/timezone.xml $(prefix)/release/etc/ && \
	echo "576i50" > $(prefix)/release/etc/videomode

release_hl101:
	echo "hl101" > $(prefix)/release/etc/hostname && \
	cp -f $(buildprefix)/root/release/fstab_hl101 $(prefix)/release/etc/fstab
	cp $(buildprefix)/root/firmware/dvb-fe-avl2108.fw $(prefix)/release/lib/firmware/ && \
	cp $(buildprefix)/root/firmware/dvb-fe-stv6306.fw $(prefix)/release/lib/firmware/
	cp -dp $(buildprefix)/root/etc/11-usbhd-automount.rules $(prefix)/release/etc/udev/rules.d
	cp -dp $(buildprefix)/root/release/auto_rb_st.sh $(prefix)/release/etc/init.d
	cp -dp $(buildprefix)/root/etc/init.d/rdate.sh $(prefix)/release/etc/init.d
	ln -sf /etc/init.d/rdate.sh $(prefix)/release/etc/rc.d/rcS.d/S99rdate.sh
	rm -Rf $(prefix)/release/usr/share/meta
	mv $(prefix)/release/etc/lircd.conf $(prefix)/release/etc/lircd_Opticum-9500HD.conf
	ln -sf /etc/lircd_Opticum-9500HD.conf $(prefix)/release/etc/lircd.conf
	cp -dp $(buildprefix)/root/etc/lircd_GI-S980.conf $(prefix)/release/etc

# The main target depends on the model.
# IMPORTANT: it is assumed that only one variable is set. Otherwise the target name won't be resolved.
#
$(DEPDIR)/release: $(DEPDIR)/%release:boot-elf release_base release_common_utils release_hl101
# Post tweaks
	$(DEPMOD) -b $(prefix)/release $(KERNELVERSION)
	touch $@

release-clean:
	rm -f $(DEPDIR)/release
	rm -f $(DEPDIR)/release_base
	rm -f $(DEPDIR)/release_$(HL101)
	rm -f $(DEPDIR)/release_common_utils 

######## FOR YOUR OWN CHANGES use these folder in cdk/own_build/enigma2 #############
	cp -RP $(buildprefix)/own_build/enigma2/* $(prefix)/release/
