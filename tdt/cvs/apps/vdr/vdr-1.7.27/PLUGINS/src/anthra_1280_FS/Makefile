#
# Makefile for the anthra_1280_FS skin for text2skin
#
# Part of this Makefile stolen from EaglT2-Skinpackage :-D
#
# $Id$
#
MAIL = $(shell sed -ne "s/.*Report-Msgid-Bugs-To: *<\(..*\)>.*/\1/p" po/*.po | head -n1)

# do not edit >>>
SKIN = anthra_1280_FS

### The directory environment:
DESTDIR ?=
PREFIX  ?= /usr
CONFDIR  = $(if $(subst /usr,,$(PREFIX)), $(PREFIX))/etc/vdr
SKINDIR  = $(CONFDIR)/plugins/text2skin/$(SKIN)
THEMESDIR  = $(CONFDIR)/themes
LOCDIR   = $(PREFIX)/share/locale

### The main target:
#all: anthra_1280_FS.skin i18n 
all: anthra_1280_FS.skin

### Internationalization (I18N):
PODIR     = po
LOCALEDIR = locale

I18Npo    = $(notdir $(wildcard $(PODIR)/*.po))
I18Npot   = $(PODIR)/$(SKIN).pot

$(I18Npot): $(SKIN).skin
	@cat $^ | sed -e "s/('/(\"/g;s/')/\")/g" | grep -o "trans([^)]*)" | \
	xgettext -C --no-wrap --no-location -k -ktrans \
	         --msgid-bugs-address='<$(MAIL)>' -o $@ -

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(LOCALEDIR)/%/LC_MESSAGES/vdr-text2skin-$(SKIN).mo: $(PODIR)/%.po
	@mkdir -p $(dir $@)
	msgfmt -c -o $@ $<

i18n: $(I18Npo:%.po=$(LOCALEDIR)/%/LC_MESSAGES/vdr-text2skin-$(SKIN).mo)

### Targets:

anthra_1280_FS.skin: 
#	@cp $(SKIN).template $(SKIN).skin

install-i18n: i18n
	@mkdir -p $(DESTDIR)$(LOCDIR)
	@cp -r $(LOCALEDIR)/* $(DESTDIR)$(LOCDIR)
	
#install: install-i18n 
install:
	@mkdir -p $(DESTDIR)$(SKINDIR)
	@cp  $(SKIN).skin $(DESTDIR)$(SKINDIR)
	@cp  $(SKIN).colors $(DESTDIR)$(SKINDIR)
	@cp  COPYING $(DESTDIR)$(SKINDIR)
#	@cp themes/*.theme $(DESTDIR)$(THEMESDIR)
	@mkdir -p $(DESTDIR)$(SKINDIR)/symbols
	@cp -rd symbols/* $(DESTDIR)$(SKINDIR)/symbols/

clean:
	@-rm -rf $(LOCALEDIR) $(I18Npot) $(SKIN).skin *~

uninstall:
	@-rm  $(DESTDIR)$(SKINDIR)/COPYING
	@-rm  $(DESTDIR)$(SKINDIR)/$(SKIN).skin
	@-rm  $(DESTDIR)$(SKINDIR)/$(SKIN).colors
	@-rmdir  $(DESTDIR)$(SKINDIR)
	@-rm  $(DESTDIR)$(SKINDIR)/themes/*
	@-rmdir  $(DESTDIR)$(SKINDIR)/themes 
