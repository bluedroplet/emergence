SUBDIRS = shared
LDADD = shared/libshared.a ../common/libcommon.a -lm -lz -lssl -lcrypto -lpthread
AM_CPPFLAGS = $(BINRELOC_CFLAGS) -DLINUX -Wall -DEMSERVER
#AM_LDFLAGS = -static

bin_PROGRAMS = em-server

em_server_SOURCES = console.c game.h  ping.h \
	console.h  main.c  \
	game.c main.h  ping.c entry.c \
	entry.h


