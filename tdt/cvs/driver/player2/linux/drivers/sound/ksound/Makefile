CCFLAGSY += -Wno-uninitialized -D__TDT__

# workaround for stlinux24
ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else ifneq (,$(findstring 3.4.5,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
endif

ksound-objs := ksound-core.o

obj-$(CONFIG_STM_UNIFY_KSOUND) += ksound.o
obj-$(CONFIG_STM_UNIFY_KTONE)  += ktone.o
