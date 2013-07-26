lib_LTLIBRARIES = libmme_host.la

if ENABLE_MULTICOM406
AM_CFLAGS = -Wall -D__LINUX__ -I../../../../driver/include/multicom -I../../../../driver/multicom/src/mme

endif

if ENABLE_MULTICOM324
AM_CFLAGS = -Wall -D__LINUX__ -I../../../../driver/include/multicom -I../../../../driver/multicom/mme

libmme_host_la_SOURCES = ../../../../driver/multicom/mme/mme_linux_user.c \
			 ../../../../driver/multicom/mme/ExecutionLoop.c \
			 ../../../../driver/multicom/mme/LocalTransformer.c \
			 ../../../../driver/multicom/mme/LookupTable.c \
			 ../../../../driver/multicom/mme/MMEMessageQueue.c \
			 ../../../../driver/multicom/mme/mme_tune.c \
			 ../../../../driver/multicom/embxshell/embx_linux.c
endif


