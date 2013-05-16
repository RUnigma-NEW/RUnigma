#
# Makefile for a Video Disk Recorder plugin
#
# $Id$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
#

# if REELVDR is defined, some hardware and software dependencies will be
# aktivated for REELBOX lite


### The version number of this plugin (taken from the main source file):
VERSION = $(shell grep 'static const char \*VERSION *=' channelscan.h | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= ccache g++
CXXFLAGS = -fPIC -O2 -Wall -Woverloaded-virtual -fno-strict-aliasing
CXXFLAGS += -fPIC
### The directory environment:

VDRDIR = ../../..
LIBDIR = ../../lib
TMPDIR = /tmp

### Allow user defined options to overwrite defaults:

ifdef REELVDR
PLUGIN = channelscan
else
PLUGIN = reelchannelscan
endif

-include $(VDRDIR)/Make.config

### The version number of VDR (taken from VDR's "config.h"):

APIVERSION = $(shell grep 'define APIVERSION ' $(VDRDIR)/config.h | awk '{ print $$3 }' | sed -e 's/"//g')

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

#INCLUDES += -I$(VDRDIR)/include -I../../$(DVBDIR)/include
#INCLUDES += -I$(VDRDIR) -I../../$(DVBDIR)/include
INCLUDES += -I$(VDRDIR) -I../../$(DVBDIR)/include -I../../../include

#DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DVDRDIR=\"$(VDRDIR)\" -DDEBUG_CHANNELSCAN
DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DVDRDIR=\"$(VDRDIR)\" -DBOOST_IOSTREAMS_NO_LIB -DNDEBUG -D__KERNEL_STRICT_NAMES

ifdef REELVDR
  DEFINES += -DREELVDR
endif

ifeq ($(shell test -f $(VDRDIR)/PLUGINS/src/rotor/rotor.h; echo $$?),0)
  DEFINES += -DHAVE_ROTOR
endif

### causes segfaults Premiere Direkt sometimes
#DEFINES += -DWITH_EIT


### The object files (add further files here):

ifdef REELVDR
OBJS = channelscan.o csmenu.o dirfiles.o filter.o scan.o transponders.o zlib.o bzip2.o channellistbackupmenu.o
else
OBJS = channelscan.o csmenu.o filter.o scan.o transponders.o channellistbackupmenu.o
endif


### Targets:
plug: libvdr-$(PLUGIN).so

all: libvdr-$(PLUGIN).so i18n

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

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
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<tobias.bratfisch@reel-multimedia.com>' -o $@ $^

%.po:
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@


### Targets:

.PHONY: i18n
i18n: $(I18Nmsgs) $(I18Npot)

.PHONY: i18n-dist
i18n-dist: $(I18Nmsgs)

all: libvdr-$(PLUGIN).so i18n

libvdr-$(PLUGIN).so: $(OBJS)
	$(CXX) $(CXXFLAGS) -L$(STAGING_DIR)/usr/lib -lbz2 -lz -shared $(OBJS) -o $@
	@cp $@ $(LIBDIR)/$@.$(APIVERSION)

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f $(OBJS) $(DEPFILE) *.so *.tgz core* *~

