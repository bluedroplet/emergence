AM_CPPFLAGS = $(BINRELOC_CFLAGS) -DLINUX -DEMSERVER -Wall -I../../../stage3s/current-stage3/usr/include/

noinst_LIBRARIES = libshared.a
libshared_a_SOURCES = bsp.c  network.c network.h parse.h sgame.c \
	bsp.h cvar.c objects.h rdtsc.h sgame.h cvar.h \
	parse.c rdtsc.S config.c config.h timer.c timer.h alarm.c alarm.h \
	openssl.c openssl.h

BUILT_SOURCES = $(libshared_a_SOURCES)
CLEANFILES = $(libshared_a_SOURCES)
                                                                                                  
$(libshared_a_SOURCES):
	ln -s ../../shared/$@
