SKIN = NarrowHD

### The directory environment:

-include Make.config

DESTDIR		?=
PREFIX		?= /usr
CONFDIR		= $(if $(subst /usr,,$(PREFIX)), $(PREFIX))/etc/vdr
SKINDIR		= $(CONFDIR)/plugins/text2skin/$(SKIN)
THEMESDIR	= $(CONFDIR)/themes

### The main target:

all: NarrowHD.skin

### Targets:

NarrowHD.skin:
	@echo '<?xml version="1.0"?>' > $(SKIN).skin
	@echo '<skin version="1.1" name="NarrowHD" screenBase="relative" >' >> $(SKIN).skin
	
	@cat xml/replayInfo.xml >> $(SKIN).skin
	@cat xml/channelInfo.xml >> $(SKIN).skin
	@cat xml/replaySmall.xml >> $(SKIN).skin
	@cat xml/menu.xml >> $(SKIN).skin
	@cat xml/audioTracks.xml >> $(SKIN).skin
	@cat xml/volume.xml >> $(SKIN).skin
	@cat xml/message.xml >> $(SKIN).skin

	@echo '</skin>' >> $(SKIN).skin

	@cat $(SKIN).skin | sed 's/BUTTON_1/$(BUTTON_1)/g' | sed 's/BUTTON_2/$(BUTTON_2)/g' | sed 's/BUTTON_3/$(BUTTON_3)/g' | sed 's/BUTTON_4/$(BUTTON_4)/g' > $(SKIN).tmp
	@cp $(SKIN).tmp $(SKIN).skin
	@cat $(SKIN).skin | sed 's/TOTALDISKSPACE/$(TOTALDISKSPACE)/g' > $(SKIN).tmp
	@cp $(SKIN).tmp $(SKIN).skin

	@rm $(SKIN).tmp
	@echo NarrowHD done...
	
install:
	@mkdir -p $(DESTDIR)$(SKINDIR)
	@cp $(SKIN).skin $(DESTDIR)$(SKINDIR)
	@cp $(SKIN).colors $(DESTDIR)$(SKINDIR)
	@cp -R logos $(DESTDIR)$(SKINDIR)
	@cp themes/*.theme $(DESTDIR)$(THEMESDIR)
	@echo Install done...

clean:
	@-rm -rf $(SKIN).skin *~
	@echo Clean done...

uninstall:
	@-rm $(DESTDIR)$(SKINDIR)/$(SKIN).skin
	@-rm $(DESTDIR)$(SKINDIR)/$(SKIN).colors
	@-rm -rf $(DESTDIR)$(SKINDIR)/logos
	@-rmdir $(DESTDIR)$(SKINDIR)
