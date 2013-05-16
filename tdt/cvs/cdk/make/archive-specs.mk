
URLDIV1=ftp://mirrors.kernel.org/fedora/core/6/i386/os/Fedora/RPMS
URL4=ftp://ftp.stlinux.com/pub/stlinux/2.4/STLinux/sh4
URL4U=ftp://ftp.stlinux.com/pub/stlinux/2.4/updates/RPMS/sh4

URL4S=ftp://ftp.stlinux.com/pub/stlinux/2.4/SRPMS
URL4SU=ftp://ftp.stlinux.com/pub/stlinux/2.4/updates/SRPMS

$(archivedir)/bash-3.1-16.1.i386.rpm:
	[ ! -f $(archivedir)/$(notdir $@) ] && \
	(cd $(archivedir) && $(WGET) $(URLDIV1)/$(notdir $@)) || true
$(archivedir)/stlinux24-sh4-%.sh4.rpm:
	[ ! -f $(archivedir)/$(notdir $@) ] && \
	(cd $(archivedir) && $(WGET) $(URL4)/$(notdir $@) || $(WGET) $(URL4U)/$(notdir $@)) || true
$(archivedir)/stlinux24-host-%.src.rpm:
	[ ! -f $(archivedir)/$(notdir $@) ] && \
	(cd $(archivedir) && $(WGET) $(URL4S)/$(notdir $@) || $(WGET) $(URL4SU)/$(notdir $@)) || true
$(archivedir)/stlinux24-cross-%.src.rpm:
	[ ! -f $(archivedir)/$(notdir $@) ] && \
	(cd $(archivedir) && $(WGET) $(URL4S)/$(notdir $@) || $(WGET) $(URL4SU)/$(notdir $@)) || true
$(archivedir)/stlinux24-target-%.src.rpm:
	[ ! -f $(archivedir)/$@ ] && \
	(cd $(archivedir) && $(WGET) $(URL4S)/$(notdir $@) || $(WGET) $(URL4SU)/$(notdir $@)) || true
