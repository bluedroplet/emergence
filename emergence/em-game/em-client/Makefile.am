SUBDIRS = shared share
LDADD = -L/usr/X11R6/lib ../gsub/libgsub.a ../common/libcommon.a shared/libshared.a -lm -lz -lpng -lX11 -lXext -lssl -lcrypto -lvorbisfile -lvorbis -logg
AM_CPPFLAGS = -DLINUX -DEMCLIENT -Wall -I/usr/X11R6/include/X11/extensions/ -DPKGDATADIR=\""$(datadir)/emergence/"\"
bin_PROGRAMS = em-client

em_client_SOURCES = bsp.c entry.c game.c main.c map.h ping.h stars.h \
	bsp.h entry.h game.h main.h network.c rcon.c tick.c console.c \
	event.c network.h rcon.h tick.h console.h  event.h \
	render.c control.c line.c input.c input.h \
	render.h control.h line.h map.c ping.c stars.c rotate.c rotate.h \
	skin.c skin.h particles.c particles.h screenshot.c screenshot.h \
	x.c x.h

