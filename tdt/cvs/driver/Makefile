ifeq ($(KERNELRELEASE),)
DRIVER_TOPDIR:=$(shell pwd)
include $(DRIVER_TOPDIR)/kernel.make
else
CCFLAGSY    += -D__TDT__ -D__LINUX__ -D__SH4__ -D__KERNEL__ -DMODULE -DEXPORT_SYMTAB

CONFIGFILE := $(DRIVER_TOPDIR)/.config

include $(CONFIGFILE)

CCFLAGSY += -DHL101

ccflags-y += $(CCFLAGSY)

export CCFLAGSY

obj-y	:= avs/
obj-y	+= multicom/
obj-y	+= stgfb/
obj-y	+= player2/
obj-y	+= boxtype/
obj-y	+= simu_button/
obj-y	+= e2_proc/
obj-y	+= frontends/
obj-y	+= frontcontroller/
obj-y	+= wireless/
obj-y	+= pti/
obj-y	+= compcache/
obj-y	+= bpamem/
obj-y	+= cic/
obj-y	+= smartcard/
obj-y	+= cpu_frequ/
obj-y	+= cec/
obj-y	+= dvbloopback/

endif
