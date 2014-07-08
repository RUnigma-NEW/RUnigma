ifeq ($(KERNELRELEASE),)
DRIVER_TOPDIR:=$(shell pwd)/..
include $(DRIVER_TOPDIR)/kernel.make
else
ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else ifneq (,$(findstring 3.4.5,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else
CFLAGS += $(CCFLAGSY)
endif

obj-m           += simu_button.o
endif

