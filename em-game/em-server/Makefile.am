SUBDIRS = shared

AM_CPPFLAGS = $(BINRELOC_CFLAGS) $(NONAUTHENTICATING_CFLAGS) -DLINUX -Wall -DEMSERVER -I../../../stage3s/current-stage3/usr/include/
LDADD = shared/libshared.a ../common/libcommon.a #../../../stage3s/current-stage3/usr/lib/libssl.a ../../../stage3s/current-stage3/usr/lib/libcrypto.a
AM_LDFLAGS = -L../../../stage3s/current-stage3/lib -L../../../stage3s/current-stage3/usr/lib -lpthread -lz -lm -lssl -lcrypto

bin_PROGRAMS = em-server

em_server_SOURCES = console.c game.h  ping.h \
	console.h  main.c  \
	game.c main.h  ping.c entry.c \
	entry.h key.c key.h download.c download.h \
	servers.c servers.h
	
