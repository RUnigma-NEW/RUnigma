# workaround for stlinux24
ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else ifneq (,$(findstring 3.4.5,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
endif

output_timer-objs := output_timer_base.o \
		     output_timer_video.o \
		     output_timer_audio.o

obj-y += $(output_timer-objs)

