SUBDIRS = shared share
LDADD = ../gsub/libgsub.a shared/libshared.a ../common/libcommon.a ../../../stage3s/current-stage3/usr/lib/libssl.a ../../../stage3s/current-stage3/usr/lib/libcrypto.a
AM_CPPFLAGS = -DLINUX -DEMCLIENT -Wall -I/usr/X11R6/include/X11/extensions/ -DPKGDATADIR=\""$(datadir)/emergence/"\"
AM_LDFLAGS = -L/usr/X11R6/lib -lvorbisfile -lvorbis -logg -lpthread -lz -lX11 -lXext -lXrandr -lasound -logg -lvorbisfile -lpng
bin_PROGRAMS = em-client

em_client_SOURCES = bsp.c entry.c game.c main.c map.h ping.h stars.h \
	bsp.h entry.h game.h main.h rcon.c tick.c console.c \
	event.c rcon.h tick.h console.h  event.h \
	render.c control.c line.c input.c input.h \
	render.h control.h line.h map.c ping.c stars.c rotate.c rotate.h \
	skin.c skin.h particles.c particles.h screenshot.c screenshot.h \
	x.c x.h ris.c ris.h sound.c sound.h floats.c floats.h
