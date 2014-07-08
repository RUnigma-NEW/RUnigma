
CCFLAGSY+=-I./linux/components/inline         \
	-I./components/include              \
	-I./components/infrastructure

CCFLAGSY+=-D__SH4__       

# workaround for stlinux24
ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else ifneq (,$(findstring 3.4.5,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
endif

obj-y += infrastructure/


