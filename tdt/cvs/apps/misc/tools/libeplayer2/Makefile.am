lib_LTLIBRARIES = libeplayer2.la

CFLAGS = -Wall
CXXFLAGS = -Wall

INCLUDES = \
	-Iinclude

libeplayer2_la_SOURCES =  \
	container/mp_msg.c container/avi.c container/aviheader.c container/aviprint.c container/container.c \
	container/demux_audio.c container/demuxer.c container/demux_ts.c \
	container/mkv.c container/mp3_hdr.c container/mp4.c container/demux_mpg.c\
	container/mpeg_hdr.c container/parse_es.c container/stream.c container/utils.c \
	container/container_asf.c container/demux_asf.c container/asfheader.c \
	manager/audio.c manager/manager.c manager/subtitle.c manager/video.c \
	output/enigma2.c output/linuxdvb.c output/output.c container/text_srt.c container/text_ssa.c\
	playback/playback.c

AM_CFLAGS = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
