TREE_ROOT?=$(DRIVER_TOPDIR)/
CONFIG_STGFB_PATH?=$(DRIVER_TOPDIR)/stgfb/stmfb
CCFLAGSY += -I$(TREE_ROOT)/linux/include/linux/stm/
CCFLAGSY += -I$(TREE_ROOT)/linux/drivers/media/video/stm/
CCFLAGSY += -I$(KERNEL_LOCATION)/drivers/media/dvb/dvb-core
CCFLAGSY += -I$(CONFIG_STGFB_PATH)/include
CCFLAGSY += -DDEBUG_DVB
CCFLAGSY += -I$(TREE_ROOT)/include

ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else
CFLAGS += $(CCFLAGSY)
endif

fe-core-objs +=        hl101/stb0899_algo.o
fe-core-objs +=        hl101/stb0899_drv.o
fe-core-objs +=        hl101/stb6100.o
fe-core-objs +=        hl101/stv090x.o
fe-core-objs +=        hl101/stv6110x.o
fe-core-objs +=        hl101/ix7306.o
fe-core-objs +=        hl101/core.o
obj-m += fe-core.o
