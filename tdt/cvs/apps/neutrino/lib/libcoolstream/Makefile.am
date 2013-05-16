AM_CPPFLAGS = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE

INCLUDES = \
	-I$(top_srcdir)/../misc/tools/libeplayer3/include \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/system \
	-I$(top_srcdir)/src/zapit/include \
	-I$(top_srcdir)/src/zapit/include/client \
	-I$(top_srcdir)/lib/connection \
	-I$(top_srcdir)/lib/libeventserver

noinst_LIBRARIES = libcoolstream.a

libcoolstream_a_SOURCES = \
		audio_cs.cpp \
		dmx_cs.cpp \
		dvb-ci.cpp \
		playback_cs.cpp \
		pwrmngr.cpp \
		record_cs.cpp \
		video_cs.cpp \
		cool.cpp \
		ringbuffer.c

# dvb_ci
libcoolstream_a_SOURCES += \
		dvbci_session.cpp \
		dvbci_mmi.cpp \
		dvbci_camgr.cpp \
		dvbci_appmgr.cpp \
		dvbci_datetimemgr.cpp \
		dvbci_resmgr.cpp

#libcoolstream_LDADD = -leplayer3
