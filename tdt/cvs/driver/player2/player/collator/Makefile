# workaround for stlinux24
ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
else ifneq (,$(findstring 3.4.5,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
endif

collator-objs:= collator_base.o \
		collator_pes.o \
		collator_pes_video.o \
		collator_pes_video_mpeg2.o \
		collator_pes_video_h264.o \
		collator_pes_video_vc1.o \
		collator_pes_video_divx.o \
		collator_pes_video_raw.o \
		collator_pes_audio.o \
		collator_pes_audio_aac.o \
		collator_pes_audio_mpeg.o \
		collator_pes_audio_dtshd.o \
		collator_pes_audio_lpcm.o \
		collator_pes_audio_wma.o \
		collator_pes_audio_eac3.o


obj-y += $(collator-objs)
