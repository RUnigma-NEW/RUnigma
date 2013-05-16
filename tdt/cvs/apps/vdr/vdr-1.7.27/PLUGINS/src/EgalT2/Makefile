#
# Makefile for the EgalT2 skin for text2skin
#
# Part of this Makefile stolen from PearlHD-Skinpackage :-D
#
# $Id$
#
MAIL = $(shell sed -ne "s/.*Report-Msgid-Bugs-To: *<\(..*\)>.*/\1/p" po/*.po | head -n1)

# for HD-Output (720p/1080i)
HDSKIN = 1

# marker for MenuCurrentItem (better visibility)
MCITEM = 1

# Type of Frontend-Signalinfo: 1 = textual, 2 = grafical, 0 = no (streaming+co)
# if grafical: %value for colored progressbar Green/Yellow/Red
SIGINFO = 1
SIGWARN = 66
SIGMALF = 50

# Text of the colored buttons: Red/Green/Yellow/Blue
# ..Channel/Live-Display
CBUTTON_1=Aufnahmen
CBUTTON_2=...
CBUTTON_3=Audio
CBUTTON_4=Timer
# ..Replay-Display
RBUTTON_1=Springen
RBUTTON_2=-1 Min.
RBUTTON_3=+1 Min.
RBUTTON_4=Stop



# do not edit >>>
SKIN = EgalT2

### The directory environment:
DESTDIR ?=
PREFIX  ?= /usr
CONFDIR  = $(if $(subst /usr,,$(PREFIX)), $(PREFIX))/etc/vdr
SKINDIR  = $(CONFDIR)/plugins/text2skin/$(SKIN)
THEMESDIR  = $(CONFDIR)/themes
LOCDIR   = $(PREFIX)/share/locale

### The main target:
all: EgalT2.skin i18n 

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

EgalT2.skin: 
	@cp $(SKIN).template $(SKIN).skin

#	prepare SD/HD-Version
	if [ "$(HDSKIN)" = "1" ]; then\
		sed -i '/<!--SDSKIN-->/d' $(SKIN).skin;\
		sed -i 's/<!--HDSKIN-->//g' $(SKIN).skin;\
	else\
		sed -i '/<!--HDSKIN-->/d' $(SKIN).skin;\
		sed -i 's/<!--SDSKIN-->//g' $(SKIN).skin;\
	fi

#	marker MenuCurrentItem
	if [ "$(MCITEM)" = "1" ]; then\
		sed -i '/<!--MCITEM_N-->/d' $(SKIN).skin;\
		sed -i 's/<!--MCITEM_Y-->//g' $(SKIN).skin;\
	else\
		sed -i '/<!--MCITEM_Y-->/d' $(SKIN).skin;\
		sed -i 's/<!--MCITEM_N-->//g' $(SKIN).skin;\
	fi

#	info STR/SNR
	if [ "$(SIGINFO)" = "2" ]; then\
		sed -i '/<!--SIGINFO_0-->/d' $(SKIN).skin;\
		sed -i '/<!--SIGINFO_1-->/d' $(SKIN).skin;\
		sed -i 's/<!--SIGINFO_2-->//g' $(SKIN).skin;\
		sed -i 's/SIGWARN/$(SIGWARN)/g' $(SKIN).skin;\
		sed -i 's/SIGMALF/$(SIGMALF)/g' $(SKIN).skin;\
	elif [ "$(SIGINFO)" = "1" ]; then\
		sed -i '/<!--SIGINFO_0-->/d' $(SKIN).skin;\
		sed -i '/<!--SIGINFO_2-->/d' $(SKIN).skin;\
		sed -i 's/<!--SIGINFO_1-->//g' $(SKIN).skin;\
	else\
		sed -i '/<!--SIGINFO_1-->/d' $(SKIN).skin;\
		sed -i '/<!--SIGINFO_2-->/d' $(SKIN).skin;\
		sed -i 's/<!--SIGINFO_0-->//g' $(SKIN).skin;\
	fi

# 	exchange Button-Description
	sed -i 's/CBUTTON_1/$(CBUTTON_1)/g' $(SKIN).skin
	sed -i 's/CBUTTON_2/$(CBUTTON_2)/g' $(SKIN).skin
	sed -i 's/CBUTTON_3/$(CBUTTON_3)/g' $(SKIN).skin
	sed -i 's/CBUTTON_4/$(CBUTTON_4)/g' $(SKIN).skin
	sed -i 's/RBUTTON_1/$(RBUTTON_1)/g' $(SKIN).skin
	sed -i 's/RBUTTON_2/$(RBUTTON_2)/g' $(SKIN).skin
	sed -i 's/RBUTTON_3/$(RBUTTON_3)/g' $(SKIN).skin
	sed -i 's/RBUTTON_4/$(RBUTTON_4)/g' $(SKIN).skin

#	possible font subst
#	sed -i 's/\"Sml\"/\"VDRSymbols Sans:Book@15\"/g' $(SKIN).skin
#	sed -i 's/\"Osd\"/\"VDRSymbols Sans:Book@21\"/g' $(SKIN).skin
	
install-i18n: i18n
	@mkdir -p $(DESTDIR)$(LOCDIR)
	@cp -r $(LOCALEDIR)/* $(DESTDIR)$(LOCDIR)
	
install: install-i18n 
	@mkdir -p $(DESTDIR)$(SKINDIR)
	@cp  $(SKIN).skin $(DESTDIR)$(SKINDIR)
	@cp  $(SKIN).colors $(DESTDIR)$(SKINDIR)
	@cp  COPYING $(DESTDIR)$(SKINDIR)
	@cp themes/*.theme $(DESTDIR)$(THEMESDIR)

clean:
	@-rm -rf $(LOCALEDIR) $(I18Npot) $(SKIN).skin *~

uninstall:
	@-rm  $(DESTDIR)$(SKINDIR)/COPYING
	@-rm  $(DESTDIR)$(SKINDIR)/$(SKIN).skin
	@-rm  $(DESTDIR)$(SKINDIR)/$(SKIN).colors
	@-rmdir  $(DESTDIR)$(SKINDIR) 
