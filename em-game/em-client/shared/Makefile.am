AM_CPPFLAGS = -DLINUX -DEMCLIENT -Wall

noinst_LIBRARIES = libshared.a
libshared_a_SOURCES = bsp.c config.h parse.h sgame.c timer.h \
	bsp.h cvar.c objects.h rdtsc.h sgame.h config.c cvar.h \
	parse.c rdtsc.S timer.c network.c network.h

BUILT_SOURCES = $(libshared_a_SOURCES)
CLEANFILES = $(libshared_a_SOURCES)
                                                                                                  
$(libshared_a_SOURCES):
	ln -s ../../shared/$@
