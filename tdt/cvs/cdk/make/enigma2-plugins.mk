#
# Plugins
#
$(DEPDIR)/enigma2-plugins: enigma2_openwebif enigma2_networkbrowser openpli-plugins

#
# enigma2-openwebif
#

DESCRIPTION_enigma2_openwebif = "open webinteface plugin for enigma2 by openpli team"
PKGR_enigma2_openwebif = r1
RDEPENDS_enigma2_openwebif = python pythoncheetah grab

$(DEPDIR)/enigma2_openwebif.do_prepare: bootstrap $(RDEPENDS_enigma2_openwebif) $(DEPENDS_enigma2_openwebif)
	$(PREPARE_enigma2_openwebif)
	touch $@

$(DEPDIR)/enigma2_openwebif: \
$(DEPDIR)/%enigma2_openwebif: $(DEPDIR)/enigma2_openwebif.do_prepare
	$(start_build)
	cd $(DIR_enigma2_openwebif) && \
		$(BUILDENV) \
		mkdir -p $(PKDIR)/usr/lib/enigma2/python/Plugins/Extensions && \
		mkdir -p $(PKDIR)/usr/bin/ && \
		cp -a plugin $(PKDIR)/usr/lib/enigma2/python/Plugins/Extensions/OpenWebif && \
		cp -a $(buildprefix)/root/usr/bin/grab.sh $(PKDIR)/usr/bin/
	$(toflash_build)
#	@DISTCLEANUP_enigma2_openwebif@
	@[ "x$*" = "x" ] && touch $@ || true

#
# enigma2-networkbrowser
#

DESCRIPTION_enigma2_networkbrowser = "networkbrowser plugin for enigma2"

$(DEPDIR)/enigma2_networkbrowser.do_prepare: $(DEPENDS_enigma2_networkbrowser)
	$(PREPARE_enigma2_networkbrowser)
	touch $@

$(DEPDIR)/enigma2_networkbrowser: \
$(DEPDIR)/%enigma2_networkbrowser: $(DEPDIR)/enigma2_networkbrowser.do_prepare
	$(start_build)
	cd $(DIR_enigma2_networkbrowser)/src/lib && \
		$(BUILDENV) \
		sh4-linux-gcc -shared -o netscan.so \
			-I $(targetprefix)/usr/include/python$(PYTHON_VERSION) \
			-include Python.h \
			errors.h \
			list.c \
			list.h \
			main.c \
			nbtscan.c \
			nbtscan.h \
			range.c \
			range.h \
			showmount.c \
			showmount.h \
			smb.h \
			smbinfo.c \
			smbinfo.h \
			statusq.c \
			statusq.h \
			time_compat.h
	cd $(DIR_enigma2_networkbrowser) && \
		mkdir -p $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/NetworkBrowser && \
		cp -a po $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/NetworkBrowser/ && \
		cp -a meta $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/NetworkBrowser/ && \
		cp -a src/* $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/NetworkBrowser/ && \
		cp -a src/lib/netscan.so $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/NetworkBrowser/ && \
		rm -rf $(PKDIR)/usr/lib/enigma2/python/Plugins/SystemPlugins/NetworkBrowser/lib
	$(e2extra_build)
#	@DISTCLEANUP_enigma2_networkbrowser@
	[ "x$*" = "x" ] && touch $@ || true

$(DEPDIR)/%-openpli:
	$(call git_fetch_prepare,$*_openpli,git://github.com/E2OpenPlugins/e2openplugin-$*.git)
	$(eval FILES_$*_openpli += /usr/lib/enigma2/python/Plugins)
	$(eval NAME_$*_openpli = enigma2-plugin-extensions-$*)
	$(start_build)
	$(get_git_version)
	cd $(DIR_$*_openpli) && \
		$(python) setup.py install --root=$(PKDIR) --install-lib=/usr/lib/enigma2/python/Plugins
	$(remove_pyo)
	$(e2extra_build)
	touch $@

DESCRIPTION_NewsReader_openpli = RSS reader
DESCRIPTION_AddStreamUrl_openpli = Add a stream url to your channellist
DESCRIPTION_Satscan_openpli = Alternative blind scan plugin for DVB-S
DESCRIPTION_SimpleUmount_openpli = list of mounted mass storage devices and umount one of them
PKGR_openpli_plugins = r1

openpli_plugin_list = \
AddStreamUrl \
NewsReader \
Satscan \
SimpleUmount

# openpli plugins that go to flash
openpli_plugin_distlist = \
SimpleUmount

openpli_plugin_list += $(openpli_plugin_distlist)

$(foreach p,$(openpli_plugin_distlist),$(eval DIST_$p_openpli = $p_openpli))

openpli-plugins: $(addprefix $(DEPDIR)/,$(addsuffix -openpli,$(openpli_plugin_list)))
