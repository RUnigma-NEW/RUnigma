#
# Adapted etc files and etc read-write files
#
GLIBC_ADAPTED_ETC_FILES =
ETC_RW_FILES += host.conf init.d/nscd ld.so.cache ld.so.conf nsswitch.conf rpc

BASE_PASSWD_ADAPTED_ETC_FILES =
ETC_RW_FILES += passwd passwd.org group group.org shadow

#BASE_FILES_ADAPTED_ETC_FILES = hosts issue.net motd profile resolv.conf
BASE_FILES_ADAPTED_ETC_FILES = timezone.xml fstab issue.net motd profile resolv.conf
ETC_RW_FILES += hosts issue.net motd profile resolv.conf \
	inputrc issue skel

BUSYBOX_ADAPTED_ETC_FILES =
ETC_RW_FILES += shells shells.conf init.d/httpd

INITSCRIPTS_ADAPTED_ETC_FILES = hostname init.d/mountvirtfs init.d/checkroot.sh init.d/mountall.sh init.d/rcS$(if $(HL101),_$(HL101)) vdstandby.cfg
ETC_RW_FILES +=  nologin hostname \
		default/halt default/tmpfs \
		init.d/bootmisc.sh \
		init.d/halt init.d/hostname.sh init.d/mountnfs.sh \
		init.d/rc init.d/reboot init.d/rmnologin init.d/sendsigs init.d/single init.d/skeleton \
		init.d/stop-bootlogd init.d/umountfs init.d/umountnfs.sh init.d/urandom

OPENRDATE_ADAPTED_ETC_FILES = init.d/rdate.sh
ETC_RW_FILES += init.d/rdate.sh \
	localtime

NETBASE_ADAPTED_ETC_FILES = network/interfaces
ETC_RW_FILES += network/interfaces \
	gateways init.d/networking network network/options protocols services

MODULE_INIT_TOOLS_ADAPTED_ETC_FILES = modules init.d/module-init-tools
ETC_RW_FILES += modules init.d/module-init-tools

SYSVINIT_ADAPTED_ETC_FILES = inittab
#ETC_RW_FILES +=

AUTOFS_ADAPTED_ETC_FILES = auto.master timezone.xml default/autofs init.d/autofs
ETC_RW_FILES += auto.master timezone.xml default/autofs init.d/autofs \
	auto.misc

TCP_WRAPPERS_ADAPTED_ETC_FILES =
ETC_RW_FILES += hosts.allow hosts.deny

PORTMAP_ADAPTED_ETC_FILES =
ETC_RW_FILES += init.d/portmap

#NFS_UTILS_ADAPTED_ETC_FILES = init.d/nfs-common init.d/nfs-kernel-server
NFS_UTILS_ADAPTED_ETC_FILES = exports init.d/nfs-common init.d/nfs-kernel-server
ETC_RW_FILES += exports init.d/nfs-common init.d/nfs-kernel-server

E2FSPROGS_ADAPTED_ETC_FILES =
ETC_RW_FILES += mke2fs.conf

SG3_UTILS_ADAPTED_ETC_FILES = default/sg_down init.d/sg_down
ETC_RW_FILES += default/sg_down init.d/sg_down

DIVERSE_TOOLS_ADAPTED_ETC_FILES = default/swap init.d/allerlei init.d/swap default/framebuffer
ETC_RW_FILES += default/swap init.d/allerlei init.d/swap default/framebuffer

FUSE_ADAPTED_ETC_FILES = init.d/fuse
#ETC_RW_FILES +=


#
# Functions for copying customized etc files from cvs/cdk/root/etc into yaud targets and
# for updating init scripts in runlevel for yaud targets
#
define adapted-etc-files
	cd root/etc && \
	for i in $(1); do \
		[ -f $$i ] && $(INSTALL) -m644 $$i $(prefix)/$*cdkroot/etc/$$i || true; \
		[ "$${i%%/*}" = "init.d" ] && chmod 755 $(prefix)/$*cdkroot/etc/$$i || true; \
	done
endef

define initdconfig
	export HHL_CROSS_TARGET_DIR=$(prefix)/release && \
	cd $(prefix)/release/etc/init.d && \
		for s in $(1) ; do \
			$(hostprefix)/bin/target-initdconfig --add $$s || echo "Unable to enable initd service: $$s" ; \
		done && \
		rm *rpmsave 2>/dev/null || true
endef
