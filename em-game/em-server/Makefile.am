SUBDIRS = shared
LDADD = ../common/libcommon.a shared/libshared.a -lm -lz -lssl -lcrypto -lpthread
AM_CPPFLAGS = -DLINUX -Wall -DEMSERVER -DPKGDATADIR=\""$(datadir)/emergence/"\"
#AM_LDFLAGS = -static

bin_PROGRAMS = em-server

em_server_SOURCES = console.c game.h  ping.h \
	console.h  main.c  \
	game.c main.h  ping.c entry.c \
	entry.h


