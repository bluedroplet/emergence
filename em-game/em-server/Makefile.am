SUBDIRS = shared
LDADD = ../common/libcommon.a shared/libshared.a -lm -lz -lssl -lcrypto
AM_CPPFLAGS = -DLINUX -Wall -DEMSERVER -DPKGDATADIR=\""$(datadir)/emergence/"\"
AM_LDFLAGS = -rdynamic -static

bin_PROGRAMS = em-server

em_server_SOURCES = console.c game.h  network.c  ping.h \
	console.h  main.c  network.h \
	console.o  game.c main.h  ping.c entry.c \
	entry.h objects.c objects.h


