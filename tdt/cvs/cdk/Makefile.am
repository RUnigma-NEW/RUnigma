## Makefile for Tuxbox

include smart-rules.mk

#Some staff for building and splitting ipk packages with python
include packaging.mk

# Set up the build environment
include make/buildenv.mk

# Include Archive targets (SPECS derived)
include make/archive-specs.mk

# Include host apps targets
include make/hostapps.mk

# Include bootstrap targets (SPECS derived)
include make/bootstrap.mk

# Include u-boot targets
include make/u-boot.mk

# Target for building and install own independ ccache for tuxbox-cdk
include make/ccache.mk

# Include adapted etc files and etc read-write files
include make/root-etc.mk

# Include bare-os targets (mainly SPECS derived)
include make/bare-os.mk

# Include linux kernel targets
include make/linux-kernel.mk

# Include busybox targets
include make/busybox.mk

# Include contrib lib targets
include make/contrib-libs.mk

# Include contrib apps (SPECS derived)
include make/contrib-apps-specs.mk

# Include contrib apps
include make/contrib-apps.mk

# Include net apps
include make/net-apps.mk

# Misc tools, not essential
include make/misc_tools.mk

# Include enigma2 plugins targets
include make/enigma2-plugins.mk

# Include enigma2-nightly gui targets
include make/enigma2-nightly.mk

# ExternePlugins
include make/externe-plugins.mk

# ExterneSkins
include make/externe-skins.mk

# Include xbmc-nightly gui targets
include make/xbmc-nightly.mk

# Include enigma2-pli-nightly gui targets
include make/enigma2-pli-nightly.mk

# Include neutrino gui targets
#include make/neutrino.mk
#include make/neutrino_hd2.mk

# Include yaud targets
include make/yaud.mk

# Include misc targets (SPECS derived)
# (splashutils)
include make/misc-specs.mk

# A number of cleaning targets
include make/cleantargets.mk

# Create the TAGS file
include make/tags.mk

#Trick
include make/smart_release.mk

include make/smart_release_xbmc.mk

# Give the user rope to hang himself :-).  (Note: read from the
# generated Makefile during make run, automake or configure does not
# see it.)
-include ./Makefile.local

# this makes sure we do not build top-level dependencies in parallel
# (which would not be too helpful anyway, running many configure and
# downloads in parallel...), but the sub-targets are still built in
# parallel, which is useful on multi-processor / multi-core machines
.NOTPARALLEL:
