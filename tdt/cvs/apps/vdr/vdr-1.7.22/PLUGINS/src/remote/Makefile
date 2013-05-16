#
# Makefile for a Video Disk Recorder plugin
#
# $Id$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
# IMPORTANT: the presence of this macro is important for the Make.config
# file. So it must be defined, even if it is not used here!
#
PLUGIN = remote

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).c | awk '{ print $$6 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -fPIC -O2 -Wall -Woverloaded-virtual

### The directory environment:

VDRDIR = ../../..
LIBDIR = ../../lib
TMPDIR = /tmp

### Make sure that necessary options are included:

-include $(VDRDIR)/Make.global

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)

ifeq ($(APIVERSION),)
### Backward compatibility: use version number of VDR!
APIVERSION = $(shell sed -ne '/define VDRVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
endif

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include

DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"'

### optional components
DEFINES += -DREMOTE_FEATURE_LIRC
DEFINES += -DREMOTE_FEATURE_TCPIP

### The object files (add further files here):

OBJS = $(PLUGIN).o ttystatus.o
ifneq (, $(findstring REMOTE_FEATURE_TCPIP, $(DEFINES)))
OBJS += remotetcp.o
endif

### The main target:

all: libvdr-$(PLUGIN).so i18n

### Implicit rules:

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) > $@

-include $(DEPFILE)

### Internationalization (I18N):

ifneq ($(shell grep 'LOCALEDIR' $(VDRDIR)/Makefile),)
PODIR     = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmo    = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Ndirs  = $(notdir $(foreach file, $(I18Npo), $(basename $(file))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c)
	xgettext -C -cTRANSLATORS --no-wrap -F -k -ktr -ktrNOOP --msgid-bugs-address='<o.endriss@gmx.de>' -o $@ $(wildcard *.c)

$(I18Npo): $(I18Npot)
	msgmerge -U --no-wrap -F --backup=none -q $@ $<

i18n: $(I18Nmo)
	@mkdir -p $(LOCALEDIR)
	for i in $(I18Ndirs); do\
	    mkdir -p $(LOCALEDIR)/$$i/LC_MESSAGES;\
	    cp $(PODIR)/$$i.mo $(LOCALEDIR)/$$i/LC_MESSAGES/vdr-$(PLUGIN).mo;\
	    done
else
OBJS += i18n.o
i18n: ;
endif

### Targets:

libvdr-$(PLUGIN).so: $(OBJS)
	$(CXX) $(CXXFLAGS) -shared $(OBJS) -o $@
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
	@-rm -f *.o $(DEPFILE) *.so *.tgz core* *~
