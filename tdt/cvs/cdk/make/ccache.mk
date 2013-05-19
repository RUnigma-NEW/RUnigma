#################################################
#  ccache
#
# You can use ccache for compiling if it is installed on your system or Tuxbox-CDK in ~/cdk/bin.
# With this rule you can install ccache independ from your system. 
# Use <make ccache> for installing in cdk/bin. This own ccache-binary is preferred from configure.
# Isn't ccache installed on your system, you can also install later, but you must configure again.
# Most distributions contain the required packages or
# get the sources from http://samba.org/ftp/ccache

ifdef ENABLE_CCACHE
# tuxbox-cdk ccache install path
CCACHE_TUXBOX_BIN = $(ccachedir)/ccache

# tuxbox-cdk ccache environment dir
CCACHE_BINDIR = $(hostprefix)/ccache-bin

# generate links
CCACHE_LINKS = \
	ln -sfv $(CCACHE_TUXBOX_BIN) $(CCACHE_BINDIR)/gcc;\
	ln -sfv $(CCACHE_TUXBOX_BIN) $(CCACHE_BINDIR)/g++; \
	ln -sfv $(CCACHE_TUXBOX_BIN) $(CCACHE_BINDIR)/$(target)-gcc; \
	ln -sfv $(CCACHE_TUXBOX_BIN) $(CCACHE_BINDIR)/$(target)-g++; \
	ln -sfv $(CCACHE_TUXBOX_BIN) $(CCACHE_BINDIR)/$(target)-cpp;\
	ln -sfv $(CCACHE_TUXBOX_BIN) $(hostprefix)/bin/$(target)-gcc; \
	ln -sfv $(CCACHE_TUXBOX_BIN) $(hostprefix)/bin/$(target)-g++

# ccache test will show you ccache statistics
CCACHE_TEST = $(CCACHE_TUXBOX_BIN) -s

# sets the options for ccache which are configured
CCACHE_SETUP = \
	test "$(maxcachesize)" != -1 && $(CCACHE_TUXBOX_BIN) -M $(maxcachesize); \
	test "$(maxcachefiles)" != -1 && $(CCACHE_TUXBOX_BIN) -F $(maxcachefiles); \
	true

# create ccache environment
CCACHE_ENV = $(INSTALL) -d $(CCACHE_BINDIR); \
		$(CCACHE_LINKS); \
		$(CCACHE_SETUP)

# use ccache from your host if is installed
ifdef USE_CCACHEHOST
$(DEPDIR)/ccache:
	$(CCACHE_ENV); \
	$(CCACHE_TEST)
	touch $@
else

#
# build own tuxbox-cdk ccache
#
BEGIN[[
ccache
  3.1.9
  {PN}-{PV}
  extract:http://samba.org/ftp/{PN}/{PN}-{PV}.tar.xz
  make:install:DESTDIR=HOST
;
]]END

$(hostprefix)/bin/ccache: $(DEPENDS_ccache)
	$(PREPARE_ccache)
	$(start_build)
	cd $(DIR_ccache) && \
		./configure \
			--build=$(build) \
			--host=$(build) \
			--prefix= && \
			$(MAKE) all && \
		$(CCACHE_ENV); \
		$(CCACHE_TEST); \
		$(INSTALL_ccache)
	$(tocdk_build)
	$(toflash_build)
	$(DISTCLEANUP_ccache)
	[ "x$*" = "x" ] && touch $@ || true

endif

endif

