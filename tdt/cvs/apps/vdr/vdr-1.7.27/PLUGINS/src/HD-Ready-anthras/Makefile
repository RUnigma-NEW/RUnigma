#
#    Simple Makefile for anthra-skins for text2skin
#
#    Tomas Saxer <tsaxer@gmx.de>   
#


##### to comment an option just place a '#' at the beginning of the line
##### to uncomment an option please remove the leading '#'
##### !!! Take care to uncomment only ONE option in each section !!!

##### CHANNELLOGOS ######
#NOLOGOS = 1
LOGOSRIGHT = 1
#LOGOSLEFT = 1

###### MENULOGOS  #####
MENULOGOS = 1

###### EPGIMAGES ######
#EPGIMAGES = 1

###### path to the epgimages, escape slashes with backslashes! ######
###### e.g. EPGIMAGESPATH=\/etc\/vdr\/plugins\/tvm2vdr\/epgimages\/ ######
EPGIMAGESPATH=

###### EPGIMAGESFORMAT #####
####### default EPGIMAGESFORMAT is png, uncomment the following line, if you have jpgs ######
#EPGIMAGESJPG = 1

######TIMER-INFO_MAIN-MENU######
TIMER-INFO_MAIN-MENU = 1

###### MAINMENUFONTS ######
#SMALLMENUFONT = 1
DEFAULTMENUFONT = 1
#LARGEMENUFONT = 1
###

### The directory environment:

DESTDIR ?= 
PREFIX  ?= /usr
CONFDIR  = $(if $(subst /usr,,$(PREFIX)), $(PREFIX))/etc/vdr
SKINDIR  = $(CONFDIR)/plugins/text2skin/$(SKIN)
THEMESDIR  = $(CONFDIR)/themes
LOCDIR   = $(PREFIX)/share/locale



###### Don't edit below this line, if you don't know what you are doing ;) ######


SKIN = anthra_1280_OS

all: $(SKIN).skin i18n


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
	@msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(LOCALEDIR)/%/LC_MESSAGES/vdr-text2skin-$(SKIN).mo: $(PODIR)/%.po
	@mkdir -p $(dir $@)
	@msgfmt -c -o $@ $<

i18n: $(I18Npo:%.po=$(LOCALEDIR)/%/LC_MESSAGES/vdr-text2skin-$(SKIN).mo)

### Targets:

anthra_1280_OS.skin:

	@cp $(SKIN).template $(SKIN).skin
	
	@echo
	@echo "The skinfile is generated with the following settings:"; \
	echo "-------------------------------------------------------"; \
	
	@if [ "$(LOGOSLEFT)" = "1" ]; then\
	  sed -e '/LOGOSRIGHT/d;/NOLOGOS/d;' -i $(SKIN).skin;\
	  echo "LOGOSLEFT"; \
	fi
	
	@if [ "$(LOGOSRIGHT)" = "1" ]; then\
	  sed -e '/LOGOSLEFT/d;/NOLOGOS/d;' -i $(SKIN).skin;\
	  echo "LOGOSRIGHT"; \
	fi
	
	@if [ "$(NOLOGOS)" = "1" ]; then\
	  sed -e '/LOGOSLEFT/d;/LOGOSRIGHT/d;' -i $(SKIN).skin;\
	  echo "NOLOGOS"; \
	fi
	
	@if [ "$(MENULOGOS)" = "" ]; then\
	  sed -e '/MENULOGOS/d' -i $(SKIN).skin;\
	  echo "NOMENULOGOS"; \
	else \
	  echo "MENULOGOS"; \
	fi
	
	@if [ "$(EPGIMAGES)" = "" ];then\
	  sed -e '/EPGIMAGES/d' -i $(SKIN).skin ; \
	  echo "NOEPGIMAGES"; \
	fi
	 
	@if [ "$(EPGIMAGES)" = "1" -a -n "$(EPGIMAGESPATH)" ]; then\
	  sed 's/EPGIMAGESPATH/$(EPGIMAGESPATH)/g' -i $(SKIN).skin;\
	  echo "EPGIMAGES";\
	  echo "EPGIMAGESPATH: "$(EPGIMAGESPATH)"";\
	  if [ "$(EPGIMAGESJPG)" = "" ]; then\
	    echo "EPGIMAGESFORMAT: png";\
	  fi;\
	fi
	
	@if [ "$(EPGIMAGES)" = "1" -a -z "$(EPGIMAGESPATH)" ];then\
	  echo "\033[93;41m  !!! ERROR !!! EPGIMAGES are activated, you have to set EPGIMAGESPATH   \033[0m";\
	fi
	
	@if [ "$(EPGIMAGES)" = "1" -a "$(EPGIMAGESJPG)" = "1" ]; then\
	  sed 's/{PresentEventID}.png/{PresentEventID}.jpg/g' -i $(SKIN).skin;\
	  echo "EPGIMAGESFORMAT: jpg"; \
	fi
	
	@if [ "$(TIMER-INFO_MAIN-MENU)" = "1" ];then\
	  echo "TIMER-INFO_MAIN-MENU"; \
	else\
	  sed -e '/TIMER-INFO_MAIN-MENU/d' -i $(SKIN).skin ;\
	  echo "NO_TIMER-INFO_MAIN-MENU";\
	fi
	
	@if [ "$(SMALLMENUFONT)" = "1" ]; then\
	  sed -e '/DEFAULTMENUFONT/d;/LARGEMENUFONT/d;' -i $(SKIN).skin;\
	  echo "SMALLMENUFONT: 20"; \
	fi
	
	@if [ "$(DEFAULTMENUFONT)" = "1" ]; then\
	  sed -e '/SMALLMENUFONT/d;/LARGEMENUFONT/d;' -i $(SKIN).skin;\
	  echo "DEFAULTMENUFONT: 22"; \
	fi
	
	@if [ "$(LARGEMENUFONT)" = "1" ]; then\
	  sed -e '/SMALLMENUFONT/d;/DEFAULTMENUFONT/d;' -i $(SKIN).skin;\
	   echo "LARGEMENUFONT: 24"; \
	fi

install-i18n: i18n
	@mkdir -p $(DESTDIR)$(LOCDIR)
	@cp -r $(LOCALEDIR)/* $(DESTDIR)$(LOCDIR)
	
install: install-i18n 
	@mkdir -p $(DESTDIR)$(SKINDIR)
	@cp  $(SKIN).skin $(DESTDIR)$(SKINDIR)
	@cp  $(SKIN).colors $(DESTDIR)$(SKINDIR)
	@cp -r images $(DESTDIR)$(SKINDIR)
	@cp  COPYING $(DESTDIR)$(SKINDIR)
	@mkdir -p $(DESTDIR)$(THEMESDIR)
	@cp themes/*.theme $(DESTDIR)$(THEMESDIR)
	@cp -r anthra-menu-logos $(DESTDIR)$(CONFDIR)/plugins/text2skin/

clean:
	@-rm -rf $(LOCALEDIR) $(I18Npot) $(SKIN).skin *~

uninstall:
	@-rm  $(DESTDIR)$(SKINDIR)/COPYING
	@-rm  $(DESTDIR)$(SKINDIR)/$(SKIN).skin
	@-rm  $(DESTDIR)$(SKINDIR)/$(SKIN).colors
	@-rmdir  $(DESTDIR)$(SKINDIR)
	@-rm  $(DESTDIR)$(CONFDIR)/plugins/text2skin/anthra-menu-logos/*
	@-rmdir  $(DESTDIR)$(CONFDIR)/plugins/text2skin/anthra-menu-logos
