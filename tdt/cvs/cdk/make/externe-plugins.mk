#
# Make Extern-Plugins
#
#
BEGIN[[
e2plugin
  git
  {PN}
  git://github.com/schpuntik/enigma2-plugins-sh4.git
ifdef ENABLE_PY332
  patch:file://python_m4.diff
  patch:file://e2plugin.diff
  patch:file://e2plugin-python3.diff
endif
;
]]END

DESCRIPTION_e2plugin := Additional plugins for Enigma2

PKGR_e2plugin = r2

NAME_e2plugin_meta := enigma2-plugins-meta
FILES_e2plugin_meta := /usr/share/meta
DESCRIPTION_e2plugin_meta := Enigma2 plugins metadata
PACKAGES_e2plugin = e2plugin_meta
DIST_e2plugin = enigma2_plugin_systemplugins_networkbrowser \
enigma2_plugin_extensions_alternativesoftcammanager \
enigma2_plugin_extensions_webinterface \
enigma2_plugin_systemplugins_libgisclubskin

# This is list of plugins that have files with non-typical path,
# that doesn't start with /usr/lib/enigma2/python/Plugins
enigma2_plugins_nontyp := \
$(DEPDIR)/enigma2-plugins-sh4-networkbrowser \
$(DEPDIR)/enigma2-plugins-sh4-multiquickbutton \
$(DEPDIR)/enigma2-plugins-sh4-libgisclubskin


$(DEPDIR)/enigma2-plugins-sh4.do_prepare: $(DEPENDS_e2plugin)
	$(PREPARE_e2plugin)
	touch $@

$(DIR_e2plugin)/config.status: enigma2-plugins-sh4.do_prepare
	cd $(DIR_e2plugin) && \
		./autogen.sh && \
		sed -e 's|#!/usr/bin/python|#!$(hostprefix)/bin/python$(PYTHON_VERSION)|' -i xml2po.py && \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--datadir=/usr/share \
			--libdir=/usr/lib \
			--prefix=/usr \
			--sysconfdir=/etc \
			STAGING_INCDIR=$(hostprefix)/usr/include \
			STAGING_LIBDIR=$(hostprefix)/usr/lib \
			PY_PATH=$(targetprefix)/usr \
			$(PLATFORM_CPPFLAGS) $(E_CONFIG_OPTS)

enigma2_plugindir = /usr/lib/enigma2/python/Plugins

$(DEPDIR)/enigma2-plugins-bla: $(DIR_e2plugin)/config.status
	$(eval enigma2-plugins-sh4-list = $(shell echo $(DIR_e2plugin)/*/Makefile | tr ' ' '\n' |sed 's:/Makefile::;s:.*/::'))
	make $(addprefix $(DEPDIR)/enigma2-plugins-sh4-,$(enigma2-plugins-sh4-list))

$(DEPDIR)/enigma2-plugins-sh4: $(DIR_e2plugin)/config.status $(enigma2_plugins_nontyp)
	$(call parent_pk,e2plugin)
#	Don't build meta
	$(start_build)
	cd $(DIR_e2plugin) && \
		$(MAKE) install DESTDIR=$(PKDIR)
	rm -rf $(ipkgbuilddir)/*
	$(flash_prebuild)

	echo -e "\
	from split_packages import * \n\
	#print bb_data \n\
	do_split_packages(bb_data, '$(enigma2_plugindir)', '(.*?/.*?)/.*', 'enigma2-plugin-%s', 'Enigma2 Plugin: %s', recursive=True, match_path=True, prepend=True) \n\
	for package in bb_get('PACKAGES').split(): \n\
		pk = bb_get('NAME_' + package).split('-')[-1] \n\
		try: \n\
			read_control_file('$(DIR_e2plugin)/' + pk + '/CONTROL/control') \n\
		except IOError: \n\
			print('skipping', pk) \n\
		for s in ['preinst', 'postinst', 'prerm', 'postrm', 'conffiles']: \n\
			try: \n\
				bb_set(s + '_' + package, open('$(DIR_e2plugin)/' + pk + '/CONTROL/' + s).read()) \n\
			except IOError: \n\
				pass \n\
	do_finish() \n\
	" | $(hostprefix)/bin/python$(PYTHON_VERSION)

	$(call do_build_pkg,none,extra)
	touch $@

#$(enigma2_plugins_nontyp):

$(DEPDIR)/enigma2-plugins-sh4-%: $(DIR_e2plugin)/config.status
	$(call parent_pk,e2plugin)
	$(start_build)
	echo $@
	cd $(DIR_e2plugin)/$* && \
		$(MAKE) install DESTDIR=$(PKDIR)
	rm -rf $(ipkgbuilddir)/*
	$(flash_prebuild)

	echo -e " \n\
	from split_packages import * \n\
	pk = '$*' \n\
	try: \n\
		package = read_control_file('$(DIR_e2plugin)/' + pk + '/CONTROL/control') \n\
		bb_set('PACKAGES', bb_get('PACKAGES') + ' ' + package) \n\
		bb_set('FILES_' + package, '/') \n\
	except IOError: \n\
		print ('skipping', pk) \n\
	for s in ['preinst', 'postinst', 'prerm', 'postrm', 'conffiles']: \n\
		try: \n\
			bb_set(s + '_' + package, open('$(DIR_e2plugin)/' + pk + '/CONTROL/' + s).read()) \n\
		except IOError: \n\
			pass \n\
	do_finish() \n\
	" | $(hostprefix)/bin/python$(PYTHON_VERSION)
	
	rm -r $(ipkgbuilddir)/e2plugin_meta
	$(call do_build_pkg,none,extra)
	touch $@

enigma2-plugins-sh4-clean:
	rm -f $(DEPDIR)/enigma2-plugins-sh4
	rm -f $(DEPDIR)/enigma2-plugins-sh4.do_compile

enigma2-plugins-sh4-distclean: enigma2-plugins-sh4-clean
	rm -f $(DEPDIR)/enigma2-plugins-sh4.do_prepare
	rm -rf $(DIR_e2plugin)
