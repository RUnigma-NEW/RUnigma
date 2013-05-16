lib_LTLIBRARIES = libstgles.la

AM_CFLAGS = -Wall -g0 -Os -ftracer -fomit-frame-pointer -fvisibility=hidden -ldirectfb \
	-Iincludes \
	-Iincludes/EGL \
	-Iincludes/GLES \
	-I../../../../../tufsbox/cdkroot/usr/include/ \
	-I../../../../../tufsbox/cdkroot/usr/include/directfb/


libstgles_la_SOURCES = \
	api/eglcore.c \
	api/glcore.c

libstgles_la_LIBADD = -lpthread -lm -lc -lpthread -lrt

libstgles_la_egldir = $(includedir)/EGL
libstgles_la_egl_HEADERS = \
	includes/EGL/egl.h \
	includes/EGL/eglext.h \
	includes/EGL/eglplatform.h


libstgles_la_glesdir = $(includedir)/GLES
libstgles_la_gles_HEADERS = \
	includes/GLES/gl.h \
	includes/GLES/glext.h \
	includes/GLES/glplatform.h

libstgles_la_pkgconfigdir = $(libdir)/pkgconfig
libstgles_la_pkgconfig_DATA = libstgles.pc
