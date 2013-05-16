#
# Makefile for a Video Disk Recorder plugin
#
# $Id: Makefile,v 1.14 2005/10/16 12:27:03 ralf Exp $

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
#
PLUGIN = setup

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).c | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -g -O2 -Wall -Woverloaded-virtual

### The directory environment:

DVBDIR = ../../../../DVB
VDRDIR = ../../..
LIBDIR = ../../lib
TMPDIR = /tmp

### Make sure that necessary options are included:

include $(VDRDIR)/Make.global

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

### The version number of VDR (taken from VDR's "config.h"):

APIVERSION = $(shell grep 'define APIVERSION ' $(VDRDIR)/config.h | awk '{ print $$3 }' | sed -e 's/"//g')

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -I$(DVBDIR)/include

#just for TESTING VDR SUBmenu
INCLUDES += -I$(VDRDIR)

DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DAPIVERSION="\"$(APIVERSION)\"" 

### The object files (add further files here):
OBJS     = $(PLUGIN).o  menus.o setupmenu.o setupsetup.o config.o plugins.o sysconfig.o util.o setupsystemmenu.o

### The main target:

all: libvdr-$(PLUGIN).so i18n

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE):  setupsystemmenu.c Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c)   > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmsgs  = $(addprefix $(LOCALEDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<see README>' -o $@ $^

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: i18n
i18n: $(I18Nmsgs)

### Targets:

libvdr-$(PLUGIN).so: setupsystemmenu.c $(OBJS)
	$(CXX) $(CXXFLAGS) -shared $(OBJS) -o $@
	@cp $@ $(LIBDIR)/$@.$(APIVERSION)

setupsystemmenu.c:  ../../../menu.c
	@echo "Extracting class definitions from VDR's menu.c for standard Setup"
	@./getVdrSetupMenu.pl ../../../menu.c

	
dist: clean
	@rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@rm -f $(TMPDIR)/$(ARCHIVE)/$(PLUGIN).kdevelop
	@rm -f $(TMPDIR)/$(ARCHIVE)/$(PLUGIN).kdevelop.filelist
	@rm -f $(TMPDIR)/$(ARCHIVE)/$(PLUGIN).kdevelop.pcs
	@rm -f $(TMPDIR)/$(ARCHIVE)/$(PLUGIN).kdevses
	@rm -rf $(TMPDIR)/$(ARCHIVE)/CVS
	@rm -rf $(TMPDIR)/$(ARCHIVE)/Examples/CVS
	@rm -rf $(TMPDIR)/$(ARCHIVE)/Examples/help/CVS
	@rm -rf $(TMPDIR)/$(ARCHIVE)/Patch/CVS
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@rm -rf $(TMPDIR)/$(ARCHIVE) $(TMPDIR)/$(PLUGIN)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f $(OBJS) $(MAIN) $(DEPFILE) *.so *.tgz core* setupsystemmenu.c Makefile.new *~
