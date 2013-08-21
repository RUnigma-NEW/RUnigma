#
# Make Extern-Skins
#
#
BEGIN[[
e2skin
  git
  {PN}
  git://github.com/schpuntik/enigma2-skins-sh4.git:b=master
;
]]END

DESCRIPTION_e2skin := Skins for Enigma2

PKGR_e2skin = r2

NAME_e2skin_meta := enigma2-skins-meta
FILES_e2skin_meta := /usr/share/meta
DESCRIPTION_e2skin_meta := Enigma2 skins metadata
PACKAGES_e2skin = e2skin_meta

$(DEPDIR)/enigma2-skins-sh4.do_prepare: $(DEPENDS_e2skin)
	$(PREPARE_e2skin)

$(DIR_e2skin)/config.status: enigma2-skins-sh4.do_prepare
	cd $(DIR_e2skin) && \
		./autogen.sh && \
		$(BUILDENV) \
		./configure \
			--host=$(target) \
			--prefix=/usr \
			--datadir=/usr/share \
			--sysconfdir=/etc \
			STAGING_INCDIR=$(hostprefix)/usr/include \
			STAGING_LIBDIR=$(hostprefix)/usr/lib \
			PY_PATH=$(targetprefix)/usr \
			$(PLATFORM_CPPFLAGS)
	touch $@

$(DEPDIR)/enigma2-skins-sh4.do_compile: $(DIR_e2skin)/config.status
	cd $(DIR_e2skin) && \
		$(MAKE) all
	touch $@

enigma2_skindir = '/usr/share/enigma2'

$(DEPDIR)/enigma2-skins-sh4: enigma2-skins-sh4.do_compile
	$(call parent_pk,e2skin)
	$(start_build)
	rm -rf $(ipkgbuilddir)/*
	cd $(DIR_e2skin) && \
		$(MAKE) install DESTDIR=$(PKDIR)
	$(flash_prebuild)

	echo -e "\
	from split_packages import * \n\
	import os \n\
	#print bb_data \n\
	do_split_packages(bb_data, $(enigma2_skindir), '(.*?)/.*', 'enigma2-plugin-skin-%s', 'Enigma2 Skin: %s', recursive=True, match_path=True, prepend=True) \n\
	for package in bb_get('PACKAGES').split(): \n\
		pk = bb_get('NAME_' + package).replace('enigma2-plugin-skin-', '') \n\
		dir = '' \n\
		for x in os.listdir('$(DIR_e2skin)/skins'): \n\
			if x.lower() == pk.lower(): \n\
				dir = '$(DIR_e2skin)/skins/' + x \n\
		if not dir: \n\
			print 'not found', pk \n\
			continue \n\
		try: \n\
			read_control_file(dir + '/CONTROL/control') \n\
		except IOError: \n\
			print 'skipping', dir + '/CONTROL/control' \n\
		for s in ['preinst', 'postinst', 'prerm', 'postrm']: \n\
			try: \n\
				bb_set(s + '_' + package, open(dir + '/CONTROL/' + s).read()) \n\
			except IOError: \n\
				pass \n\
	do_finish() \n\
	" | $(hostprefix)/bin/python$(PYTHON_VERSION)

	$(call do_build_pkg,none,extra)
	touch $@

enigma2-skins-sh4-clean:
	rm -f $(DEPDIR)/enigma2-skins-sh4.do_compile

enigma2-skins-sh4-distclean: enigma2-skins-sh4-clean
	rm -f $(DEPDIR)/enigma2-skins-sh4.do_prepare
	rm -rf $(DIR_e2skin)
